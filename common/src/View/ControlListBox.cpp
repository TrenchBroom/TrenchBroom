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

#include "ControlListBox.h"

#include "Macros.h"
#include "View/BorderLine.h"
#include "View/ViewConstants.h"

#include <wx/listbox.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/wupdlock.h>

wxDEFINE_EVENT(wxEVT_LISTBOX_RCLICK, wxCommandEvent);

namespace TrenchBroom {
    namespace View {
        ControlListBox::Item::Item(wxWindow* parent) :
        wxWindow(parent, wxID_ANY) {}

        ControlListBox::Item::~Item() {}

        bool ControlListBox::Item::AcceptsFocus() const {
            return false;
        }

        void ControlListBox::Item::setSelectionColours(const wxColour& foreground, const wxColour& background) {
            setColours(this, foreground, background);
        }

        void ControlListBox::Item::setDefaultColours(const wxColour& foreground, const wxColour& background) {
            setColours(this, foreground, background);
        }

        void ControlListBox::Item::setColours(wxWindow* window, const wxColour& foreground, const wxColour& background) {
            if (!window->GetChildren().IsEmpty() || window->ShouldInheritColours()) {
                if (window->GetForegroundColour() != foreground)
                    window->SetForegroundColour(foreground);
                if (window->GetBackgroundColour() != background)
                    window->SetBackgroundColour(background);
            }

            const wxWindowList& children = window->GetChildren();
            wxWindowList::const_iterator it, end;
            for (it = children.begin(), end = children.end(); it != end; ++it) {
                wxWindow* child = *it;
                setColours(child, foreground, background);
            }
        }

        class ControlListBox::Sizer : public wxBoxSizer {
        private:
            const bool m_restrictToClientWidth;
        public:
            Sizer(const int orient, const bool restrictToClientWidth) :
            wxBoxSizer(orient),
            m_restrictToClientWidth(restrictToClientWidth){}

            wxSize CalcMin() {
                const wxSize originalSize = wxBoxSizer::CalcMin();
                if (!m_restrictToClientWidth)
                    return originalSize;
                const wxSize containerSize = GetContainingWindow()->GetClientSize();
                const wxSize result(containerSize.x, originalSize.y);
                return result;
            }
        };

        ControlListBox::ControlListBox(wxWindow* parent, const bool restrictToClientWidth, const wxString& emptyText) :
        wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxVSCROLL),
        m_itemMargin(LayoutConstants::MediumHMargin, LayoutConstants::WideVMargin),
        m_restrictToClientWidth(restrictToClientWidth),
        m_emptyText(emptyText),
        m_emptyTextLabel(NULL),
        m_showLastDivider(true),
        m_valid(true),
        m_newItemCount(0),
        m_selectionIndex(0) {
            SetSizer(new Sizer(wxVERTICAL, m_restrictToClientWidth));
            SetScrollRate(5, 5);
            SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
            Bind(wxEVT_IDLE, &ControlListBox::OnIdle, this);
            Bind(wxEVT_LEFT_DOWN, &ControlListBox::OnLeftClickVoid, this);
            Bind(wxEVT_SIZE, &ControlListBox::OnSize, this);
        }

        size_t ControlListBox::GetItemCount() const {
            if (!m_valid)
                return m_newItemCount;
            return m_items.size();
        }

        int ControlListBox::GetSelection() const {
            if (m_selectionIndex == GetItemCount())
                return  wxNOT_FOUND;
            return static_cast<int>(m_selectionIndex);
        }

        void ControlListBox::SetItemCount(const size_t itemCount) {
            if (m_selectionIndex == GetItemCount())
                m_selectionIndex = itemCount;
            else
                m_selectionIndex = wxMin(itemCount, m_selectionIndex);
            m_newItemCount = itemCount;
            m_valid = false;
        }

