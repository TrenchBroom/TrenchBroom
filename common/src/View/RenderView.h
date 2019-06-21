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

#ifndef TrenchBroom_RenderView
#define TrenchBroom_RenderView

#include "Color.h"
#include "Renderer/Vbo.h"
#include "View/InputEvent.h"

/*
 * - glew requires it is included before <OpenGL/gl.h>
 *
 * - Qt requires that glew is included after <qopengl.h> and <QOpenGLFunctions>
 * - QOpenGLWindow includes <qopengl.h> (via QOpenGLContext)
 * - qopengl.h includes OpenGL/gl.h
 *
 * therefore
 * - glew wants to be included first
 * - and so does QOpenGLWindow
 *
 * Since including glew before QOpenGLWindow only generates a warning and does not seem to incur any ill effects,
 * we silence the warning here.
 */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-W#warnings"
#include <QOpenGLWindow>
#pragma clang diagnostic pop

#include <QElapsedTimer>

#undef Bool
#undef Status
#undef CursorShape

namespace TrenchBroom {
    namespace Renderer {
        class FontManager;
        class RenderContext;
        class ShaderManager;
    }

    namespace View {
        class GLContextManager;

        class RenderView : public QOpenGLWindow, public InputEventProcessor {
            Q_OBJECT
        private:
            Color m_focusColor;
            GLContextManager* m_glContext;
            InputEventRecorder m_eventRecorder;
        private: // FPS counter
            // stats since the last counter update
            int m_framesRendered;
            int m_maxFrameTimeMsecs;
            // other
            int64_t m_lastFPSCounterUpdate;
            QElapsedTimer m_timeSinceLastFrame;
        protected:
            String m_currentFPS;

        private:
            QWidget* m_windowContainer;
        protected:
            explicit RenderView(GLContextManager& contextManager);
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
        public:
            QWidget* widgetContainer() const;
        public: // wxWidgets compat
            bool hasFocus() const;
            bool IsBeingDeleted() const;
            void Refresh();
            void update();
        protected:
            Renderer::Vbo& vertexVbo();
            Renderer::Vbo& indexVbo();
            Renderer::FontManager& fontManager();
            Renderer::ShaderManager& shaderManager();

            int depthBits() const;
            bool multisample() const;

        protected: // QOpenGLWindow overrides
            void initializeGL() override;
            void paintGL() override;
            void resizeGL(int w, int h) override;
        private:
            void render();
            void processInput();
            void clearBackground();
            void renderFocusIndicator();
        private:
            virtual void doUpdateViewport(int x, int y, int width, int height);
            virtual bool doShouldRenderFocusIndicator() const = 0;
            virtual void doRender() = 0;
        };
    }
}

#endif /* defined(TrenchBroom_RenderView) */
