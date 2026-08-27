# Local automation API

TrenchBroom starts a same-user JSON-RPC 2.0 service on a local socket. It is intended
for agent-assisted editing, batch tools, and experiments which need the editor's live
in-memory document, renderer, selection, and undo system instead of modifying a saved
map behind the editor's back.

The service writes a discovery file to `automation/<pid>-<instance>.json` below TrenchBroom's user
data directory. `tbctl` finds the newest discovery file automatically; `--socket` and
`--discovery` override this when several instances are running.

```sh
tbctl --method system.ping --pretty
tbctl --method documents.list --pretty
tbctl --method context.capture \
  --params '{"documentId":"document-...","screenshot":true}' --pretty
```

Requests and responses are one compact JSON object per line. The server only accepts
local same-user connections. Mutating model methods require the revision returned by a
context capture or query and fail with error `-32001` if the document changed in the
meantime.

## Inspection and editing

- `system.ping`
- `documents.list`
- `context.capture` — semantic view/document/selection/camera context and an optional
  framebuffer PNG
- `view.pick` — editor picking at view-local pixel coordinates without changing focus
  or selection
- `view.camera.set` and `view.frame` — manipulate a branch/reference camera without
  activating its window
- `nodes.query` — revision-scoped semantic search returning node paths, types, bounds,
  classnames, and materials; supports pagination, descendant and spatial filtering, and
  aggregate-only summaries
- `nodes.describe` — read exact brush geometry and per-face attributes by explicit path
- `nodes.group.create` — create a named, empty group below an explicit parent or the
  current layer
- `nodes.entity.create` — create an empty point or brush entity below an explicit
  parent or the current layer
- `nodes.select` and `nodes.delete`
- `faces.copyAttributes` — atomically copy complete material, UV, and surface attributes
  from explicit source faces in an open document to explicit target faces in an open
  workspace document, without changing either document's selection or current material
- `brushes.optimize.preview` and `brushes.optimize.apply` — analyze or apply exact
  brush-count reductions for compatible axis-aligned cuboids or coplanar convex prisms;
  preview is read-only and apply is one undoable, revision-guarded transaction
- `brushes.optimize.batch.preview` and `brushes.optimize.batch.apply` — discover
  independent optimizable cohorts within a supplied brush set and apply every cohort's
  best candidate in one revision-guarded transaction
- `geometry.bridgeEdgeChains`, `geometry.volumeToPlane`, and `geometry.eqWater` —
  generate brushes using the editor's native geometry commands, with explicit selected
  edges, brushes, or faces supplied by revision-scoped node paths
- `geometry.extractFootprints` — read canonical convex polygons from named brush faces
  at an axis-aligned plane, without changing selection, focus, or map contents
- `geometry.createBrushes` — create one or more explicit convex brushes below a chosen
  parent in one revision-guarded, undoable operation
- `geometry.sweepPath.preview` and `geometry.sweepPath.apply` — turn snapped planar
  paths into low-count, gap-free rectangular-profile brush runs for walls, curbs,
  beams, paths, trim, fences, or hedges; preview is read-only and apply is atomic
- `geometry.planarProfile.preview` and `geometry.planarProfile.apply` — decompose a
  closed XY contour into grid-snapped, mitered inset bands and an optional convex-prism
  core for basins, curbs, plinths, terraces, roof borders, and paved areas
- `geometry.extrudeProfile.preview` and `geometry.extrudeProfile.apply` — extrude a
  closed profile in the XY, XZ, or YZ plane into one convex prism or a deterministic
  set of triangular prisms for a concave profile
- `geometry.csg` — apply `convexMerge`, `subtract`, `intersect`, or `hollow` to an
  explicit brush set using the editor's native CSG commands. `subtract` can additionally
  receive explicit, disjoint `targetPaths` to cut only those brushes.
- `document.exportSelection` and `document.paste` — round-trip native map fragments,
  preserving the normal parser, validation, notifications, and undo behavior
