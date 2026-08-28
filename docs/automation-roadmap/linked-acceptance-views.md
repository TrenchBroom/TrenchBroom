# Linked cameras, named acceptance views, and visual QA

**Status:** The AV0-AV6 foundation is implemented with color, linear-depth, silhouette,
sightline, bounds-visibility, and opening-clearance evaluation. Landmark solving,
edge/object/material buffers, the Unrest seed suite, and the optional UI remain scoped
follow-ups rather than silently approximated features.

Comparison contexts now also support `acceptance.geometry.compare`, a bounded,
reference-space brush-volume sampler with neutral `referenceOnly` / `candidateOnly`
findings, face-connected discrepancy regions, and an optional evidence-backed divergence
policy. `acceptance.evidence.run` publishes an immutable, hashed bundle containing the
acceptance project, exact captured map revisions, normalized cameras, render settings,
color/depth images, renderer version, and the full suite report. Persisted policy,
adaptive sampling, material/content classification, and player connectivity remain
follow-ups.

## Goal

Map reconstruction gains repeatable, reviewable visual acceptance tests. A named view
captures design intent once; paired source/target rendering and structural assertions
can then be rerun on every branch revision without moving the user's editor camera.

This plan depends on the virtual render contract in
`explicit-views-and-offscreen-rendering.md`. It must not introduce a second path that
captures the active GUI viewport implicitly.

## Persisted concepts

### Comparison context

A durable project-level assignment of reference and candidate roles:

```json
{
  "id": "unrest-rebuild",
  "name": "Unrest rebuild",
  "reference": {"documentPath": "../maps/unrest.map"},
  "candidate": {"documentPath": "../maps/unrest_rebuilt.map"},
  "alignment": {"type": "identity"}
}
```

The alignment maps reference coordinates into candidate coordinates. Context-bound
operations treat the reference as read-only and never resolve either role from the
active GUI document. Read-only does not mean infallible: raw comparison is neutral and
accepted repairs require evidence-backed policy. See
[ADR 0002](adr-0002-first-class-comparison-contexts.md) and
[ADR 0003](adr-0003-reference-is-evidence.md).

### Named view

A durable camera and render configuration:

```json
{
  "id": "view-entry-mansion-reveal",
  "name": "Entry mansion reveal",
  "camera": {
    "projection": "perspective",
    "position": [21840, 3012, -1520],
    "direction": [0.05, 0.99, 0.02],
    "up": [0, 0, 1],
    "verticalFov": 75
  },
  "size": [1600, 900],
  "renderMode": "textured",
  "overlays": {"brushEdges": true, "grid": false}
}
```

For rapid iteration, a later session-scoped `camera.create/set/capture/pick` facade may
hold a mutable automation camera handle bound to one non-reusable document instance.
It is convenience state over the same virtual render contract, not a GUI view. Writes
to a real main viewport remain a separate, explicit API. Named views persist only the
camera pose/profile, never the session camera handle.

### Comparison

A pairing of named views, normally bound to a reusable comparison context:

```json
{
  "id": "comparison-entry-reveal",
  "name": "Entry mansion reveal",
  "contextId": "unrest-rebuild",
  "reference": {"viewId": "view-entry-mansion-reveal-source"},
  "target": {"viewId": "view-entry-mansion-reveal-target"},
  "metrics": ["depth", "silhouette"],
  "assertions": []
}
```

Alignment types:

- `identity`: documents share world coordinates.
- `matrix`: explicit affine transform between coordinate systems.
- `landmarks`: derive and persist a transform from corresponding points.
- `independent`: reference and target use unrelated manually authored cameras.

### Acceptance suite

A versioned collection of comparisons and assertions:

```json
{
  "schemaVersion": 2,
  "suiteId": "front-gardens",
  "name": "Front gardens",
  "comparisons": [
    "comparison-entry-reveal",
    "comparison-pools-overhead",
    "comparison-right-pool-to-maze"
  ]
}
```

Store these in a project-side JSON file chosen by project configuration. Avoid guessing
a sidecar path solely from a transient document filename. The store must support an
explicit project path and portable relative document references.

## Capture and comparison outputs

`comparison.capture` renders both documents using the same normalized request contract
and returns:

