#!/usr/bin/env python3
"""Verify a durable automation workspace across an isolated TrenchBroom restart.

The driver deliberately never uses tbctl's automatic discovery. It launches a private
copy of a macOS app bundle in portable mode, reads only that copy's discovery file, and
will send SIGTERM only to the PID recorded there after verifying the process command
still belongs to the private copy.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass
from typing import Any


class VerificationError(RuntimeError):
    """A failed safety check or workspace expectation."""


@dataclass(frozen=True)
class Instance:
    app_bundle: Path
    discovery_file: Path
    pid: int
    socket: str


def require(condition: bool, message: str) -> None:
    if not condition:
        raise VerificationError(message)


def response_result(response: dict[str, Any], operation: str) -> Any:
    require(response.get("jsonrpc") == "2.0", f"{operation}: invalid JSON-RPC version")
    require("error" not in response, f"{operation}: RPC error {response.get('error')}")
    require("result" in response, f"{operation}: response has no result")
    return response["result"]


def source_document_id(documents: Any, source_map: Path) -> str:
    require(isinstance(documents, list), "documents.list: result is not an array")
    for document in documents:
        if not isinstance(document, dict):
            continue
        candidate = document.get("path")
        if isinstance(candidate, str) and Path(candidate).resolve() == source_map.resolve():
            document_id = document.get("id")
            require(isinstance(document_id, str) and document_id, "source document has no ID")
            return document_id
    raise VerificationError(f"documents.list did not contain the isolated source map {source_map}")


def wait_for_source_document(
    tbctl: Path, discovery_file: Path, source_map: Path, timeout_seconds: float = 20.0
) -> str:
    """Wait until command-line document opening catches up with service discovery."""
    deadline = time.monotonic() + timeout_seconds
    last_error: VerificationError | None = None
    while time.monotonic() < deadline:
        try:
            return source_document_id(
                run_tbctl(tbctl, discovery_file, "documents.list", {}), source_map
            )
        except VerificationError as error:
            last_error = error
            time.sleep(0.1)
    detail = f": {last_error}" if last_error is not None else ""
    raise VerificationError(f"timed out waiting for the isolated source document{detail}")


def workspace_by_id(workspaces: Any, workspace_id: str) -> dict[str, Any]:
    require(isinstance(workspaces, list), "workspace.list: result is not an array")
    for workspace in workspaces:
        if isinstance(workspace, dict) and workspace.get("workspaceId") == workspace_id:
            return workspace
    raise VerificationError(f"workspace.list did not contain workspace {workspace_id}")


def run_tbctl(
    tbctl: Path, discovery_file: Path, method: str, params: dict[str, Any]
) -> Any:
    command = [
        str(tbctl),
        "--discovery",
        str(discovery_file),
        "--method",
        method,
        "--params",
        json.dumps(params, separators=(",", ":")),
        "--timeout",
        "15000",
    ]
    completed = subprocess.run(command, text=True, capture_output=True, check=False)
    if completed.returncode != 0:
        raise VerificationError(
            f"{method}: tbctl exited {completed.returncode}: {completed.stderr.strip()}"
        )
    try:
        response = json.loads(completed.stdout)
    except json.JSONDecodeError as error:
        raise VerificationError(f"{method}: tbctl did not return JSON: {error}") from error
    require(isinstance(response, dict), f"{method}: tbctl response is not an object")
    return response_result(response, method)


def process_command(pid: int) -> str:
    completed = subprocess.run(
        ["ps", "-p", str(pid), "-o", "command="], text=True, capture_output=True, check=False
    )
    return completed.stdout.strip() if completed.returncode == 0 else ""


def assert_owned_instance(instance: Instance) -> None:
    command = process_command(instance.pid)
    require(command, f"private TrenchBroom PID {instance.pid} is no longer running")
    require(
        str(instance.app_bundle) in command,
        f"refusing to signal PID {instance.pid}: it is not the private app copy ({command})",
    )


def wait_for_discovery(app_bundle: Path, timeout_seconds: float) -> Instance:
    discovery_directory = app_bundle / "Contents" / "MacOS" / "config" / "automation"
    deadline = time.monotonic() + timeout_seconds
    while time.monotonic() < deadline:
        for discovery_file in sorted(discovery_directory.glob("*.json")):
            try:
                discovery = json.loads(discovery_file.read_text(encoding="utf-8"))
            except (OSError, json.JSONDecodeError):
                continue
            pid = discovery.get("pid")
            socket = discovery.get("socket")
            if isinstance(pid, int) and pid > 0 and isinstance(socket, str) and socket:
                instance = Instance(app_bundle, discovery_file, pid, socket)
                try:
                    assert_owned_instance(instance)
                except VerificationError:
                    continue
                return instance
        time.sleep(0.1)
    raise VerificationError(
        f"timed out waiting for private discovery under {discovery_directory}; "
        "the active user's discovery files were not consulted"
    )


def launch(app_bundle: Path, source_map: Path) -> Instance:
    subprocess.run(
        ["open", "-n", "-g", "-j", str(app_bundle), "--args", "--portable", str(source_map)],
        check=True,
    )
    return wait_for_discovery(app_bundle, timeout_seconds=20.0)


def terminate(instance: Instance) -> None:
    assert_owned_instance(instance)
    os.kill(instance.pid, signal.SIGTERM)
    deadline = time.monotonic() + 10.0
    while time.monotonic() < deadline:
        if not process_command(instance.pid):
            return
        time.sleep(0.1)
    raise VerificationError(
        f"private PID {instance.pid} did not exit after SIGTERM; leaving it and its artifacts intact"
    )


def copy_private_app(source_app: Path, root: Path) -> Path:
    private_app = root / "TrenchBroom.app"
    shutil.copytree(source_app, private_app, symlinks=True)
    return private_app


def verify_run(app: Path, tbctl: Path, source_map: Path, keep_artifacts: bool) -> None:
    require(sys.platform == "darwin", "PW5 external restart verification requires macOS open -gj")
    require(app.is_dir() and app.suffix == ".app", "--app must name a TrenchBroom.app bundle")
    require(tbctl.is_file() and os.access(tbctl, os.X_OK), "--tbctl must be executable")
    require(source_map.is_file(), "--source-map must name an existing saved map")

    root = Path(tempfile.mkdtemp(prefix="trenchbroom-pw5-"))
    first: Instance | None = None
    second: Instance | None = None
    completed = False
    try:
        private_app = copy_private_app(app, root)
        private_source = root / source_map.name
        shutil.copy2(source_map, private_source)

        first = launch(private_app, private_source)
        first_source_id = wait_for_source_document(
            tbctl, first.discovery_file, private_source
        )
        forked = run_tbctl(
            tbctl,
            first.discovery_file,
            "workspace.fork",
            {"documentId": first_source_id, "name": "PW5 restart verification"},
        )
        require(isinstance(forked, dict), "workspace.fork: result is not an object")
        workspace_id = forked.get("workspaceId")
        require(isinstance(workspace_id, str) and workspace_id, "workspace.fork returned no ID")
        checkpointed = run_tbctl(
            tbctl, first.discovery_file, "workspace.checkpoint", {"workspaceId": workspace_id}
        )
        require(
            isinstance(checkpointed, dict)
            and int(checkpointed.get("checkpointGeneration", -1)) >= 1,
            "workspace.checkpoint did not publish a durable generation",
        )
        run_tbctl(tbctl, first.discovery_file, "workspace.close", {"workspaceId": workspace_id})
        terminate(first)
        first = None

        second = launch(private_app, private_source)
        second_source_id = wait_for_source_document(
            tbctl, second.discovery_file, private_source
        )
        discovered = workspace_by_id(
            run_tbctl(tbctl, second.discovery_file, "workspace.list", {}), workspace_id
        )
        require(discovered.get("runtimeStatus") == "dormant", "restarted workspace is not dormant")
        recovered = run_tbctl(
            tbctl,
            second.discovery_file,
            "workspace.recover",
            {"workspaceId": workspace_id},
        )
        require(
            isinstance(recovered, dict) and recovered.get("branchDocumentId"),
            "workspace.recover did not return a branch document ID",
        )
        attached = run_tbctl(
            tbctl,
            second.discovery_file,
            "workspace.attachSource",
            {"workspaceId": workspace_id, "documentId": second_source_id},
        )
        require(
            isinstance(attached, dict) and attached.get("sourceDocumentId") == second_source_id,
            "workspace.attachSource did not retain the explicit source document",
        )
        diff = run_tbctl(
            tbctl, second.discovery_file, "workspace.diff", {"workspaceId": workspace_id}
        )
        require(isinstance(diff, dict), "workspace.diff: result is not an object")
        require(diff.get("conflicts") == [], "restart verification found merge conflicts")
        require(diff.get("operationCount") == 0, "unchanged restart branch has merge operations")
        run_tbctl(tbctl, second.discovery_file, "workspace.close", {"workspaceId": workspace_id})
        terminate(second)
        second = None
        completed = True
        print(f"PW5 restart verification passed for workspace {workspace_id}")
    finally:
        cleanup_safe = True
        for instance in (second, first):
            if instance is None:
                continue
            try:
                terminate(instance)
            except VerificationError as error:
                cleanup_safe = False
                print(f"PW5 cleanup warning: {error}", file=sys.stderr)
        if completed and cleanup_safe and not keep_artifacts:
            shutil.rmtree(root)
        else:
            print(f"PW5 artifacts retained at {root}", file=sys.stderr)


def dry_run(app: Path | None, tbctl: Path | None, source_map: Path | None) -> None:
    print("PW5 dry run: no application is launched and no discovery file is read.")
    print("1. Copy the supplied TrenchBroom.app to a unique temporary directory.")
    print("2. Launch only that copy with: open -n -g -j <copy> --args --portable <source.map>")
    print("3. Address tbctl only through a verified <copy>/Contents/MacOS/config/automation/*.json record.")
    print("4. fork -> checkpoint -> close -> SIGTERM verified private PID -> restart.")
    print("5. list -> recover(workspaceId) -> attachSource -> diff -> cleanup.")
    if app is not None:
        print(f"app: {app}")
    if tbctl is not None:
        print(f"tbctl: {tbctl}")
    if source_map is not None:
        print(f"source map: {source_map}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--app", type=Path, help="TrenchBroom.app to copy and launch")
    parser.add_argument("--tbctl", type=Path, help="tbctl executable")
    parser.add_argument("--source-map", type=Path, help="saved map to copy into the private run")
    parser.add_argument("--run", action="store_true", help="perform the external restart verification")
    parser.add_argument("--dry-run", action="store_true", help="print the isolated workflow only")
    parser.add_argument("--keep-artifacts", action="store_true", help="retain private bundle/config")
    arguments = parser.parse_args()

    if not arguments.run:
        dry_run(arguments.app, arguments.tbctl, arguments.source_map)
        return 0
    if arguments.app is None or arguments.tbctl is None or arguments.source_map is None:
        parser.error("--run requires --app, --tbctl, and --source-map")
    try:
        verify_run(
            arguments.app.resolve(),
            arguments.tbctl.resolve(),
            arguments.source_map.resolve(),
            arguments.keep_artifacts,
        )
    except (OSError, subprocess.SubprocessError, VerificationError) as error:
        print(f"PW5 restart verification failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
