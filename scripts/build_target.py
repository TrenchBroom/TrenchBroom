#!/usr/bin/env python3
"""
Interactive helper that lists CMake targets via list_cmake_targets.py,
lets you pick one with an InquirerPy fuzzy prompt, and then builds that target.
"""

from __future__ import annotations

import argparse
import shlex
import subprocess
import sys
from pathlib import Path
from typing import List, Sequence

try:
    from InquirerPy.prompts import FuzzyPrompt
except ModuleNotFoundError as exc:
    raise SystemExit(
        "Fuzzy prompt support requires InquirerPy. Install it via 'pip install InquirerPy' and rerun."
    ) from exc


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Interactively select a CMake target with an InquirerPy fuzzy prompt and build it."
        )
    )
    parser.add_argument(
        "-B",
        "--build-dir",
        default="build",
        help="Path to the CMake build directory (default: %(default)s).",
    )
    parser.add_argument(
        "--config-name",
        default=None,
        help="Configuration to inspect/build (e.g. Debug). Omit for single-config generators.",
    )
    parser.add_argument(
        "--list-script",
        default=None,
        help=(
            "Path to list_cmake_targets.py. Defaults to the script next to this helper."
        ),
    )
    parser.add_argument(
        "--cmake",
        default="cmake",
        help="Path to the CMake executable used for building (default: %(default)s).",
    )
    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=None,
        help="Number of parallel build jobs to pass to CMake.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show the build command without executing it.",
    )
    parser.add_argument(
        "--target",
        default=None,
        help="Skip the interactive prompt and build this target directly (use 'all' to rebuild everything).",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    build_dir = Path(args.build_dir).resolve()
    list_script = (
        Path(args.list_script).resolve()
        if args.list_script
        else Path(__file__).with_name("list_cmake_targets.py")
    )

    targets = fetch_targets(list_script, build_dir, args.config_name)
    if not targets:
        raise SystemExit("No eligible targets were found in the build tree.")
    if "all" not in targets:
        targets = ["all", *targets]

    if args.target:
        target = validate_target_choice(args.target, targets)
    else:
        target = prompt_for_target(targets)

    build_cmd = compose_build_command(
        args.cmake, build_dir, target, args.config_name, args.jobs
    )

    print("Building target with command:")
    print("  ", " ".join(shlex.quote(part) for part in build_cmd))

    if args.dry_run:
        return

    try:
        subprocess.run(build_cmd, check=True)
    except subprocess.CalledProcessError as exc:
        raise SystemExit(
            f"CMake exited with status {exc.returncode} while building '{target}'."
        ) from exc


def fetch_targets(
    list_script: Path, build_dir: Path, config_name: str | None
) -> List[str]:
    if not list_script.is_file():
        raise SystemExit(f"Target listing script not found at: {list_script}")

    cmd = [sys.executable, str(list_script), "-B", str(build_dir)]
    if config_name:
        cmd.extend(["--config-name", config_name])

    try:
        result = subprocess.run(
            cmd, check=True, capture_output=True, text=True, cwd=list_script.parent
        )
    except subprocess.CalledProcessError as exc:
        stderr = exc.stderr or ""
        raise SystemExit(
            f"Failed to list targets (exit code {exc.returncode}).\n{stderr}"
        ) from exc

    return [
        line
        for line in (line.strip() for line in result.stdout.splitlines())
        if line and not line.startswith("No targets")
    ]


def prompt_for_target(targets: Sequence[str]) -> str:
    unique_targets = list(dict.fromkeys(targets))
    prompt = FuzzyPrompt(
        message="Select target:",
        choices=unique_targets,
        match_exact=True,
        height="80%",
    )

    print("Type to fuzzy-search targets. Press Enter to select, Ctrl+C to abort.")
    try:
        selection = prompt.execute()
    except KeyboardInterrupt:
        raise SystemExit("\nAborted by user.")
    except EOFError:
        raise SystemExit("\nNo target selected.")

    if not selection:
        raise SystemExit("No target selected.")

    return selection


def validate_target_choice(choice: str, targets: Sequence[str]) -> str:
    resolved = resolve_choice(choice.strip(), targets)
    if resolved:
        return resolved
    raise SystemExit(
        f"Target '{choice}' was not found. Available targets include: {', '.join(targets[:10])}..."
    )


def resolve_choice(choice: str, targets: Sequence[str]) -> str | None:
    if choice in targets:
        return choice
    matches = [t for t in targets if t.startswith(choice)]
    if len(matches) == 1:
        return matches[0]
    return None


def compose_build_command(
    cmake_exe: str,
    build_dir: Path,
    target: str,
    config_name: str | None,
    jobs: int | None,
) -> List[str]:
    cmd = [cmake_exe, "--build", str(build_dir), "--target", target]
    if config_name:
        cmd.extend(["--config", config_name])
    if jobs:
        cmd.extend(["-j", str(jobs)])
    return cmd





if __name__ == "__main__":
    main()
