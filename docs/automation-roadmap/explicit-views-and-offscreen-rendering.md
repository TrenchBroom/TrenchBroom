# Explicit view identities and focus-neutral rendering

**Status:** EV0-EV5 and linear-depth EV6 are implemented. The EV7 driver is present
and defaults to a no-launch dry run; the opt-in external run and additional structural
buffers remain explicit follow-up gates.

## Goal

Automation can either target one real editor viewport by ID or render/pick through a
virtual camera that is independent of every visible window. Neither path depends on
which document or widget the user most recently focused.

## Two distinct concepts

The API must stop conflating:

1. **Real view inspection:** "show me exactly what this editor pane shows."
2. **Virtual rendering:** "show me this document from this supplied camera."

A real view has a process-lifetime `viewId`. A virtual view is an immutable request
value and has no widget, focus state, or persistent process identity.

## Prerequisite: non-reusable document identities

The current automation document ID is derived from a `MapWindow` address. Closing a
window and later reusing that address can make a stale ID refer to a different document.
Before adding view IDs, replace this with a registry-minted random UUID or monotonically
unique process-lifetime instance token. Closing the document invalidates the token; it
is never rebound during that process.

Responses should echo the resolved document ID, path, revision, and optional workspace
role. Persisted plans store paths/workspace IDs, never process-lifetime document IDs.
Clients that need an additional stale-target guard may supply an expected path or
document-instance token, but there is never a fallback from a stale ID to a path match.

## Explicit real-view identity

Each live `MapViewBase` receives an opaque ID when registered:

```text
view-3d-7f10...
view-xy-623a...
```

The registry stores `QPointer<MapViewBase>`, its owning document/window identity, and
semantic view type. IDs are never memory addresses and are invalidated when the view is
destroyed.

Proposed methods:

- `views.list`: list real panes for one explicit document.
- `view.state.get`: capture one explicit real view.
- `view.state.set`: set one explicit real view's camera.
- `view.capture`: capture one explicit real view's framebuffer.
- `view.pick`: pick through one explicit real view.

Every real-view method requires both `documentId` and `viewId`. The registry validates
ownership. Supplying a valid view from another document is an error, not a fallback.

Document-only forms may be retained temporarily for compatibility, but must resolve a
view once at the boundary and report the resolved `viewId`. New workflows should never
use them.

## Virtual render contract

Virtual operations use explicit camera and render options:

```json
{
  "documentId": "document-...",
  "camera": {
    "projection": "perspective",
    "position": [120, 300, 80],
    "direction": [0.2, 0.9, -0.1],
    "up": [0, 0, 1],
    "verticalFov": 75,
    "near": 1,
    "far": 65536
  },
  "size": [1600, 900],
  "renderMode": "textured",
  "overlays": {
    "brushEdges": true,
    "selection": false,
    "grid": false
  }
}
```

Methods:

- `render.capture`: render to automation-owned output without a visible widget.
- `render.pick`: perform a model pick using the same resolved camera and image size.
- `render.context`: return semantic camera/document/layer context without an image.

The response echoes the normalized camera, document revision, size, render mode, and
output paths. A repeated request against the same revision and renderer version should
be deterministic.

`overlays.selection` controls selection highlighting, not object visibility. When it
is false, selected objects render with their ordinary material and edge style. This is
important for automation because modeling operations select newly created brushes; an
immediate unhighlighted capture must still contain the changed geometry without
mutating or clearing the document selection.

Initial render mode is `textured`. Planned structural modes are `flat`, `wireframe`,
`depth`, `surfaceNormals`, `materialId`, and `objectId`.

## Render architecture decision

Offscreen image generation needs an explicit spike before the implementation contract
is frozen. Evaluate two approaches:

### A. Renderer extraction (preferred long term)

Create a render service that consumes a document render scene, camera, options, and
framebuffer without depending on `MapWindow` or `QOpenGLWidget`. Keep scene rendering
in the render library; Qt context scheduling and file output may remain in the UI
library.

Advantages: truly windowless, reusable for structural buffers, clean tests. Cost:
existing map-view rendering may need refactoring to expose a non-widget entry point.

### B. Automation-owned hidden view (bounded fallback)

Create a dedicated hidden/offscreen surface and automation-owned map view. It must
never be inserted into a user window, activated, or registered as a current GUI view.

Advantages: faster initial delivery. Cost: retains widget lifecycle and platform GL
quirks; structural render modes may be harder.

The spike must measure macOS reliability, context sharing, material/resource readiness,
capture latency, output determinism, and behavior with no map windows shown. The chosen
path and rejected alternative are recorded in an ADR before implementation fan-out.

OpenGL work may remain on the Qt GUI thread. Focus-neutral does not require a worker
thread; it requires independence from active widgets and GUI state.

## Invariants

