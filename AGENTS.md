# TrenchBroom Coding Agent Guidance

## Project structure
- The project contains several applications under /app. The main application target is TrenchBroom.
- Most code lives in libraries under /lib.
- Each application target and library target has its own CMakeLists.txt file.
- Shared CMake utilities are in /cmake.
- Each library usually has a <Name>LibTest target for its tests, for example TbMdlLibTest.
- Some libraries also have a <Name>TestUtilsLib target for shared test helpers, and some of those have a matching <Name>TestUtilsLibTest target.

## Local RPC automation

TrenchBroom has a local JSON-RPC service and a small command-line client, `tbctl`.
Use this interface for live editor inspection, map changes, offscreen rendering, and
branch work. Do not automate the GUI when the RPC can express the operation.

The implementation and design notes live in:

- `lib/TbUiLib/src/AutomationService*.cpp`: RPC routing and operation schemas.
- `lib/TbUiLib/src/AcceptanceAutomationService.cpp`: acceptance-project RPCs.
- `docs/automation-roadmap/`: identities, workspaces, offscreen rendering, and visual QA.
- `scripts/automation/`: external restart and focus-neutral verification drivers.

### Start without stealing focus

On macOS, launch a development build in the background:

```sh
open -n -g -j <build-dir>/app/TrenchBroom/TrenchBroom.app --args /absolute/path/to/map.map
```

Keep one automation instance when practical. Do not repeatedly open ordinary visible
windows merely to inspect another map. Development builds suppress the updater opt-in
dialog and automatic update checks; Release builds intentionally retain them.

Never terminate TrenchBroom broadly with `pkill`, `killall`, or a name match. Resolve
the exact process from its discovery record, verify that it belongs to the intended
build, and terminate only that PID. Preserve any user-owned TrenchBroom instances.

### Select the exact instance

Every running service writes a per-process discovery JSON file under the platform's
TrenchBroom application-data `automation` directory. There may be stale files or more
than one valid instance. When that is possible, pass the selected record explicitly:

```sh
TBCTL=<build-dir>/app/TbCtl/tbctl
TB_DISCOVERY=/absolute/path/to/automation/<pid>-<uuid>.json

"$TBCTL" \
  --discovery "$TB_DISCOVERY" \
  --timeout 30000 \
  --pretty \
  --method documents.list \
  --params '{}'
```

Do not rely on `tbctl` automatic discovery in multi-instance work. Validate that the
discovery record's PID is alive and that `documents.list` contains the expected
absolute map path.

Discovery means that the RPC server is ready; it does not guarantee that a map passed
on the command line has finished opening. Poll `documents.list` until the exact path
appears before issuing document operations. Use a bounded timeout and report the last
RPC error if readiness never arrives. The scripts under `scripts/automation/` contain
the canonical polling pattern.

### Identity model

Never use "active document," "front window," or current GUI selection as an implicit
RPC target. The user may navigate or select things while automation is running.

| Identity | Lifetime | Rule |
| --- | --- | --- |
| Discovery file / socket | One process | Select it explicitly and re-discover after restart. |
| `documentId` | One open document in one process | Resolve it from the exact saved path; send it on every document operation. |
| `viewId` | One view in one process | Resolve it with `views.list`; send it whenever an operation concerns a real view. |
| `workspaceId` | Durable across restarts | Store this for branch recovery; re-resolve its current branch `documentId`. |
| Acceptance view/comparison/suite ID | Durable in one acceptance project | Address it together with the explicit `projectPath`. |
| Node path | A particular document hierarchy state | Re-query after hierarchy-changing mutations; do not treat it as a durable object ID. |

Responses include document revisions where relevant. Carry the expected identity and
revision through multi-step workflows, and stop or re-query if either differs from the
state on which a proposed edit was based.

### Method map and schema discipline

The high-value RPC families are:

- Health and identity: `system.ping`, `documents.list`, `views.list`.
- Document data: `document.exportSelection`, `document.paste`, `document.reload`,
  `document.save`.
- Scene data: `nodes.query`, `nodes.describe`, `nodes.select`, `nodes.delete`,
  `nodes.entity.create`, `nodes.group.create`.