- `document.save` — persist a document without activating its window
- `document.reload` — reload a persistent document without activating its window;
  modified documents require explicit `discardChanges: true`
- `reference.open` — open another map with the source document's game configuration

## Stable documents, views, and virtual cameras

`documents.list` returns a process-lifetime `id` for each open document. These IDs are
random, never pointer-derived, never reused, and become permanently stale when their
document closes. Persist paths or workspace IDs between TrenchBroom launches; do not
persist document or view IDs.

- `views.list` requires `documentId` and returns every real editor pane with an opaque
  `viewId` and semantic view type.
- Real-view operations require both `documentId` and `viewId`; a missing ID or a view
  owned by another document is rejected. They never resolve the current pane.
- `render.context`, `render.capture`, and `render.pick` require an explicit
  `documentId` plus an immutable camera/size request. They do not inspect or move a GUI
  pane. Capture renders through an application-owned offscreen context and returns a
  unique PNG path; pick constructs the matching model ray without OpenGL. Set
  `outputs.depth: true` to receive a matching linear depth buffer.
- `cameras.create/get/update/delete/capture` provide optional session camera handles.
  A handle is permanently bound to one live document ID, invalidates when that document
  closes, and never changes a real pane.

Perspective virtual requests use `verticalFov`; orthographic requests use `zoom`. Both
use explicit `position`, `direction`, `up`, `near`, `far`, `size`, `renderMode`, and
`overlays`. For example:

```sh
tbctl --method render.capture --params \
  '{"documentId":"document-...","camera":{"projection":"perspective","position":[0,-128,64],"direction":[0,1,0],"up":[0,0,1],"verticalFov":75,"near":1,"far":65536},"size":[1280,720],"renderMode":"textured","overlays":{"brushEdges":false,"selection":false,"grid":false},"outputs":{"depth":true}}' --pretty
```

Depth is returned as little-endian float32 grayscale PFM (`Pf`). Samples are linear
camera-forward distances in map units; positive infinity denotes uncovered background.
PFM stores scanlines bottom-to-top, while TrenchBroom's decoded in-memory convention is
top-left origin matching the color PNG.

## Durable branch workspaces

Automation workspaces persist a manifest, immutable base map, identity table, and
hash-validated branch checkpoint generations beneath the automation workspace root.
They survive branch-window closure and TrenchBroom restart. A recovered branch uses a
hidden, non-activating editor window so all normal editing and undo commands remain
available without changing the user's foreground document.

- `workspace.fork` requires an explicit source `documentId`.
- `workspace.list` and `workspace.status` inspect attached and dormant records without
  opening them.
- `workspace.recover` reconstructs the latest valid base/branch model from its
  `workspaceId` alone. Older metadata-free version 1 records additionally require an
  explicit live `documentId` solely for game/map-format context.
- `workspace.attachSource` conservatively validates and binds one explicit source
  document; it never searches the active window.
- `workspace.checkpoint`, `workspace.close`, `workspace.rename`, `workspace.diff`, and
  `workspace.merge` operate by durable `workspaceId`.
- `workspace.abandon` is terminal but retains every artifact. There is intentionally no
  one-step destructive discard method.

Checkpoint publication is atomic. A torn newest generation is ignored in favor of the
last complete, fingerprint-valid generation. Closing a checkpointed workspace marks
the hidden branch clean and must not display a save dialog.

## Named acceptance views and suites

Acceptance data lives in a caller-chosen project JSON file; every `acceptance.*`
request requires its explicit `projectPath`. Document references inside that file are
portable paths relative to the project file. Store mutations use `expectedRevision`
and commit atomically.

- `acceptance.views.list/create/update/delete`
- `acceptance.comparisons.list/create/update/delete`
- `acceptance.suites.list/create/update/delete`
- `acceptance.capture` with `comparisonId`
- `acceptance.run` with `suiteId`, optional `comparisonIds`, and optional bounded
  `maxCpuConcurrency`
