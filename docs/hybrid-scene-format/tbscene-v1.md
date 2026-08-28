# TrenchBroom Hybrid Scene (`tbscene`) v1

This document is normative unless it uses the words *example*, *note*, or
*non-normative*. RFC 2119 keywords have their usual meanings.

## 1. Package and top-level manifest

A `.tbscene` file is a ZIP archive using only the `store` compression method. It MUST
contain exactly one `manifest.json` at archive root. Entry names MUST be UTF-8,
relative, use `/`, contain no `.` or `..` segment, and occur once. Entries MUST be
ordered by UTF-8 bytewise lexicographic path. ZIP timestamps, extra fields, comments,
encryption, and data descriptors are forbidden; all timestamps are zero. This makes a
package reproducible from identical logical content.

`manifest.json` is UTF-8 JSON conforming to
[`schema/tbscene-v1.schema.json`](schema/tbscene-v1.schema.json). It begins with:

```json
{
  "format": "org.trenchbroom.tbscene",
  "version": 1,
  "documentId": "urn:uuid:...",
  "coordinateSystem": { "handedness": "right", "up": "z", "unit": "metre", "metresPerUnit": 1 },
  "entries": [],
  "integrity": { "algorithm": "sha256", "entries": {} }
}
```

Every numeric geometric value is IEEE-754 binary64 and MUST be finite. Coordinates are
expressed in the declared document unit. `metresPerUnit` converts a document-space
distance to metres. A game profile may supply an import transform, but coordinates in
the package are never silently rescaled. Transform matrices are affine, column-major
4x4 matrices acting on column vectors. v1 uses right-handed coordinates and `z` up.

An importer records its source coordinate system and the exact source-to-document
affine transform in provenance. For example, a game profile whose legacy map unit is
one thirty-second of an in-game unit records `mapUnitsPerGameUnit: 32` in its
profile-owned provenance extension; it does not alter the common coordinate contract.
This keeps the document generic while making unit conversion auditable.

All IDs are immutable strings matching either `urn:uuid:` plus a UUID or a
`sha256:`-prefixed lowercase hexadecimal digest. IDs are unique across the manifest.

## 2. Canonical serialization and integrity

Canonical JSON uses NFC-normalized UTF-8 strings, object keys sorted by UTF-8 byte
sequence, no insignificant whitespace, booleans/null in lowercase, and the shortest
round-trippable ECMAScript/IEEE-754 decimal spelling for finite numbers. Negative zero
MUST be serialized as `0`. Newlines are LF. Arrays retain declared order.

Each payload entry has a byte length and `sha256:<hex>` hash over its uncompressed
bytes. `integrity.entries` repeats every archive payload hash except `manifest.json`.
The document hash is SHA-256 over canonical `manifest.json` after replacing
`integrity.document` with the empty string; the result is then stored as
`integrity.document`. A writer MUST reject an archive whose length or hash disagrees.

Entries and records may include `extensions`, an object keyed by reverse-DNS owner ID
(for example `org.norrath.exporter`). An extension value is canonical JSON. A reader
MUST preserve unknown extensions byte-for-byte at the canonical JSON value level when
rewriting. `requiredExtensions` identifies extensions a reader MUST understand in
order to edit, as an owner-ID to minimum-version string map; a reader may inspect
read-only without understanding them.

## 3. Indexed geometry

`entries` indexes independently loadable records. Each entry has `{id, kind, path,
offset, length, sha256}`. `path` identifies an archive payload and `offset`/`length`
select a byte range in it. Ranges MUST not overlap unless they have the same ID and
identical range. Record kinds in v1 are `mesh`, `solid`, `entity`, `component`,
`asset`, `provenance`, `referenceContext`, and `acceptance`.

### Triangle meshes

A mesh record points to a `TBM1` binary chunk. The chunk has little-endian fixed-size
header fields for magic/version, vertex count, index count, attribute mask, and stream
offsets. It contains indexed triangles (`uint32` triples) and a vertex stream with at
least position `float64[3]`, normal `float32[3]`, UV `float64[2]`, and material index
`uint32`. Optional vertex streams are color `unorm8[4]`, tangent `float32[4]`, and
surface-attribute index `uint32`. A triangle MUST not use an index outside the vertex
count. Material and surface-attribute tables are indexed records, not implicit names.

Every material record includes a stable ID, an exact material token, optional external
asset reference, and render properties. Surface attributes include contents, flags,
value, collision class, and arbitrary extension-owned data. Per-vertex UVs are first
class; no reader may replace them with a fitted planar mapping on save.

### Convex brush solids

A `solid` record represents one convex closed solid. It contains an ID, transform,
and an ordered array of outward planes. Each plane supplies a finite normal, distance,
material ID, surface-attribute ID, and a world-to-texture affine mapping. The
intersection MUST be bounded and non-empty. Plane order is semantically significant
only for canonical serialization (lexicographic plane tuple order); it is not a brush
identity. Solids are the editable CSG representation, not a lossy mesh fallback.

## 4. Nodes, entities, transforms, and semantic components