        void ControlListBox::SetSelection(const int index) {
            wxWindowUpdateLocker lock(this);

            if (index < 0 || static_cast<size_t>(index) > GetItemCount()) {
                setSelection(GetItemCount());
            } else {
                setSelection(static_cast<size_t>(index));
            }
            Refresh();
        }

        void ControlListBox::MakeVisible(const size_t index) {
            validate();
            ensure(index < m_items.size(), "index out of range");
            MakeVisible(m_items[index]);
        }

        void ControlListBox::MakeVisible(const Item* item) {
            MakeVisible(item->GetPosition().y, item->GetSize().y);
        }

        void ControlListBox::MakeVisible(wxCoord y, const wxCoord size) {
            wxWindowUpdateLocker lock(this);

            y = CalcUnscrolledPosition(wxPoint(0, y)).y;
            int xUnit, yUnit;
            GetScrollPixelsPerUnit(&xUnit, &yUnit);
            const wxCoord startY = GetViewStart().y * yUnit;
            const wxCoord sizeY  = GetClientSize().y;

            if (y >= startY && y + size <= sizeY)
                return;

            if (size >= sizeY || y < startY) {
                Scroll(wxDefaultCoord, y / yUnit);
            } else if (y + size > startY + sizeY) {
                Scroll(wxDefaultCoord, (y + size - sizeY) / yUnit + 1);
            }
        }

        void ControlListBox::SetItemMargin(const wxSize& margin) {
            if (m_itemMargin == margin)
                return;
            m_itemMargin = margin;
            m_valid = false;
        }

        void ControlListBox::SetShowLastDivider(const bool showLastDivider) {
            if (m_showLastDivider == showLastDivider)
                return;
            m_showLastDivider = showLastDivider;
            m_valid = false;
        }

        void ControlListBox::SetEmptyText(const wxString& emptyText) {
            if (m_emptyText == emptyText)
                return;
            
            m_emptyText = emptyText;
            if (GetItemCount() == 0)
                m_valid = false;
        }

        void ControlListBox::invalidate() {
            m_valid = false;
        }
        
        void ControlListBox::validate() {
            if (!m_valid) {
                m_valid = true;

                wxWindowUpdateLocker lock(this);
                refresh(m_newItemCount);
                setSelection(m_selectionIndex);
                Refresh();
                
            }
        }
        
