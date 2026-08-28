# TrenchBroom Hybrid Scene Format

`tbscene` is a versioned, game-neutral scene-document format for TrenchBroom. It is
intended for games that need both editor-native constructive solid geometry and
runtime-native mesh or semantic data. Norrath is the first client, not a special
case in the format.

- [ADR 0001](adr-0001-tbscene-package.md) records the package decision.
- [The v1 specification](tbscene-v1.md) is the normative format contract.
- [The JSON Schema](schema/tbscene-v1.schema.json) validates the manifest shape.
- [The representative fixture](fixtures/minimal.tbscene.fixture.json) is a small,
  deterministic manifest used by future reader/writer tests.

The initial implementation deliberately has no production reader, writer, UI, or
runtime exporter. Those are separate, independently reviewable work packages.

## Ownership and staged implementation

The following packages are intentionally separable so agents can work concurrently
once the manifest contract is stable:

1. **Core model and IDs** — `SceneDocument`, stable IDs, transforms, extension
   preservation, and document hashing.
2. **Package codec** — deterministic ZIP reader/writer, canonical JSON, binary mesh
   chunk codecs, and fixture/golden tests.
3. **TrenchBroom bridge** — lossless `.map` import, editable convex-brush projection,
   mesh rendering/picking, and save/export commands.
4. **Semantic components** — door poses, lights, water volumes, animated props, and
   game-definition hooks.
5. **Reference and acceptance integration** — comparison contexts, provenance joins,
   occupancy/clearance checks, and evidence snapshots.
6. **Norrath exporter and runtime hand-off** — exporter emission, asset resolver,
   runtime consumer, and scene-preview parity tests.

No package may reinterpret an unknown extension. A reader that cannot preserve an
unknown required extension must refuse to rewrite the document.
