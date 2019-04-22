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

#include "TrenchBroomApp.h"
#include "RenderView.h"
#include "Exceptions.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Transformation.h"
#include "Renderer/VertexArray.h"
#include "Renderer/GLVertexType.h"
#include "View/GLContextManager.h"
#include "View/InputEvent.h"
#include "View/wxUtils.h"

#include <QPalette>
#include <QTimer>
#include <QDateTime>
#include <QGridLayout>
#include <QWidget>
#include <QGuiApplication>

#ifdef _WIN32
#include <GL/wglew.h>
#endif

#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>

#include <iostream>

namespace TrenchBroom {
    namespace View {
        RenderView::RenderView(GLContextManager& contextManager) :
        QOpenGLWindow(),
        m_glContext(&contextManager),
        m_framesRendered(0),
        m_maxFrameTimeMsecs(0),
        m_lastFPSCounterUpdate(0) {
            QPalette pal;
            const QColor color = pal.color(QPalette::Highlight);
            m_focusColor = fromQColor(color);

            // FPS counter
            QTimer* fpsCounter = new QTimer(this);

            connect(fpsCounter, &QTimer::timeout, [&](){
                const int64_t currentTime = QDateTime::currentMSecsSinceEpoch();
                const int framesRenderedInPeriod = m_framesRendered;
                const int maxFrameTime = m_maxFrameTimeMsecs;
                const int64_t fpsCounterPeriod = currentTime - m_lastFPSCounterUpdate;
                const double avgFps = framesRenderedInPeriod / (fpsCounterPeriod / 1000.0);

                m_framesRendered = 0;
                m_maxFrameTimeMsecs = 0;
                m_lastFPSCounterUpdate = currentTime;

                m_currentFPS = std::string("Avg FPS: ") + std::to_string(avgFps) + " Max time between frames: " +
                        std::to_string(maxFrameTime) + "ms. 1000ms QTimer actually took: " + std::to_string(fpsCounterPeriod);
            });

            fpsCounter->start(1000);

            m_windowContainer = QWidget::createWindowContainer(this);
        }

        RenderView::~RenderView() = default;

        void RenderView::keyPressEvent(QKeyEvent* event) {
            m_eventRecorder.recordEvent(event);
            requestUpdate();
        }

        void RenderView::keyReleaseEvent(QKeyEvent* event) {
            m_eventRecorder.recordEvent(event);
            requestUpdate();
        }

        void RenderView::mouseDoubleClickEvent(QMouseEvent* event) {
            m_eventRecorder.recordEvent(event);
            requestUpdate();
        }

        void RenderView::mouseMoveEvent(QMouseEvent* event) {
            m_eventRecorder.recordEvent(event);
            requestUpdate();
        }

        void RenderView::mousePressEvent(QMouseEvent* event) {
            m_eventRecorder.recordEvent(event);
            requestUpdate();
        }

        void RenderView::mouseReleaseEvent(QMouseEvent* event) {
            m_eventRecorder.recordEvent(event);
            requestUpdate();
        }

        void RenderView::wheelEvent(QWheelEvent* event) {
            m_eventRecorder.recordEvent(event);
            requestUpdate();
        }

        QWidget* RenderView::widgetContainer() {
            return m_windowContainer;
        }

        bool RenderView::hasFocus() const {
            return QGuiApplication::focusWindow() == this;
        }

        bool RenderView::IsBeingDeleted() const {
            // FIXME: remove this
            return false;
        }

        void RenderView::Refresh() {
            // FIXME: remove this
            // Schedules a repaint with Qt
            requestUpdate();
        }

        void RenderView::paintGL() {
            // FIXME: crash
#if 0
            if (TrenchBroom::View::isReportingCrash()) return;
#endif
            render();

            // Update stats
            m_framesRendered++;
            if (m_timeSinceLastFrame.isValid()) {
                int frameTime = static_cast<int>(m_timeSinceLastFrame.restart());
                if (frameTime > m_maxFrameTimeMsecs) {
                    m_maxFrameTimeMsecs = frameTime;
                }
            } else {
                m_timeSinceLastFrame.start();
            }
        }