        void ControlListBox::refresh(const size_t itemCount) {
            wxSizer* listSizer = GetSizer();
            listSizer->Clear(true);
            m_emptyTextLabel = NULL;

            m_items.clear();
            m_items.reserve(itemCount);

            if (itemCount > 0) {
                for (size_t i = 0; i < itemCount; ++i) {
                    Item* item = createItem(this, m_itemMargin, i);

                    listSizer->Add(item, wxSizerFlags().Expand());
                    if (i < itemCount - 1 || m_showLastDivider)
                        listSizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), wxSizerFlags().Expand());

                    bindEvents(item, i);
                    m_items.push_back(item);
                }
            } else if (!m_emptyText.empty()) {
                m_emptyTextLabel = new wxStaticText(this, wxID_ANY, m_emptyText, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
                m_emptyTextLabel->SetFont(m_emptyTextLabel->GetFont().Bold());
                m_emptyTextLabel->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
                if (m_restrictToClientWidth)
                    m_emptyTextLabel->Wrap(GetClientSize().x - LayoutConstants::WideVMargin * 2);

                wxSizer* justifySizer = new wxBoxSizer(wxHORIZONTAL);
                justifySizer->AddStretchSpacer();
                justifySizer->AddSpacer(LayoutConstants::WideHMargin);
                justifySizer->Add(m_emptyTextLabel);
                justifySizer->AddSpacer(LayoutConstants::WideHMargin);
                justifySizer->AddStretchSpacer();

                listSizer->Add(justifySizer, wxSizerFlags().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin).Expand());
                listSizer->AddStretchSpacer();
            }
            if (m_restrictToClientWidth)
                FitInside();
            else
                GetParent()->Fit();
            InvalidateBestSize();
        }

        void ControlListBox::bindEvents(wxWindow* window, const size_t itemIndex) {
            if (window->IsFocusable()) {
                window->Bind(wxEVT_SET_FOCUS, &ControlListBox::OnFocusChild, this, wxID_ANY, wxID_ANY, new wxVariant(long(itemIndex)));
            } else {
                window->Bind(wxEVT_LEFT_DOWN, &ControlListBox::OnLeftClickChild, this, wxID_ANY, wxID_ANY, new wxVariant(long(itemIndex)));
                window->Bind(wxEVT_RIGHT_DOWN, &ControlListBox::OnRightClickChild, this, wxID_ANY, wxID_ANY, new wxVariant(long(itemIndex)));
                window->Bind(wxEVT_LEFT_DCLICK, &ControlListBox::OnDoubleClickChild, this);
            }

            const wxWindowList& children = window->GetChildren();
            wxWindowList::const_iterator it, end;
            for (it = children.begin(), end = children.end(); it != end; ++it) {
                wxWindow* child = *it;
                bindEvents(child, itemIndex);
            }
        }

        void ControlListBox::OnIdle(wxIdleEvent& event) {
            validate();
        }

        void ControlListBox::OnSize(wxSizeEvent& event) {
            wxWindowUpdateLocker lock(this);

            if (m_emptyTextLabel != NULL && m_restrictToClientWidth) {
				m_emptyTextLabel->SetLabel(m_emptyText);
                m_emptyTextLabel->Wrap(GetClientSize().x - LayoutConstants::WideVMargin * 2);
			}
            event.Skip();
        }

        void ControlListBox::OnFocusChild(wxFocusEvent& event) {
            wxWindowUpdateLocker lock(this);
            setSelection(event);

            event.Skip();
        }

        void ControlListBox::OnLeftClickChild(wxMouseEvent& event) {
            wxWindowUpdateLocker lock(this);
            setSelection(event);
        }

        void ControlListBox::OnRightClickChild(wxMouseEvent& event) {
            wxWindowUpdateLocker lock(this);
            setSelection(event);

            wxCommandEvent* command = new wxCommandEvent(wxEVT_LISTBOX_RCLICK, GetId());
            command->SetInt(GetSelection());
            command->SetEventObject(this);
            QueueEvent(command);
        }

        void ControlListBox::OnDoubleClickChild(wxMouseEvent& event) {
            wxCommandEvent* command = new wxCommandEvent(wxEVT_LISTBOX_DCLICK, GetId());
            command->SetInt(GetSelection());
            command->SetEventObject(this);
            QueueEvent(command);
        }

        void ControlListBox::OnLeftClickVoid(wxMouseEvent& event) {
            wxWindowUpdateLocker lock(this);
            setSelection(GetItemCount());
        }

        void ControlListBox::setSelection(const wxEvent& event) {
            const wxVariant* variant = static_cast<wxVariant*>(event.GetEventUserData());
            const size_t itemIndex = static_cast<size_t>(variant->GetLong());
            setSelection(itemIndex);
        }

        void ControlListBox::setSelection(const size_t index) {
            validate();
            ensure(index <= m_items.size(), "index out of range");
            const bool changed = m_selectionIndex != index;
            m_selectionIndex = index;

            for (size_t i = 0; i < m_items.size(); ++i)
                m_items[i]->setDefaultColours(GetForegroundColour(), GetBackgroundColour());

            if (m_selectionIndex < m_items.size()) {
                m_items[m_selectionIndex]->setSelectionColours(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT), wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
                MakeVisible(index);
            }

            Refresh();

            if (changed) {
                wxCommandEvent* command = new wxCommandEvent(wxEVT_LISTBOX, GetId());
                command->SetInt(GetSelection());
                command->SetEventObject(this);
                QueueEvent(command);
            }
        }
    }
}