```json
{
  "reference": {"colorPath": "/tmp/reference.png", "revision": 0},
  "target": {"colorPath": "/tmp/target.png", "revision": 52},
  "overlayPath": "/tmp/overlay.png",
  "differencePath": "/tmp/difference.png",
  "metrics": {
    "silhouetteMismatchPercent": 4.2,
    "meanDepthDifference": 0.034
  }
}
```

Raw RGB difference is diagnostic only. Pass/fail thresholds should initially use
structural outputs:

- Depth difference inside a configurable mask.
- Foreground silhouette mismatch.
- Object/material ID presence and coverage.
- Edge-map mismatch with a positional tolerance.

Every report records both document fingerprints/revisions, renderer version, normalized
cameras, image dimensions, metric configuration, and assertion results.

## Structural assertions

Assertions should be geometric or render-buffer based, not natural-language image
judgments. Initial types:

- `boundsVisible`: projected coverage of tagged or explicit bounds exceeds a threshold.
- `boundsNotVisible`: occluder/hidden-region check.
- `clearSightline`: one ray or a sampled ray corridor reaches the intended target.
- `openingClearance`: projected or world-space opening exceeds width/height bounds.
- `materialCoverage`: a material appears within a region at an expected coverage.
- `depthRange`: visible geometry in a mask lies within an expected range.

Assertions may reference stable semantic node IDs/tags when available, but the first
version must also accept explicit world bounds so it does not block on a separate node
identity project.

For the Unrest entrance, a suite should include at least:

- Player-height mansion reveal from the entry corridor.
- Pools overhead and each pool-to-maze boundary.
- Gate arch profile from both directions.
- River mouth, surface, and endpoint views.

## RPC contract

Implemented methods:

- `acceptance.contexts.list/create/update/delete`
- `acceptance.views.list/create/update/delete`
- `acceptance.comparisons.list/create/update/delete`
- `acceptance.suites.list/create/update/delete`
- `acceptance.capture`
- `acceptance.run`
- `acceptance.evidence.run`
- `acceptance.assertions.evaluate`
- `acceptance.geometry.compare`

Create/update/delete methods require an expected store revision and are atomic. Capture
and run methods are read-only with respect to maps, views, and the persisted suite.

`acceptance.run` and `acceptance.evidence.run` support a comparison filter and bounded
concurrency. GPU captures may serialize on the render service even when CPU metrics and
assertion evaluation run concurrently.

Use `acceptance.evidence.run` when a result must survive the process:

```json
{
  "projectPath": "/absolute/path/unrest-acceptance.json",
  "suiteId": "unrest-rebuild",
  "comparisonIds": ["mansion-close-reference"],
  "outputDirectory": "/absolute/nonexistent/path/evidence-run-2026-08-28",
  "maxCpuConcurrency": 4
}
```

The output directory must not exist. The service snapshots the exact in-memory document
revision used by each capture, hashes every artifact, writes through a sibling staging
directory, and atomically publishes the bundle. A failed suite is still valid evidence;
its manifest records the failure and diagnostics. Never overwrite an older evidence run.

`acceptance.geometry.compare` always reports neutral `referenceOnly` and `candidateOnly`
occupancy. Each side includes compact face-connected `regions`, largest first, even when
raw `cells` are omitted. Use `includeCells: true` only when a downstream operation needs
the individual sampling cells. Treat regions as leads for investigation, not proof that
the candidate or reference is correct.

## Work packages

### AV0 - Freeze persisted schemas and store revision model

- **Dependencies:** virtual camera/render schema EV0.
- **Primary ownership:** new acceptance value-type/JSON files and schema tests.
- **Deliverable:** version 1 named view, comparison, suite, alignment, mask, metric, and
  assertion schemas.
- **Test gate:** round trips, relative path resolution, unknown versions/types,
  referential integrity, deterministic serialization.

### AV1 - Project acceptance store

- **Dependencies:** AV0.
- **Primary ownership:** new `AcceptanceViewStore.h/.cpp` and store tests.
- **Deliverable:** atomic load/update APIs with optimistic store revision and validation
  across views, comparisons, and suites.
- **Test gate:** concurrent revision conflict, broken references, document relocation,
  corrupt file, atomic write failure.

