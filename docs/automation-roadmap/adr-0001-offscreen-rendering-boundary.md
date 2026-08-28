# ADR 0001: virtual capture uses the map renderer with an automation-owned offscreen context

**Status:** Implemented; external cross-platform runtime gates remain
**Date:** 2026-08-27
**Deciders:** Automation roadmap maintainers

## Context

EV3 must make `render.capture` independent of the currently active `MapWindow` and
`MapViewBase`. It must work with no visible map window and must not change focus or a
user's camera, selection, current material, grid, or layout.

The codebase already separates most scene drawing from widget ownership:

- `render::MapRenderer` is constructed from `mdl::Map` and renders from a supplied
  `render::RenderContext` and `render::RenderBatch`. It has no `QWidget` dependency.
- `MapDocument` owns that `MapRenderer`.
- `MapViewBase::renderContents` supplies the view-owned camera, tool overlays, and
  GUI preferences to the renderer. `RenderView` is a `QOpenGLWidget` and also owns
  input, focus, timers, and visible-view lifecycle.
- `AppController::processGlResources` already creates a private `QOffscreenSurface`
  and a `QOpenGLContext` sharing `QOpenGLContext::globalShareContext()` on macOS and
  uses it to process GL resources. This establishes that an app-owned shared
  offscreen context is an existing, supported platform path.

## Decision

Use **renderer extraction with an automation-owned offscreen context and FBO**. The
production boundary starts with `ui::AutomationOffscreenRenderer`:

```text
MapDocument + AutomationRenderRequest
        |
AutomationOffscreenRenderer
  - injected QOpenGLContext/QOffscreenSurface
  - injected GlManager
  - local PerspectiveCamera or OrthographicCamera
  - RenderContext + MapRenderer + RenderBatch
  - QOpenGLFramebufferObject readback
        |
QImage + source revision, or a structured error
```

The renderer has no `QWidget`, `MapWindow`, `MapViewBase`, tool, active-window, or
current-view dependency. It temporarily makes only its injected context current and
restores the previous Qt context on return. It returns `DocumentChanged` if the map
revision changes during capture. Its initial scope is the EV0 textured mode, brush
edges, selection highlighting, and the orthographic grid; visible-editor-only overlays
(tools, FPS, compass, axes, and portal diagnostics) are deliberately excluded.

`AutomationOffscreenRenderer` is registered in CMake and reachable through the strict
`render.capture` RPC route. `AppController` owns its bounded shared-context callback;
the renderer never owns or borrows a GUI view.

## Alternatives considered

### Hidden `MapViewBase` / `QOpenGLWidget`

Rejected. It would reuse more of `MapViewBase::renderContents`, but it also imports
widget focus, input, timer, window-device-pixel-ratio, tool, and observer lifecycle.
Those are precisely the active-GUI dependencies virtual rendering must avoid. A hidden
widget is also a less direct route to FBO/structural buffers and leaves macOS widget
surface scheduling in the virtual-render path.

### Rebuild map rendering in an automation service

Rejected. `MapRenderer` already provides the needed scene-render boundary. Duplicating
it in the automation service would fork material, VBO, and map-change behavior instead
of reusing the renderer that visible views use.

## Spike evidence

| Question | Evidence and result |
| --- | --- |
| Can scene drawing be used without a widget? | Yes. `MapRenderer` consumes a supplied `RenderContext`, camera, and `RenderBatch`; the prototype uses that path directly. |
| Is a windowless macOS context/share path available? | Yes. `AppController::processGlResources` already creates `QOffscreenSurface` plus `QOpenGLContext`, shares `globalShareContext`, and processes resources there. |
| Can the proposed primitive compile in this tree? | Yes. `AutomationOffscreenRenderer.cpp` is built into `TbUiLib`. |
| Is the no-window/focus behavior covered? | Yes. Focused renderer and socket tests create only hidden documents/offscreen contexts and assert unchanged focus, camera, selection, and revision. |
| Are runtime macOS latency and byte-repeatability results final? | No. They require the EV4 app-owned context provider; no reliable measurement can borrow AppController's private context without changing the prohibited integration files. |

The existing private resource-processing context is especially important: virtual
capture must not create a second, unscheduled resource protocol. The spike therefore
returns `ResourceNotReady` rather than trying to process resources itself.

## EV4 integration requirements

1. Add the new production source and focused test to the relevant CMake targets.
2. Add one narrow, GUI-thread-only AppController/provider operation that owns the
   existing shared `QOffscreenSurface` and `QOpenGLContext`, makes it available for a
   bounded automation render callback, and serializes it with
   `processGlResources`. Do not expose a long-lived raw context to RPC handlers.
3. Before the callback, drain pending resource work with the same `GlManager` and
   `ProcessContext` used by `processGlResources`; retain `ResourceNotReady` if work
   cannot safely complete. Recreate the surface/context after an invalidation.
4. Construct `AutomationOffscreenRenderer` from that provider and a resolved explicit
   `MapDocument`; never resolve a document through the active window or current view.
5. Write the returned image under the automation temporary directory only after the
   revision check succeeds. The output response must echo normalized camera, image
   size, renderer version, and revision.
6. Keep all GL calls on the owning Qt GUI thread. Context independence, not a worker
   thread, is the focus-neutrality mechanism.

## EV4 runtime acceptance gates

Run the focused test and an application-level scenario on supported macOS hardware
(Apple Silicon and Intel where supported), plus the other desktop platforms:

- with no visible map window, capture a textured frame from a loaded document;
- repeat one unchanged request ten times and record image hashes and latency;
- verify focus widget, active window, GUI camera, selection, material, grid, and map
  revision before/after every capture;
- verify pending material/VBO work yields `ResourceNotReady` or succeeds only after
  the app-owned resource pump drains it;
- verify surface/context recreation after an invalid GL context; and
- run captures while a different visible document is foregrounded, proving the
  explicit target is unchanged.

The image-hash acceptance criterion is byte-identical repeatability within one
renderer/driver session. Cross-driver equivalence is evaluated visually and by
documented tolerances, not by raw pixel equality.
