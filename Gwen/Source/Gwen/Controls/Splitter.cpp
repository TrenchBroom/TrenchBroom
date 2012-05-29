/*
 GWEN
 Copyright (c) 2012 Kristian Duske
 See license in Gwen.h
 */

#include "Splitter.h"
#include "Gwen/Controls/SplitterBar.h"

namespace Gwen {
    namespace Controls {
        Splitter::Splitter(Gwen::Controls::Base* parent, bool horizontal, int initialPosition) : Base(parent), m_horizontal(horizontal), m_initialPosition(initialPosition) {
            m_splitter = new Gwen::Controls::SplitterBar(this);
            m_splitter->onDragged.Add(this, &Splitter::OnSplitterMoved);
            if (m_horizontal)
                m_splitter->SetCursor(Gwen::CursorType::SizeWE);
            else
                m_splitter->SetCursor(Gwen::CursorType::SizeNS);
            m_balance = 0.5f;

            for (int i = 0; i < 2; i++) {
                m_minSize[i] = -1;
                m_maxSize[i] = -1;
                m_resize[i] = true;
                SetPanel(i, NULL);
            }
            
            SetSplitterSize(5);
            SetSplitterVisible(true);
            m_zoomedSection = -1;
        }
        
        void Splitter::UpdateSplitter() {
            if (m_horizontal) m_splitter->MoveTo((Width() - m_splitter->Width()) * m_balance, m_splitter->Y());
            else m_splitter->MoveTo(m_splitter->X(), (Height() - m_splitter->Height()) * m_balance);
        }
        
        void Splitter::OnSplitterMoved(Gwen::Controls::Base *control) {
            m_balance = CalculateBalance();
            Invalidate();
        }
        
        void Splitter::OnBoundsChanged(Gwen::Rect oldBounds) {
            Base::OnBoundsChanged(oldBounds);
            
            Gwen::Rect newBounds = GetBounds();
            if (m_horizontal) {
                if (m_resize[0] && m_resize[1]) {
                    int newPos = static_cast<int>(static_cast<float>(m_splitter->X()) / static_cast<float>(oldBounds.w) * static_cast<float>(newBounds.w));
                    m_splitter->SetPos(newPos, 0);
                } else if (m_resize[0]) {
                    m_splitter->SetPos(m_splitter->X() + newBounds.w - oldBounds.w, 0);
                }
            } else {
                if (m_resize[0] && m_resize[1]) {
                    int newPos = static_cast<int>(static_cast<float>(m_splitter->Y()) / static_cast<float>(oldBounds.h) * static_cast<float>(newBounds.h));
                    m_splitter->SetPos(0, newPos);
                } else if (m_resize[0]) {
                    m_splitter->SetPos(0, m_splitter->Y() + newBounds.h - oldBounds.h);
                }
            }
        }
        
        float Splitter::CalculateBalance() {
            if (m_horizontal)
                return m_splitter->X() / static_cast<float>(Width() - m_splitter->Width());
            return m_splitter->Y() / static_cast<float>(Height() - m_splitter->Height());
        }
        
