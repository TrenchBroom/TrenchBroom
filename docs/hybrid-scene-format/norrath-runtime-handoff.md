# Norrath `.tbscene` runtime handoff

This is the implementation brief for the Norrath-side agent who makes the
game consume TrenchBroom Hybrid Scene (`.tbscene`) documents. It deliberately
does not change Norrath code, and it does not define the common format; see
the [format specification](tbscene-v1.md) and
[ADR 0001](adr-0001-tbscene-package.md) for those contracts.

The useful migration boundary is already present: Norrath's browser renderer
and authoritative worker consume Lantern-style meshes, materials, collision,
regions, and authored extras. They do **not** read Valve `.map` directly. The
Node development server currently parses a `.map`, solves its brushes, then
serves the resulting Lantern-compatible asset bundle. A `.tbscene` reader
should produce that same intermediate build first. This gets first-class mesh
and semantic data into the game without a renderer or gameplay rewrite.

## Current pipeline and target pipeline

```
today:      .map -> mapFile.ts -> AuthoredZone{Source,Geometry,Build}
           -> dev asset routes -> Babylon renderer + worker collision

transition: .map -> legacy importer -> SceneDocument (.tbscene)
            .tbscene -> scene adapter -> AuthoredZone{Source,Geometry,Build}
           -> unchanged dev asset routes -> unchanged renderer + worker

later:      exporter -> .tbscene directly
            .tbscene -> cached/runtime package as an optional build product
```

`.map` remains a supported compatibility input and export projection. It is
not the canonical source for data which it cannot faithfully express: indexed
triangle meshes, per-corner UVs, source provenance, high-precision presentation
coordinates, or explicit runtime components. A migration must never quietly
drop those fields in order to pretend that a `.map` round trip is lossless.

## Scope and ownership

The future Norrath agent owns a reader/compiler and the dev-server integration
in an agent-owned Norrath worktree. The TrenchBroom agent owns the package
codec/editor bridge and the common specification. Coordinate on the public
`SceneDocument` contract before either side reinterprets it.

The Norrath agent must not edit the player's protected worktree at
`/Users/atroche/personal_code/norrath`. Start from an agent-owned worktree,
run `bun run agent:preflight` before code changes and `bun run agent:done`
before handoff, as that repository's own guidance requires.

## Existing Norrath seams to preserve

These are confirmed implementation seams, not speculative target names.

| Concern | Current module | Handoff action |
| --- | --- | --- |
| Valve map parsing, convex solving, map-derived entities and hot-reload diffs | `packages/shared/src/mapFile.ts` | Retain as the legacy `.map` importer/exporter. Move its consumers behind a format-neutral authored-scene interface; do not make `.tbscene` emulate brushes just to use it. |
| Development discovery, file watching, in-memory authored-zone cache, dev asset routes | `packages/client/dev.ts` | Discover both `.map` and `.tbscene`; select the parser by extension/manifest; preserve the existing `map:<name>` namespace until an explicit zone-ID migration. Watch package/entry changes atomically. |
| Portable runtime mesh/material/animation/light/object representation | `packages/shared/src/lantern.ts` | Make the `.tbscene` compiler emit this existing IR in the first slice. Keep Babylon out of the reader. |
| Browser acquisition and worker bootstrap | `packages/client/src/browserZoneSource.ts`, `packages/server/src/zoneBootstrap.ts` | Keep the all-or-nothing `AuthorityZoneBootstrap` boundary. The output still supplies regions, optional lift collision, authored extras, and collision positions/indices. |
| Rendering and asset loading | `packages/client/src/zoneBuilder.ts`, `packages/client/src/scene.ts`, `packages/client/src/eqMaterial.ts`, `packages/client/src/textureAnimator.ts`, `packages/client/src/worldLighting.ts`, `packages/client/src/zoneLight.ts` | Initially unchanged: feed them compiled Lantern meshes/material lists. Extend only after there is a measured representation gap. |
| Collision and movement | `packages/shared/src/collision.ts`, `packages/server/src/world.ts`, `packages/client/src/gameSession.ts` | Continue giving it world-space triangle positions and indices. Do not substitute render meshes for collision without an explicit collision class/policy. |
| Game semantic application | `packages/server/src/authoredExtras.ts`, `packages/server/src/content.ts`, `packages/client/src/doors.ts` | Compile components/extensions into the same explicit extras. Preserve door collision and motion semantics separately from their visible mesh. |
| Map-scale contract and legacy export validation | `tools/eqMapScale.ts`, `tools/zoneToMap.ts`, `tools/validateZoneMap.ts` | Preserve `EQ_MAP_SCALE = 32` for legacy-map conversion. Add `.tbscene` validation in parallel; do not weaken existing map validation. |
| Existing integration/regression tests | `packages/shared/src/mapFile.asset.ts`, `packages/client/src/authoredMapReload.test.ts`, `packages/client/src/browserZoneSource.test.ts`, `packages/server/src/zoning.asset.ts`, `e2e/first-playable-frame.pw.ts` | Extend these through the format-neutral adapter rather than duplicating renderer/gameplay test suites. |

