/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "RenderView.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/GLVertexType.h"
#include "Renderer/PrimType.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboManager.h"
#include "Renderer/VertexArray.h"
#include "TrenchBroomApp.h"
#include "View/GLContextManager.h"
#include "View/InputEvent.h"
#include "View/QtUtils.h"

/*
 * - glew requires it is included before <OpenGL/gl.h>
 *
 * - Qt requires that glew is included after <qopengl.h> and <QOpenGLFunctions>
 * - QOpenGLWidget includes <qopengl.h>
 * - qopengl.h includes OpenGL/gl.h
 *
 * therefore
 * - glew wants to be included first
 * - and so does QOpenGLWidget
 *
 * Since including glew before QOpenGLWidget only generates a warning and does not seem to
 * incur any ill effects, we silence the warning here.
 *
 * Note that GCC does not let us silence this warning using diagnostic pragmas, so it is
 * disabled in the CXX_FLAGS!
 */
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcpp"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#endif

#include <QOpenGLContext>

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include <QDateTime>
#include <QPalette>
#include <QTimer>
#include <QWidget>

#ifdef _WIN32
#endif

#include "vecmath/mat.h"
#include "vecmath/mat_ext.h"

#include <iostream>

namespace TrenchBroom
{
namespace View
{
RenderView::RenderView(GLContextManager& contextManager, QWidget* parent)
  : QOpenGLWidget(parent)
  , m_glContext(&contextManager)
  , m_framesRendered(0)
  , m_maxFrameTimeMsecs(0)
  , m_lastFPSCounterUpdate(0)
{
  QPalette pal;
  const QColor color = pal.color(QPalette::Highlight);
  m_focusColor = fromQColor(color);

  // FPS counter
  QTimer* fpsCounter = new QTimer(this);

  connect(fpsCounter, &QTimer::timeout, [&]() {
    const int64_t currentTime = QDateTime::currentMSecsSinceEpoch();
    const int framesRenderedInPeriod = m_framesRendered;
    const int maxFrameTime = m_maxFrameTimeMsecs;
    const int64_t fpsCounterPeriod = currentTime - m_lastFPSCounterUpdate;
    const double avgFps = static_cast<double>(framesRenderedInPeriod)
                          / (static_cast<double>(fpsCounterPeriod) / 1000.0);

    m_framesRendered = 0;
    m_maxFrameTimeMsecs = 0;
    m_lastFPSCounterUpdate = currentTime;

    m_currentFPS =
      std::string("Avg FPS: ") + std::to_string(avgFps)
      + " Max time between frames: " + std::to_string(maxFrameTime) + "ms. "
      + std::to_string(m_glContext->vboManager().currentVboCount()) + " current VBOs ("
      + std::to_string(m_glContext->vboManager().peakVboCount()) + " peak) totalling "
      + std::to_string(m_glContext->vboManager().currentVboSize() / 1024u) + " KiB";
  });

  fpsCounter->start(1000);

  setMouseTracking(true); // request mouse move events even when no button is held down
  setFocusPolicy(Qt::StrongFocus); // accept focus by clicking or tab
}

RenderView::~RenderView() = default;

void RenderView::keyPressEvent(QKeyEvent* event)
{
  m_eventRecorder.recordEvent(*event);
  update();
}

void RenderView::keyReleaseEvent(QKeyEvent* event)
{
  m_eventRecorder.recordEvent(*event);
  update();
}

static auto mouseEventWithFullPrecisionLocalPos(
  const QWidget* widget, const QMouseEvent* event)
{
  // The localPos of a Qt mouse event is only in integer coordinates, but window pos
  // and screen pos have full precision. We can't directly map the windowPos because
  // mapTo takes QPoint, so we just map the origin and subtract that.
  const auto localPos =
    event->windowPos() - QPointF(widget->mapTo(widget->window(), QPoint(0, 0)));
  return QMouseEvent(
    event->type(),
    localPos,
    event->windowPos(),
    event->screenPos(),
    event->button(),
    event->buttons(),
    event->modifiers(),
    event->source());
}

void RenderView::mouseDoubleClickEvent(QMouseEvent* event)
{
  m_eventRecorder.recordEvent(mouseEventWithFullPrecisionLocalPos(this, event));
  update();
}

void RenderView::mouseMoveEvent(QMouseEvent* event)
{
  m_eventRecorder.recordEvent(mouseEventWithFullPrecisionLocalPos(this, event));
  update();
}

void RenderView::mousePressEvent(QMouseEvent* event)
{
  m_eventRecorder.recordEvent(mouseEventWithFullPrecisionLocalPos(this, event));
  update();
}

void RenderView::mouseReleaseEvent(QMouseEvent* event)
{
  m_eventRecorder.recordEvent(mouseEventWithFullPrecisionLocalPos(this, event));
  update();
}

void RenderView::wheelEvent(QWheelEvent* event)
{
  m_eventRecorder.recordEvent(*event);
  update();
}

void RenderView::paintGL()
{
  if (TrenchBroom::View::isReportingCrash())
    return;

  render();

  // Update stats
  m_framesRendered++;
  if (m_timeSinceLastFrame.isValid())
  {
    int frameTime = static_cast<int>(m_timeSinceLastFrame.restart());
    if (frameTime > m_maxFrameTimeMsecs)
    {
      m_maxFrameTimeMsecs = frameTime;
    }
  }
  else
  {
    m_timeSinceLastFrame.start();
  }
}

Renderer::VboManager& RenderView::vboManager()
{
  return m_glContext->vboManager();
}

Renderer::FontManager& RenderView::fontManager()
{
  return m_glContext->fontManager();
}

Renderer::ShaderManager& RenderView::shaderManager()
{
  return m_glContext->shaderManager();
}

int RenderView::depthBits() const
{
  const auto format = this->context()->format();
  return format.depthBufferSize();
}

bool RenderView::multisample() const
{
  const auto format = this->context()->format();
  return format.samples() != -1;
}

void RenderView::initializeGL()
{
  doInitializeGL();
}

void RenderView::resizeGL(int w, int h)
{
  // These are in points, not pixels
  doUpdateViewport(0, 0, w, h);
}

void RenderView::render()
{
  processInput();
  clearBackground();
  doRender();
  renderFocusIndicator();
}

void RenderView::processInput()
{
  m_eventRecorder.processEvents(*this);
}

void RenderView::clearBackground()
{
  const auto backgroundColor = getBackgroundColor();

  glAssert(glClearColor(
    backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a()));
  glAssert(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

const Color& RenderView::getBackgroundColor()
{
  return pref(Preferences::BackgroundColor);
}

void RenderView::renderFocusIndicator()
{
  if (!doShouldRenderFocusIndicator() || !hasFocus())
    return;

  const Color& outer = m_focusColor;
  const Color& inner = m_focusColor;

  const qreal r = devicePixelRatioF();
  const auto w = static_cast<float>(width() * r);
  const auto h = static_cast<float>(height() * r);
  glAssert(glViewport(0, 0, static_cast<int>(w), static_cast<int>(h)));

  const auto t = 1.0f;

  const auto projection = vm::ortho_matrix(
    -1.0f, 1.0f, 0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h));
  Renderer::Transformation transformation(projection, vm::mat4x4f::identity());

  glAssert(glDisable(GL_DEPTH_TEST));

  using Vertex = Renderer::GLVertexTypes::P3C4::Vertex;
  auto array = Renderer::VertexArray::move(
    std::vector<Vertex>({// top
                         Vertex(vm::vec3f(0.0f, 0.0f, 0.0f), outer),
                         Vertex(vm::vec3f(w, 0.0f, 0.0f), outer),
                         Vertex(vm::vec3f(w - t, t, 0.0f), inner),
                         Vertex(vm::vec3f(t, t, 0.0f), inner),

                         // right
                         Vertex(vm::vec3f(w, 0.0f, 0.0f), outer),
                         Vertex(vm::vec3f(w, h, 0.0f), outer),
                         Vertex(vm::vec3f(w - t, h - t, 0.0f), inner),
                         Vertex(vm::vec3f(w - t, t, 0.0f), inner),

                         // bottom
                         Vertex(vm::vec3f(w, h, 0.0f), outer),
                         Vertex(vm::vec3f(0.0f, h, 0.0f), outer),
                         Vertex(vm::vec3f(t, h - t, 0.0f), inner),
                         Vertex(vm::vec3f(w - t, h - t, 0.0f), inner),

                         // left
                         Vertex(vm::vec3f(0.0f, h, 0.0f), outer),
                         Vertex(vm::vec3f(0.0f, 0.0f, 0.0f), outer),
                         Vertex(vm::vec3f(t, t, 0.0f), inner),
                         Vertex(vm::vec3f(t, h - t, 0.0f), inner)}));

  array.prepare(vboManager());
  array.render(Renderer::PrimType::Quads);
  glAssert(glEnable(GL_DEPTH_TEST));
}

bool RenderView::doInitializeGL()
{
  return m_glContext->initialize();
}

void RenderView::doUpdateViewport(
  const int /* x */, const int /* y */, const int /* width */, const int /* height */)
{
}
} // namespace View
} // namespace TrenchBroom
