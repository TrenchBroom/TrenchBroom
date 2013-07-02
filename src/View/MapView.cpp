/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "MapView.h"

#include "Exceptions.h"
#include "Preferences.h"
#include "View/Console.h"

#include <wx/dcclient.h>

namespace TrenchBroom {
    namespace View {
        MapView::MapView(wxWindow* parent, Console& console) :
        wxGLCanvas(parent, wxID_ANY, attribs()),
        m_initialized(false),
        m_console(console),
        m_glContext(new wxGLContext(this)) {
            Bind(wxEVT_PAINT, &MapView::OnPaint, this);
        }
        
        MapView::~MapView() {
            delete m_glContext;
            m_glContext = NULL;
        }
        
        void MapView::makeCurrent() {
            if (!SetCurrent(*m_glContext))
                throw RenderException("Failed to set current OpenGL context");
        }

        void MapView::OnPaint(wxPaintEvent& event) {
            if (!IsShownOnScreen())
                return;
            
            if (!m_initialized)
                initializeGL();
            
            wxPaintDC(this);
			if (SetCurrent(*m_glContext)) {
                glEnable(GL_MULTISAMPLE);
                
                PreferenceManager& prefs = PreferenceManager::instance();
                const Color& backgroundColor = prefs.getColor(Preferences::BackgroundColor);
                
				glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a());
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
				glDisableClientState(GL_VERTEX_ARRAY);
				glDisableClientState(GL_COLOR_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDisable(GL_TEXTURE_2D);
                
                SwapBuffers();
            }
        }

        void MapView::initializeGL() {
            if (SetCurrent(*m_glContext)) {
                const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
                const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
                const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
                m_console.info("Renderer info: %s version %s from %s", renderer, version, vendor);
                m_console.info("Depth buffer bits: %d", depthBits());
                
                if (multisample())
                    m_console.info("Multisampling enabled");
                else
                    m_console.info("Multisampling disabled");
                
#ifndef TESTING
                glewExperimental = GL_TRUE;
                const GLenum glewState = glewInit();
                if (glewState != GLEW_OK)
                    m_console.error("Unable to initialize glew: %s", glewGetErrorString(glewState));
#endif
            } else {
                m_console.info("Unable to set current GL context");
            }
            m_initialized = true;
        }
        
        const int* MapView::attribs() {
            typedef std::vector<int> Attribs;

            static bool initialized = false;
            static Attribs attribs;
            if (initialized)
                return &attribs[0];

            int testAttribs[] =
            {
                // 32 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 24 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 32 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 24 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 16 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 16 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 32 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                0,
                // 24 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                0,
                // 16 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                0,
                0,
            };

            size_t index = 0;
            while (!initialized && testAttribs[index] != 0) {
                size_t count = 0;
                for (; testAttribs[index + count] != 0; count++);
                if (wxGLCanvas::IsDisplaySupported(&testAttribs[index])) {
                    for (size_t i = 0; i < count; i++)
                        attribs.push_back(testAttribs[index + i]);
                    attribs.push_back(0);
                    initialized = true;
                }
                index += count + 1;
            }

            assert(initialized);
            assert(!attribs.empty());
            return &attribs[0];
        }

        int MapView::depthBits() {
            return attribs()[3];
        }
        
        bool MapView::multisample() {
            return attribs()[4] != 0;
        }
    }
}
