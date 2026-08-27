/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include <QImage>
#include <QString>

#include "ui/AutomationRenderRequest.h"

#include <cstddef>
#include <optional>

class QOffscreenSurface;
class QOpenGLContext;

namespace tb
{
namespace gl
{
class GlManager;
} // namespace gl

namespace ui
{
class MapDocument;

enum class AutomationOffscreenRenderError
{
  None,
  InvalidRequest,
  ContextUnavailable,
  ResourceNotReady,
  FramebufferUnavailable,
  DocumentChanged,
  RenderFailed,
};

struct AutomationOffscreenRenderResult
{
  AutomationOffscreenRenderError error = AutomationOffscreenRenderError::None;
  QString message;
  QImage image;
  std::optional<automation::AutomationDepthImage> depth;
  size_t revision = 0u;

  explicit operator bool() const { return error == AutomationOffscreenRenderError::None; }
};

/**
 * Focus-neutral renderer for a supplied camera and a caller-owned shared offscreen
 * OpenGL context. It deliberately has no QWidget, MapWindow, or input/tool dependency.
 *
 * The application integration layer owns context creation and resource scheduling. This
 * class only makes the supplied context current for the duration of a capture and
 * restores the prior Qt context before returning.
 */
class AutomationOffscreenRenderer
{
private:
  QOpenGLContext& m_context;
  QOffscreenSurface& m_surface;
  gl::GlManager& m_glManager;

public:
  AutomationOffscreenRenderer(
    QOpenGLContext& context, QOffscreenSurface& surface, gl::GlManager& glManager);

  AutomationOffscreenRenderResult capture(
    MapDocument& document, const automation::AutomationRenderRequest& request);
};

} // namespace ui
} // namespace tb
