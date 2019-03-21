/*
 Copyright (C) 2010-2017 Kristian Duske

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
#include "View/wxUtils.h"

#include <wx/debug.h>
#include <wx/log.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

wxDEFINE_EVENT(wxEVT_DELAYED_TEXT, wxCommandEvent);

namespace TrenchBroom {
    namespace View {
        wxIMPLEMENT_DYNAMIC_CLASS(AutoCompleteTextControl, wxTextCtrl)

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

        size_t AutoCompleteTextControl::Helper::ShouldStartCompletionAfterInput(const wxString& str, const wxUniChar& c, const size_t insertPos) const {
            wxASSERT(insertPos <= str.Length());
            return DoShouldStartCompletionAfterInput(str, c, insertPos);
        }

        size_t AutoCompleteTextControl::Helper::ShouldStartCompletionAfterRequest(const wxString& str, const size_t insertPos) const {
            wxASSERT(insertPos <= str.Length());
            return DoShouldStartCompletionAfterRequest(str, insertPos);
        }

        AutoCompleteTextControl::CompletionResult AutoCompleteTextControl::Helper::GetCompletions(const wxString& str, const size_t startIndex, const size_t count) const {
            wxASSERT(startIndex + count <= str.Length());
            return DoGetCompletions(str, startIndex, count);
        }

        size_t AutoCompleteTextControl::DefaultHelper::DoShouldStartCompletionAfterInput(const wxString& str, const wxUniChar& c, const size_t insertPos) const {
            return str.Length();
        }

        size_t AutoCompleteTextControl::DefaultHelper::DoShouldStartCompletionAfterRequest(const wxString& str, size_t insertPos) const {
            return str.Length();
        }

        AutoCompleteTextControl::CompletionResult AutoCompleteTextControl::DefaultHelper::DoGetCompletions(const wxString& str, const size_t startIndex, const size_t count) const {
            return CompletionResult();
        }

        AutoCompleteTextControl::AutoCompletionList::AutoCompletionList(wxWindow* parent) :
        ControlListBox(parent, false, "No completions available.") {
            SetItemMargin(wxSize(1, 1));
            SetShowLastDivider(false);
        }

        void AutoCompleteTextControl::AutoCompletionList::SetResult(const CompletionResult& result) {
            m_result = result;
            SetItemCount(m_result.Count());
            Fit();
        }

        const wxString AutoCompleteTextControl::AutoCompletionList::CurrentSelection() const {
            wxASSERT(GetSelection() != wxNOT_FOUND);
            const size_t index = static_cast<size_t>(GetSelection());
            return m_result.GetValue(index);
        }

        class AutoCompleteTextControl::AutoCompletionList::AutoCompletionListItem : public Item {
        private:
            wxStaticText* m_valueText;
            wxStaticText* m_descriptionText;
        public:
            AutoCompletionListItem(wxWindow* parent, const wxSize& margins, const wxString& value, const wxString& description) :
            Item(parent),
            m_valueText(nullptr),
            m_descriptionText(nullptr) {
                m_valueText = new wxStaticText(this, wxID_ANY, value);
                m_descriptionText = new wxStaticText(this, wxID_ANY, description);
                m_descriptionText->SetForegroundColour(makeLighter(m_descriptionText->GetForegroundColour()));
#ifndef _WIN32
                m_descriptionText->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif

                wxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
                vSizer->Add(m_valueText);
                vSizer->Add(m_descriptionText);

                wxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
                hSizer->Add(vSizer, wxSizerFlags().Border(wxTOP | wxBOTTOM, margins.y).Border(wxLEFT | wxRIGHT, margins.x));

                SetSizer(hSizer);
            }

            void setDefaultColours(const wxColour& foreground, const wxColour& background) override {
                Item::setDefaultColours(foreground, background);
                m_descriptionText->SetForegroundColour(makeLighter(m_descriptionText->GetForegroundColour()));
            }
        };

        ControlListBox::Item* AutoCompleteTextControl::AutoCompletionList::createItem(wxWindow* parent, const wxSize& margins, const size_t index) {
            return new AutoCompletionListItem(parent, margins, m_result.GetValue(index), m_result.GetDescription(index));
        }

        AutoCompleteTextControl::AutoCompletionPopup::AutoCompletionPopup(AutoCompleteTextControl* textControl) :
        wxPopupWindow(textControl),
        m_textControl(textControl),
        m_list(nullptr) {
            BorderPanel* panel = new BorderPanel(this, wxALL);

            m_list = new AutoCompletionList(panel);
            wxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
            panelSizer->Add(m_list, wxSizerFlags().Expand().Proportion(1).Border(wxALL, 1));
            panel->SetSizer(panelSizer);

            wxSizer* windowSizer = new wxBoxSizer(wxVERTICAL);
            windowSizer->Add(panel, wxSizerFlags().Expand().Proportion(1));
            SetSizer(windowSizer);

            SetSize(m_list->GetVirtualSize() + wxSize(2, 2));

            m_textControl->Bind(wxEVT_KEY_DOWN, &AutoCompletionPopup::OnTextCtrlKeyDown, this);
            m_textControl->Bind(wxEVT_TEXT_ENTER, &AutoCompletionPopup::OnTextCtrlEnter, this);
            m_textControl->Bind(wxEVT_LEFT_DOWN, &AutoCompletionPopup::OnTextCtrlMouseDown, this);
            m_textControl->Bind(wxEVT_MIDDLE_DOWN, &AutoCompletionPopup::OnTextCtrlMouseDown, this);
            m_textControl->Bind(wxEVT_RIGHT_DOWN, &AutoCompletionPopup::OnTextCtrlMouseDown, this);
        }

        AutoCompleteTextControl::AutoCompletionPopup::~AutoCompletionPopup() {
            m_textControl->Unbind(wxEVT_KEY_DOWN, &AutoCompletionPopup::OnTextCtrlKeyDown, this);
            m_textControl->Unbind(wxEVT_TEXT_ENTER, &AutoCompletionPopup::OnTextCtrlEnter, this);
            m_textControl->Unbind(wxEVT_LEFT_DOWN, &AutoCompletionPopup::OnTextCtrlMouseDown, this);
            m_textControl->Unbind(wxEVT_MIDDLE_DOWN, &AutoCompletionPopup::OnTextCtrlMouseDown, this);
            m_textControl->Unbind(wxEVT_RIGHT_DOWN, &AutoCompletionPopup::OnTextCtrlMouseDown, this);
        }

        void AutoCompleteTextControl::AutoCompletionPopup::SetResult(const AutoCompleteTextControl::CompletionResult& result) {
            m_list->SetResult(result);
            if (m_list->GetItemCount() > 0)
                m_list->SetSelection(0);
            Fit();
            SetClientSize(m_list->GetVirtualSize() + wxSize(2, 2));
        }

        void AutoCompleteTextControl::AutoCompletionPopup::OnTextCtrlKeyDown(wxKeyEvent& event) {
            if (event.GetKeyCode() == WXK_ESCAPE && !event.HasAnyModifiers()) {
                m_textControl->EndAutoCompletion();
            } else if ((event.GetKeyCode() == WXK_UP && !event.HasAnyModifiers()) ||
                       (event.GetKeyCode() == WXK_TAB && event.GetModifiers() == wxMOD_SHIFT)) {
                SelectPreviousCompletion();
            } else if ((event.GetKeyCode() == WXK_DOWN && !event.HasAnyModifiers()) ||
                       (event.GetKeyCode() == WXK_TAB && !event.HasAnyModifiers())) {
                SelectNextCompletion();

            } else {
                if (event.GetKeyCode() == WXK_LEFT ||
                    event.GetKeyCode() == WXK_RIGHT ||
                    event.GetKeyCode() == WXK_UP ||
                    event.GetKeyCode() == WXK_DOWN ||
                    event.GetKeyCode() == WXK_PAGEUP ||
                    event.GetKeyCode() == WXK_PAGEDOWN ||
                    event.GetKeyCode() == WXK_HOME ||
                    event.GetKeyCode() == WXK_END)
                    m_textControl->EndAutoCompletion();
                event.Skip();
            }
        }

        void AutoCompleteTextControl::AutoCompletionPopup::OnTextCtrlEnter(wxCommandEvent& event) {
            DoAutoComplete();
            m_textControl->EndAutoCompletion();
        }

        void AutoCompleteTextControl::AutoCompletionPopup::OnTextCtrlMouseDown(wxMouseEvent& event) {
            m_textControl->EndAutoCompletion();
            event.Skip();
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
            m_textControl->PerformAutoComplete(m_list->CurrentSelection());
        }

        AutoCompleteTextControl::AutoCompleteTextControl() :
        wxTextCtrl(),
        m_helper(nullptr),
        m_autoCompletionPopup(nullptr),
        m_currentStartIndex(0) {}

        AutoCompleteTextControl::AutoCompleteTextControl(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) :
        wxTextCtrl(),
        m_helper(nullptr),
        m_autoCompletionPopup(nullptr),
        m_currentStartIndex(0) {
            Create(parent, id, value, pos, size, style, validator, name);
        }

        AutoCompleteTextControl::~AutoCompleteTextControl() {
            delete m_helper;
        }

        void AutoCompleteTextControl::Create(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) {
            wxTextCtrl::Create(parent, id, value, pos, size, style | wxTE_PROCESS_ENTER, validator, name);
            wxASSERT(IsSingleLine());
            m_helper = new DefaultHelper();
            Bind(wxEVT_KILL_FOCUS, &AutoCompleteTextControl::OnKillFocus, this);
            Bind(wxEVT_IDLE, &AutoCompleteTextControl::OnDelayedEventBinding, this);
        }

        void AutoCompleteTextControl::SetHelper(Helper* helper) {
            if (m_helper == helper)
                return;
            delete m_helper;
            m_helper = helper;
            if (m_helper == nullptr)
                m_helper = new DefaultHelper();
            if (IsAutoCompleting())
                EndAutoCompletion();
        }

        void AutoCompleteTextControl::OnChar(wxKeyEvent& event) {
            if (!IsAutoCompleting()) {
                const size_t index = static_cast<size_t>(GetInsertionPoint());
                const size_t startIndex = m_helper->ShouldStartCompletionAfterInput(GetValue(), event.GetUnicodeKey(), index);
                if (startIndex <= GetValue().Length())
                    StartAutoCompletion(startIndex);
            }
            event.Skip();
        }

        void AutoCompleteTextControl::OnKeyDown(wxKeyEvent& event) {
            if (event.GetKeyCode() == WXK_SPACE && event.RawControlDown()) {
                if (!IsAutoCompleting()) {
                    const size_t index = static_cast<size_t>(GetInsertionPoint());
                    const size_t startIndex = m_helper->ShouldStartCompletionAfterRequest(GetValue(), index);
                    if (startIndex <= GetValue().Length()) {
                        StartAutoCompletion(startIndex);
                        UpdateAutoCompletion();
                    }
                } else {
                    EndAutoCompletion();
                }
            } else {
                event.Skip();
            }
        }

        void AutoCompleteTextControl::OnText(wxCommandEvent& event) {
            // On GTK2, the insertion point is not yet updated when this event is fired.
            // That's why we postpone processing by re-queuing a delayed text event until
            // the insertion point is hopefully updated.
            GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_DELAYED_TEXT, GetId()));
            event.Skip();
        }

        void AutoCompleteTextControl::OnDelayedText(wxCommandEvent& event) {
            if (IsAutoCompleting()) {
                const size_t index = static_cast<size_t>(GetInsertionPoint());
                if (index <= m_currentStartIndex)
                    EndAutoCompletion();
                else
                    UpdateAutoCompletion();
            }
        }

        bool AutoCompleteTextControl::IsAutoCompleting() const {
            return m_autoCompletionPopup != nullptr && m_autoCompletionPopup->IsShown();
        }

        void AutoCompleteTextControl::StartAutoCompletion(const size_t startIndex) {
            wxASSERT(!IsAutoCompleting());
            m_currentStartIndex = startIndex;
            const wxString prefix = GetRange(0, static_cast<long>(m_currentStartIndex));
            const wxPoint offset = wxPoint(GetTextExtent(prefix).x, 0);
            const wxPoint relPos = GetRect().GetBottomLeft() + offset;
            const wxPoint absPos = GetParent()->ClientToScreen(relPos);
            m_autoCompletionPopup = new AutoCompletionPopup(this);
            m_autoCompletionPopup->Position(absPos, wxSize());
            m_autoCompletionPopup->Show();
        }

        void AutoCompleteTextControl::UpdateAutoCompletion() {
            wxASSERT(IsAutoCompleting());
            const size_t index = static_cast<size_t>(GetInsertionPoint());
            const size_t count = index - m_currentStartIndex;
            const CompletionResult result = m_helper->GetCompletions(GetValue(), m_currentStartIndex, count);
            m_autoCompletionPopup->SetResult(result);
        }

        void AutoCompleteTextControl::EndAutoCompletion() {
            wxASSERT(IsAutoCompleting());
            m_autoCompletionPopup->Hide();
            m_autoCompletionPopup->Destroy();
            m_autoCompletionPopup = nullptr;
        }

        void AutoCompleteTextControl::PerformAutoComplete(const wxString& replacement) {
            wxASSERT(IsAutoCompleting());
            const long from = static_cast<long>(m_currentStartIndex);
            const long to   = GetInsertionPoint();
            Replace(from, to, replacement);
        }

        void AutoCompleteTextControl::OnKillFocus(wxFocusEvent& event) {
            if (IsAutoCompleting())
                EndAutoCompletion();
            event.Skip();
        }

        void AutoCompleteTextControl::OnDelayedEventBinding(wxIdleEvent& event) {
            Unbind(wxEVT_IDLE, &AutoCompleteTextControl::OnDelayedEventBinding, this);
            Bind(wxEVT_TEXT, &AutoCompleteTextControl::OnText, this);
            Bind(wxEVT_DELAYED_TEXT, &AutoCompleteTextControl::OnDelayedText, this);
            Bind(wxEVT_CHAR, &AutoCompleteTextControl::OnChar, this);
            Bind(wxEVT_KEY_DOWN, &AutoCompleteTextControl::OnKeyDown, this);
        }
    }
}
