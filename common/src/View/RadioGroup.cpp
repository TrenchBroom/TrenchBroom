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

#include "RadioGroup.h"

#include "Macros.h"

#include <wx/radiobut.h>
#include <wx/sizer.h>

wxDEFINE_EVENT(wxEVT_RADIOGROUP, wxCommandEvent);

namespace TrenchBroom {
    namespace View {
        wxIMPLEMENT_DYNAMIC_CLASS(RadioGroup, wxControl)

        RadioGroup::RadioGroup() :
        wxControl() {}

        RadioGroup::RadioGroup(wxWindow* parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, const size_t n, const wxString choices[]) :
        wxControl() {
            Create(parent, id, pos, size, n, choices);
        }

        bool RadioGroup::Create(wxWindow* parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, const size_t n, const wxString choices[]) {
            if (!wxControl::Create(parent, id, pos, size, wxBORDER_NONE))
                return false;

            m_buttons.reserve(n);
            if (n > 0) {
                wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
                m_buttons.push_back(new wxRadioButton(this, wxID_ANY, choices[0], wxDefaultPosition, wxDefaultSize, wxRB_GROUP));
                m_buttons.back()->Bind(wxEVT_RADIOBUTTON, &RadioGroup::OnRadioButton, this);
                sizer->Add(m_buttons.back());
                for (size_t i = 1; i < n; ++i) {
                    m_buttons.push_back(new wxRadioButton(this, wxID_ANY, choices[i]));
                    m_buttons.back()->Bind(wxEVT_RADIOBUTTON, &RadioGroup::OnRadioButton, this);
                    sizer->Add(m_buttons.back());
                }
                SetSizer(sizer);
            }

            return true;
        }

        unsigned int RadioGroup::GetCount() const {
            return static_cast<unsigned int>(m_buttons.size());
        }

        wxString RadioGroup::GetString(const unsigned int index) const {
            if (index > GetCount())
                return "";
            return m_buttons[static_cast<size_t>(index)]->GetLabel();
        }

        void RadioGroup::SetString(const unsigned int index, const wxString& str) {
            assert(index < GetCount());
            m_buttons[static_cast<size_t>(index)]->SetLabel(str);
        }

        int RadioGroup::FindString(const wxString& str, const bool caseSensitive) const {
            for (size_t i = 0; i < m_buttons.size(); ++i) {
                if (m_buttons[i]->GetLabel().IsSameAs(str, caseSensitive))
                    return static_cast<int>(i);
            }
            return wxNOT_FOUND;
        }

        void RadioGroup::SetSelection(const int index) {
            assert(index >= 0);
            const size_t indexS = static_cast<size_t>(index);
            ensure(indexS < m_buttons.size(), "index out of range");
            m_buttons[indexS]->SetValue(true);
        }

        int RadioGroup::GetSelection() const {
            for (size_t i = 0; i < m_buttons.size(); ++i)
                if (m_buttons[i]->GetValue())
                    return static_cast<int>(i);
            return wxNOT_FOUND;
        }

        wxString RadioGroup::GetStringSelection() const {
            const int index = GetSelection();
            if (index == wxNOT_FOUND)
                return "";
            return GetString(static_cast<unsigned int>(index));
        }

        void RadioGroup::OnRadioButton(wxCommandEvent& event) {
            wxCommandEvent newEvent(wxEVT_RADIOGROUP, GetId());
            newEvent.SetInt(GetSelection());
            ProcessEvent(newEvent);
        }
    }
}