        void RenderView::update() {
            Refresh();
        }

        Renderer::Vbo& RenderView::vertexVbo() {
            return m_glContext->vertexVbo();
        }

        Renderer::Vbo& RenderView::indexVbo() {
            return m_glContext->indexVbo();
        }

        Renderer::FontManager& RenderView::fontManager() {
            return m_glContext->fontManager();
        }

        Renderer::ShaderManager& RenderView::shaderManager() {
            return m_glContext->shaderManager();
        }

        int RenderView::depthBits() const {
            // FIXME: implement for Qt
            return -1;
        }

        bool RenderView::multisample() const {
            // FIXME: implement for Qt
            return false;
        }

        void RenderView::initializeGL() {
            m_glContext->initialize();
        }

        void RenderView::resizeGL(int w, int h) {
            // These are in points, not pixels
            doUpdateViewport(0, 0, w, h);
        }

        void RenderView::render() {
            processInput();
            clearBackground();
            doRender();
            renderFocusIndicator();
        }

        void RenderView::processInput() {
            m_eventRecorder.processEvents(*this);
        }

        void RenderView::clearBackground() {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& backgroundColor = prefs.get(Preferences::BackgroundColor);

            glAssert(glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a()));
            glAssert(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        }

        void RenderView::renderFocusIndicator() {
            if (!doShouldRenderFocusIndicator() || !hasFocus())
                return;

            const Color& outer = m_focusColor;
            const Color& inner = m_focusColor;

            const QSize clientSize = size();
            const auto w = static_cast<float>(clientSize.width());
            const auto h = static_cast<float>(clientSize.height());
            const auto t = 1.0f;

            glAssert(glViewport(0, 0, w, h));

            const auto projection = vm::orthoMatrix(-1.0f, 1.0f, 0.0f, 0.0f, w, h);
            Renderer::Transformation transformation(projection, vm::mat4x4f::identity);

            glAssert(glDisable(GL_DEPTH_TEST));

            using Vertex = Renderer::GLVertexTypes::P3C4::Vertex;
            auto array = Renderer::VertexArray::move(Vertex::List {
            // top
                Vertex(vm::vec3f(0.0f, 0.0f, 0.0f), outer),
                Vertex(vm::vec3f(w, 0.0f, 0.0f), outer),
                Vertex(vm::vec3f(w-t, t, 0.0f), inner),
                Vertex(vm::vec3f(t, t, 0.0f), inner),

            // right
                Vertex(vm::vec3f(w, 0.0f, 0.0f), outer),
                Vertex(vm::vec3f(w, h, 0.0f), outer),
                Vertex(vm::vec3f(w-t, h-t, 0.0f), inner),
                Vertex(vm::vec3f(w-t, t, 0.0f), inner),

            // bottom
                Vertex(vm::vec3f(w, h, 0.0f), outer),
                Vertex(vm::vec3f(0.0f, h, 0.0f), outer),
                Vertex(vm::vec3f(t, h-t, 0.0f), inner),
                Vertex(vm::vec3f(w-t, h-t, 0.0f), inner),

            // left
                Vertex(vm::vec3f(0.0f, h, 0.0f), outer),
                Vertex(vm::vec3f(0.0f, 0.0f, 0.0f), outer),
                Vertex(vm::vec3f(t, t, 0.0f), inner),
                Vertex(vm::vec3f(t, h-t, 0.0f), inner)
            });

            Renderer::ActivateVbo activate(vertexVbo());
            array.prepare(vertexVbo());
            array.render(GL_QUADS);
            glAssert(glEnable(GL_DEPTH_TEST));
        }

        void RenderView::doUpdateViewport(const int x, const int y, const int width, const int height) {}
    }
}
