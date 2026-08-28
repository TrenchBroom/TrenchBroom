# ADR 0001: Use a deterministic `.tbscene` package for hybrid scene documents

- **Status:** Accepted
- **Date:** 2026-08-28
- **Owners:** TrenchBroom document, automation, and game-integration infrastructure

## Context

Valve-style `.map` is an excellent interchange format for entity key/value data and
convex brush solids. It is not a suitable source-of-truth format for a modern hybrid
scene. In particular, it cannot directly represent indexed triangle meshes with
per-vertex attributes, source lineage, explicit runtime components, animation state,
large auxiliary data, or round-trip-safe extensions. Its textual number formatting
can also make texture reconstruction needlessly lossy.

The format must remain practical for TrenchBroom editing and review. It must support
large zone meshes without allocating a huge JSON DOM, maintain stable identities, and
allow an exporter, editor, acceptance tooling, and game runtime to agree on exactly
what was read. It must not bake EverQuest/Norrath assumptions into the common model.

## Decision

The canonical document is a **`.tbscene` package**: a deterministic ZIP archive with
the ZIP `store` method (no compression in v1). The archive contains a UTF-8 canonical
JSON manifest at `manifest.json` and optional binary payloads below `meshes/`,
`blobs/`, and `attachments/`.

The format name is **TrenchBroom Hybrid Scene**, identifier `org.trenchbroom.tbscene`,
and file extension `.tbscene`. `manifest.json` is the normative logical document;
archive ordering, metadata, and payload integrity rules make its physical package
reproducible. Uncompressed payloads give ordinary ZIP readers random access and avoid
turning loading a single mesh into loading an entire zone.

`manifest.json` has an index of records and binary chunks. A reader may stream records
and seek payloads. It must not require that all mesh vertices or provenance records
live in one parsed JSON value. A development tool may expose an unpacked directory
with the same entry names, but a saved interchange document uses `.tbscene`.

`.map` remains supported as an import/export compatibility format. It is not a
serialization mode of `.tbscene` and cannot be expected to preserve all v1 data.

## Consequences

- Geometry may be represented as editable convex solids, indexed triangle meshes, or
  both, without pretending a mesh is decomposable into brushes.
- Exact per-vertex UVs and source provenance survive editor and exporter round trips.
- Semantic components (doors, lights, water, animated props) can be inspected and
  previewed without smuggling state through arbitrary entity properties.
- Large data can be indexed and integrity-checked before it is consumed.
- A `.tbscene` document requires a new reader/writer and cannot yet be handed to
  tools that only accept `.map`.

## Rejected alternatives

### Extend Valve `.map` in place

Rejected as the canonical format. Comments and entity properties can annotate a map,
but do not provide structured binary payloads, robust extension preservation, or a
way to distinguish authored semantics from exporter conventions.

### One monolithic JSON file

Rejected for production documents. It is convenient for examples but makes large
meshes/provenance memory-heavy and gives no natural binary representation for indexed
vertex data.

### An opaque game-specific binary

Rejected because TrenchBroom needs inspectable, cross-game semantics and a stable
handoff contract. Game-specific packed artifacts may be referenced as assets or
generated exports, but are not the editable source document.
