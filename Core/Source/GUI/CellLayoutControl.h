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

#ifndef TrenchBroom_CellLayoutControl_h
#define TrenchBroom_CellLayoutControl_h

#include "Gwen/Controls/Base.h"
#include "Gwen/Skin.h"
#include "GL/GLee.h"
#include "GUI/CellLayout.h"

namespace TrenchBroom {
    namespace Gui {
        
        template<typename CellData>
        class CellDragControl : public Gwen::Controls::Base {
        protected:
            typename CellRow<CellData>::CellPtr m_cell;
            bool m_overlayVisible;
        public:
            CellDragControl(Gwen::Controls::Base* parent, typename CellRow<CellData>::CellPtr cell) : Base(parent), m_cell(cell), m_overlayVisible(true) {}
            virtual ~CellDragControl() {}
            
            virtual void Render(Gwen::Skin::Base* skin) {
                if (m_overlayVisible) {
                    const Gwen::Point& offset = skin->GetRender()->GetRenderOffset();
                    const LayoutBounds& itemBounds = m_cell->itemBounds();
                    
                    glPushAttrib(GL_ENABLE_BIT);
                    glDisable(GL_SCISSOR_TEST);
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glTranslatef(offset.x - itemBounds.left(), offset.y - itemBounds.top(), 0);
                    
                    RenderOverlay(skin);
                    glPopMatrix();
                    glPopAttrib();
                }
            }
            
            virtual void RenderOverlay(Gwen::Skin::Base* skin) = 0;

            virtual void SetOverlayVisible(bool overlayVisible) {
                if (m_overlayVisible == overlayVisible)
                    return;
                m_overlayVisible = overlayVisible;
                Redraw();
            }
        };

        template <typename CellData, typename GroupData>
        class CellLayoutControl : public Gwen::Controls::Base {
        protected:
            CellLayout<CellData, GroupData> m_layout;
            typename CellRow<CellData>::CellPtr m_selectedCell;
            Gwen::Font* m_font;
            CellDragControl<CellData>* m_dragControl;

            virtual void reloadLayout() {
                m_layout.clear();
                doReloadLayout();

                const Gwen::Padding& padding = GetPadding();
                int controlHeight = static_cast<int>(m_layout.height()) + padding.top + padding.bottom;
                SetBounds(GetBounds().x, GetBounds().y, GetBounds().w, controlHeight);
            }
            
            virtual void doReloadLayout() = 0;
            
            virtual void SetDragAndDropPackage(typename CellRow<CellData>::CellPtr cell) {}
            virtual CellDragControl<CellData>* createDragControl(typename CellRow<CellData>::CellPtr cell) {
                return NULL;
            }
        public:
            CellLayoutControl(Gwen::Controls::Base* parent) : Base(parent), m_dragControl(NULL) {
                m_layout.setGroupMargin(8);
                m_layout.setRowMargin(8);
                m_layout.setCellMargin(8);
                m_layout.setWidth(GetBounds().w);
                m_layout.setFixedCellWidth(64);
                SetFont(GetSkin()->GetDefaultFont());
            }
            
            virtual ~CellLayoutControl() {}

            virtual void DragAndDrop_StartDragging( Gwen::DragAndDrop::Package* package, int x, int y) {
                if (m_dragControl != NULL)
                    m_dragControl->DelayedDelete();
                
                m_dragControl = createDragControl(m_selectedCell);
                if (m_dragControl != NULL) {
                    m_dragControl->SetHidden(true);
                    
                    const LayoutBounds& bounds = m_selectedCell->itemBounds();
                    Gwen::Point global = LocalPosToCanvas(Gwen::Point(bounds.left() + GetPadding().left, bounds.top() + GetPadding().top));
                    m_dragControl->SetBounds(global.x, global.y, bounds.width(), bounds.height());
                    
                    m_DragAndDrop_Package->holdoffset = m_dragControl->CanvasPosToLocal(Gwen::Point(x, y));
                    m_DragAndDrop_Package->drawcontrol = m_dragControl;
                }
            }
            
            virtual void DragAndDrop_EndDragging(bool success, int x, int y) {
                if (m_dragControl != NULL) {
                    m_dragControl->DelayedDelete();
                    m_dragControl = NULL;
                }
            }

            virtual void DragAndDrop_SetOverlayVisible(bool visible) {
                if (m_dragControl != NULL)
                    m_dragControl->SetOverlayVisible(visible);
            }

            virtual void SetFont(Gwen::Font* font) {
                m_font = font;
            }
            
            virtual Gwen::Font* GetFont() {
                return m_font;
            }

            virtual bool OnMouseClickLeft(int x, int y, bool down) {
                if (down) {
                    Gwen::Point local = CanvasPosToLocal(Gwen::Point(x, y));
                    typename CellRow<CellData>::CellPtr cell;
                    if (m_layout.cellAt(static_cast<float>(local.x), static_cast<float>(local.y), cell)) {
                        m_selectedCell = cell;
                        OnCellSelected();
                        SetDragAndDropPackage(cell);
                    }
                }
                
                return true;
            }
            
            virtual void SetPadding(const Gwen::Padding& padding) {
                Base::SetPadding(padding);
                m_layout.setWidth(GetBounds().w - padding.left - padding.right);
            }
            
            virtual void OnBoundsChanged(Gwen::Rect oldBounds) {
                Base::OnBoundsChanged(oldBounds);

                const Gwen::Padding& padding = GetPadding();
                m_layout.setWidth(GetBounds().w - padding.left - padding.right);
                
                int controlHeight = static_cast<int>(m_layout.height()) + padding.top + padding.bottom;
                SetBounds(GetBounds().x, GetBounds().y, GetBounds().w, controlHeight);
            }

            virtual void OnCellSelected() {
                m_bCacheTextureDirty = true;
                onCellSelected.Call(this);
                Redraw();
            }

            virtual typename CellRow<CellData>::CellPtr selectedCell() {
                return m_selectedCell;
            }
            
            Gwen::Event::Caller onCellSelected;
        };
    }
}

#endif