- `acceptance.assertions.evaluate` for a one-shot assertion against an explicit live or
  acceptance-owned hidden `documentId`

Paired capture loads an exact live path when one is registered, otherwise it owns a
hidden `MapDocument` with no `MapWindow`. Reports echo paths, non-reusable document IDs,
map revisions, normalized cameras, renderer version, image diagnostics, and geometric
assertions. CPU sightline and opening checks resolve only the captured document ID; they
never fall back to the active map. Color metrics compare RGBA; depth metrics compare
finite linear depth; silhouette metrics compare finite-depth coverage. These metric
types are deliberately separate, so an RGB difference is never mislabeled as a
structural depth or silhouette result.

Node paths are arrays of child indices rooted at the world node. They are deliberately
revision-scoped instead of exposing process pointers or writing automation IDs into map
files.

### `nodes.query`

`pattern` is an optional case-insensitive search over each node's semantic JSON. Results
are depth-first ordered. The default page is `offset: 0`, `limit: 200`; `limit` is capped
at 5,000. Responses contain `total`, `offset`, `truncated`, and, when another page is
available, `nextOffset`. Reuse the returned `revision` when walking pages; if it changes,
restart the query rather than mixing paths from different revisions.

`ancestorPath` restricts the search to that node and all of its descendants. `bounds`
accepts `{"min":[x,y,z],"max":[x,y,z]}` and retains nodes whose logical bounds intersect
the box. `types`, `materials`, and `classnames` are optional arrays of exact,
case-sensitive values. Each array is an OR filter; different filters compose as AND.
All filters compose with `pattern`.

Set `projection: "paths"` when only node paths are needed. The response then uses a
compact `paths` array instead of full `nodes` objects. This is useful for fetching a
large precise selection before calling another path-based operation. The default
projection is `"full"`.

Set `aggregate: true` to receive no individual nodes and an `aggregate` object instead.
It contains `total`, plus `byType`, `byClassname`, and (when applicable) `byMaterial`
counts for every matching node. Material counts are per node, not per face; paging options
are ignored in aggregate mode.

```sh
tbctl --method nodes.query --params \
  '{"documentId":"document-...","ancestorPath":[0,12],"bounds":{"min":[-256,-256,-64],"max":[256,256,128]},"limit":500}' --pretty
tbctl --method nodes.query --params \
  '{"documentId":"document-...","pattern":"river","aggregate":true}' --pretty
tbctl --method nodes.query --params \
  '{"documentId":"document-...","types":["brush"],"materials":["t50_agua1"],"projection":"paths","limit":5000}' --pretty
```

### `nodes.describe`

`nodes.describe` is the precise, read-only companion to spatial search. It currently
supports `detail: "brushFaces"` and requires explicit, distinct brush `paths`. It returns
the document `revision`, each brush's bounds and vertices, and every face's boundary
plane, polygon vertices, material, explicit surface attributes, and UV attributes
(including resolved U/V axes). It never changes selection, focus, or map contents.

```sh
tbctl --method nodes.describe --params \
  '{"documentId":"document-...","detail":"brushFaces","paths":[[0,12],[0,13]]}' --pretty
```

### `faces.copyAttributes`

`faces.copyAttributes` copies the complete editable attribute set for every explicit
face mapping: material, UV offset/scale/rotation, stored U/V axes when both source and
target formats support them, plus surface contents, flags, value, and color. It never
changes brush geometry. A cross-format copy never fabricates stored axes from a
paraxial face's resolved axes. It can copy from a reference or other open workspace
document, including the current document. Both documents are revision-guarded: pass the
target's `documentId` and `expectedRevision`,
then the source's `sourceDocumentId` and `sourceRevision`. Every mapping is validated
before the target changes; invalid documents, revisions, paths, face indices, empty
mapping lists, and duplicate target faces fail without mutation. All mappings are
applied as one undoable target-document transaction. Target selection and current
material are restored exactly.

