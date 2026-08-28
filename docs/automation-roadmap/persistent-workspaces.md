# Persistent, recoverable workspaces

**Status:** Implemented on 2026-08-27. The work packages below remain as the design and
ownership record for future extensions.

## Goal

A copied-map workspace remains a coherent base/source/branch relationship across
window closure, TrenchBroom restart, and source-document changes. It can be inspected,
opened, diffed, merged, or abandoned without depending on live `MapWindow` pointers.

This is not session restoration. A workspace is durable project state with an explicit
lifecycle.

## Prior limitation

`AutomationWorkspaceManager` created `base.map` and `branch.map`, but its workspace ID,
source-window association, merge model, and merged flag are held in memory. A restart
leaves useful map files but loses the object that knows how to diff and merge them.

The old manager also made an open branch window part of workspace identity. The
implemented durable record instead owns paths and metadata, with a hidden live window
attached only while a session is recovered.

## Persisted layout and schema

Each workspace directory contains:

```text
automation/workspaces/<workspace-id>/
  workspace.json
  base.map
  snapshots/<generation>/
    branch.map
    workspace.json
```

Version 1 manifest:

```json
{
  "schemaVersion": 1,
  "workspaceId": "7d9472dd-137d-48ce-8781-4503e090abf1",
  "name": "D06 north vault",
  "createdAt": "2026-08-27T19:42:00Z",
  "state": "active",
  "checkpointGeneration": 12,
  "source": {
    "path": "/absolute/path/unrest_rebuilt.map",
    "fingerprintAtFork": "sha256:...",
    "revisionAtFork": 50
  },
  "mapMetadata": {
    "gameName": "Quake",
    "mapFormat": "Valve",
    "worldBounds": {
      "min": [-8192, -8192, -8192],
      "max": [8192, 8192, 8192]
    }
  },
  "base": {"path": "base.map", "fingerprint": "sha256:..."},
  "branch": {
    "path": "snapshots/12/branch.map",
    "fingerprint": "sha256:..."
  },
  "nodeIdentities": [
    {
      "id": 1,
      "type": "group",
      "basePath": [0, 4],
      "baseParentId": 0,
      "branchPath": [0, 5]
    }
  ]
}
```

Rules:

- Relative base/branch paths resolve beneath the manifest directory and may not escape
  it.
- The source path is absolute in version 1. Recovery may update it atomically.
- `mapMetadata` records the game configuration, map format, and world bounds used to
  load hidden base/branch documents. It is optional only for old version 1 manifests.
- `fingerprintAtFork` is a content fingerprint, not the in-process map revision.
- `nodeIdentities` records fork-time node identity independently of process pointers.
  Removed branch nodes have a null `branchPath`; branch additions do not need a
  fork-time ID.
- Runtime states such as attached windows are not serialized.
- Unknown fields are preserved when rewriting a known schema version where practical.
- A manifest update is written to a temporary sibling, flushed, and atomically renamed.

Publishing a checkpoint means fully writing the next generation's branch map and
manifest, hashing them, then atomically advancing the root manifest. A torn newest
generation is ignored in favor of the last complete, hash-valid generation.

Durable lifecycle states are `active`, `merged`, and `abandoned`. Runtime status is
computed separately: `dormant`, `attached`, `sourceChanged`, `orphaned`, or `invalid`.

## Model and ownership changes

Split the current concept into three layers:

1. `AutomationWorkspaceManifest`: serializable durable value object.
2. `AutomationWorkspaceRecord`: manifest plus validated filesystem/runtime status.
3. `AutomationWorkspaceSession`: optional loaded source/branch documents and
   `mdl::MapWorkspace` merge model.

`AutomationWorkspaceManager` owns records. Sessions may be attached or released
without deleting records. `QPointer<MapWindow>` belongs only in a session.

The merge model must use `base.map` as the common ancestor:

```text
base at fork
 +-- current source
 +-- current branch
```

Stable link IDs or equivalent persisted node identities must be validated across the
three maps. The existing `mdl::MapWorkspace` pointer-based records cannot be
reconstructed from a directory scan alone. It needs an exportable/importable identity
table mapping durable workspace node IDs to immutable base paths and current branch
paths.

Recovery is deliberately conservative. A restart discovers a detached branch, but
does not automatically bind a source map merely because its path matches. An explicit
source attachment validates the live source against the fork-time identity table.
Opening or attaching must fail diagnostically if identity reconstruction is unsafe; it
must not silently guess or downgrade to a two-way merge.

New manifests persist the source game name, map format, and world bounds, allowing a
hidden branch to recover after restart from its `workspaceId` alone. Metadata-free
version 1 manifests remain readable; recovering one requires an explicit open-document
context until it is replaced by a newly forked workspace.

## RPC contract

### `workspace.list`

Lists dormant and attached records. It does not open windows.

```json
{
  "workspaceId": "...",
  "name": "D06 north vault",
  "state": "active",
  "runtimeStatus": "dormant",
  "sourcePath": "/.../unrest_rebuilt.map",
  "sourceChanged": true,
  "mergeable": true
}
```

### `workspace.recover`

Loads the latest valid branch checkpoint and reconstructs its base/branch model without
binding a live source.

```json
{
  "workspaceId": "..."
}
```

