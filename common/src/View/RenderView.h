/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "View/GLAttribs.h"
#include "View/GLContext.h"

#include <wx/glcanvas.h>

namespace TrenchBroom {
    namespace Renderer {
        class FontManager;
        class RenderContext;
        class ShaderManager;
    }

    namespace View {
        class GLContextManager;
        
        class RenderView : public wxGLCanvas {
        private:
            GLContext::Ptr m_glContext;
            GLAttribs m_attribs;
            bool m_initialized;
            Color m_focusColor;
        protected:
            RenderView(wxWindow* parent, GLContextManager& contextManager, const GLAttribs& attribs);
        public:
            void OnPaint(wxPaintEvent& event);
            void OnSize(wxSizeEvent& event);
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
        protected:
            Renderer::Vbo& vertexVbo();
            Renderer::Vbo& indexVbo();
            Renderer::FontManager& fontManager();
            Renderer::ShaderManager& shaderManager();
            
            int depthBits() const;
            bool multisample() const;
        private:
            void bindEvents();

            void initializeGL();
            void updateViewport();
            void render();
            void clearBackground();
            void renderFocusIndicator();
        private:
            virtual void doInitializeGL(bool firstInitialization);
            virtual void doUpdateViewport(int x, int y, int width, int height);
            virtual bool doShouldRenderFocusIndicator() const = 0;
            virtual void doRender() = 0;
        };
    }
}

#endif /* defined(TrenchBroom_RenderView) */
