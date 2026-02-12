# Python Utilities for TrenchBroom CMake Builds

This directory contains helper scripts that streamline working with the TrenchBroom CMake build through Python.

## Available scripts

### `list_cmake_targets.py`

Queries the CMake File API and prints the names of all executable and library targets in the configured build tree.

Typical usage:

```
python3 cmake/list_cmake_targets.py -B build --config-name Debug
```

Key options:

- `-B/--build-dir`: Path to the CMake build directory (default: `build`).
- `--config-name`: Multi-config generators can specify `Debug`, `Release`, etc.
- `--refresh`: Re-run CMake configuration to refresh File API replies before listing.

### `build_target.py`

Provides a fuzzy-search interface (powered by `InquirerPy`) to select a target and kick off a `cmake --build` command. It can also run non-interactively when a target name is supplied.

Examples:

```
# Interactive fuzzy prompt
python3 cmake/build_target.py -B build --config-name Debug

# Dry-run to preview the command
python3 cmake/build_target.py -B build --config-name Debug --dry-run

# Direct build without prompt
python3 cmake/build_target.py -B build --config-name Debug --target TrenchBroom
```

## Managing dependencies with uv

The Python utilities use [`uv`](https://github.com/astral-sh/uv) for dependency management. The project metadata lives in the repository root `pyproject.toml`, which declares the required packages (currently `InquirerPy>=0.3.4`).

Common workflows:

1. **Install uv** (once per machine):

   ```
   curl -LsSf https://astral.sh/uv/install.sh | sh
   ```

2. **Create a virtual environment** (once per project):

   ```
   uv venv create
   ```

3. **Install dependencies** (from repo root):

   ```
   uv pip install -r pyproject.toml
   ```

   This installs `InquirerPy` into uv’s managed environment.

4. **Run scripts via uv** to ensure they see the managed interpreter and packages:

   ```
   uv run python cmake/list_cmake_targets.py -B build
   uv run python cmake/build_target.py -B build
   ```

   `uv run` automatically resolves the virtual environment according to `pyproject.toml`.

5. **Add new dependencies** by editing `pyproject.toml` and re-running `uv pip install -r pyproject.toml`.

## Tips

- The scripts expect that you have already configured the CMake build directory (e.g., via `cmake -S . -B build`).
- If you switch generators or configurations, re-run `cmake` so the File API replies stay up-to-date.
- Use `--dry-run` in `build_target.py` to confirm the command before launching a long build.