The scene graph is rooted by one `scene` record with ordered child IDs. Nodes have
stable IDs, optional parent ID, transform, visibility/editor-lock state, tags, and
extensions. An entity is a node with a `class` and a string property map. Components
are separately identified objects attached to nodes, allowing runtime semantics to be
visible without requiring a game engine to parse editor-only metadata.

v1 defines these component shapes:

- `door`: `closedTransform`, `openTransform`, optional `halfOpenTransform`, collision
  policy, and motion parameters. The three named poses allow editor preview without a
  running game.
- `light`: color, intensity, range, cone, static/dynamic state, and optional day/night
  schedule. Game profiles may map this to a native light entity.
- `waterVolume`: solid or mesh boundary IDs, surface material, fluid type, and optional
  flow vector. Surface, volume, and bottom are distinct references.
- `animatedProp`: asset ID, animation set/state, optional default clip/time, and an
  optional runtime binding. Static geometry is not inferred from an animation name.

Other semantics belong in extensions until standardized. Components never change a
node's transform implicitly: all poses and bindings state their space explicitly.

A component record is canonical JSON with `id`, `type`, `nodeId`, `space`, `data`,
and optional `extensions`. The fields in the preceding list belong in `data`. A game
profile can map standardized components to native entities or author richer components
under its own extension ID. Common profile-owned components include spawn points,
safepoints, climb volumes, zonelines, and regions; the common format deliberately does
not hard-code them in v1.

## 5. Assets, provenance, and acceptance metadata

Asset records use a logical `uri`, MIME type, byte hash, and optional package entry.
Relative URIs resolve against the package root; external URIs require an explicit
scheme. A missing external asset is a diagnostic, not permission to substitute a
different asset silently.

Every geometric or semantic record MAY declare `provenance` references. A provenance
record stores stable source IDs, source locator/version, import transform, operation
lineage (`preserved`, `triangulated`, `decomposed`, `repaired`, etc.), and diagnostic
references. Provenance is many-to-many: duplicate output geometry MUST remain
ambiguous rather than gaining order-derived IDs.

`referenceContext` records explicitly name two or more documents and their roles
(`reference`, `candidate`, `observation`). They are evidence roles, not truth claims.
`acceptance` records store named virtual cameras, geometry/visual measurements,
assertions, divergence policies, and immutable evidence references. They MUST carry
the document hashes and asset revision used to obtain a result.

## 6. Compatibility and migration

### Importing Valve `.map`

An importer MUST create a `.tbscene` document whose brush solids retain the exact
loaded geometry, material token, surface attributes, and loaded world-to-texture
mapping. Entity key/values become entity properties. The importer MUST record source
path/hash, source map format, and any precision conversion in provenance. It MUST NOT
invent mesh topology or semantic components merely because an entity name resembles a
door or light; profile-owned import rules may add an explicit extension-owned binding.

### Exporting to Valve `.map`

Export is a named compatibility projection. Convex solids and representable entities
may export directly. Meshes, transforms, components, extensions, and high-precision
UVs that lack a target encoding MUST be reported as preserved sidecar data, baked
output, omitted data, or a hard error according to an explicit export policy. An
exporter MUST never claim a lossless round trip when its report contains any omitted
or approximated record.

### Existing exporters

An exporter may emit `.tbscene` directly and MAY additionally emit a `.map` projection.
It SHOULD preserve source-level mesh triangles and per-vertex UVs in `TBM1`, stable
source IDs/provenance in records, and runtime data as semantic components or
game-owned extensions. The old `.eqexport.json` sidecar maps naturally into v1
provenance, but its fingerprint definition remains independently versioned until a
formal bridge is tested.

A game/runtime adapter consumes the `SceneDocument` representation rather than parsing
Valve text. It may compile mesh/material/entity outputs that already exist in that
runtime, retaining an explicit source-document hash and record IDs for diagnostics.

## 7. v1 non-goals

v1 does not specify collaborative operations/CRDTs, material image embedding,
procedural geometry, skeletal animation data, navigation meshes, light baking,
runtime compilation, compression, encryption, a public plugin ABI, or a general
replacement for every legacy map format. It also does not require a game engine to
consume `.tbscene` directly; that is a client integration decision.

## 8. Validation and implementation gates

An implementation MUST validate manifest schema, canonical JSON, IDs, package entry
rules, hashes, mesh range/index bounds, finite values, solid boundedness, reference
integrity, and unknown-required-extension policy before it saves a modified document.

The fixture provides a minimal manifest covering one mesh, one convex solid, one
entity, the four standardized components, external assets, provenance, and reference
acceptance metadata. Future tests MUST include:

1. schema validation of the fixture;
2. package write/read/write byte identity;
3. payload-hash and range corruption rejection;
4. unknown optional-extension preservation and unknown required-extension refusal;
5. `.map` import/export loss reports; and
6. mesh/solid/render/acceptance round trips with stable IDs.

Production work packages are intentionally: core model/ID support; package codec;
TrenchBroom editor bridge; semantic previews; acceptance integration; and game-specific
exporter/runtime adapters. They can be developed in parallel once the codec's public
interfaces are pinned.
