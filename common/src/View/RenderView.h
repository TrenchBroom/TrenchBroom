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

#ifndef __TrenchBroom__RenderView__
#define __TrenchBroom__RenderView__

#include "Color.h"
#include "Hit.h"
#include "Renderer/Vbo.h"
#include "View/GLContextHolder.h"

#include <wx/glcanvas.h>

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
    }

    namespace View {
        class RenderView : public wxGLCanvas {
        private:
            GLContextHolder::Ptr m_contextHolder;
            bool m_initialized;
            Color m_focusColor;
        protected:
            Renderer::Vbo m_vbo;
        protected:
            RenderView(wxWindow* parent, const GLContextHolder::GLAttribs& attribs);
            RenderView(wxWindow* parent, GLContextHolder::Ptr sharedContext);
        public:
            void OnPaint(wxPaintEvent& event);
            void OnSize(wxSizeEvent& event);
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);

            const GLContextHolder::Ptr contextHolder() const;
        private:
            void bindEvents();

            void initializeGL();
            void updateViewport();
            void render();
            void clearBackground();
            void renderFocusIndicator();
        private:
            virtual void doInitializeGL();
            virtual void doUpdateViewport(int x, int y, int width, int height);
            virtual void doRender() = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__RenderView__) */
