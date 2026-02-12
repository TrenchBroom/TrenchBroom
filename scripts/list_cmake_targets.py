#!/usr/bin/env python3

"""List CMake targets by querying the CMake File API."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path
from typing import Any, List, Sequence

CLIENT_NAME = "list-cmake-targets"
FILE_API_RELATIVE = Path(".cmake") / "api" / "v1"
ALLOWED_TYPES = {"EXECUTABLE", "STATIC_LIBRARY", "SHARED_LIBRARY"}


class CMakeFileAPIError(RuntimeError):
    """Raised when the CMake File API is not in the expected state."""


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "List the CMake targets available in a build directory using the "
            "CMake File API."
        )
    )
    parser.add_argument(
        "-B",
        "--build-dir",
        default="build",
        help="Path to the build directory (default: %(default)s).",
    )
    parser.add_argument(
        "-S",
        "--source-dir",
        default=None,
        help=(
            "Path to the source directory used when configuring with --refresh. "
            "Defaults to this script's parent directory."
        ),
    )
    parser.add_argument(
        "--refresh",
        action="store_true",
        help="Re-run CMake to refresh the File API replies before listing targets.",
    )
    parser.add_argument(
        "--cmake",
        default="cmake",
        help="Path to the CMake executable used with --refresh (default: %(default)s).",
    )
    parser.add_argument(
        "-G",
        "--generator",
        default=None,
        help="CMake generator to use when --refresh is supplied.",
    )
    parser.add_argument(
        "--config-name",
        default=None,
        help=(
            "Name of the configuration to inspect. Defaults to the first configuration "
            "reported by CMake."
        ),
    )
    parser.add_argument(
        "cmake_args",
        nargs=argparse.REMAINDER,
        help=(
            "Additional arguments forwarded to CMake when --refresh is used. "
            "Prefix them with '--' to separate them from this script's options."
        ),
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    build_dir = Path(args.build_dir).resolve()

    if not build_dir.exists():
        if args.refresh:
            build_dir.mkdir(parents=True, exist_ok=True)
        else:
            raise SystemExit(
                f"Build directory '{build_dir}' does not exist. "
                "Create it or supply --refresh to configure it."
            )

    ensure_query(build_dir)

    if args.refresh:
        source_dir = (
            Path(args.source_dir).resolve()
            if args.source_dir
            else Path(__file__).resolve().parent
        )
        cmake_args = normalize_cmake_args(args.cmake_args)
        run_cmake(args.cmake, source_dir, build_dir, args.generator, cmake_args)

    try:
        targets, config_name = read_targets_from_file_api(build_dir, args.config_name)
    except CMakeFileAPIError as exc:
        raise SystemExit(f"Error: {exc}") from exc

    filtered_targets = filter_targets_by_type(targets, ALLOWED_TYPES)
    if not filtered_targets:
        allowed_display = ", ".join(sorted(ALLOWED_TYPES))
        print(
            f"No targets of types {allowed_display} were found in configuration '{config_name}'."
        )
        return

    print_target_names(filtered_targets)


def normalize_cmake_args(args: Sequence[str] | None) -> List[str]:
    if not args:
        return []
    normalized = list(args)
    if normalized and normalized[0] == "--":
        normalized = normalized[1:]
    return normalized


def ensure_query(build_dir: Path) -> Path:
    query_dir = build_dir / FILE_API_RELATIVE / f"query/client-{CLIENT_NAME}"
    query_dir.mkdir(parents=True, exist_ok=True)
    query_file = query_dir / "query.json"
    desired_query = {
        "requests": [
            {
                "kind": "codemodel",
                "version": {"major": 2},
            }
        ]
    }

    should_write = True
    if query_file.exists():
        try:
            current = json.loads(query_file.read_text())
            should_write = current != desired_query
        except json.JSONDecodeError:
            should_write = True

    if should_write:
        query_file.write_text(json.dumps(desired_query, indent=2) + "\n")

    return query_file


def run_cmake(
    executable: str,
    source_dir: Path,
    build_dir: Path,
    generator: str | None,
    cmake_args: Sequence[str],
) -> None:
    cmd = [executable, "-S", str(source_dir), "-B", str(build_dir)]
    if generator:
        cmd.extend(["-G", generator])
    cmd.extend(cmake_args)

    print("Running:", " ".join(cmd), file=sys.stderr)
    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as exc:
        raise CMakeFileAPIError(
            f"CMake exited with status {exc.returncode} while refreshing the File API."
        ) from exc


def read_targets_from_file_api(
    build_dir: Path, requested_config: str | None
) -> tuple[List[dict[str, Any]], str]:
    reply_dir = build_dir / FILE_API_RELATIVE / "reply"
    if not reply_dir.is_dir():
        raise CMakeFileAPIError(
            f"Reply directory '{reply_dir}' does not exist. "
            "Run CMake at least once or pass --refresh."
        )

    index_data = load_index_data(reply_dir)
    codemodel_file = locate_codemodel_reply(index_data)
    codemodel = load_json(reply_dir / codemodel_file)
    configuration = select_configuration(codemodel, requested_config)
    targets = load_targets(reply_dir, configuration)
    config_name = configuration.get("name") or "<unnamed>"
    return targets, config_name


def load_index_data(reply_dir: Path) -> dict[str, Any]:
    index_files = sorted(
        reply_dir.glob("index-*.json"),
        key=lambda path: path.stat().st_mtime,
        reverse=True,
    )
    if not index_files:
        raise CMakeFileAPIError(
            f"No index files found in '{reply_dir}'. "
            "Ensure CMake has produced File API replies."
        )

    last_error: CMakeFileAPIError | None = None
    for candidate in index_files:
        try:
            return load_json(candidate)
        except CMakeFileAPIError as exc:
            last_error = exc
            continue

    if last_error is not None:
        raise last_error
    raise CMakeFileAPIError(
        f"Failed to read any valid index files from '{reply_dir}'."
    )


def locate_codemodel_reply(index_data: dict[str, Any]) -> str:
    candidates: List[dict[str, Any]] = []

    reply_section = index_data.get("reply")
    if isinstance(reply_section, list):
        candidates.extend(entry for entry in reply_section if isinstance(entry, dict))
    elif isinstance(reply_section, dict):
        for client_data in reply_section.values():
            if not isinstance(client_data, dict):
                continue

            responses = client_data.get("responses")
            if isinstance(responses, list):
                candidates.extend(entry for entry in responses if isinstance(entry, dict))

            for value in client_data.values():
                if isinstance(value, dict):
                    nested_responses = value.get("responses")
                    if isinstance(nested_responses, list):
                        candidates.extend(
                            entry for entry in nested_responses if isinstance(entry, dict)
                        )

    objects_section = index_data.get("objects")
    if isinstance(objects_section, list):
        candidates.extend(entry for entry in objects_section if isinstance(entry, dict))

    for entry in candidates:
        if (
            entry.get("kind") == "codemodel"
            and entry.get("version", {}).get("major") == 2
            and entry.get("jsonFile")
        ):
            return entry["jsonFile"]

    raise CMakeFileAPIError(
        "The File API index does not reference a codemodel reply. "
        "Re-run CMake to generate one."
    )


def select_configuration(
    codemodel: dict[str, Any], requested_name: str | None
) -> dict[str, Any]:
    configurations = codemodel.get("configurations") or []
    if not configurations:
        raise CMakeFileAPIError("Codemodel reply does not contain any configurations.")

    if requested_name is None:
        return configurations[0]

    for config in configurations:
        if config.get("name") == requested_name:
            return config

    available = ", ".join(
        config.get("name") or "<unnamed>" for config in configurations
    )
    raise CMakeFileAPIError(
        f"Configuration '{requested_name}' not found. Available configurations: {available}"
    )


def load_targets(
    reply_dir: Path, configuration: dict[str, Any]
) -> List[dict[str, Any]]:
    targets: List[dict[str, Any]] = []
    target_refs = configuration.get("targets")
    if target_refs is None:
        return targets
    if not isinstance(target_refs, list):
        raise CMakeFileAPIError("Codemodel configuration 'targets' entry is not a list.")

    for target_ref in target_refs:
        if not isinstance(target_ref, dict):
            continue

        json_name = target_ref.get("jsonFile")
        if not json_name:
            continue

        target_path = reply_dir / json_name
        if not target_path.is_file():
            raise CMakeFileAPIError(
                f"Target reply '{target_path}' referenced in the codemodel is missing."
            )

        target_data = load_json(target_path)
        artifacts_field = target_data.get("artifacts") or []
        if not isinstance(artifacts_field, list):
            raise CMakeFileAPIError(
                f"Target reply '{target_path}' has a non-list 'artifacts' field."
            )
        artifacts = [
            artifact["path"]
            for artifact in artifacts_field
            if isinstance(artifact, dict) and artifact.get("path")
        ]

        targets.append(
            {
                "name": target_data.get("name")
                or target_ref.get("name")
                or "<unnamed>",
                "type": target_data.get("type") or "unknown",
                "artifacts": artifacts,
            }
        )

    return targets


def load_json(path: Path) -> dict[str, Any]:
    try:
        data = json.loads(path.read_text())
    except json.JSONDecodeError as exc:
        raise CMakeFileAPIError(
            f"File '{path}' contains invalid JSON: line {exc.lineno} column {exc.colno}."
        ) from exc
    except OSError as exc:
        raise CMakeFileAPIError(
            f"Unable to read '{path}': {exc.strerror or exc}"
        ) from exc

    if not isinstance(data, dict):
        raise CMakeFileAPIError(
            f"File '{path}' does not contain a JSON object."
        )

    return data


def filter_targets_by_type(
    targets: List[dict[str, Any]], allowed_types: set[str]
) -> List[dict[str, Any]]:
    return [target for target in targets if target.get("type") in allowed_types]


def print_target_names(targets: List[dict[str, Any]]) -> None:
    for target in sorted(targets, key=lambda item: item["name"]):
        print(target["name"])


if __name__ == "__main__":
    main()