`mapFile.ts` currently documents the important fact directly: it is an
engine-free Node-side map reader that turns brushes into the same shapes
`lantern.ts` parses from extracted files. Preserve that layering.

## Staged implementation plan

### 1. Define the Norrath profile extension and compiler input

Implement a pure, engine-free reader for the package manifest and indexed
payloads. Validate archive entry hashes/ranges, finite coordinates, unknown
required extensions, stable IDs, and the standard mesh/solid constraints before
creating a runtime object.

Add a Norrath-owned extension such as `org.norrath.scene` rather than adding
EQ fields to the game-neutral manifest. Version it independently and declare it
in `requiredExtensions` only when its meaning is necessary to play the zone.
The profile should carry:

- source/import transform, including the auditable `mapUnitsPerGameUnit: 32`
  relationship when the source came from EQ map units;
- EQ material token, shader/render mode, texture-frame list, animation rate,
  alpha and light-response policy;
- collision class for each mesh/solid/face (`solid`, `nonSolid`, trigger,
  water, climb, or profile-defined values);
- region, safespot, spawn, zoneline and climb-volume semantics;
- native door binding, its closed/half/open/open transforms and collision
  behavior; and
- source/provenance lineage using the `eqexport` fingerprint contract without
  treating a generated brush number as a stable ID.

The compiler must use the package's declared coordinate system and recorded
import transform. It must not rely on the current Lantern convention by
accident: Lantern Intermediate is passed through in its current axes, while a
`.tbscene` document is right-handed/Z-up. The first implementation needs one
explicit, tested profile transform at the scene-to-Lantern boundary.

### 2. Compile a `SceneDocument` to existing authored-zone outputs

Introduce a format-neutral adapter beside `mapFile.ts` (for example
`packages/shared/src/authoredScene.ts`) with input-independent types whose
output is the existing `AuthoredZoneSource`, `AuthoredZoneGeometry`, and
`AuthoredZoneBuild` equivalent. The initial compiler should produce:

- `LanternMesh` data with exact package mesh positions, normals, per-corner
  UVs, material groups, baked RGBA where present, and an explicit policy when
  a required runtime field is absent;
- material-list data including render modes and texture animation frames;
- a separate collision mesh selected by collision class, not an accidental
  mirror of visible geometry;
- region and water information for environment/underwater presentation;
- existing authored extras for props, lights, doors, spawns, safespots, climbs
  and zonelines; and
- a source document hash plus stable record IDs in diagnostics and reload
  messages.

Do not lower mesh UVs to Valve planar axes. The immediate reason for this work
is precisely that source triangles can have incompatible or non-affine UVs.
Six-decimal imported Valve axes are a legacy compatibility detail; mesh UVs
must remain the authoritative per-corner values in a `.tbscene` document.

### 3. Add dev-server discovery and cache support

Generalize `packages/client/dev.ts` from `AUTHORED_MAPS_DIR/*.map` to an
authored-document resolver. Keep map names namespaced as `map:<name>` during
the migration, even for `.tbscene`, so authored content cannot shadow an
extracted zone. Reject duplicate stems across enabled formats unless the
developer explicitly selects one.

The first hot-reload implementation may classify every `.tbscene` change as a
full geometry reload. The existing fine-grained map paths depend on source
text spans, brush hashes and array positions; copying those assumptions would
give meshes unstable identities. Add incremental patches later only after
specifying stable record IDs and revisions for mesh ranges, component updates,
material changes and provenance. Never emit a partial patch after a failed
package validation.

### 4. Direct exporter emission, after reader parity

Once the reader/compiler passes the gates below, let the Norrath exporter emit
`.tbscene` directly. Keep optional `.map` + `.eqexport.json` output as a
compatibility/debug projection. The exporter should make mesh triangles,
per-corner UVs, materials, source IDs, static prop transforms and known
animation availability direct records rather than reconstructing them as
convex prisms first.

The exporter must preserve the distinction discovered in the Unrest tower
work: source-authored cross-wedge texture phase resets are evidence, not an
automatic bug. It may improve an exporter fit only when raw source UVs and
loaded output prove a mismatch; it must not normalize an intentional seam.

## Required test fixtures and acceptance gates

Build the following fixtures before enabling a `.tbscene` zone in normal
play. Fixtures should be small packages plus corresponding expected runtime
IR, not screenshots alone.

1. **Codec integrity fixture:** deterministic write/read/write package bytes;
   bad hash, range, index, non-finite number, unknown required extension and
   duplicate-ID rejection.
2. **Coordinate fixture:** legacy EQ source transform (32 map units per game
   unit), a known handedness/up conversion, and a landmark/normal assertion.
   The test must show the adapter cannot silently mirror a zone.
3. **Mesh/material fixture:** two triangles with different per-corner UVs,
   adjacent material groups, texture frames, transparency/additive settings,
   normals and baked vertex colors. It must prove no planar-UV fitting occurs.
4. **Collision fixture:** visible-but-nonsolid, solid-but-invisible, water
   volume, step, door threshold, and placed-prop collision. Verify movement,
   nearest ground and worker/client parity.
