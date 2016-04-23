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

#ifndef AutoCompleteTextControl_h
#define AutoCompleteTextControl_h

#include "View/ControlListBox.h"

#include <wx/popupwin.h>
#include <wx/textctrl.h>

#include <vector>

namespace TrenchBroom {
    namespace View {
        class AutoCompleteTextControl : public wxTextCtrl {
        public:
            class CompletionResult {
            private:
                struct SingleResult {
                    wxString value;
                    wxString description;
                    SingleResult(const wxString& i_value, const wxString& i_description);
                };
                
                typedef std::vector<SingleResult> List;
                List m_results;
            public:
                bool IsEmpty() const;
                size_t Count() const;
                
                const wxString GetValue(size_t index) const;
                const wxString GetDescription(size_t index) const;
                
                void Add(const wxString& value, const wxString& description);
            };
            
            class Helper {
            public:
                virtual ~Helper();
                
                bool ShowCompletions(const wxString& str, size_t index) const;
                CompletionResult GetCompletions(const wxString& str, size_t index) const;
            private:
                virtual bool DoShowCompletions(const wxString& str, size_t index) const = 0;
                virtual CompletionResult DoGetCompletions(const wxString& str, size_t index) const = 0;
            };
        private:
            class DefaultHelper : public Helper {
            private:
                bool DoShowCompletions(const wxString& str, size_t index) const;
                CompletionResult DoGetCompletions(const wxString& str, size_t index) const;
            };
        private:
            class AutoCompletionList : public ControlListBox {
            private:
                CompletionResult m_result;
            public:
                AutoCompletionList(wxWindow* parent);
                void SetResult(const CompletionResult& result);
            private:
                Item* createItem(wxWindow* parent, const wxSize& margin, size_t index);
            };
            
            class AutoCompletionPopup : public wxPopupWindow {
            private:
                AutoCompleteTextControl* m_textControl;
                AutoCompletionList* m_list;
            public:
                AutoCompletionPopup(AutoCompleteTextControl* textControl);
                void SetResult(const CompletionResult& result);
            private:
                void OnShowHide(wxShowEvent& event);
                void OnTextCtrlKeyDown(wxKeyEvent& event);
                
                void SelectNextCompletion();
                void SelectPreviousCompletion();
                void DoAutoComplete();
            };
        private:
            Helper* m_helper;
            AutoCompletionPopup* m_autoCompletionPopup;
        public:
            AutoCompleteTextControl();
            AutoCompleteTextControl(wxWindow* parent, wxWindowID id, const wxString& value = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxTextCtrlNameStr);
            ~AutoCompleteTextControl();
            
            void Create(wxWindow* parent, wxWindowID id, const wxString& value = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxTextCtrlNameStr);
            
            void SetHelper(Helper* helper);
        private:
            void OnChar(wxKeyEvent& event);
            void OnKeyDown(wxKeyEvent& event);
            void OnText(wxCommandEvent& event);
            
            bool AutoCompletionListVisible() const;
            void ShowAutoCompletionList();
            void UpdateCompletionList(const CompletionResult& result);
            void HideAutoCompletionList();
            
            void OnKillFocus(wxFocusEvent& event);
            void OnIdle(wxIdleEvent& event);
        public:
            DECLARE_DYNAMIC_CLASS(AutoCompleteTextControl)
        };
    }
}

#endif /* AutoCompleteTextControl_h */
