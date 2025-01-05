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

#include "RenderView.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "TrenchBroomApp.h"
#include "render/GLVertexType.h"
#include "render/PrimType.h"
#include "render/Transformation.h"
#include "render/VboManager.h"
#include "render/VertexArray.h"
#include "ui/GLContextManager.h"
#include "ui/InputEvent.h"
#include "ui/QtUtils.h"

#include <fmt/format.h>

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

#include "vm/mat.h"
#include "vm/mat_ext.h"

namespace tb::ui
{

RenderView::RenderView(GLContextManager& contextManager, QWidget* parent)
  : QOpenGLWidget{parent}
  , m_glContext{&contextManager}
{
  auto pal = QPalette{};
  const auto color = pal.color(QPalette::Highlight);
  m_focusColor = fromQColor(color);

  // FPS counter
  auto* fpsCounter = new QTimer{this};

  connect(fpsCounter, &QTimer::timeout, [&]() {
    const int64_t currentTime = QDateTime::currentMSecsSinceEpoch();
    const int framesRenderedInPeriod = m_framesRendered;
    const int maxFrameTime = m_maxFrameTimeMsecs;
    const int64_t fpsCounterPeriod = currentTime - m_lastFPSCounterUpdate;
    const double avgFps =
      double(framesRenderedInPeriod) / (double(fpsCounterPeriod) / 1000.0);

    m_framesRendered = 0;
    m_maxFrameTimeMsecs = 0;
    m_lastFPSCounterUpdate = currentTime;

    m_currentFPS = fmt::format(
      R"(Avg FPS: {} Max time between frames: {}ms. {} currentVBOS({} peak) totalling {} KiB)",
      avgFps,
      maxFrameTime,
      m_glContext->vboManager().currentVboCount(),
      m_glContext->vboManager().peakVboCount(),
      m_glContext->vboManager().currentVboSize() / 1024u);
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
    event->scenePosition() - QPointF{widget->mapTo(widget->window(), QPoint{0, 0})};
  return QMouseEvent{
    event->type(),
    localPos,
    event->scenePosition(),
    event->globalPosition(),
    event->button(),
    event->buttons(),
    event->modifiers(),
    event->source()};
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

bool RenderView::event(QEvent* event)
{
  // Unfortunately, QWidget doesn't define a specialized handler for QNativeGestureEvent,
  // so we must override the main event handler to handle it.
  if (event->type() == QEvent::NativeGesture)
  {
    const auto gestureEvent = static_cast<QNativeGestureEvent*>(event);
    m_eventRecorder.recordEvent(*gestureEvent);
    update();
    return true;
  }

  // Let the base class handle all other events normally
  return QOpenGLWidget::event(event);
}

void RenderView::paintGL()
{
  if (tb::ui::isReportingCrash())
  {
    return;
  }

  render();

  // Update stats
  m_framesRendered++;
  if (m_timeSinceLastFrame.isValid())
  {
    auto frameTime = int(m_timeSinceLastFrame.restart());
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

render::VboManager& RenderView::vboManager()
{
  return m_glContext->vboManager();
}

render::FontManager& RenderView::fontManager()
{
  return m_glContext->fontManager();
}

render::ShaderManager& RenderView::shaderManager()
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
  updateViewport(0, 0, w, h);
}

void RenderView::render()
{
  processInput();
  clearBackground();
  renderContents();
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
  if (shouldRenderFocusIndicator() && hasFocus())
  {
    const auto& outer = m_focusColor;
    const auto& inner = m_focusColor;

    const auto r = devicePixelRatioF();
    const auto w = float(width() * r);
    const auto h = float(height() * r);
    glAssert(glViewport(0, 0, int(w), int(h)));

    const auto t = 1.0f;

    const auto projection = vm::ortho_matrix(-1.0f, 1.0f, 0.0f, 0.0f, float(w), float(h));
    auto transformation = render::Transformation{projection, vm::mat4x4f::identity()};

    glAssert(glDisable(GL_DEPTH_TEST));

    using Vertex = render::GLVertexTypes::P3C4::Vertex;
    auto array = render::VertexArray::move(std::vector{
      // top
      Vertex{{0.0f, 0.0f, 0.0f}, outer},
      Vertex{{w, 0.0f, 0.0f}, outer},
      Vertex{{w - t, t, 0.0f}, inner},
      Vertex{{t, t, 0.0f}, inner},

      // right
      Vertex{{w, 0.0f, 0.0f}, outer},
      Vertex{{w, h, 0.0f}, outer},
      Vertex{{w - t, h - t, 0.0f}, inner},
      Vertex{{w - t, t, 0.0f}, inner},

      // bottom
      Vertex{{w, h, 0.0f}, outer},
      Vertex{{0.0f, h, 0.0f}, outer},
      Vertex{{t, h - t, 0.0f}, inner},
      Vertex{{w - t, h - t, 0.0f}, inner},

      // left
      Vertex{{0.0f, h, 0.0f}, outer},
      Vertex{{0.0f, 0.0f, 0.0f}, outer},
      Vertex{{t, t, 0.0f}, inner},
      Vertex{{t, h - t, 0.0f}, inner},
    });

    array.prepare(vboManager());
    array.render(render::PrimType::Quads);
    glAssert(glEnable(GL_DEPTH_TEST));
  }
}

bool RenderView::doInitializeGL()
{
  return m_glContext->initialize();
}

void RenderView::updateViewport(
  const int /* x */, const int /* y */, const int /* width */, const int /* height */)
{
}

} // namespace tb::ui
