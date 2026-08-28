# EverQuest player-view lighting preview

TrenchBroom's ordinary textured view is intentionally full-bright. EverQuest maps can
instead opt into an approximate player-view preview from **View Options → Renderer →
Preview EQ player lighting**.

The preview provides:

- human / barbarian vision;
- infravision, with warmer lifted night visibility;
- ultravision, with stronger cool night visibility;
- a 0–24 hour time-of-day control that also changes the viewport sky; and
- diffuse lighting from `eq_light` and `eq_prop_light` entities.

`eq_light` uses its `origin`, `_color` (RGB 0–255), and `radius` properties.
`eq_prop_light` uses optional `_color` and `radius` overrides; otherwise the editor uses
a warm fixture light with conservative candelabra, brazier, and generic defaults.

This mode is an editing approximation, not an EQ renderer emulator. It deliberately
does not bake lightmaps, trace hard shadows, reproduce fog tables, or model per-race
adaptation exactly. Its purpose is to expose bad fixture placement, unlit routes, and
time-of-day readability while geometry is still being edited.

## Focus-neutral automation

`render.capture` accepts an optional `scenePreview` object. Because it is part of the
immutable virtual-render request, it does not depend on GUI preferences or the active
window:

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
  "overlays": {"brushEdges": false, "selection": false, "grid": false},
  "scenePreview": {
    "vision": "infravision",
    "timeOfDay": 22.5,
    "entityLights": true
  }
}
```

Valid `vision` values are `human`, `infravision`, and `ultravision`. `timeOfDay` is in
the half-open range `[0, 24)`. Omit `scenePreview` to retain the ordinary full-bright
automation render.

For visual QA, capture the same camera at the same document revision with several
`scenePreview` values. This isolates lighting and vision differences from camera,
selection, focus, and geometry changes.
