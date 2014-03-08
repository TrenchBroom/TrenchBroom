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

#include "RenderView.h"
#include "Renderer/Camera.h"

#include <wx/dcclient.h>

namespace TrenchBroom {
    namespace View {
        RenderView::RenderView(wxWindow* parent, const GLAttribs& attribs, const wxGLContext* sharedContext) :
        wxGLCanvas(parent, wxID_ANY, &attribs.front()),
        m_glContext(new wxGLContext(this, sharedContext)),
        m_initialized(false)  {
            bindEvents();
        }
        
        RenderView::~RenderView() {
            delete m_glContext;
            m_glContext = NULL;
        }
        
        void RenderView::OnPaint(wxPaintEvent& event) {
#ifndef TESTING
            if (!IsShownOnScreen())
                return;
            
            if (!m_initialized)
                initializeGL();
            
            if (SetCurrent(*m_glContext)) {
                wxPaintDC paintDC(this);
                render();
                SwapBuffers();
            }
#endif
        }
        
        void RenderView::OnSize(wxSizeEvent& event) {
            updateViewport();
            event.Skip();
        }

        const wxGLContext* RenderView::glContext() const {
            return m_glContext;
        }
        
        void RenderView::bindEvents() {
            Bind(wxEVT_PAINT, &RenderView::OnPaint, this);
            Bind(wxEVT_SIZE, &RenderView::OnSize, this);
        }

        void RenderView::initializeGL() {
            if (SetCurrent(*m_glContext))
                doInitializeGL();
            m_initialized = true;
        }

        void RenderView::updateViewport() {
            const wxSize clientSize = GetClientSize();
            doUpdateViewport(0, 0, clientSize.x, clientSize.y);
        }
        
        void RenderView::render() {
            doRender();
        }
        
        void RenderView::doInitializeGL() {}
    }
}