- Modeling: `geometry.createBrushes`, `geometry.csg`, `geometry.eqWater`,
  `geometry.extractFootprints`, `geometry.bridgeEdgeChains`,
  `geometry.volumeToPlane`, the planar/profile/sweep preview-and-apply methods, and
  `brushes.optimize.*`.
- Real and virtual views: `context.capture`, `view.camera.set`, `view.frame`,
  `view.pick`, `render.context`, `render.capture`, `render.pick`, and
  `cameras.create/get/update/delete/capture`.
- Durable branches: `workspace.fork`, `workspace.list`, `workspace.recover`,
  `workspace.status`, `workspace.diff`, `workspace.checkpoint`, `workspace.merge`,
  `workspace.close`, `workspace.abandon`.
- Visual QA: `acceptance.views.*`, `acceptance.comparisons.*`,
  `acceptance.suites.*`, `acceptance.capture`, `acceptance.run`, and
  `acceptance.assertions.evaluate`.

This is an orientation map, not a substitute for the actual schema. Before using an
unfamiliar method, inspect its handler and the corresponding `tst_AutomationService.cpp`
or focused component test. Do not infer parameter nesting or apply payloads from method
names. Start with a read-only or preview call, preserve the complete error response,
and only then construct the mutation.

### Read and inspect without changing the UI

Useful read-only methods include:

- `documents.list` and `views.list` for explicit identities.
- `nodes.query` for bounded, filtered scene queries.
- `nodes.describe` for detailed brush-face data.
- `workspace.status` and `workspace.diff` for branch state.
- `context.capture` for the state of a specific real view.
- `render.capture` and `render.pick` for a virtual camera that does not change the
  user's camera, selection, active document, or focus.

`nodes.query` filters are top-level RPC parameters, not nested under a `query` object.
For example:

```json
{
  "documentId": "document-...",
  "types": ["brush"],
  "materials": ["unrest/t50_agua1"],
  "projection": "paths",
  "limit": 5000
}
```

Use `aggregate: true` for counts by type, classname, and material when individual node
data is unnecessary. Paginate large full queries rather than silently accepting a
truncated result.

Treat selection-dependent methods such as `document.exportSelection` as unsafe on a
user-controlled source view unless the selection was established explicitly and its
revision was rechecked. Prefer explicit node paths, or perform the selection-dependent
step inside the hidden branch where user navigation cannot race it.

Prefer `render.capture` with an explicit virtual camera for visual inspection.
`context.capture` is for cases where the user's exact real-view camera is material to
the task. Do not open a reference map in a visible window just to render it; acceptance
captures can load an explicit saved reference path into an automation-owned hidden
document.

In virtual render requests, `overlays.selection: false` suppresses selection
highlighting; it does not hide selected geometry. Modeling RPCs commonly select the
brushes they create, so a capture taken immediately after a mutation must still show
those brushes. Treat a revision increase with an unchanged image as a defect to
investigate, not as a reason to save/reload the branch. Saving, reloading, closing, or
recovering a workspace solely to refresh a render masks state bugs and can invalidate
process-lifetime document identities.

For EverQuest player-view QA, `render.capture` accepts an optional deterministic
`scenePreview` object with `vision` (`human`, `infravision`, or `ultravision`),
`timeOfDay` in `[0, 24)`, and `entityLights`. Prefer a matrix of captures from one camera
and one revision when checking night readability or fixture placement. See
`docs/eq-scene-preview.md`; do not substitute GUI brightness changes for this mode.

### Make changes in a durable workspace

For nontrivial or visual map edits, use this workflow:

1. Resolve the exact source `documentId` from `documents.list`.
2. Call `workspace.fork` and retain the returned durable `workspaceId`.
3. Perform queries, previews, and mutations only against the returned branch
   `documentId`. A branch is a separate hidden map document and separate map file.
4. Use `workspace.checkpoint` before risky stages or materially different variants.
5. Inspect with virtual renders and acceptance comparisons; do not foreground the
   branch merely to see it.
6. Call `workspace.status` and `workspace.diff` before integration.
7. Call `workspace.merge` only when the diff and validation are understood. Merging is
   the deliberate operation that changes the real source session.
8. Use `workspace.abandon` for rejected work and `workspace.close` when the durable
   record should remain but its live branch document is no longer needed.

