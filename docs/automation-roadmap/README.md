# Automation reliability and visual QA roadmap

Status: core roadmap implementation complete; opt-in external macOS gates and the
explicitly deferred structural buffers/UI remain follow-up work.

This roadmap records the three infrastructure changes that would most improve
agent-assisted map reconstruction:

1. [Persistent, recoverable workspaces](persistent-workspaces.md)
2. [Explicit view identities and focus-neutral rendering](explicit-views-and-offscreen-rendering.md)
3. [Linked cameras, named acceptance views, and visual QA](linked-acceptance-views.md)

They deliberately form a dependency stack. Persistent workspaces make experiments
recoverable. Explicit views and virtual rendering make observation deterministic.
Named comparisons then turn visual intent into a repeatable acceptance suite.

## Product principles

- An explicit document, workspace, or view identifier must never fall back to active
  GUI state.
- Read-only automation must not change focus, selection, camera, current layer, or
  current material.
- Automation-owned state must survive a normal application restart when its backing
  files survive.
- Preview and inspection should work without showing a window.
- Persisted formats are versioned, validated, and written atomically.
- Geometry and visual results are revision-scoped and reproducible.
- Texture differences are not a substitute for structural comparison; depth,
  silhouette, and object identity are first-class outputs.

## Delivery order

The recommended integration sequence is:

```text
R0 Contract freeze and test fixtures
 |
 +-- R1 Persistent workspace manifests and dormant recovery
 |
 +-- R2 Explicit GUI view IDs
       |
       +-- R3 Focus-neutral virtual capture
             |
             +-- R4 Named views and paired captures
                   |
                   +-- R5 Structural comparison and acceptance suites
```

Workspace persistence and explicit GUI view IDs may proceed concurrently after R0.
Virtual capture depends on the camera/render request contract from the explicit-view
plan. Named comparisons must consume virtual capture rather than reaching back into a
focused `MapWindow`.

## Contract-first integration rule

Before implementation agents start, a lead agent lands a small contract commit for the
milestone. It contains only:

- Public value types and JSON schemas.
- Abstract interfaces or narrow service boundaries.
- RPC method names and success/error shapes.
- Empty or disabled wiring where necessary.
- Test helpers that workers will share.

After that commit, workers receive packages with disjoint primary files. A designated
integration agent alone owns shared dispatch, top-level CMake lists, and API
documentation during the fan-out. This avoids several workers making incompatible
edits to the same central files.

## Concurrent-agent operating model

Each work package in the three plans specifies:

- **Dependencies**: packages that must already be merged.
- **Primary ownership**: files the worker may create or edit freely.
- **Integration touchpoints**: shared files changed only by the integration agent.
- **Deliverable**: observable behavior rather than an implementation activity.
- **Test gate**: the narrow target and focused tests that must pass.

When spawning workers:

1. Give each worker exactly one package ID.
2. Include the frozen contract commit and this directory in its prompt.
3. Require a short interface note if the contract is insufficient; workers must not
   silently broaden shared APIs.
4. Keep production code and its focused tests in the same worker package when their
   files do not overlap another package.
5. Assign one integration worker per milestone to perform shared CMake/dispatch/docs
   edits and run the combined test target.
6. Assign a separate verification worker to exercise restart, multi-window, and
   focus-neutral behavior from the external `tbctl` boundary.

Workers should prefer adding narrowly named compilation units over growing
`AutomationService.cpp` or `AutomationJson.cpp`. Central request routing should remain
thin; feature-specific parsing and behavior belong in their own service or handler.

## Shared vocabulary

- **Document ID**: process-lifetime identity of an open map document.
- **View ID**: process-lifetime identity of one real GUI viewport.
- **Virtual view**: a camera and render configuration not backed by a visible widget.
- **Workspace ID**: durable identity of a base/source/branch editing workspace.
- **Named view**: durable camera and render configuration.
- **Comparison**: a reference/target pairing of named or inline virtual views.
- **Acceptance suite**: a versioned set of comparisons and structural assertions.

Persisted IDs and process-lifetime IDs must remain visibly different in API schemas.
No persisted file may contain a `MapWindow*`, `MapViewBase*`, memory address, or a
process-lifetime document/view ID.

## Milestone-wide quality gates

Every milestone must satisfy all of the following before the next one starts:

- `git diff --check` and formatting are clean.
- The narrow library test target builds before tests run.
- Focused Catch2/CTest tests pass.
- Existing automation tests pass.
- At least one external `tbctl` scenario exercises the feature.
- Explicit targets are tested while a different document/window is foregrounded.
- Failure leaves documents and persisted metadata unchanged.
- Restart behavior is tested where persistence is involved.

The plans intentionally leave UI polish behind the automation and model contracts. The
RPC workflow can prove the architecture before a panel is built around it.
