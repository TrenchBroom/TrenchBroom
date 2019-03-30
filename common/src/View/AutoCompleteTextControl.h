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

#ifndef AutoCompleteTextControl_h
#define AutoCompleteTextControl_h

#include "View/ControlListBox.h"

#include <wx/popupwin.h>
#include <wx/textctrl.h>

#include <vector>

wxDECLARE_EVENT(wxEVT_DELAYED_TEXT, wxCommandEvent);

class QLabel;

namespace TrenchBroom {
    namespace View {
        class AutoCompleteTextControl : public wxTextCtrl {
        public:
            class CompletionResult {
            private:
                struct SingleResult {
                    QString value;
                    QString description;
                    SingleResult(const QString& i_value, const QString& i_description);
                };

                using List = std::vector<SingleResult>;
                List m_results;
            public:
                bool IsEmpty() const;
                size_t Count() const;

                const QString GetValue(size_t index) const;
                const QString GetDescription(size_t index) const;

                void Add(const QString& value, const QString& description);
            };

            class Helper {
            public:
                virtual ~Helper();

                size_t ShouldStartCompletionAfterInput(const wxString& str, const wxUniChar& c, size_t insertPos) const;
                size_t ShouldStartCompletionAfterRequest(const wxString& str, size_t insertPos) const;
                CompletionResult GetCompletions(const wxString& str, size_t startIndex, size_t count) const;
            private:
                virtual size_t DoShouldStartCompletionAfterInput(const wxString& str, const wxUniChar& c, size_t insertPos) const = 0;
                virtual size_t DoShouldStartCompletionAfterRequest(const wxString& str, size_t insertPos) const = 0;
                virtual CompletionResult DoGetCompletions(const wxString& str, size_t startIndex, size_t count) const = 0;
            };
        private:
            class DefaultHelper : public Helper {
            private:
                size_t DoShouldStartCompletionAfterInput(const wxString& str, const wxUniChar& c, size_t insertPos) const override;
                size_t DoShouldStartCompletionAfterRequest(const wxString& str, size_t insertPos) const override;
                CompletionResult DoGetCompletions(const wxString& str, size_t startIndex, size_t count) const override;
            };
        private:
            class AutoCompletionList : public ControlListBox {
            private:
                CompletionResult m_result;
            public:
                AutoCompletionList(QWidget* parent);
                void SetResult(const CompletionResult& result);
                const QString CurrentSelection() const;
            private:
                class AutoCompletionListItem;
                Item* createItem(QWidget* parent, const wxSize& margin, size_t index) override;
            };

            class AutoCompletionPopup : public wxPopupWindow {
            private:
                AutoCompleteTextControl* m_textControl;
                AutoCompletionList* m_list;
            public:
                AutoCompletionPopup(AutoCompleteTextControl* textControl);
                ~AutoCompletionPopup();

                void SetResult(const CompletionResult& result);
            private:
                void OnShowHide(wxShowEvent& event);
                void OnTextCtrlKeyDown(wxKeyEvent& event);
                void OnTextCtrlEnter();
                void OnTextCtrlMouseDown(wxMouseEvent& event);

                void SelectNextCompletion();
                void SelectPreviousCompletion();
                void DoAutoComplete();
            };
        private:
            Helper* m_helper;
            AutoCompletionPopup* m_autoCompletionPopup;
            size_t m_currentStartIndex;
        public:
            AutoCompleteTextControl();
            AutoCompleteTextControl(QWidget* parent, wxWindowID id, const QString& value = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, const QString& name = wxTextCtrlNameStr);
            ~AutoCompleteTextControl();

            void Create(QWidget* parent, wxWindowID id, const QString& value = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, const QString& name = wxTextCtrlNameStr);

            void SetHelper(Helper* helper);
        private:
            void OnChar(wxKeyEvent& event);
            void OnKeyDown(wxKeyEvent& event);
            void OnText();
            void OnDelayedText();

            bool IsAutoCompleting() const;
            void StartAutoCompletion(size_t startIndex);
            void UpdateAutoCompletion();
            void EndAutoCompletion();

            void PerformAutoComplete(const QString& replacement);

            void OnKillFocus(wxFocusEvent& event);
            void OnDelayedEventBinding(wxIdleEvent& event);
        public:
            wxDECLARE_DYNAMIC_CLASS(AutoCompleteTextControl);
        };
    }
}

#endif /* AutoCompleteTextControl_h */