        void Splitter::Layout(Gwen::Skin::Base* /*skin*/) {
            if (m_horizontal) m_splitter->SetSize(m_barSize, Height());
            else m_splitter->SetSize(Width(), m_barSize);

            if (m_initialPosition > 0) {
                if (m_horizontal)
                    m_splitter->SetPos(m_initialPosition, 0);
                else
                    m_splitter->SetPos(0, m_initialPosition);
                m_initialPosition = 0;
            } else if (m_initialPosition < 0) {
                if (m_horizontal)
                    m_splitter->SetPos(Width() - m_splitter->Width() + m_initialPosition, 0);
                else
                    m_splitter->SetPos(0, Height() - m_splitter->Height() + m_initialPosition);
                m_initialPosition = 0;
            }
            
            if (m_horizontal) {
                if (m_minSize[0] > -1 && m_splitter->X() < m_minSize[0])
                    m_splitter->SetPos(m_minSize[0], 0);
                if (m_maxSize[0] > -1 && m_splitter->X() > m_maxSize[0])
                    m_splitter->SetPos(m_maxSize[0], 0);
                if (m_minSize[1] > -1 && m_splitter->X() > Width() - m_splitter->Width() - m_minSize[1])
                    m_splitter->SetPos(Width() - m_splitter->Width() - m_minSize[1], 0);
                if (m_maxSize[1] > -1 && m_splitter->X() < Width() - m_splitter->Width() - m_maxSize[1])
                    m_splitter->SetPos(Width() - m_splitter->Width() - m_maxSize[1], 0);
            } else {
                if (m_minSize[0] > -1 && m_splitter->Y() < m_minSize[0])
                    m_splitter->SetPos(0, m_minSize[0]);
                if (m_maxSize[0] > -1 && m_splitter->Y() > m_maxSize[0])
                    m_splitter->SetPos(0, m_maxSize[0]);
                if (m_minSize[1] > -1 && m_splitter->Y() > Height() - m_splitter->Height() - m_minSize[1])
                    m_splitter->SetPos(0, m_minSize[1]);
                if (m_maxSize[1] > -1 && m_splitter->Y() < Height() - m_splitter->Height() - m_maxSize[1])
                    m_splitter->SetPos(0, m_maxSize[1]);
            }

            m_balance = CalculateBalance();
            UpdateSplitter();
            
            if (m_zoomedSection == -1) {
                if (m_sections[0] != NULL) {
                    const Gwen::Margin& margin = m_sections[0]->GetMargin();
                    if (m_horizontal) m_sections[0]->SetBounds(margin.left, margin.top, m_splitter->X() - margin.left - margin.right, Height() - margin.top - margin.bottom);
                    else m_sections[0]->SetBounds(margin.left, margin.top, Width() - margin.left - margin.right, m_splitter->Y() - margin.top - margin.bottom);
                }
                if (m_sections[1] != NULL) {
                    const Gwen::Margin& margin = m_sections[1]->GetMargin();
                    if (m_horizontal) m_sections[1]->SetBounds(m_splitter->X() + m_barSize + margin.left, margin.top, Width() - m_splitter->X() - m_barSize - margin.left - margin.right, Height() - margin.top - margin.bottom);
                    else m_sections[1]->SetBounds(margin.left, m_splitter->Y() + m_barSize + margin.top, Width() - margin.left - margin.right, Height() - m_splitter->Y() - m_barSize - margin.top - margin.bottom);
                }
            } else {
                const Gwen::Margin& margin = m_sections[m_zoomedSection]->GetMargin();
                m_sections[m_zoomedSection]->SetBounds(margin.left, margin.top, Width() - margin.left - margin.right, Height() - margin.top - margin.bottom);
            }
        }
        
        void Splitter::SetPanel(int index, Gwen::Controls::Base* panel) {
            Gwen::Debug::AssertCheck(index >= 0 && index < 2, "Splitter::SetPanel out of range");
            m_sections[index] = panel;
            if (panel) {
                panel->Dock(Gwen::Pos::None);
                panel->SetParent(this);
            }
            Invalidate();
        }
        
        Gwen::Controls::Base* Splitter::GetPanel(int index) {
            return m_sections[index];
        }
        
        void Splitter::SetMinSize(int index, int minSize) {
            Gwen::Debug::AssertCheck(index >= 0 && index < 2, "Splitter::SetMinSize out of range");
            if (m_minSize[index] == minSize)
                return;
            m_minSize[index] = minSize;
            Invalidate();
        }
        
        void Splitter::SetMaxSize(int index, int maxSize) {
            Gwen::Debug::AssertCheck(index >= 0 && index < 2, "Splitter::SetMaxSize out of range");
            if (m_maxSize[index] == maxSize)
                return;
            m_maxSize[index] = maxSize;
            Invalidate();
        }
        
        void Splitter::SetResize(int index, bool resize) {
            Gwen::Debug::AssertCheck(index >= 0 && index < 2, "Splitter::SetResize out of range");
            if (m_resize[index] == resize)
                return;
            m_resize[index] = resize;
        }

        void Splitter::ZoomChanged() {
            onZoomChange.Call(this);
            if (m_zoomedSection == -1) onUnZoomed.Call(this);
            else onZoomed.Call(this);
        }
        
        void Splitter::Zoom(int index) {
            UnZoom();
            if (m_sections[index] != NULL) {
                if (m_sections[1 - index] != NULL) m_sections[1 - index]->SetHidden(true);
                m_zoomedSection = index;
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

        void Splitter::SetSplitterVisible(bool b) {
            m_splitter->SetShouldDrawBackground( b );
        }
        
        void Splitter::SetSplitterSize(int size) {
            m_barSize = size;
        }
    }
}
