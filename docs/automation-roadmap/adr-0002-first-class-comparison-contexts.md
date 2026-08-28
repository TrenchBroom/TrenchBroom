# ADR 0002: First-class reference/candidate comparison contexts

- **Status:** Accepted
- **Date:** 2026-08-28
- **Owners:** TrenchBroom automation and acceptance infrastructure

## Context

The acceptance model currently binds a reference document, a target document, and an
alignment independently on every saved comparison. Workspaces separately distinguish a
source map from editing branches, while low-level automation correctly requires an
explicit document ID. None of these concepts says, once and durably, that one document
is the read-only oracle and another is the candidate being changed.

Repeating this relationship has practical costs:

- comparisons for the same reconstruction can silently disagree about document paths or
  alignment;
- higher-level occupancy, traversal, and camera-coverage tools have no shared context;
- agents must repeatedly provide and verify the same pair of documents;
- active-window state is tempting as an accidental fallback when a request omits one
  side; and
- changing the candidate path or alignment requires editing every comparison.

The relationship is useful beyond map reconstruction. It also describes import cleanup,
regression testing, alternate implementations, and deliberate redesigns measured against
a baseline.

## Decision

Acceptance projects will contain durable, explicitly identified **comparison contexts**.
A context owns:

- a portable reference document path;
- a portable candidate document path; and
- the coordinate alignment from reference space to candidate space.

The roles are semantic and asymmetric. Context-aware mutation may target the candidate
but must reject the reference. This does not weaken low-level RPC rules: low-level
operations continue to require explicit document IDs and never infer a target from GUI
focus.

A saved comparison may either:

1. retain its legacy self-contained reference/target paths and alignment; or
2. bind to a comparison context and store only its reference and candidate view IDs plus
   comparison-specific masks, metrics, and assertions.

For context-bound comparisons, document paths and alignment are resolved from the
context at load/use time. They are not duplicated as independently editable values.
Updating a context therefore updates all of its comparisons atomically. Deleting a
context that still has comparisons is rejected by whole-project validation.

The persisted acceptance schema advances to version 2. Version 1 projects remain
readable and are upgraded in memory; they preserve their legacy standalone comparisons.
New writes use the canonical version 2 form.

The initial RPC surface follows the existing optimistic CRUD convention:

- `acceptance.contexts.list`
- `acceptance.contexts.create`
- `acceptance.contexts.update`
- `acceptance.contexts.delete`

Each successful mutation requires `expectedRevision`, validates the complete project,
increments the store revision, and writes atomically. Context IDs are durable portable
IDs, not process-lifetime document IDs.

## Invariants

- Reference and candidate paths are non-empty, portable, project-relative paths.
- Reference and candidate resolve to different normalized paths.
- Context IDs are unique within an acceptance project.
- A context-bound comparison references an existing context.
- Context-bound comparison endpoints contain valid named-view IDs.
- No context operation falls back to the active document, active window, or active view.
- Alignment has the same validation and reference-to-candidate direction everywhere.
- Captures and reports continue to echo the concrete document identities and revisions
  actually used.

## Consequences

Solid-space comparison, player connectivity, and automatic matched-camera coverage can
all accept a single `contextId` and share the same role and alignment semantics. Named
comparisons become smaller and cannot drift away from their project-level document pair.

The acceptance parser and serializer become responsible for version-1 migration and for
resolving context-bound comparison endpoints. This is preferable to spreading context
lookup through renderers, geometry queries, and individual RPC handlers.

The first implementation does not introduce an implicit current context, automatically
open documents, or redirect existing geometry mutation RPCs. Those are separate product
decisions. A future context-aware command layer may resolve a context to explicit live
document identities, but it must retain the invariants above.

## Rejected alternatives

### A process-global active pair

Rejected because it recreates the focus/identity failures that explicit document and
view IDs were designed to remove, and it cannot safely support concurrent workspaces.

### Continue repeating paths on every comparison

Rejected because it provides no reliable shared input for structural and traversal
validation and permits configuration drift.

### Store process-lifetime document IDs

Rejected because acceptance projects are durable and portable, while live document IDs
expire at process exit. Runtime resolution belongs at the automation boundary.

### Put the pair only in workspace manifests

Rejected because acceptance comparisons also operate on ordinary files and because a
workspace's source/branch lifecycle is not identical to reference/candidate semantics.
The two systems may be linked later without conflating their responsibilities.
