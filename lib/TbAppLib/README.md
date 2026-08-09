# TrenchBroom App Library

Contains TrenchBroom's Qt-free application logic: the map document model and the
editing tool framework built on top of it. This library has no dependency on Qt,
which is enforced by the build (it links against none of the `Qt6::` targets) —
`TbUiLib` links against it to provide the actual Qt-based user interface on top
of this Qt-free core.

## Map Document

`MapDocument` owns the currently open map and mediates access to it, notifying
observers (undo/redo, selection changes, document lifecycle, and so on) via a
set of `Notifier`s. UI-specific concerns that would otherwise pull Qt into the
document — such as caching menu-ready actions for tags and entity definitions
— live in `TbUiLib` instead (see `MapDocumentActionCache`), connected purely
through those notifiers.

## Tools

`Tool`, `ToolChain`, and `ToolController` form the base framework that the
editing tools (clip, vertex, extrude, rotate, scale, shear, UV editing, and so
on) are built on, along with the shape-drawing tool extension system
(`DrawShapeToolExtension` and its subclasses). Tools receive input and render
through Qt-free abstractions (`InputState`, `PickRequest`, drag trackers) so
that the tool logic itself never touches Qt, even though it's ultimately
driven by Qt input events translated in `TbUiLib`.
