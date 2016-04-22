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

#include "AutoCompleteTextControl.h"

#include "View/BorderPanel.h"

#include <wx/debug.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        bool AutoCompleteTextControl::CompletionResult::IsEmpty() const {
            return Count() == 0;
        }
        
        size_t AutoCompleteTextControl::CompletionResult::Count() const {
            return 3;
        }

        const wxString AutoCompleteTextControl::CompletionResult::GetValue(const size_t index) const {
            wxASSERT(index < Count());
            wxString result;
            result << "Completion " << index;
            return result;
        }
        
        const wxString AutoCompleteTextControl::CompletionResult::GetDescription(const size_t index) const {
            wxASSERT(index < Count());
            wxString result;
            result << "This is the description " << index;
            return result;
        }

        AutoCompleteTextControl::Helper::~Helper() {}
        
        AutoCompleteTextControl::CompletionResult AutoCompleteTextControl::Helper::GetCompletionResult(const wxString& fullPrefix) const {
            return DoGetCompletionResult(fullPrefix);
        }

        AutoCompleteTextControl::CompletionResult AutoCompleteTextControl::DefaultHelper::DoGetCompletionResult(const wxString& fullPrefix) const {
            return CompletionResult();
        }

        AutoCompleteTextControl::AutoCompletionList::AutoCompletionList(wxWindow* parent) :
        ControlListBox(parent, false) {
            SetItemMargin(wxSize(1, 1));
            SetShowLastDivider(false);
        }
        
        void AutoCompleteTextControl::AutoCompletionList::SetResult(const CompletionResult& result) {
            m_result = result;
            wxASSERT(!m_result.IsEmpty());
            SetItemCount(m_result.Count());
        }

        ControlListBox::Item* AutoCompleteTextControl::AutoCompletionList::createItem(wxWindow* parent, const wxSize& margins, size_t index) {
            Item* container = new Item(parent);
            wxStaticText* valueText = new wxStaticText(container, wxID_ANY, m_result.GetValue(index));
            wxStaticText* descriptionText = new wxStaticText(container, wxID_ANY, m_result.GetDescription(index));
            
#ifndef _WIN32
            descriptionText->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
            
            wxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
            vSizer->Add(valueText);
            vSizer->Add(descriptionText);
            
            wxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
            hSizer->Add(vSizer, wxSizerFlags().Border(wxTOP | wxBOTTOM, margins.y).Border(wxLEFT | wxRIGHT, margins.x));
            
            container->SetSizer(hSizer);
            return container;
        }

        AutoCompleteTextControl::AutoCompletionPopup::AutoCompletionPopup(AutoCompleteTextControl* textControl) :
        wxPopupWindow(textControl),
        m_textControl(textControl),
        m_list(NULL) {
            BorderPanel* panel = new BorderPanel(this, wxALL);
            
            m_list = new AutoCompletionList(panel);
            wxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
            panelSizer->Add(m_list, wxSizerFlags().Expand().Proportion(1).Border(wxALL, 1));
            panel->SetSizer(panelSizer);
            
            wxSizer* windowSizer = new wxBoxSizer(wxVERTICAL);
            windowSizer->Add(panel, wxSizerFlags().Expand().Proportion(1));
            SetSizer(windowSizer);
            
            SetSize(m_list->GetVirtualSize() + wxSize(2, 2));
        }
        
        void AutoCompleteTextControl::AutoCompletionPopup::SetResult(const AutoCompleteTextControl::CompletionResult& result) {
            m_list->SetResult(result);
            SetSize(m_list->GetVirtualSize() + wxSize(2, 2));
        }

        AutoCompleteTextControl::AutoCompleteTextControl(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) :
        wxTextCtrl(parent, id, value, pos, size, style & ~wxTE_MULTILINE, validator, name),
        m_helper(new DefaultHelper()),
        m_autoCompletionPopup(NULL) {
            wxASSERT(IsSingleLine());
            m_autoCompletionPopup = new AutoCompletionPopup(this);
            Bind(wxEVT_CHAR, &AutoCompleteTextControl::OnChar, this);
            Bind(wxEVT_KILL_FOCUS, &AutoCompleteTextControl::OnKillFocus, this);
        }
        
        AutoCompleteTextControl::~AutoCompleteTextControl() {
            delete m_helper;
        }
        
        void AutoCompleteTextControl::SetHelper(Helper* helper) {
            delete m_helper;
            helper = m_helper;
            if (m_helper == NULL)
                m_helper = new DefaultHelper();
            if (AutoCompletionListVisible()) {
                const wxString prefix = GetRange(0, GetInsertionPoint());
                UpdateCompletionList(m_helper->GetCompletionResult(prefix));
            }
            
        }

        void AutoCompleteTextControl::OnChar(wxKeyEvent& event) {
            const wxChar key = event.GetUnicodeKey();
            if (key != wxKEY_NONE) {
                wxString prefix = GetRange(0, GetInsertionPoint());
                prefix << key;
                
                UpdateCompletionList(m_helper->GetCompletionResult(prefix));
            }
            event.Skip();
        }

        void AutoCompleteTextControl::UpdateCompletionList(const CompletionResult& result) {
            if (result.IsEmpty()) {
                if (AutoCompletionListVisible())
                    HideAutoCompletionList();
            } else {
                if (!AutoCompletionListVisible())
                    ShowAutoCompletionList();
                m_autoCompletionPopup->SetResult(result);
            }
        }
        
        bool AutoCompleteTextControl::AutoCompletionListVisible() const {
            return m_autoCompletionPopup->IsShown();
        }

        void AutoCompleteTextControl::ShowAutoCompletionList() {
            wxASSERT(!AutoCompletionListVisible());
            const wxString prefix = GetRange(0, GetInsertionPoint());
            const wxPoint offset = wxPoint(GetTextExtent(prefix).x, 0);
            const wxPoint relPos = GetRect().GetBottomLeft() + offset;
            const wxPoint absPos = GetParent()->ClientToScreen(relPos);
            m_autoCompletionPopup->Position(absPos, wxSize());
            m_autoCompletionPopup->Show();
            
        }
        
        void AutoCompleteTextControl::HideAutoCompletionList() {
            wxASSERT(AutoCompletionListVisible());
            m_autoCompletionPopup->Hide();
        }

        void AutoCompleteTextControl::OnKillFocus(wxFocusEvent& event) {
            if (AutoCompletionListVisible())
                HideAutoCompletionList();
        }
    }
}
