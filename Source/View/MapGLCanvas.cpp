/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapGLCanvas.h"

#include "Utility/GLee.h"

#include <wx/wx.h>

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(MapGLCanvas, wxGLCanvas)
        EVT_PAINT(MapGLCanvas::OnPaint)
        END_EVENT_TABLE()
        
        int* MapGLCanvas::Attribs() {
            m_attribs = new int[10];
            m_attribs[0] = WX_GL_RGBA;
            m_attribs[1] = WX_GL_DOUBLEBUFFER;
            m_attribs[2] = WX_GL_SAMPLE_BUFFERS;
            m_attribs[3] = GL_TRUE;
            m_attribs[4] = WX_GL_SAMPLES;
            m_attribs[5] = 4;
            m_attribs[6] = WX_GL_DEPTH_SIZE;
            m_attribs[7] = 32;
            m_attribs[8] = 0;
            m_attribs[9] = 0;
            return m_attribs;
        }
        
        MapGLCanvas::MapGLCanvas(wxWindow* parent) : wxGLCanvas(parent, wxID_ANY, Attribs()) {
            m_glContext = new wxGLContext(this);
        }

        MapGLCanvas::~MapGLCanvas() {
            delete [] m_attribs;
        }
        
        void MapGLCanvas::OnPaint(wxPaintEvent& event) {
            SetCurrent(*m_glContext);
            wxPaintDC(this);
            
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, static_cast<GLint>(GetSize().x), static_cast<GLint>(GetSize().y));
            
            SwapBuffers();
        }
    }
}