Use `nodes.describe` on each document to obtain revision-scoped `{path, faceIndex}`
references. A source face may be copied to multiple targets, but each target may occur
only once in a request. The result contains `mappingCount`, target `revision`, and the
source document's resulting `sourceRevision` (which equals `revision` when source and
target are the same document).

```sh
tbctl --method faces.copyAttributes --params \
  '{"documentId":"document-target","expectedRevision":42,"sourceDocumentId":"document-reference","sourceRevision":17,"mappings":[{"source":{"path":[0,12],"faceIndex":4},"target":{"path":[0,9],"faceIndex":1}},{"source":{"path":[0,13],"faceIndex":0},"target":{"path":[0,10],"faceIndex":5}}]}' --pretty
```

### `nodes.group.create`

`nodes.group.create` creates one empty named group in one revision-guarded, undoable
transaction. `parentPath` is optional and defaults to the document's current layer. The
parent must be a layer or group that can contain a group; the returned path identifies the
new group and can be used immediately as `parentPath` for geometry creation.

```sh
tbctl --method nodes.group.create --params \
  '{"documentId":"document-...","name":"South Garden - Hedge Maze","parentPath":[0],"expectedRevision":42}' --pretty
```

### `nodes.entity.create`

`nodes.entity.create` creates one empty entity in one revision-guarded, undoable
operation. Supply the exact `classname` from the active game's entity definitions and
an `entityType` of `"point"` or `"brush"`; the type must match that definition. The
optional `properties` object accepts only non-empty string keys and string values.
`classname` is explicit and must not be repeated in `properties`. Custom properties are
accepted, so the API is useful for game-specific keys without hard-coding entity names.

`parentPath` is optional and defaults to the document's current layer. Its target must
accept an entity. A successful brush entity is deliberately empty, and its returned
path can immediately be supplied as `parentPath` to `geometry.createBrushes`. The
operation preserves the document's selection and current material.

```sh
# Create an empty brush entity, then populate it with explicit brushes.
tbctl --method nodes.entity.create --params \
  '{"documentId":"document-...","classname":"func_detail","entityType":"brush","properties":{"targetname":"fountain_shell"},"parentPath":[0],"expectedRevision":42}' --pretty
# The result's path, such as [0,7], is a valid geometry.createBrushes parentPath.

# Create a point entity with only explicit properties.
tbctl --method nodes.entity.create --params \
  '{"documentId":"document-...","classname":"light","entityType":"point","properties":{"origin":"128 64 32","light":"300"},"expectedRevision":43}' --pretty
```

For example, optimize a queried set of triangulated river-surface prisms without first
selecting them or activating their window:

```sh
tbctl --method brushes.optimize.preview --params \
  '{"documentId":"document-...","paths":[[0,12],[0,13],[0,14]]}' --pretty
tbctl --method brushes.optimize.apply --params \
  '{"documentId":"document-...","paths":[[0,12],[0,13],[0,14]],"candidateIndex":0,"expectedRevision":42}' --pretty
```

Candidates are ordered by brush count, then remaining internal face area. Coplanar
prisms must share an extrusion direction and depth, may use triangular or larger convex
footprints, and are merged only when their exact union remains convex. Visible material
and UV seams are preserved.

Batch preview returns each discovered cohort and its revision-scoped paths. Batch apply
recomputes the cohorts at `expectedRevision` and uses the first (lowest brush count,
lowest seam area) candidate for each:

```sh
tbctl --method brushes.optimize.batch.preview --params \
  '{"documentId":"document-...","paths":[[0,12],[0,13],[0,20],[0,21]]}' --pretty
tbctl --method brushes.optimize.batch.apply --params \
  '{"documentId":"document-...","paths":[[0,12],[0,13],[0,20],[0,21]],"expectedRevision":42}' --pretty
```

## Geometry operations

### `geometry.extractFootprints`

