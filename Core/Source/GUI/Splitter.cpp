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

#include "Splitter.h"

namespace TrenchBroom {
    namespace Gui {
        Splitter::Splitter(Controls::Base* parent, bool horizontal) : Base(parent), m_horizontal(horizontal) {
            m_splitter = new Controls::SplitterBar(this);
            m_splitter->onDragged.Add(this, &Splitter::OnSplitterMoved);
            if (m_horizontal) {
                m_splitter->SetPos(128, 0);
                m_splitter->SetCursor(Gwen::CursorType::SizeWE);
            } else {
                m_splitter->SetPos(0, 128);
                m_splitter->SetCursor(Gwen::CursorType::SizeNS);
            }
            m_balance = 0.5f;
            
            SetPanel(0, NULL);
            SetPanel(1, NULL);
            
            SetSplitterSize(5);
            SetSplitterVisible(true);
            m_zoomedSection = -1;
        }
        
        void Splitter::UpdateSplitter() {
            if (m_horizontal) m_splitter->MoveTo((Width() - m_splitter->Width()) * m_balance, m_splitter->Y());
            else m_splitter->MoveTo(m_splitter->X(), (Height() - m_splitter->Height()) * m_balance);
        }
        
        void Splitter::OnSplitterMoved(Controls::Base *control) {
            m_balance = CalculateBalance();
            Invalidate();
        }
        
        float Splitter::CalculateBalance() {
            if (m_horizontal)
                return m_splitter->X() / (float)(Width() - m_splitter->Width());
            return m_splitter->Y() / (float)(Height() - m_splitter->Height());
        }
        
        void Splitter::Layout(Skin::Base* /*skin*/) {
            if (m_horizontal) m_splitter->SetSize(m_barSize, Height());
            else m_splitter->SetSize(Width(), m_barSize);
            
            UpdateSplitter();
            
            if (m_zoomedSection == -1) {
                if (m_sections[0] != NULL) {
                    if (m_horizontal) m_sections[0]->SetBounds(0, 0, m_splitter->X(), Height());
                    else m_sections[0]->SetBounds(0, 0, Width(), m_splitter->Y());
                }
                if (m_sections[1] != NULL) {
                    if (m_horizontal) m_sections[1]->SetBounds(m_splitter->X() + m_barSize, 0, Width() - (m_splitter->X() + m_barSize), Height());
                    else m_sections[1]->SetBounds(0, m_splitter->Y() + m_barSize, Width(), Height() - (m_splitter->Y() + m_barSize));
                }
            } else {
                m_sections[m_zoomedSection]->SetBounds(0, 0, Width(), Height());
            }
        }
        
        void Splitter::SetPanel(int index, Controls::Base* panel) {
            Debug::AssertCheck(index >= 0 && index < 2, "Splitter::SetPanel out of range");
            m_sections[index] = panel;
            if (panel) {
                panel->Dock(Pos::None);
                panel->SetParent(this);
            }
            Invalidate();
        }
        
        Controls::Base* Splitter::GetPanel(int i) {
            return m_sections[i];
        }
        
        void Splitter::ZoomChanged() {
            onZoomChange.Call(this);
            if (m_zoomedSection == -1) onUnZoomed.Call(this);
            else onZoomed.Call(this);
        }
        
        void Splitter::Zoom(int section) {
            UnZoom();
            if (m_sections[section] != NULL) {
                if (m_sections[1 - section] != NULL) m_sections[1 - section]->SetHidden(true);
                m_zoomedSection = section;
                Invalidate();
            }
            ZoomChanged();
        }
        
        void Splitter::UnZoom() {
            m_zoomedSection = -1;
            for (unsigned int i = 0; i < 2; i++)
                if (m_sections[i] != NULL) m_sections[i]->SetHidden(false);
            Invalidate();
            ZoomChanged();
        }
    }
}
