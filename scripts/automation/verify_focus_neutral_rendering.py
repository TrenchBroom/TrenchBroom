#!/usr/bin/env python3
"""Externally verify focus-neutral real and virtual rendering on macOS.

This opt-in driver uses only a copied portable app bundle and its own discovery file.
It defaults to a no-launch dry run.
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
    """A failed safety check or focus-neutrality expectation."""


@dataclass(frozen=True)
class Instance:
    app_bundle: Path
    discovery_file: Path
    pid: int


def require(condition: bool, message: str) -> None:
    if not condition:
        raise VerificationError(message)


def response_result(response: dict[str, Any], operation: str) -> Any:
    require(response.get("jsonrpc") == "2.0", f"{operation}: invalid JSON-RPC version")
    require("error" not in response, f"{operation}: RPC error {response.get('error')}")
    require("result" in response, f"{operation}: response has no result")
    return response["result"]


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


def run_tbctl(instance: Instance, tbctl: Path, method: str, params: dict[str, Any]) -> Any:
    assert_owned_instance(instance)
    completed = subprocess.run(
        [
            str(tbctl),
            "--discovery",
            str(instance.discovery_file),
            "--method",
            method,
            "--params",
            json.dumps(params, separators=(",", ":")),
            "--timeout",
            "15000",
        ],
        text=True,
        capture_output=True,
        check=False,
    )
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


def wait_for_discovery(app_bundle: Path, timeout_seconds: float) -> Instance:
    directory = app_bundle / "Contents" / "MacOS" / "config" / "automation"
    deadline = time.monotonic() + timeout_seconds
    while time.monotonic() < deadline:
        for discovery_file in sorted(directory.glob("*.json")):
            try:
                discovery = json.loads(discovery_file.read_text(encoding="utf-8"))
            except (OSError, json.JSONDecodeError):
                continue
            pid = discovery.get("pid")
            if not isinstance(pid, int) or pid <= 0:
                continue
            instance = Instance(app_bundle, discovery_file, pid)
            try:
                assert_owned_instance(instance)
            except VerificationError:
                continue
            return instance
        time.sleep(0.1)
    raise VerificationError(
        f"timed out waiting for private discovery under {directory}; active user discovery was ignored"
    )


def launch(app_bundle: Path, source_map: Path) -> Instance:
    subprocess.run(
        ["open", "-n", "-g", "-j", str(app_bundle), "--args", "--portable", str(source_map)],
        check=True,
    )
    return wait_for_discovery(app_bundle, 20.0)


def terminate(instance: Instance) -> None:
    assert_owned_instance(instance)
    os.kill(instance.pid, signal.SIGTERM)
    deadline = time.monotonic() + 10.0
    while time.monotonic() < deadline:
        if not process_command(instance.pid):
            return
        time.sleep(0.1)
    raise VerificationError(f"private PID {instance.pid} did not exit after SIGTERM")


def document_id(documents: Any, path: Path) -> str:
    require(isinstance(documents, list), "documents.list: result is not an array")
    for document in documents:
        if not isinstance(document, dict):
            continue
        candidate = document.get("path")
        if isinstance(candidate, str) and Path(candidate).resolve() == path.resolve():
            value = document.get("id")
            require(isinstance(value, str) and value, "document has no explicit ID")
            return value
    raise VerificationError(f"documents.list did not contain {path}")


def wait_for_document(
    instance: Instance, tbctl: Path, path: Path, timeout_seconds: float = 20.0
) -> str:
    """Wait until command-line document opening catches up with service discovery."""
    deadline = time.monotonic() + timeout_seconds
    last_error: VerificationError | None = None
    while time.monotonic() < deadline:
        try:
            return document_id(run_tbctl(instance, tbctl, "documents.list", {}), path)
        except VerificationError as error:
            last_error = error
            time.sleep(0.1)
    detail = f": {last_error}" if last_error is not None else ""
    raise VerificationError(f"timed out waiting for the isolated target document{detail}")


def active_document_id(documents: Any) -> str:
    require(isinstance(documents, list), "documents.list: result is not an array")
    active = [item for item in documents if isinstance(item, dict) and item.get("active")]
    require(len(active) == 1, "documents.list did not report exactly one active private document")
    value = active[0].get("id")
    require(isinstance(value, str) and value, "active document has no explicit ID")
    return value


def three_d_view_id(views: Any, document: str) -> str:
    require(isinstance(views, list), "views.list: result is not an array")
    for view in views:
        if isinstance(view, dict) and view.get("documentId") == document and view.get("type") == "3d":
            value = view.get("viewId")
            require(isinstance(value, str) and value, "3D view has no explicit viewId")
            return value
    raise VerificationError(f"views.list did not contain a 3D view for {document}")


def target_state(context: Any) -> dict[str, Any]:
    require(isinstance(context, dict), "context.capture: result is not an object")
    return {
        "camera": context.get("camera"),
        "selectedNodes": context.get("selectedNodes"),
        "selectedFaces": context.get("selectedFaces"),
    }


def render_request(document: str) -> dict[str, Any]:
    return {
        "documentId": document,
        "camera": {
            "projection": "perspective",
            "position": [0.0, -128.0, 64.0],
            "direction": [0.0, 1.0, 0.0],
            "up": [0.0, 0.0, 1.0],
            "verticalFov": 90.0,
            "near": 1.0,
            "far": 65536.0,
        },
        "size": [160, 120],
        "renderMode": "textured",
        "overlays": {"brushEdges": False, "selection": False, "grid": False},
        "outputs": {"depth": False},
    }


def require_render_result(result: Any, document: str, operation: str) -> dict[str, Any]:
    require(isinstance(result, dict), f"{operation}: result is not an object")
    require(result.get("documentId") == document, f"{operation}: echoed the wrong documentId")
    require(isinstance(result.get("revision"), int), f"{operation}: has no stable revision")
    return result


def require_image_path(result: dict[str, Any], operation: str) -> None:
    path = result.get("imagePath")
    require(isinstance(path, str) and Path(path).is_file(), f"{operation}: no published image")


def require_target_context(
    instance: Instance, tbctl: Path, document: str, view: str, baseline: dict[str, Any]
) -> None:
    context = require_render_result(
        run_tbctl(
            instance,
            tbctl,
            "context.capture",
            {"documentId": document, "viewId": view, "screenshot": False},
        ),
        document,
        "context.capture",
    )
    require(context.get("viewId") == view, "context.capture echoed the wrong viewId")
    require(target_state(context) == baseline, "target camera or selection changed")


def require_foreground_document(instance: Instance, tbctl: Path, document: str) -> None:
    require(
        active_document_id(run_tbctl(instance, tbctl, "documents.list", {})) == document,
        "a target automation operation activated or changed the private foreground document",
    )


def verify_run(app: Path, tbctl: Path, source_map: Path, keep_artifacts: bool) -> None:
    require(sys.platform == "darwin", "EV7 external verification requires macOS open -gj")
    require(app.is_dir() and app.suffix == ".app", "--app must name a TrenchBroom.app bundle")
    require(tbctl.is_file() and os.access(tbctl, os.X_OK), "--tbctl must be executable")
    require(source_map.is_file(), "--source-map must name an existing saved map")

    root = Path(tempfile.mkdtemp(prefix="trenchbroom-ev7-"))
    instance: Instance | None = None
    completed = False
    try:
        private_app = root / "TrenchBroom.app"
        shutil.copytree(app, private_app, symlinks=True)
        target_path = root / "target.map"
        foreground_path = root / "foreground.map"
        shutil.copy2(source_map, target_path)
        shutil.copy2(source_map, foreground_path)
        instance = launch(private_app, target_path)

        target_document = wait_for_document(instance, tbctl, target_path)
        target_view = three_d_view_id(
            run_tbctl(instance, tbctl, "views.list", {"documentId": target_document}),
            target_document,
        )
        baseline = target_state(
            run_tbctl(
                instance,
                tbctl,
                "context.capture",
                {"documentId": target_document, "viewId": target_view, "screenshot": False},
            )
        )

        foreground = run_tbctl(
            instance,
            tbctl,
            "reference.open",
            {"documentId": target_document, "path": str(foreground_path)},
        )
        require(isinstance(foreground, dict), "reference.open: result is not an object")
        foreground_document = foreground.get("documentId")
        require(
            isinstance(foreground_document, str) and foreground_document != target_document,
            "reference.open did not create a distinct foreground document",
        )
        require_foreground_document(instance, tbctl, foreground_document)
        require_target_context(instance, tbctl, target_document, target_view, baseline)

        request = render_request(target_document)
        virtual_capture = require_render_result(
            run_tbctl(instance, tbctl, "render.capture", request),
            target_document,
            "render.capture",
        )
        require_image_path(virtual_capture, "render.capture")
        require_target_context(instance, tbctl, target_document, target_view, baseline)
        require_foreground_document(instance, tbctl, foreground_document)

        virtual_pick = require_render_result(
            run_tbctl(instance, tbctl, "render.pick", request | {"x": 80.0, "y": 60.0}),
            target_document,
            "render.pick",
        )
        require(isinstance(virtual_pick.get("ray"), dict), "render.pick: missing model ray")
        require_target_context(instance, tbctl, target_document, target_view, baseline)
        require_foreground_document(instance, tbctl, foreground_document)

        real_capture = require_render_result(
            run_tbctl(
                instance,
                tbctl,
                "context.capture",
                {"documentId": target_document, "viewId": target_view, "screenshot": True},
            ),
            target_document,
            "context.capture",
        )
        require(real_capture.get("viewId") == target_view, "context.capture echoed wrong viewId")
        screenshot = real_capture.get("screenshotPath")
        require(
            isinstance(screenshot, str) and Path(screenshot).is_file(),
            "context.capture did not publish a real-view image",
        )
        require_target_context(instance, tbctl, target_document, target_view, baseline)
        require_foreground_document(instance, tbctl, foreground_document)

        created = run_tbctl(
            instance,
            tbctl,
            "cameras.create",
            {"documentId": target_document, "request": request},
        )
        require(isinstance(created, dict), "cameras.create: result is not an object")
        camera_id = created.get("cameraId")
        require(isinstance(camera_id, str) and camera_id, "cameras.create returned no cameraId")
        require(created.get("documentId") == target_document, "cameras.create echoed wrong documentId")
        camera_capture = run_tbctl(instance, tbctl, "cameras.capture", {"cameraId": camera_id})
        require(isinstance(camera_capture, dict), "cameras.capture: result is not an object")
        require(camera_capture.get("cameraId") == camera_id, "cameras.capture echoed wrong cameraId")
        require(camera_capture.get("documentId") == target_document, "cameras.capture echoed wrong documentId")
        output = camera_capture.get("output")
        require(isinstance(output, dict), "cameras.capture: missing output")
        require_image_path(output, "cameras.capture")
        run_tbctl(instance, tbctl, "cameras.delete", {"cameraId": camera_id})
        require_target_context(instance, tbctl, target_document, target_view, baseline)
        require_foreground_document(instance, tbctl, foreground_document)

        terminate(instance)
        instance = None
        completed = True
        print(f"EV7 focus-neutral verification passed for {target_document}/{target_view}")
    finally:
        cleanup_safe = True
        if instance is not None:
            try:
                terminate(instance)
            except VerificationError as error:
                cleanup_safe = False
                print(f"EV7 cleanup warning: {error}", file=sys.stderr)
        if completed and cleanup_safe and not keep_artifacts:
            shutil.rmtree(root)
        else:
            print(f"EV7 artifacts retained at {root}", file=sys.stderr)


def dry_run(app: Path | None, tbctl: Path | None, source_map: Path | None) -> None:
    print("EV7 dry run: no application is launched and no discovery file is read.")
    print("1. Copy the app bundle and two maps into a unique private temporary directory.")
    print("2. Launch only that copy with: open -n -g -j <copy> --args --portable <target.map>")
    print("3. Open the second map, then verify explicit target real/virtual render IDs.")
    print("4. Compare target camera/selection before and after every capture or pick.")
    print("5. SIGTERM only a PID verified to belong to the private app copy.")
    for label, value in (("app", app), ("tbctl", tbctl), ("source map", source_map)):
        if value is not None:
            print(f"{label}: {value}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--app", type=Path, help="TrenchBroom.app to copy and launch")
    parser.add_argument("--tbctl", type=Path, help="tbctl executable")
    parser.add_argument("--source-map", type=Path, help="saved map to copy twice")
    parser.add_argument("--run", action="store_true", help="perform the external verification")
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
        print(f"EV7 focus-neutral verification failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
