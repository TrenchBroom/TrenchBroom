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
#include "View/DragAndDrop.h"

#include <wx/wx.h>
#include <wx/dnd.h>
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
        protected:
            inline wxGLContext* glContext() const {
                return m_glContext;
            }
            
            virtual void doInitLayout(Layout& layout) = 0;
            virtual void doReloadLayout(Layout& layout) = 0;
            virtual void doClear() {}
            virtual void doRender(Layout& layout, float y, float height) = 0;
            virtual void handleLeftClick(Layout& layout, float x, float y) {}
            virtual bool dndEnabled() { return false; }
            virtual wxImage* dndImage(const typename Layout::Group::Row::Cell& cell) { return NULL; }
            virtual wxDataObject* dndData(const typename Layout::Group::Row::Cell& cell) { return NULL; }
        public:
            CellLayoutGLCanvas(wxWindow* parent, wxWindowID windowId, const int* attribs, wxGLContext* sharedContext, wxScrollBar* scrollBar = NULL) :
            wxGLCanvas(parent, windowId, attribs, wxDefaultPosition, wxDefaultSize),
            m_layoutInitialized(false),
            m_scrollBar(scrollBar) {
                m_glContext = new wxGLContext(this, sharedContext);
                
                Bind(wxEVT_PAINT, &CellLayoutGLCanvas::OnPaint, this);
                Bind(wxEVT_SIZE, &CellLayoutGLCanvas::OnSize, this);
                Bind(wxEVT_LEFT_UP, &CellLayoutGLCanvas::OnMouseLeftUp, this);
                Bind(wxEVT_MOTION, &CellLayoutGLCanvas::OnMouseMove, this);
                
                if (m_scrollBar != NULL) {
                    m_scrollBar->Bind(wxEVT_SCROLL_LINEUP, &CellLayoutGLCanvas::OnScrollBarLineUp, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_LINEDOWN, &CellLayoutGLCanvas::OnScrollBarLineDown, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_PAGEUP, &CellLayoutGLCanvas::OnScrollBarPageUp, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_PAGEDOWN, &CellLayoutGLCanvas::OnScrollBarPageDown, this);
                    
                    m_scrollBar->Bind(wxEVT_SCROLL_TOP, &CellLayoutGLCanvas::OnScrollBarChange, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_BOTTOM, &CellLayoutGLCanvas::OnScrollBarChange, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_THUMBTRACK, &CellLayoutGLCanvas::OnScrollBarChange, this);
                    
                    Bind(wxEVT_MOUSEWHEEL, &CellLayoutGLCanvas::OnMouseWheel, this);
                }
            }
            
            virtual ~CellLayoutGLCanvas() {
                if (m_glContext != NULL) {
                    wxDELETE(m_glContext);
                    m_glContext = NULL;
                }
            }
            
            void reload() {
                reloadLayout();
                Refresh();
            }
            
            void clear() {
                m_layout.clear();
                doClear();
            }
            
            void OnPaint(wxPaintEvent& event) {
                if (!m_layoutInitialized)
                    initLayout();
                
                Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
                const Color& backgroundColor = prefs.getColor(Preferences::BackgroundColor);

                /* This prevents a minor flickering issue when resizing the canvas on OS X, but it introduces a worse
                   issue on Windows.
                wxColour wxBackgroundColor(static_cast<unsigned char>(backgroundColor.x * 0xFF),
                                           static_cast<unsigned char>(backgroundColor.y * 0xFF),
                                           static_cast<unsigned char>(backgroundColor.z * 0xFF),
                                           static_cast<unsigned char>(backgroundColor.w * 0xFF));
                
                wxPaintDC dc(this);
                dc.SetPen(wxPen(wxBackgroundColor));
                dc.SetBrush(wxBrush(wxBackgroundColor));
                dc.DrawRectangle(GetRect());
                */
                 
                if (SetCurrent(*m_glContext)) {
                    glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    int top = m_scrollBar != NULL ? m_scrollBar->GetThumbPosition() : 0;
                    wxRect visibleRect = wxRect(wxPoint(0, top), GetClientSize());
                    
                    float y = static_cast<float>(visibleRect.GetY());
                    float height = static_cast<float>(visibleRect.GetHeight());
                    
                    GLint viewLeft      = static_cast<GLint>(GetClientRect().GetLeft());
                    GLint viewTop       = static_cast<GLint>(GetClientRect().GetBottom());
                    GLint viewRight     = static_cast<GLint>(GetClientRect().GetRight());
                    GLint viewBottom    = static_cast<GLint>(GetClientRect().GetTop());
                    glViewport(viewLeft, viewBottom, viewRight - viewLeft, viewTop - viewBottom);

                    doRender(m_layout, y, height);
                    
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
            
            void OnScrollBarLineUp(wxScrollEvent& event) {
                float top = static_cast<float>(m_scrollBar->GetThumbPosition());
                m_scrollBar->SetThumbPosition(static_cast<int>(m_layout.rowPosition(top, -1)));
                Refresh();
            }
            
            void OnScrollBarLineDown(wxScrollEvent& event) {
                float top = static_cast<float>(m_scrollBar->GetThumbPosition());
                m_scrollBar->SetThumbPosition(static_cast<int>(m_layout.rowPosition(top, 1)));
                Refresh();
            }
            
            void OnScrollBarPageUp(wxScrollEvent& event) {
                float top = static_cast<float>(m_scrollBar->GetThumbPosition());
                float height = static_cast<float>(GetClientSize().y);
                m_scrollBar->SetThumbPosition(static_cast<int>(m_layout.rowPosition(std::max(0.0f, top - height), 0)));
                Refresh();
            }
            
            void OnScrollBarPageDown(wxScrollEvent& event) {
                float top = static_cast<float>(m_scrollBar->GetThumbPosition());
                m_scrollBar->SetThumbPosition(static_cast<int>(m_layout.rowPosition(top, 0)));
                Refresh();
            }
            
            void OnMouseMove(wxMouseEvent& event) {
                if (event.LeftIsDown() && dndEnabled()) {
                    int top = m_scrollBar != NULL ? m_scrollBar->GetThumbPosition() : 0;
                    float x = static_cast<float>(event.GetX());
                    float y = static_cast<float>(event.GetY() + top);
                    const typename Layout::Group::Row::Cell* cell = NULL;
                    if (m_layout.cellAt(x, y, &cell)) {
                        wxImage* feedbackImage = dndImage(*cell);
                        wxDataObject* dropData = dndData(*cell);
                        
                        int xOffset = event.GetX() - static_cast<int>(cell->itemBounds().left());
                        int yOffset = event.GetY() - static_cast<int>(cell->itemBounds().top()) + top;
                        
                        DropSource dropSource(this, *feedbackImage, wxPoint(xOffset, yOffset));
                        dropSource.SetData(*dropData);
                        dropSource.DoDragDrop();
                        
                        delete feedbackImage;
                        delete dropData;
                    }
                }
            }
            
            void OnMouseLeftUp(wxMouseEvent& event) {
                int top = m_scrollBar != NULL ? m_scrollBar->GetThumbPosition() : 0;
                float x = static_cast<float>(event.GetX());
                float y = static_cast<float>(event.GetY() + top);
                handleLeftClick(m_layout, x, y);
            }
            
            void OnMouseWheel(wxMouseEvent& event) {
                if (m_scrollBar != NULL) {
                    float top = static_cast<float>(m_scrollBar->GetThumbPosition());
                    if (event.GetWheelRotation() < 0)
                        m_scrollBar->SetThumbPosition(static_cast<int>(m_layout.rowPosition(top, 1)));
                    else
                        m_scrollBar->SetThumbPosition(static_cast<int>(m_layout.rowPosition(top, -1)));
                    Refresh();
                }
            }
        };
    }
}

#endif
