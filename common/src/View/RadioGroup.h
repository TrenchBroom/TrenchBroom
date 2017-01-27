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

#ifndef RadioGroup_h
#define RadioGroup_h

#include <wx/ctrlsub.h>
#include <wx/control.h>

#include <vector>

class wxRadioButton;

wxDECLARE_EVENT(wxEVT_RADIOGROUP, wxCommandEvent);

namespace TrenchBroom {
    namespace View {
        class RadioGroup : public wxControl, public wxItemContainerImmutable {
        private:
            typedef std::vector<wxRadioButton*> ButtonList;
            ButtonList m_buttons;
        public:
            RadioGroup();
            RadioGroup(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, size_t n, const wxString choices[]);
            
            bool Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, size_t n, const wxString choices[]);
        public:
            unsigned int GetCount() const;
            wxString GetString(unsigned int index) const;
            void SetString(unsigned int index, const wxString& str);
            int FindString(const wxString& str, bool caseSensitive = false) const;
            
            void SetSelection(int index);
            int GetSelection() const;
            wxString GetStringSelection() const;
        private:
            void OnRadioButton(wxCommandEvent& event);
        private:
            wxDECLARE_DYNAMIC_CLASS(RadioGroup);
        };
    }
}

#endif /* RadioGroup_h */
