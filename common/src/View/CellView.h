/*
 Copyright (C) 2010-2016 Kristian Duske

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

#ifndef TrenchBroom_CellView_h
#define TrenchBroom_CellView_h

#include "Renderer/RenderUtils.h"
#include "Renderer/Transformation.h"
#include "Renderer/FontDescriptor.h"
#include "Preferences.h"
#include "View/CellLayout.h"
#include "View/DragAndDrop.h"
#include "View/GLAttribs.h"
#include "View/RenderView.h"

#include <wx/wx.h>
#include <wx/dnd.h>
#include <wx/event.h>

namespace TrenchBroom {
    namespace View {
        class GLContextManager;
        
        template <typename CellData, typename GroupData>
        class CellView : public RenderView {
        protected:
            typedef CellLayout<CellData, GroupData> Layout;
        private:
            Layout m_layout;
            typename Layout::Group::Row::Cell* m_selectedCell;
            bool m_layoutInitialized;
            
            bool m_valid;

            wxScrollBar* m_scrollBar;
            wxPoint m_lastMousePos;

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
                initLayout(); // always initialize the layout when reloading

                m_layout.clear();
                doReloadLayout(m_layout);
                updateScrollBar();

                m_valid = true;
            }
            
            void validate() {
                if (!m_valid)
                    reloadLayout();
            }
        public:
            CellView(wxWindow* parent, GLContextManager& contextManager, wxGLAttributes attribs, wxScrollBar* scrollBar = NULL) :
            RenderView(parent, contextManager, attribs),
            m_layoutInitialized(false),
            m_valid(false),
            m_scrollBar(scrollBar) {
                Bind(wxEVT_SIZE, &CellView::OnSize, this);
                Bind(wxEVT_LEFT_UP, &CellView::OnMouseLeftUp, this);
                Bind(wxEVT_RIGHT_DOWN, &CellView::OnMouseRightDown, this);
                Bind(wxEVT_RIGHT_UP, &CellView::OnMouseRightUp, this);
                Bind(wxEVT_MOTION, &CellView::OnMouseMove, this);
                Bind(wxEVT_MOUSE_CAPTURE_LOST, &CellView::OnMouseCaptureLost, this);

                if (m_scrollBar != NULL) {
                    m_scrollBar->Bind(wxEVT_SCROLL_LINEUP, &CellView::OnScrollBarLineUp, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_LINEDOWN, &CellView::OnScrollBarLineDown, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_PAGEUP, &CellView::OnScrollBarPageUp, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_PAGEDOWN, &CellView::OnScrollBarPageDown, this);

                    m_scrollBar->Bind(wxEVT_SCROLL_TOP, &CellView::OnScrollBarChange, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_BOTTOM, &CellView::OnScrollBarChange, this);
                    m_scrollBar->Bind(wxEVT_SCROLL_THUMBTRACK, &CellView::OnScrollBarChange, this);

                    Bind(wxEVT_MOUSEWHEEL, &CellView::OnMouseWheel, this);
                }
            }

            void invalidate() {
                m_valid = false;
            }
            
            void clear() {
                m_layout.clear();
                doClear();
                m_valid = true;
            }

            void OnSize(wxSizeEvent& event) {
                m_layout.setWidth(static_cast<float>(GetClientSize().x));
                updateScrollBar();
                event.Skip();
            }

            void OnScrollBarChange(wxScrollEvent& event) {
                Refresh();
                event.Skip();
            }

            void OnScrollBarLineUp(wxScrollEvent& event) {
                float top = static_cast<float>(m_scrollBar->GetThumbPosition());
                m_scrollBar->SetThumbPosition(static_cast<int>(m_layout.rowPosition(top, -1)));
                Refresh();
                event.Skip();
            }

            void OnScrollBarLineDown(wxScrollEvent& event) {
                float top = static_cast<float>(m_scrollBar->GetThumbPosition());
                m_scrollBar->SetThumbPosition(static_cast<int>(m_layout.rowPosition(top, 1)));
                Refresh();
                event.Skip();
            }

            void OnScrollBarPageUp(wxScrollEvent& event) {
                float top = static_cast<float>(m_scrollBar->GetThumbPosition());
                float height = static_cast<float>(GetClientSize().y);
                m_scrollBar->SetThumbPosition(static_cast<int>(m_layout.rowPosition(std::max(0.0f, top - height), 0)));
                Refresh();
                event.Skip();
            }

            void OnScrollBarPageDown(wxScrollEvent& event) {
                float top = static_cast<float>(m_scrollBar->GetThumbPosition());
                m_scrollBar->SetThumbPosition(static_cast<int>(m_layout.rowPosition(top, 0)));
                Refresh();
                event.Skip();
            }

            class DndHelper {
            private:
                CellView& m_cellView;
            public:
                DndHelper(CellView& cellView) :
                m_cellView(cellView) {
                    m_cellView.dndWillStart();
                }
                
                ~DndHelper() {
                    m_cellView.dndDidEnd();
                }
            };
            
            void OnMouseMove(wxMouseEvent& event) {
                if (event.LeftIsDown()) {
                    startDrag(event);
                } else if (event.RightIsDown() && event.AltDown()) {
                    scroll(event);
                } else {
                    updateTooltip(event);
                }
                m_lastMousePos = event.GetPosition();
            }

            void startDrag(const wxMouseEvent& event) {
                if (dndEnabled()) {
                    int top = m_scrollBar != NULL ? m_scrollBar->GetThumbPosition() : 0;
                    float x = static_cast<float>(event.GetX());
                    float y = static_cast<float>(event.GetY() + top);
                    const typename Layout::Group::Row::Cell* cell = NULL;
                    if (m_layout.cellAt(x, y, &cell)) {
                        /*
                         wxImage* feedbackImage = dndImage(*cell);
                         int xOffset = event.GetX() - static_cast<int>(cell->itemBounds().left());
                         int yOffset = event.GetY() - static_cast<int>(cell->itemBounds().top()) + top;
                         */
                        
                        const DndHelper dndHelper(*this);
                        wxTextDataObject dropData(dndData(*cell));
                        DropSource dropSource(dropData, this);
                        dropSource.DoDragDrop();
                    }
                }
            }
            
            void scroll(const wxMouseEvent& event) {
                if (m_scrollBar != NULL) {
                    const wxPoint mousePosition = event.GetPosition();
                    const wxCoord delta = mousePosition.y - m_lastMousePos.y;
                    const wxCoord newThumbPosition = m_scrollBar->GetThumbPosition() - delta;
                    m_scrollBar->SetThumbPosition(newThumbPosition);
                    Refresh();
                }
            }
            
            void updateTooltip(const wxMouseEvent& event) {
                int top = m_scrollBar != NULL ? m_scrollBar->GetThumbPosition() : 0;
                float x = static_cast<float>(event.GetX());
                float y = static_cast<float>(event.GetY() + top);
                const typename Layout::Group::Row::Cell* cell = NULL;
                if (m_layout.cellAt(x, y, &cell))
                    SetToolTip(tooltip(*cell));
                else
                    SetToolTip("");
            }
            
            void OnMouseRightDown(wxMouseEvent& event) {
                if (event.AltDown()) {
                    m_lastMousePos = event.GetPosition();
                    CaptureMouse();
                }
            }
            
            void OnMouseRightUp(wxMouseEvent& event) {
                if (HasCapture())
                    ReleaseMouse();
            }
            
            void OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            }
            
            void OnMouseLeftUp(wxMouseEvent& event) {
                int top = m_scrollBar != NULL ? m_scrollBar->GetThumbPosition() : 0;
                float x = static_cast<float>(event.GetX());
                float y = static_cast<float>(event.GetY() + top);
                doLeftClick(m_layout, x, y);
            }

            void OnMouseWheel(wxMouseEvent& event) {
                if (m_scrollBar != NULL) {
                    const float top = static_cast<float>(m_scrollBar->GetThumbPosition());
                    float newTop = event.GetWheelRotation() < 0 ? m_layout.rowPosition(top, 1) : m_layout.rowPosition(top, -1);
                    newTop = std::max(0.0f, std::ceil(newTop - m_layout.rowMargin()));

                    m_scrollBar->SetThumbPosition(static_cast<int>(newTop));
                    Refresh();
                }
            }
        private:
            void doRender() {
                if (!m_valid)
                    validate();
                if (!m_layoutInitialized)
                    initLayout();

                const int top = m_scrollBar != NULL ? m_scrollBar->GetThumbPosition() : 0;
                const wxRect visibleRect = wxRect(wxPoint(0, top), GetClientSize());
                
                const float y = static_cast<float>(visibleRect.GetY());
                const float height = static_cast<float>(visibleRect.GetHeight());
                
                const GLint viewLeft      = static_cast<GLint>(GetClientRect().GetLeft());
                const GLint viewTop       = static_cast<GLint>(GetClientRect().GetBottom());
                const GLint viewRight     = static_cast<GLint>(GetClientRect().GetRight());
                const GLint viewBottom    = static_cast<GLint>(GetClientRect().GetTop());
                glViewport(viewLeft, viewBottom, viewRight - viewLeft, viewTop - viewBottom);

                setupGL();
                doRender(m_layout, y, height);
            }
            
            void setupGL() {
                glEnable(GL_MULTISAMPLE);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_CULL_FACE);
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LEQUAL);
                glShadeModel(GL_SMOOTH);
            }
            
            virtual void doInitLayout(Layout& layout) = 0;
            virtual void doReloadLayout(Layout& layout) = 0;
            virtual void doClear() {}
            virtual void doRender(Layout& layout, float y, float height) = 0;
            virtual void doLeftClick(Layout& layout, float x, float y) {}
            
            virtual bool dndEnabled() { return false; }
            virtual void dndWillStart() {}
            virtual void dndDidEnd() {}
            virtual wxImage dndImage(const typename Layout::Group::Row::Cell& cell) { assert(false); return wxImage(); }
            virtual wxString dndData(const typename Layout::Group::Row::Cell& cell) { assert(false); return ""; }
            virtual wxString tooltip(const typename Layout::Group::Row::Cell& cell) { return ""; }
        };
    }
}

#endif