`geometry.extractFootprints` removes the need to fetch every face through
`nodes.describe` and reconstruct footprints in a client script. It is read-only (so it
does not need `expectedRevision`) and requires explicit brush `paths`, an `axis`, and a
`coordinate`. An optional exact `material` filter distinguishes coincident surfaces,
such as a river's visible `t50_agua1` top, underwater floor, and `eq_water` volume.

Every returned polygon is convex and has deterministic winding around the positive
selector axis and a deterministic first vertex. Identical polygons are coalesced, but
their `sources` retain every `{path, faceIndex}` that produced them. The response
includes per-polygon and aggregate bounds, `sourceFaceCount`, `duplicateFaceCount`, and
`duplicatePathCount`; repeated input paths are accepted and counted rather than turning
an otherwise useful query into an error. The document `revision` scopes the paths just
like `nodes.query`.

```sh
# Read the visible river-surface polygons at Z=128.
tbctl --method geometry.extractFootprints --params \
  '{"documentId":"document-...","paths":[[0,12],[0,13]],"axis":"z","coordinate":128,"material":"unrest/t50_agua1"}' --pretty
```

Geometry mutations require `expectedRevision` and return the resulting selected brush
count and new revision. `paths` is an array of distinct brush node paths. Material
parameters are names exactly as accepted by the current game configuration.

`geometry.createBrushes` is the low-level construction primitive for authored wall
runs, towers, and terrain pieces. Each brush has an explicit `material` and the point
cloud of one convex polyhedron. All brush specifications are validated before the map is
changed; one invalid brush leaves the document untouched. `parentPath` is optional and
defaults to the current layer. The result includes each newly-created path and bounds.

```sh
tbctl --method geometry.createBrushes --params \
  '{"documentId":"document-...","parentPath":[0,3],"brushes":[{"material":"unrest/wall","points":[[0,0,0],[64,0,0],[64,16,0],[0,16,0],[0,0,128],[64,0,128],[64,16,128],[0,16,128]]}],"expectedRevision":42}' --pretty
```

`geometry.sweepPath` is the higher-level construction primitive for any upright,
rectangular-profile path: a wall, curb, beam, hedge, fence, trim strip, or paved run.
It accepts either `paths` (each an ordered polyline) or `segments` (each an independent
two-point run). Every `[x,y]` point is snapped to `gridSize` (default `1`). Supply the
profile either as a `profile` object or flat parameters: `width`, `bottom`, and exactly
one of `top` or `height`. Collinear points are coalesced. Ordinary corners are mitered
into adjacent convex brushes with shared faces; reversing and overly acute corners are
rejected rather than producing a spike or unintended fill. Preview returns the exact
brush point clouds and bounds without changing the map. Apply requires `expectedRevision`
and creates all brushes below `parentPath` (or the current layer) in one undoable
transaction. The current safe policies are `join:"miter"` and `cap:"butt"` (both the
defaults); naming them explicitly leaves the request shape ready for future bevel and
extended-cap modes without pretending to support them today.

```sh
# First inspect the exact two generated brushes. The 64-unit grid snapping is useful
# when reconstructing large structural runs from reference-map measurements.
tbctl --method geometry.sweepPath.preview --params \
  '{"documentId":"document-...","paths":[[[0,0],[256,0],[256,192]]],"gridSize":64,"profile":{"width":32,"bottom":0,"height":192},"material":"unrest/wall"}' --pretty

# Create that same path only if the document is still at the previewed revision.
tbctl --method geometry.sweepPath.apply --params \
  '{"documentId":"document-...","paths":[[[0,0],[256,0],[256,192]]],"gridSize":64,"profile":{"width":32,"bottom":0,"height":192},"material":"unrest/wall","parentPath":[0,3],"expectedRevision":42}' --pretty
```

`geometry.planarProfile` fills closed, simple XY contours with one or more mitered
inset bands and, optionally, the remaining core. It is intended for construction that
has a meaningful footprint—basins, curbs, plinths, terraces, roof borders, paths, and
paving—rather than a centreline. Points, inset distances, and vertical extents are
snapped to `gridSize` (default `1`). The optional `axis` is currently constrained to
`"z"`; that explicit constraint keeps the generated brushes predictable while other
planar orientations are not yet implemented.