After a restart, use `workspace.list` / `workspace.recover` and resolve the new branch
`documentId`; process-lifetime IDs from the old run are invalid. If the user edits the
source while a branch is open, re-check status before merging rather than assuming the
branch base is still current.

Prefer preview/apply pairs such as `geometry.*.preview` / `geometry.*.apply` and
`brushes.optimize.preview` / `brushes.optimize.apply`. Review preview counts, bounds,
warnings, and material effects before applying. Create a checkpoint before broad CSG,
optimization, or generated-geometry operations.

### Visual acceptance projects

Acceptance projects make reconstruction work repeatable. Every `acceptance.*` request
must include an explicit absolute `projectPath`. The JSON file is durable and stores
portable document paths relative to itself, named virtual cameras, comparisons,
metrics, assertions, suites, and an optimistic-concurrency `revision`.

A normal flow is:

1. Create a named view with `acceptance.views.create` and its current
   `expectedRevision`.
2. Create a comparison with relative reference and target document paths, explicit
   view IDs, alignment, metrics, and assertions.
3. Create a suite containing the comparison IDs.
4. Use `acceptance.capture` while authoring and `acceptance.run` for the repeatable
   gate.

Use exact depth and silhouette checks when geometry should match. Give color a stated
tolerance when the reconstruction intentionally improves materials instead of cloning
the export. Suite status includes both image metrics and structural assertions; inspect
the per-metric diagnostics even when the aggregate passes. Captured image and depth
paths are temporary artifacts, while the acceptance project itself is durable.

### Map reconstruction principles

An exported map is a visual and spatial reference, not a brush-structure template.
Rebuild intended forms with clear, maintainable construction:

- Establish named acceptance cameras before a large geometry pass.
- Match important bounds, silhouettes, openings, traversal, and sightlines first.
- Use general operations such as planar profiles, profile extrusion, path sweeps, edge
  bridging, water construction, and CSG where they make the intent clearer.
- Prefer a small number of meaningful convex brushes and named groups/layers over
  copied triangulation or incidental exporter fragmentation.
- Apply correct materials deliberately after the structural form is sound.
- Compare reference and target continuously from player views and diagnostic overhead
  views; do not wait until the whole area is rebuilt.

The goal is not byte-for-byte parity with exported brushwork. The goal is a cleaner map
that preserves the important visual and gameplay relationships.

### Complete-surface visual QA

Do not declare a textured structure complete from one frontal or distant comparison.
Before editing, make a coverage matrix of every exposed surface family. For a symmetric
building this normally includes front, back, both outer returns, both inward returns,
upper and lower bands, roof transitions, and any player-visible underside. Symmetry is
not evidence: validate both sides independently.

Use this gate for material or facade work:

1. Inspect the source material bitmap at native resolution. Determine whether a pale
   block, diagonal, pilaster, shadow, or trim line is baked into the bitmap before
   changing UVs or geometry.
2. Use `render.pick` and bounded `nodes.query` / `nodes.describe` results to identify
   the actual exposed planes and bounds. Do not infer a return plane from a nearby
   facade coordinate.
3. Add a close named acceptance view for every exposed surface family, including
   inward-facing and mirrored returns. Keep separate player-distance views for
   silhouette and composition.
4. Capture the exact branch `documentId` and confirm the response revision matches the
   mutation under review. Compare all close views after each material pass.
5. Check seams and transitions as well as broad faces: corners, band boundaries, roof
   trim, and intersections with adjacent wings often reveal a bad material or an
   over-wide repair veneer.
6. Record all views in the acceptance project before merging. A clean hero view is not
   a substitute for complete close-view coverage.

For a repair that uses thin facade veneers, derive each veneer from picked surface
bounds, split it at intentional material bands, keep it behind preserved trim, and
inspect both adjoining corners. Prefer correcting the underlying faces when practical;
when a veneer is the maintainable choice, group and name it by architectural purpose.

### Verify RPC and focus-neutral changes

Build `TbUiLibTest` before running its CTest suite. For focused automation work, list
the registered tests and run the narrow filters first, then the complete suite:

```sh
cmake --build <build-dir> --target TbUiLibTest TrenchBroom
ctest --test-dir <build-dir>/lib/TbUiLib/test -N
ctest --test-dir <build-dir>/lib/TbUiLib/test -j --output-on-failure
```

Run the Python driver tests after changing the external harnesses:

```sh
python3 -m unittest \
  scripts.automation.test_verify_workspace_restart \
  scripts.automation.test_verify_focus_neutral_rendering
```

On macOS, use the opt-in external drivers for an actual copied-app restart and
focus-neutral render check. They launch a private portable app with `open -n -g -j`,
use only its explicit discovery file, and clean up only the verified private PID:

```sh
python3 scripts/automation/verify_workspace_restart.py \
  --run --app <build-dir>/app/TrenchBroom/TrenchBroom.app \
  --tbctl <build-dir>/app/TbCtl/tbctl --source-map /absolute/path/to/fixture.map

python3 scripts/automation/verify_focus_neutral_rendering.py \
  --run --app <build-dir>/app/TrenchBroom/TrenchBroom.app \
  --tbctl <build-dir>/app/TbCtl/tbctl --source-map /absolute/path/to/fixture.map
```

Treat focus theft, unexpected windows, save prompts, identity drift, and resource
readiness races as product defects. Reproduce and fix them at the RPC/automation
boundary instead of adding GUI-driving workarounds or arbitrary sleeps.

## Build and test
- TrenchBroom uses CMake as its build system.
- In Visual Studio Code, prefer CMake Tools for builds.
- Build the narrowest relevant target instead of building the whole workspace when possible.
- For library changes, prefer the corresponding <Name>LibTest target to validate the change.
- Always build the relevant test target before running tests.
- Tests use Catch2.
- If VS Code test discovery is unavailable, run the built test executable directly from the build tree, for example build/lib/TbMdlLib/test/TbMdlLibTest.
- Use --list-tests to discover available tests and Catch2 filters to run a focused subset.
- Prefer `ctest --test-dir <build>/lib/<Name>/test -j` over invoking the test binary directly. `catch_discover_tests` registers every Catch2 test case as its own CTest test, so `-j` (no thread count — let ctest pick) runs them in parallel and is markedly faster than one sequential process. Build the test target first; ctest does not build.
- Use Build.md for platform-specific setup and dependency details.

### Code coverage
- **Enable coverage instrumentation**: Pass `-DTB_ENABLE_GCOV=1` for gcov-compatible coverage (works with GCC or Clang) or `-DTB_ENABLE_LCOV=1` for LLVM source-based coverage (Clang only).
- **Generate coverage data**: 
  - For gcov: Build and run tests normally. `.gcno` and `.gcda` files are automatically generated in the build tree.
  - For LLVM/lcov: Run tests with `LLVM_PROFILE_FILE=default.profraw <test-executable>` to generate `.profraw` profile data.
- **Analyze coverage**: Use coverage tools to identify uncovered code paths, untested branches, and low-coverage functions.
- **Guide test improvements**: When reviewing or creating tests, examine coverage reports to identify and address gaps:
  - Suggest new tests for uncovered code paths or error conditions.
  - Improve existing tests to cover branch conditions not yet exercised.
  - Identify edge cases or exception handling that lack test coverage.
- **Reference coverage in commit messages**: When submitting test improvements motivated by coverage analysis, mention that coverage-guided testing was used to identify gaps.

## Test structure
- For each compilation unit, tests are usually in one file named tst_<CompilationUnit>.cpp.
- Prefer one test case per class.
- Prefer one section per member function.
- For free functions, prefer one test case per file and one section per function.

## Code style
- Format changes with clang-format. The repository style is defined in /.clang-format.
- Respect the existing include ordering rules from /.clang-format. In particular, Qt headers must come first.
- Follow the surrounding file's style and patterns unless there is a clear reason not to.

## Git History
- Keep the git history as clean as possible.
- Avoid unnecessary churn, including changing the same code multiple times in a branch when a cleaner edit is possible.
- Prefer changes that read like a clean transformation from the original state to the desired result.
- When creating a series of commits, keep each commit coherent, buildable, and with the relevant tests passing when practical.
- When asked to write commit messages, explain why the change was made in the context of a feature or bug fix, not just what changed.