5. **Semantic fixture:** a door at all three poses, a static/dynamic light,
   water surface/volume/bottom, a textured animated prop, region/zoneline,
   safespot, climb and spawn. Verify absence/degradation behavior as well as
   the happy path.
6. **Compatibility fixture:** import a representative `.map` into a scene,
   compile both source forms, and compare runtime mesh/collision/extras.
   Separately assert an explicit loss report when exporting a mesh/component
   back to `.map` cannot be exact.
7. **Unrest acceptance fixture:** use the existing reference/candidate
   acceptance context for player-height interior/exterior cameras, occupancy,
   player clearance and door traversal. Preserve source-authored seams but
   flag a mismatch between a source triangle's UVs and loaded runtime UVs.
8. **Full runtime gate:** serve the package through the normal dev routes,
   load it through `browserZoneSource`, install it via
   `applyAuthorityZoneBootstrap`, and run the existing first-playable-frame
   / authored-zone smoke tests. Compare `LanternMesh`, collision arrays,
   authored extras and stable source-document hash against expected output.

Run `tools/validateZoneMap.ts` unchanged for legacy maps and add a sibling
`validateScenePackage.ts` for the package-specific checks. Neither validator
is a substitute for a playability smoke test.

## Assets and runtime semantics that require deliberate mapping

- **Materials:** retain the exact material token, texture assets, render mode,
  alpha, frame order/delay, two-sided policy and baked vertex colour. Do not
  let an asset lookup failure silently choose a lookalike texture.
- **Animated props:** carry model asset, skeleton/clip/state/time and a
  runtime binding. The exporter currently has explicit unavailable-animation
  metadata for static reductions; surface this as a diagnostic, not an
  invented animation.
- **Doors:** closed, half-open and open poses are editor-preview state, but
  runtime needs native motion duration/sweep/hitbox/collision semantics. The
  visible door mesh and blocker must be independently represented.
- **Lighting:** distinguish baked mesh colour, world time/ambient lighting,
  static light records and dynamic carried/entity lights. Do not double-light
  a source that already has baked contributions.
- **Water:** surface, fluid volume and physical bottom are separate records.
  Underwater state comes from the volume/region, rendering from the surface,
  and collision/navigation from the boundary/bottom policy.
- **Props and collision:** static object meshes, companion collision meshes,
  lifts and animation-driven collision need an explicit inclusion policy.
- **Extensions:** a runtime that cannot understand a required Norrath profile
  extension must refuse to play that scene rather than guess. Optional unknown
  extensions must survive editor/packaging rewrites.

## Feature negotiation and backward compatibility

Treat format support as a capability exchange, not extension-name optimism.

1. The reader supports the common `format`/`version`, then validates every
   `requiredExtensions` owner/version before it accepts a playable document.
2. The Norrath profile exposes a capability set in diagnostics and dev-server
   responses: supported profile version, mesh attributes, semantic component
   types, asset URI schemes and animation/collision support.
3. A document may be inspected read-only when an optional extension is unknown,
   but it may not be rewritten unless the package implementation can preserve
   that extension's canonical value exactly.
4. `.map` stays accepted and keeps its `map:<name>` URL contract. A `.tbscene`
   is additionally accepted under the same namespace only when the resolver
   can prove there is no stem collision; no silent precedence rule.
5. A fallback to a `.map` projection requires an explicit export policy and
   loss report. Runtime fallback must be opt-in, recorded in diagnostics and
   never hide missing collision/door/water semantics.

## Questions for the Norrath engine owner

Resolve these in code/tests before direct exporter output becomes the default:

1. What exact scene-to-Lantern transform preserves Norrath's current
   left-handed/Y-up runtime while `.tbscene` remains right-handed/Z-up? Which
   single conversion point owns winding and normal reversal?
2. Are normal maps/tangents, lightmaps, or only baked RGBA needed for the
   intended scenes? The format allows tangents, but Norrath must state its
   runtime material contract.
3. Is a static-light component sufficient for zone lights, or do dynamic
   lights need a separate authoritative replication/budget contract?
4. Which door component fields are authoritative game behavior versus editor
   preview data, especially for collision during the half-open pose?
5. How should moving/animated prop collision be represented to the worker and
   client prediction without turning static `collision.ts` into an unbounded
   scene graph?
6. Which asset URI schemes are permitted in shipped/browser packages, and how
   are source-client assets versioned and integrity-checked?
7. Does runtime navigation consume only collision triangles initially, or
   should authored off-mesh links/nav data be a required profile extension?
8. What is the archive/cache strategy for a large zone: direct package fetch,
   unpacked dev directory, content-addressed extracted cache, or a compiled
   Lantern bundle? Measure first; do not make a 132 MB provenance JSON part of
   a hot path.

## Completion criteria for the handoff

The first Norrath `.tbscene` slice is complete when a package with a mesh,
solid, materials, collision, door, light, water and provenance can be opened
through the normal dev route; compiled to existing Lantern/runtime inputs;
loaded by both the browser renderer and authority worker; and passes the
fixture, integrity, collision, visual-acceptance and first-playable-frame
gates above. Direct exporter emission and high-granularity hot reload are
separate follow-up slices.