- Explicit view operations validate document/view ownership.
- Virtual operations never call `MapWindow::currentMapViewBase()`.
- Virtual capture does not change GUI camera, focus, selection, current material, layer,
  grid, or pane layout.
- Disabling the selection overlay never removes selected geometry from the image.
- Real-view capture observes GUI state but does not mutate it.
- Output filenames are unique and written beneath the configured automation temporary
  directory.
- A stale/destroyed view ID returns a structured error.
- A request records the map revision before rendering and either completes against that
  revision or reports that the document changed.

## Work packages

### EV0 - Freeze view and virtual-camera schemas

- **Dependencies:** roadmap contract rules only.
- **Primary ownership:** new focused value-type/JSON files and tests; avoid adding more
  unrelated code to `AutomationJson.cpp`.
- **Deliverable:** normalized camera, projection, size, overlay, render-mode, and output
  descriptor types.
- **Test gate:** malformed and degenerate cameras, orthographic/perspective round trips,
  size limits, unknown modes, deterministic JSON.

### EV1 - Real view registry

- **Dependencies:** EV0 only for shared view type names.
- **Primary ownership:** new `AutomationDocumentRegistry.h/.cpp`,
  `AutomationViewRegistry.h/.cpp`, and registry tests.
- **Integration touchpoints:** `MapViewBase` lifecycle hooks, applied by the integration
  agent if other view work is concurrent.
- **Deliverable:** non-reusable document/view IDs, ownership validation, destruction
  invalidation, pane enumeration.
- **Test gate:** multiple documents and panes, pointer-address reuse, document reopen,
  layout replacement, destroyed views, cross-document rejection.

### EV2 - Strict real-view RPC migration

- **Dependencies:** EV1.
- **Primary ownership:** a dedicated real-view handler and its socket-level tests.
- **Integration touchpoints:** thin dispatch from `AutomationService_View.cpp` and public
  API docs, owned by the integration agent.
- **Deliverable:** `views.list` plus strict state/capture/pick methods.
- **Test gate:** foreground document B while every explicit operation on document A
  remains tied to A's requested view.

### EV3 - Offscreen rendering architecture spike

- **Dependencies:** EV0; can run concurrently with EV1 and EV2.
- **Primary ownership:** disposable prototype files and an ADR in this directory.
- **Deliverable:** evidence-backed selection of renderer extraction or hidden automation
  surface, including macOS results and a proposed production boundary.
- **Test gate:** capture a textured frame with no visible map window and prove no focus
  or GUI camera changes.

### EV4 - Production virtual capture

- **Dependencies:** EV0 and EV3.
- **Primary ownership:** new render-service compilation units and focused tests.
- **Integration touchpoints:** resource initialization and app-owned service lifetime,
  handled by the milestone integration agent.
- **Deliverable:** `render.capture` for textured output and `render.context`.
- **Test gate:** hidden document, repeatability, resource-not-ready behavior, revision
  change during render, invalid output directory, no user-state mutation.

### EV5 - Virtual picking

- **Dependencies:** EV0; may run in parallel with EV4 because model picking already
  accepts a supplied camera.
- **Primary ownership:** a virtual-pick handler and tests around `pickMapView`.
- **Deliverable:** `render.pick` using the same camera normalization and pixel
  conventions as capture.
- **Test gate:** known brush/face hits in perspective and orthographic projections;
  capture and pick rays agree.

### EV6 - Structural render outputs

- **Dependencies:** EV4 and its chosen renderer boundary.
- **Primary ownership:** render-mode passes and image/output encoders, separated by mode
  where possible.
- **Deliverable:** depth and object/material identity first; normals and flat/wireframe
  subsequently.
- **Test gate:** stable synthetic scenes with exact IDs/depth tolerances and documented
  file formats.

### EV7 - External focus-neutral verification

- **Dependencies:** EV2, EV4, and EV5.
- **Primary ownership:** external integration scenario/script.
- **Deliverable:** `tbctl` drives real and virtual views while a human-facing document
  is repeatedly foregrounded and edited.
- **Test gate:** requested IDs and outputs stay correct; no application activation or
  camera/selection changes.

## Parallel execution plan

After EV0, start EV1, EV3, and EV5 concurrently. EV2 follows EV1. EV4 follows EV3. EV6
follows EV4. EV7 is the convergence gate. One integration agent owns `MapViewBase`, app
service construction, central RPC dispatch, CMake aggregation, and public docs to avoid
hot-file conflicts.

## Completion criteria

- Every real viewport exposed to automation has an opaque, ownership-checked ID.
- A virtual capture succeeds with no visible map window.
- Neither real-view inspection nor virtual rendering steals macOS focus.
- Virtual capture/pick never reads the active window or its current pane.
- Main, reference, and branch cameras can be manipulated independently and verified by
  tests at the external RPC boundary.