### AV2 - Named-view RPC and virtual capture adapter

- **Dependencies:** AV1 and virtual capture EV4.
- **Primary ownership:** a dedicated acceptance-view handler and tests.
- **Integration touchpoints:** central dispatch/docs owned by the integration agent.
- **Deliverable:** CRUD and capture for named views without touching GUI view state.
- **Test gate:** capture hidden source/target documents; repeated capture is stable;
  foreground window is unchanged.

### AV3 - Paired capture and alignment

- **Dependencies:** AV0 and EV4; can begin before AV1 is complete using inline fixtures.
- **Primary ownership:** new comparison runner and alignment math/tests.
- **Deliverable:** identity, matrix, and independent-camera paired captures; landmark
  alignment may follow as a subpackage.
- **Test gate:** synthetic translated/rotated scenes, invalid/non-invertible transforms,
  normalized cameras included in output.

### AV3a - Optional automation camera handles

- **Dependencies:** non-reusable document identities EV1 and virtual capture EV4.
- **Primary ownership:** a small automation-camera registry and lifecycle tests.
- **Deliverable:** create, update, capture, and pick through a session camera without
  changing any GUI pane; stale document instances invalidate their cameras.
- **Test gate:** main-camera immutability, close/reopen invalidation, independent source
  and branch cameras, capture/pick consistency.

### AV4 - Structural image outputs and metrics

- **Dependencies:** structural render output EV6 and AV3.
- **Primary ownership:** separate metric compilation units and synthetic image tests.
- **Deliverable:** silhouette and depth metrics first, then edge/object/material metrics.
- **Test gate:** exact identical scenes, known pixel/depth changes, masks, tolerances,
  missing structural buffers, deterministic reports.

### AV5 - CPU sightline and clearance assertions

- **Dependencies:** AV0; can proceed concurrently with AV2-AV4.
- **Primary ownership:** assertion evaluator and model-query tests.
- **Deliverable:** `clearSightline`, `boundsVisible` approximation, and
  `openingClearance` using explicit bounds/rays.
- **Test gate:** synthetic portals, partial occlusion, start-inside-solid, boundary
  tolerance, transformed comparison coordinates.

### AV6 - Acceptance suite runner

- **Dependencies:** AV1-AV5, though an initial capture-only runner may land after AV3.
- **Primary ownership:** suite scheduler/report types and tests.
- **Deliverable:** filtered suite execution, bounded CPU concurrency, serialized GPU
  work, stable report ordering, cancellation.
- **Test gate:** mixed pass/fail/error results, cancellation, document revision change,
  missing document, deterministic aggregate status.

### AV7 - External `tbctl` workflow and Unrest seed suite

- **Dependencies:** AV6.
- **Primary ownership:** integration fixtures/scripts and project acceptance data, not
  production service code.
- **Deliverable:** create/capture/run the front-gardens suite entirely through RPC.
- **Test gate:** detects a deliberately inserted transverse wall, pool boundary gap,
  and blocked mansion sightline.

### AV8 - Optional Saved Views / Acceptance UI

- **Dependencies:** AV6 and a stable RPC/model contract.
- **Primary ownership:** new panel/model/delegate files.
- **Deliverable:** browse suites, capture or update a view from the current pane,
  run/review results, and explicitly apply a named camera to a chosen pane.
- **Integration touchpoints:** menus/layout persistence owned by the UI integration
  agent.
- **Test gate:** viewing results does not move a camera; applying a camera requires an
  explicit pane and user action.

## Parallel execution plan

After AV0, start AV1, AV3, and AV5 concurrently. AV2 follows AV1 plus EV4. AV4 follows
AV3 plus EV6. AV6 is the convergence point. AV7 and AV8 can then proceed independently.

Keep GPU render work, CPU comparison metrics, persistence, assertion evaluation, and UI
in separate packages. One integration agent owns RPC registration, shared CMake edits,
and the final API documentation update.

## Completion criteria

- A named comparison can render source and target without moving any GUI camera.
- A suite report is reproducible and records exact inputs and revisions.
- Structural metrics distinguish geometry changes from harmless texture differences.
- The entrance suite detects the known blocked-mansion and pool-boundary regressions.
- Results can be reviewed through files/RPC before any optional UI is implemented.
