/*
 Copyright (C) 2010 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <GL/glew.h> // must be included here, before QOpenGLWidget

#include <QElapsedTimer>
#include <QOpenGLWidget>

#include "Color.h"
#include "ui/InputEvent.h"

#include <string>

#undef Bool
#undef Status
#undef CursorShape

namespace tb
{
namespace render
{
class FontManager;
class ShaderManager;
class VboManager;
} // namespace render

namespace ui
{
class GLContextManager;

class RenderView : public QOpenGLWidget, public InputEventProcessor
{
  Q_OBJECT
private:
  Color m_focusColor;
  GLContextManager* m_glContext;
  InputEventRecorder m_eventRecorder;

private: // FPS counter
  // stats since the last counter update
  int m_framesRendered = 0;
  int m_maxFrameTimeMsecs = 0;
  // other
  int64_t m_lastFPSCounterUpdate = 0;
  QElapsedTimer m_timeSinceLastFrame;

protected:
  std::string m_currentFPS;

protected:
  explicit RenderView(GLContextManager& contextManager, QWidget* parent = nullptr);

public:
  ~RenderView() override;

protected: // QWindow overrides
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  bool event(QEvent* event) override;

protected:
  render::VboManager& vboManager();
  render::FontManager& fontManager();
  render::ShaderManager& shaderManager();

  int depthBits() const;
  bool multisample() const;

protected: // QOpenGLWidget overrides
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;

private:
  void render();
  void processInput();
  void clearBackground();
  void renderFocusIndicator();

protected:
  // called by initializeGL by default
  virtual bool doInitializeGL();

private:
  virtual const Color& getBackgroundColor();
  virtual void updateViewport(int x, int y, int width, int height);
  virtual bool shouldRenderFocusIndicator() const = 0;
  virtual void renderContents() = 0;
};

} // namespace ui
} // namespace tb
