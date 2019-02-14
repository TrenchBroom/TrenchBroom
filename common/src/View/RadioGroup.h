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
            using ButtonList = std::vector<wxRadioButton*>;
            ButtonList m_buttons;
        public:
            RadioGroup();
            RadioGroup(QWidget* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, size_t n, const QString choices[]);
            
            bool Create(QWidget* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, size_t n, const QString choices[]);
        public:
            unsigned int GetCount() const override;
            QString GetString(unsigned int index) const override;
            void SetString(unsigned int index, const QString& str) override;
            int FindString(const QString& str, bool caseSensitive = false) const override;
            
            void SetSelection(int index) override;
            int GetSelection() const override;
            QString GetStringSelection() const override;
        private:
            void OnRadioButton(wxCommandEvent& event);
        private:
            wxDECLARE_DYNAMIC_CLASS(RadioGroup);
        };
    }
}

#endif /* RadioGroup_h */
