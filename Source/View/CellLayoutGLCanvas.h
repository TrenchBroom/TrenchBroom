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

#ifndef TrenchBroom_CellLayoutGLCanvas_h
#define TrenchBroom_CellLayoutGLCanvas_h

#include "GL/glew.h"
#include <wx/glcanvas.h>
#include <wx/wx.h>

#include "GL/Capabilities.h"
#include "Renderer/Text/FontDescriptor.h"
#include "Utility/Preferences.h"
#include "View/CellLayout.h"

namespace TrenchBroom {
    namespace View {
        template <typename CellData, typename GroupData>
        class CellLayoutGLCanvas : public wxGLCanvas {
        protected:
            typedef CellLayout<CellData, GroupData> Layout;
        private:
            Layout m_layout;
            typename Layout::Group::Row::Cell* m_selectedCell;
            bool m_layoutInitialized;
            
            wxGLContext* m_glContext;
            int* m_attribs;

            void initLayout() {
                doInitLayout(m_layout);
                m_layoutInitialized = true;
            }
            
            void reloadLayout() {
                if (!m_layoutInitialized)
                    initLayout();

                m_layout.setWidth(static_cast<float>(GetClientSize().x));
                m_layout.clear();
                doReloadLayout(m_layout);
                
                int height = static_cast<int>(m_layout.height());
                
                int position = GetScrollPos(wxVERTICAL);
                SetScrollbar(wxVERTICAL, position, GetClientSize().y, height);
            }

            int* Attribs() {
                GL::Capabilities capabilities = GL::glCapabilities();
                if (capabilities.multisample) {
                    m_attribs = new int[9];
                    m_attribs[0] = WX_GL_RGBA;
                    m_attribs[1] = WX_GL_DOUBLEBUFFER;
                    m_attribs[2] = WX_GL_SAMPLE_BUFFERS;
                    m_attribs[3] = 1;
                    m_attribs[4] = WX_GL_SAMPLES;
                    m_attribs[5] = capabilities.samples;
                    m_attribs[6] = WX_GL_DEPTH_SIZE;
                    m_attribs[7] = capabilities.depthBits;
                    m_attribs[8] = 0;
                } else {
                    m_attribs = new int[5];
                    m_attribs[0] = WX_GL_RGBA;
                    m_attribs[1] = WX_GL_DOUBLEBUFFER;
                    m_attribs[2] = WX_GL_DEPTH_SIZE;
                    m_attribs[3] = capabilities.depthBits;
                    m_attribs[4] = 0;
                }
                
                return m_attribs;
            }
        protected:
            virtual void doInitLayout(Layout& layout) = 0;
            virtual void doReloadLayout(Layout& layout) = 0;
            virtual void doRender(Layout& layout, const wxRect& rect) = 0;
        public:
            CellLayoutGLCanvas(wxWindow* parent, wxGLContext* sharedContext, int flags = wxBORDER_SUNKEN) :
            wxGLCanvas(parent, wxID_ANY, Attribs(), wxDefaultPosition, wxDefaultSize, flags | wxVSCROLL),
            m_layoutInitialized(false) {
                m_glContext = new wxGLContext(this, sharedContext);
                delete [] m_attribs;
                m_attribs = NULL;
                
                Bind(wxEVT_PAINT, &CellLayoutGLCanvas::OnPaint, this);
                Bind(wxEVT_SIZE, &CellLayoutGLCanvas::OnSize, this);
            }
            
            virtual ~CellLayoutGLCanvas() {
                if (m_glContext != NULL) {
                    wxDELETE(m_glContext);
                    m_glContext = NULL;
                }
                if (m_attribs != NULL) {
                    delete [] m_attribs;
                    m_attribs = NULL;
                }
            }
            
            void reload() {
                reloadLayout();
            }
            
            void OnPaint(wxPaintEvent& event) {
                if (!m_layoutInitialized)
                    initLayout();
                
                wxPaintDC(this);
                if (SetCurrent(*m_glContext)) {
                    Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
                    const Color& backgroundColor = prefs.getColor(Preferences::BackgroundColor);
                    glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    int top = GetScrollPos(wxVERTICAL);
                    doRender(m_layout, wxRect(wxPoint(0, top), GetClientSize()));
                    
                    SwapBuffers();
                }
            }
            
            void OnSize(wxSizeEvent& event) {
                reloadLayout();
            }
        };
    }
}

#endif