Each band supplies an absolute `inset` from the source contour and its `bottom`/`top`.
Insets must increase strictly. `material` may be supplied globally or per band/core,
and `role` may be supplied per part (defaulting to `band-N` and `core`). Roles are
returned for every generated brush, along with its exact points and bounds, so later
face-specific material rules can be applied unambiguously. Concave cores are
deterministically ear-triangulated; rings use one convex mitered prism per contour edge.
Self-intersections, collapsed/inverted offsets, degenerate contours, and invalid
vertical spans are rejected during preview and leave the document unchanged during
apply. Apply is revision-guarded and creates all generated brushes atomically below the
optional `parentPath`.

```sh
# Preview a two-level curb and a lower paved core. The per-part materials deliberately
# make the API useful for architectural profiles as well as simple single-material paths.
tbctl --method geometry.planarProfile.preview --params \
  '{"documentId":"document-...","contour":[[0,0],[256,0],[256,192],[0,192]],"gridSize":16,"bands":[{"inset":16,"bottom":0,"top":32,"material":"unrest/trim","role":"curb"},{"inset":48,"bottom":-16,"top":0,"material":"unrest/stone","role":"ledge"}],"core":{"bottom":-32,"top":-16,"material":"unrest/paving","role":"floor"}}' --pretty

# Apply exactly that preview only if its document revision still matches.
tbctl --method geometry.planarProfile.apply --params \
  '{"documentId":"document-...","contour":[[0,0],[256,0],[256,192],[0,192]],"gridSize":16,"bands":[{"inset":16,"bottom":0,"top":32,"material":"unrest/trim","role":"curb"},{"inset":48,"bottom":-16,"top":0,"material":"unrest/stone","role":"ledge"}],"core":{"bottom":-32,"top":-16,"material":"unrest/paving","role":"floor"},"parentPath":[0,3],"expectedRevision":42}' --pretty
```

`geometry.extrudeProfile` is the corresponding primitive for a single closed outline
on any principal plane. It is useful for gables, pediments, arches, roof ends, custom
wall sections, and other shapes whose useful outline is vertical rather than an XY
footprint. `plane` selects the two coordinates used by `profile`: XY extrudes along Z,
XZ along Y, and YZ along X. `interval:[minimum,maximum]` supplies that remaining-axis
extent. The profile, interval, and generated points are snapped to `gridSize` (default
`1`). Convex profiles make one brush; simple concave profiles are deterministically
ear-triangulated, so every created brush remains valid convex map geometry. Preview
returns the exact point clouds and does not mutate the map. Apply creates all brushes
in one revision-guarded transaction below optional `parentPath`; `role` defaults to
`"profile"` and is returned for every created brush.

```sh
# Preview a symmetric triangular XZ gable, extruded 64 units through Y.
tbctl --method geometry.extrudeProfile.preview --params \
  '{"documentId":"document-...","plane":"xz","profile":[[0,0],[256,0],[128,160]],"interval":[-32,32],"gridSize":16,"material":"unrest/stone","role":"gable"}' --pretty

# Apply that same profile only if nothing changed after the preview.
tbctl --method geometry.extrudeProfile.apply --params \
  '{"documentId":"document-...","plane":"xz","profile":[[0,0],[256,0],[128,160]],"interval":[-32,32],"gridSize":16,"material":"unrest/stone","role":"gable","parentPath":[0,3],"expectedRevision":42}' --pretty
```