It returns a process-lifetime branch document ID when loaded. Recovery is always hidden
and non-activating. Current manifests need only `workspaceId`; an older metadata-free
version 1 manifest must additionally provide `documentId` as its loading context.

### `workspace.attachSource`

Explicitly attaches one open source document after fork-identity validation:

```json
{
  "workspaceId": "...",
  "documentId": "document-..."
}
```

It reports structural attachment diagnostics and performs no merge.

### Additional methods

- `workspace.status`: validate files and report diagnostics without opening maps.
- `workspace.open`: convenience composition of recover plus explicit source attachment;
  it still requires `sourceDocumentId` and never searches active windows.
- `workspace.close`: release the session but retain durable files and record.
- `workspace.relocateSource`: atomically replace advisory source-path metadata after
  validation; this does not attach or merge.
- `workspace.checkpoint`: force publication of a new branch generation.
- `workspace.rename`: change only user-facing metadata.
- `workspace.diff`: compute a three-way plan without applying it.
- `workspace.merge`: apply a conflict-free plan once, then persist `merged`.
- `workspace.abandon`: persist `abandoned` but retain files.
- `workspace.discard`: destructive removal requiring an explicit confirmation token or
  equivalent two-step API.

All methods identify workspaces by durable workspace ID. No method falls back to the
active window.

## Work packages

### PW0 - Freeze manifest and lifecycle contracts

- **Dependencies:** none.
- **Primary ownership:** new
  `AutomationWorkspaceManifest.h/.cpp` and schema-focused tests.
- **Integration touchpoints:** none after the contract commit.
- **Deliverable:** version 1 parse/serialize/validate API and enumerated lifecycle/status
  types.
- **Test gate:** round trip, unknown version, missing fields, path traversal, malformed
  fingerprints, deterministic output.

### PW1 - Atomic workspace store and startup discovery

- **Dependencies:** PW0.
- **Primary ownership:** new `AutomationWorkspaceStore.h/.cpp` and
  `tst_AutomationWorkspaceStore.cpp`.
- **Deliverable:** create/read/update/scan operations with atomic manifest writes and
  precise per-record diagnostics.
- **Test gate:** interrupted temporary file, corrupt manifest beside valid records,
  missing base/branch, scan ordering, no mutation during read.

### PW2 - Three-way model reconstruction

- **Dependencies:** PW0 contract; independent of PW1 implementation.
- **Primary ownership:** `mdl::MapWorkspace` extensions and
  `tst_MapWorkspace.cpp` sections dedicated to base/source/branch reconstruction.
- **Deliverable:** export/import the fork identity table, reconstruct base/branch records
  after serialization, and explicitly bind a structurally validated live source.
- **Test gate:** branch-only edit, source-only edit, compatible independent edits,
  divergent edit, delete-versus-edit, reparenting, added descendants, duplicate IDs,
  stale branch paths, unsafe source topology, repeated merge.

### PW3 - Manager record/session split

- **Dependencies:** PW1 and PW2.
- **Primary ownership:** `AutomationWorkspaceManager.h/.cpp` plus focused manager tests.
- **Deliverable:** dormant records discovered at startup; publish branch checkpoints on
  completed transactions; attach/detach sessions without losing workspace identity.
- **Test gate:** fork/close/reopen, source already open, branch already open, windows
  closed in either order, torn newest generation, application shutdown with dormant and
  attached records.

### PW4 - RPC lifecycle surface

- **Dependencies:** PW3.
- **Primary ownership:** `AutomationService_Workspaces.cpp` and workspace RPC tests.
- **Integration touchpoints:** central method registration and `Automation.md`, owned by
  the milestone integration agent.
- **Deliverable:** the RPC contract above with structured error data.
- **Test gate:** every method through a local socket; `showWindow: false` does not change
  focus; unknown IDs and invalid states do not mutate files.

### PW5 - External restart verification

- **Dependencies:** PW4.
- **Primary ownership:** a new automation integration test or script isolated from unit
  test sources.
- **Deliverable:** fork, edit, save, terminate, restart, discover, reopen, diff, merge.
- **Test gate:** final live map equals the expected merge and the workspace is marked
  merged.

### PW6 - Optional workspace UI

- **Dependencies:** PW4 and a stable lifecycle proven by PW5.
- **Primary ownership:** new workspace model/panel classes and their tests.
- **Deliverable:** list dormant/open/problem workspaces, open without surprise focus,
  show diff/conflicts, merge, abandon, and reveal files.
- **Integration touchpoints:** menu/action registration, owned by the UI integration
  agent.

## Parallel execution plan

After PW0 lands, PW1 and PW2 can run concurrently. PW3 is the convergence point. Once
PW3 lands, PW4 and preparatory PW5 fixture work can overlap. PW6 should remain separate
until the headless lifecycle is stable.

The integration agent exclusively owns CMake list edits shared by these packages and
the final public API documentation update.

## Completion criteria

- A workspace created before restart appears in `workspace.list` after restart and
  recovers its last complete branch checkpoint.
- Diff and merge use the saved base even when the source changed after the fork.
- Post-restart merge remains disabled until an explicit source attachment validates
  fork identities.
- Opening or inspecting a workspace can be completely windowless.
- Closing a branch window does not abandon or delete the workspace.
- Corruption or source movement produces recoverable, structured diagnostics.
- The same workspace cannot be applied twice.
