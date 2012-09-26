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

#include "GL/Capabilities.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Transformation.h"
#include "Renderer/Text/FontDescriptor.h"
#include "Utility/Preferences.h"
#include "View/CellLayout.h"

#include <wx/wx.h>
#include <wx/event.h>
#include <wx/glcanvas.h>

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

            wxScrollBar* m_scrollBar;
            
            void updateScrollBar() {
                if (m_scrollBar != NULL) {
                    int position = m_scrollBar->GetThumbPosition();
                    int thumbSize = GetClientSize().y;
                    int range = static_cast<int>(m_layout.height());
                    m_scrollBar->SetScrollbar(position, thumbSize, range, thumbSize);
                }
            }
            
            void initLayout() {
                doInitLayout(m_layout);
                m_layoutInitialized = true;
            }
            
            void reloadLayout() {
                if (!m_layoutInitialized)
                    initLayout();

                m_layout.clear();
                doReloadLayout(m_layout);
                updateScrollBar();
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
            virtual void doRender(Layout& layout, Renderer::Transformation& transformation, float y, float height) = 0;
            virtual void handleLeftClick(Layout& layout, float x, float y) {}
        public:
            CellLayoutGLCanvas(wxWindow* parent, wxWindowID windowId, wxGLContext* sharedContext, wxScrollBar* scrollBar = NULL) :
            wxGLCanvas(parent, windowId, Attribs(), wxDefaultPosition, wxDefaultSize),
            m_layoutInitialized(false),
            m_scrollBar(scrollBar) {
                m_glContext = new wxGLContext(this, sharedContext);
                delete [] m_attribs;
                m_attribs = NULL;
                
                Bind(wxEVT_PAINT, &CellLayoutGLCanvas::OnPaint, this);
                Bind(wxEVT_SIZE, &CellLayoutGLCanvas::OnSize, this);
                Bind(wxEVT_LEFT_UP, &CellLayoutGLCanvas::OnMouseLeftUp, this);
                
                if (m_scrollBar != NULL) {
                    m_scrollBar->Bind(wxEVT_SCROLL_TOP, &CellLayoutGLCanvas::OnScrollBarChange, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_BOTTOM, &CellLayoutGLCanvas::OnScrollBarChange, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_LINEUP, &CellLayoutGLCanvas::OnScrollBarChange, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_LINEDOWN, &CellLayoutGLCanvas::OnScrollBarChange, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_PAGEUP, &CellLayoutGLCanvas::OnScrollBarChange, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_PAGEDOWN, &CellLayoutGLCanvas::OnScrollBarChange, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_THUMBTRACK, &CellLayoutGLCanvas::OnScrollBarChange, this);
                    
                    Bind(wxEVT_MOUSEWHEEL, &CellLayoutGLCanvas::OnMouseWheel, this);
                }
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
                Refresh();
            }
            
            void clear() {
                m_layout.clear();
            }
            
            void OnPaint(wxPaintEvent& event) {
                if (!m_layoutInitialized)
                    initLayout();
                
                Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
                const Color& backgroundColor = prefs.getColor(Preferences::BackgroundColor);

                wxColour wxBackgroundColor(static_cast<unsigned char>(backgroundColor.x * 0xFF),
                                           static_cast<unsigned char>(backgroundColor.y * 0xFF),
                                           static_cast<unsigned char>(backgroundColor.z * 0xFF),
                                           static_cast<unsigned char>(backgroundColor.w * 0xFF));
                
                wxPaintDC dc(this);
                dc.SetPen(wxPen(wxBackgroundColor));
                dc.SetBrush(wxBrush(wxBackgroundColor));
                dc.DrawRectangle(GetRect());
                
                if (SetCurrent(*m_glContext)) {
                    glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    int top = m_scrollBar != NULL ? m_scrollBar->GetThumbPosition() : 0;
                    wxRect visibleRect = wxRect(wxPoint(0, top), GetClientSize());
                    
                    float y = static_cast<float>(visibleRect.GetY());
                    float height = static_cast<float>(visibleRect.GetHeight());
                    
                    float viewLeft      = static_cast<float>(GetClientRect().GetLeft());
                    float viewTop       = static_cast<float>(GetClientRect().GetBottom());
                    float viewRight     = static_cast<float>(GetClientRect().GetRight());
                    float viewBottom    = static_cast<float>(GetClientRect().GetTop());
                    
                    Mat4f projection;
                    projection.setOrtho(-1.0f, 1.0f, viewLeft, viewTop, viewRight, viewBottom);
                    
                    Mat4f view;
                    view.setView(Vec3f::NegZ, Vec3f::PosY);
                    view.translate(Vec3f(0.0f, 0.0f, 0.1f));
                    
                    glViewport(viewLeft, viewBottom, viewRight - viewLeft, viewTop - viewBottom);
                    Renderer::Transformation transformation(projection * view, true);

                    doRender(m_layout, transformation, y, height);
                    
                    SwapBuffers();
                }
            }
            
            void OnSize(wxSizeEvent& event) {
                m_layout.setWidth(static_cast<float>(GetClientSize().x));
                updateScrollBar();
            }
            
            void OnScrollBarChange(wxScrollEvent& event) {
                Refresh();
            }
            
            void OnMouseLeftUp(wxMouseEvent& event) {
                int top = m_scrollBar != NULL ? m_scrollBar->GetThumbPosition() : 0;
                float x = static_cast<float>(event.GetX());
                float y = static_cast<float>(event.GetY() + top);
                handleLeftClick(m_layout, x, y);
            }
            
            void OnMouseWheel(wxMouseEvent& event) {
                if (m_scrollBar != NULL) {
                    int lines = event.GetLinesPerAction();
                    float delta = static_cast<float>(event.GetWheelRotation()) / lines;
                    m_scrollBar->SetThumbPosition(m_scrollBar->GetThumbPosition() - static_cast<int>(delta));
                    Refresh();
                }
            }
        };
    }
}

#endif