```sh
# Build volumes from the positive Z side of selected brushes to Z=256.
tbctl --method geometry.volumeToPlane --params \
  '{"documentId":"document-...","paths":[[0,12]],"axis":"z","coordinate":256,"material":"unrest/stone","expectedRevision":42}' --pretty

# Create EQ water and its visible surface from selected riverbed brushes.
tbctl --method geometry.eqWater --params \
  '{"documentId":"document-...","paths":[[0,12],[0,13]],"surfaceHeight":128,"surfaceThickness":8,"waterMaterial":"unrest/eq_water","surfaceMaterial":"unrest/t50_agua1","expectedRevision":42}' --pretty

# `paths` supplies the CSG selection. For `subtract`, those brushes are subtrahends;
# without `targetPaths`, every visible brush they touch is also cut, as in the editor UI.
# Supply non-empty, distinct and disjoint `targetPaths` to cut exactly those brushes
# instead. The response then reports `targetBrushCount`, `replacementBrushCount`, and
# `replacementBrushes` with revision-scoped paths. `material` is optional and is used for
# any faces that cannot inherit attributes from the source geometry.
tbctl --method geometry.csg --params \
  '{"documentId":"document-...","operation":"subtract","paths":[[0,12]],"material":"unrest/stone","expectedRevision":42}' --pretty

# Cut a pool cavity from just the ground, leaving touching walls intact.
tbctl --method geometry.csg --params \
  '{"documentId":"document-...","operation":"subtract","paths":[[0,12]],"targetPaths":[[0,3]],"expectedRevision":42}' --pretty
```

`geometry.volumeToPlane` accepts either `paths`, or `faces`, whose entries are
`{"path":[...],"faceIndex":N}`. The latter preserves the face-specific behavior of
the editor command. `geometry.bridgeEdgeChains` takes an `edges` array. Each edge is
`{"path":[...],"start":[x,y,z],"end":[x,y,z]}` and must exactly identify a brush edge;
the array must form the same two connected, coplanar, open chains required by the UI.
It also requires a positive `thickness`, a `direction` of `below`, `above`, or
`centered`, and a `material`.

## Copied-map workspaces

`workspace.fork` snapshots the live map—including unsaved edits—to recoverable
`base.map` and `branch.map` files below `automation/workspaces/<workspaceId>/`, then
opens `branch.map` in a separate ordinary TrenchBroom window. The branch therefore has
its own selection, camera, undo stack, rendering, and on-disk recovery while the source
document remains untouched and active. Automation camera operations do not focus or
activate the branch window.

```sh
tbctl --method workspace.fork \
  --params '{"documentId":"document-...","name":"water experiment"}' --pretty
tbctl --method workspace.diff \
  --params '{"workspaceId":"..."}' --pretty
tbctl --method workspace.merge \
  --params '{"workspaceId":"..."}' --pretty
```

The workspace records stable in-memory identities for every node present at fork time.
Diff reports added/removed subtree roots, content changes, and reparenting. When a set
of sibling brushes is replaced by an exact lower-count result of the brush optimizer,
diff reports one `brushes_optimized` change with its before/after brush counts instead
of one removal and addition per brush. This recognition compares the complete generated
brushes, including their face attributes; it is not a loose bounds-only heuristic.
Merge still plans the individual safe add/remove operations underneath that summary and
first compares the base, branch, and current live trees. Concurrent live content edits,
moves, deletions, or missing destination parents are returned as conflicts without
changing the source. A conflict-free merge is applied as one outer map transaction and
is therefore one undoable source-document operation. A workspace can be merged only
once; fork again for another iteration.

Opening `/Users/atroche/norrath-maps/maps/unrest.map` as a reference and forking the
active document is enough to support the water workflow: query reference nodes and
materials, capture/render it from independent cameras, generate or copy native map
fragments into the branch, visually iterate, inspect the diff, then merge explicitly.

## Deliberate boundaries

This is a local editor-control API, not an in-process ABI for loading third-party C++.
Keeping clients out of process isolates crashes and makes any language capable of
speaking JSON-RPC usable. The semantic API should grow in task-sized operations; native
map text remains the lossless escape hatch for geometry batches. Branch files are kept
after merge for recovery and auditing.
