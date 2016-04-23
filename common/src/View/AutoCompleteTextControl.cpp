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
        IMPLEMENT_DYNAMIC_CLASS(AutoCompleteTextControl, wxTextCtrl)

        AutoCompleteTextControl::CompletionResult::SingleResult::SingleResult(const wxString& i_value, const wxString& i_description) :
        value(i_value),
        description(i_description) {}

        bool AutoCompleteTextControl::CompletionResult::IsEmpty() const {
            return Count() == 0;
        }
        
        size_t AutoCompleteTextControl::CompletionResult::Count() const {
            return m_results.size();
        }

        const wxString AutoCompleteTextControl::CompletionResult::GetValue(const size_t index) const {
            wxASSERT(index < Count());
            return m_results[index].value;
        }
        
        const wxString AutoCompleteTextControl::CompletionResult::GetDescription(const size_t index) const {
            wxASSERT(index < Count());
            return m_results[index].description;
        }

        void AutoCompleteTextControl::CompletionResult::Add(const wxString& value, const wxString& description) {
            m_results.push_back(SingleResult(value, description));
        }

        AutoCompleteTextControl::Helper::~Helper() {}
        
        bool AutoCompleteTextControl::Helper::ShowCompletions(const wxString& str, const size_t index) const {
            wxASSERT(index < str.Len());
            return DoShowCompletions(str, index);
        }
        
        AutoCompleteTextControl::CompletionResult AutoCompleteTextControl::Helper::GetCompletions(const wxString& str, size_t index) const {
            wxASSERT(index < str.Len());
            return DoGetCompletions(str, index);
        }

        bool AutoCompleteTextControl::DefaultHelper::DoShowCompletions(const wxString& str, const size_t index) const {
            return false;
        }
        
        AutoCompleteTextControl::CompletionResult AutoCompleteTextControl::DefaultHelper::DoGetCompletions(const wxString& str, const size_t index) const {
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
            Fit();
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
            
            Bind(wxEVT_SHOW, &AutoCompletionPopup::OnShowHide, this);
        }
        
        void AutoCompleteTextControl::AutoCompletionPopup::SetResult(const AutoCompleteTextControl::CompletionResult& result) {
            m_list->SetResult(result);
            m_list->SetSelection(0);
            Fit();
            SetClientSize(m_list->GetVirtualSize() + wxSize(2, 2));
        }

        void AutoCompleteTextControl::AutoCompletionPopup::OnShowHide(wxShowEvent& event) {
            if (event.IsShown()) {
                m_textControl->Bind(wxEVT_KEY_DOWN, &AutoCompletionPopup::OnTextCtrlKeyDown, this);
            } else {
                m_textControl->Unbind(wxEVT_KEY_DOWN, &AutoCompletionPopup::OnTextCtrlKeyDown, this);
            }
        }

        void AutoCompleteTextControl::AutoCompletionPopup::OnTextCtrlKeyDown(wxKeyEvent& event) {
            if (event.GetKeyCode() == WXK_ESCAPE && !event.HasAnyModifiers()) {
                Hide();
            } else if (event.GetKeyCode() == WXK_RETURN && !event.HasAnyModifiers()) {
                DoAutoComplete();
                Hide();
            } else if ((event.GetKeyCode() == WXK_UP && !event.HasAnyModifiers()) ||
                       (event.GetKeyCode() == WXK_TAB && event.GetModifiers() == wxMOD_SHIFT)) {
                SelectPreviousCompletion();
            } else if ((event.GetKeyCode() == WXK_DOWN && !event.HasAnyModifiers()) ||
                       (event.GetKeyCode() == WXK_TAB && !event.HasAnyModifiers())) {
                SelectNextCompletion();
            } else {
                event.Skip();
            }
        }

        void AutoCompleteTextControl::AutoCompletionPopup::SelectNextCompletion() {
            if (m_list->GetSelection() == wxNOT_FOUND) {
                m_list->SetSelection(0);
            } else if (static_cast<size_t>(m_list->GetSelection()) < m_list->GetItemCount() - 1) {
                m_list->SetSelection(m_list->GetSelection() + 1);
            }
        }
        
        void AutoCompleteTextControl::AutoCompletionPopup::SelectPreviousCompletion() {
            if (m_list->GetSelection() == wxNOT_FOUND) {
                m_list->SetSelection(static_cast<int>(m_list->GetItemCount() - 1));
            } else if (static_cast<size_t>(m_list->GetSelection()) > 0) {
                m_list->SetSelection(m_list->GetSelection() - 1);
            }
        }

        void AutoCompleteTextControl::AutoCompletionPopup::DoAutoComplete() {
        }

        AutoCompleteTextControl::AutoCompleteTextControl() :
        wxTextCtrl(),
        m_helper(NULL),
        m_autoCompletionPopup(NULL) {}
        
        AutoCompleteTextControl::AutoCompleteTextControl(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) :
        wxTextCtrl(),
        m_helper(NULL),
        m_autoCompletionPopup(NULL) {
            Create(parent, id, value, pos, size, style, validator, name);
        }
        
        AutoCompleteTextControl::~AutoCompleteTextControl() {
            delete m_helper;
        }
        
        void AutoCompleteTextControl::Create(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) {
            wxTextCtrl::Create(parent, id, value, pos, size, style, validator, name);
            wxASSERT(IsSingleLine());
            m_helper = new DefaultHelper();
            m_autoCompletionPopup = new AutoCompletionPopup(this);
            Bind(wxEVT_CHAR, &AutoCompleteTextControl::OnChar, this);
            Bind(wxEVT_KEY_UP, &AutoCompleteTextControl::OnKeyDown, this);
            Bind(wxEVT_KILL_FOCUS, &AutoCompleteTextControl::OnKillFocus, this);
            Bind(wxEVT_IDLE, &AutoCompleteTextControl::OnIdle, this);
        }

        void AutoCompleteTextControl::SetHelper(Helper* helper) {
            if (m_helper == helper)
                return;
            delete m_helper;
            m_helper = helper;
            if (m_helper == NULL)
                m_helper = new DefaultHelper();
            if (AutoCompletionListVisible())
                HideAutoCompletionList();
        }

        void AutoCompleteTextControl::OnChar(wxKeyEvent& event) {
            const wxChar key = event.GetUnicodeKey();
            if (key != wxKEY_NONE) {
                const size_t index = static_cast<size_t>(GetInsertionPoint());
                wxString str = GetValue();
                str.insert(index, key);

                if (!AutoCompletionListVisible()) {
                    if (m_helper->ShowCompletions(str, index))
                        ShowAutoCompletionList();
                }
            }
            event.Skip();
        }

        void AutoCompleteTextControl::OnKeyDown(wxKeyEvent& event) {
            if (event.GetKeyCode() == WXK_SPACE && event.RawControlDown()) {
                if (!AutoCompletionListVisible()) {
                    const size_t index = static_cast<size_t>(GetInsertionPoint() - 1);
                    ShowAutoCompletionList();
                    UpdateCompletionList(m_helper->GetCompletions(GetValue(), index));
                }
            } else {
                event.Skip();
            }
        }
        
        void AutoCompleteTextControl::OnText(wxCommandEvent& event) {
            if (AutoCompletionListVisible()) {
                static int count = 0;
                if (count++ >= 3)
                    bool b= true;
                const size_t index = static_cast<size_t>(GetInsertionPoint()-1);
                UpdateCompletionList(m_helper->GetCompletions(GetValue(), index));
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
        
        void AutoCompleteTextControl::UpdateCompletionList(const CompletionResult& result) {
            wxASSERT(AutoCompletionListVisible());
            if (result.IsEmpty())
                HideAutoCompletionList();
            else
                m_autoCompletionPopup->SetResult(result);
        }
        
        void AutoCompleteTextControl::HideAutoCompletionList() {
            wxASSERT(AutoCompletionListVisible());
            m_autoCompletionPopup->Hide();
        }

        void AutoCompleteTextControl::OnKillFocus(wxFocusEvent& event) {
            if (AutoCompletionListVisible())
                HideAutoCompletionList();
        }

        void AutoCompleteTextControl::OnIdle(wxIdleEvent& event) {
            Unbind(wxEVT_IDLE, &AutoCompleteTextControl::OnIdle, this);
            Bind(wxEVT_TEXT, &AutoCompleteTextControl::OnText, this);
        }
    }
}
