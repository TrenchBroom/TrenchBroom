/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "FlagsEditor.h"

#include "View/FlagChangedCommand.h"
#include "View/ViewConstants.h"

#include <cassert>
#include <wx/checkbox.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        FlagsEditor::FlagsEditor(wxWindow* parent, const size_t numCols) :
        wxPanel(parent),
        m_numCols(numCols) {
            assert(m_numCols > 0);
        }
        
        void FlagsEditor::setFlags(const wxArrayString& labels, const wxArrayString& tooltips) {
            wxArrayInt values(labels.size());
            for (size_t i = 0; i < labels.size(); ++i)
                values[i] = (1 << i);
            setFlags(values, labels, tooltips);
        }

        void FlagsEditor::setFlags(const wxArrayInt& values, const wxArrayString& labels, const wxArrayString& tooltips) {
            const size_t count = values.size();
            setCheckBoxCount(count);
            
            const size_t numRows = count / m_numCols;
            wxFlexGridSizer* sizer = new wxFlexGridSizer(static_cast<int>(numRows),
                                                         static_cast<int>(m_numCols),
                                                         0, LayoutConstants::WideHMargin);
            
            for (size_t row = 0; row < numRows; ++row) {
                for (size_t col = 0; col < m_numCols; ++col) {
                    const size_t index = col * numRows + row;
                    if (index < count) {
                        m_checkBoxes[index]->SetLabel(index < labels.size() ? labels[index] : wxString() << (1 << index));
                        m_checkBoxes[index]->SetToolTip(index < tooltips.size() ? tooltips[index] : "");
                        m_values[index] = values[index];
                        sizer->Add(m_checkBoxes[index]);
                    }
                }
            }
            
            SetSizerAndFit(sizer);
        }
        
        void FlagsEditor::setFlagValue(const int on, const int mixed) {
            for (size_t i = 0; i < m_checkBoxes.size(); ++i) {
                wxCheckBox* checkBox = m_checkBoxes[i];
                const int value = m_values[i];
                const bool isMixed = (mixed & value) != 0;
                const bool isChecked = (on & value) != 0;
                if (isMixed)
                    checkBox->Set3StateValue(wxCHK_UNDETERMINED);
                else if (isChecked)
                    checkBox->Set3StateValue(wxCHK_CHECKED);
                else
                    checkBox->Set3StateValue(wxCHK_UNCHECKED);
            }
        }
        
        size_t FlagsEditor::getNumFlags() const {
            return m_checkBoxes.size();
        }

        bool FlagsEditor::isFlagSet(const size_t index) const {
            assert(index < m_checkBoxes.size());
            return m_checkBoxes[index]->Get3StateValue() == wxCHK_CHECKED;
        }
        
        bool FlagsEditor::isFlagMixed(const size_t index) const {
            assert(index < m_checkBoxes.size());
            return m_checkBoxes[index]->Get3StateValue() == wxCHK_UNDETERMINED;
        }

        int FlagsEditor::getSetFlagValue() const {
            int value = 0;
            for (size_t i = 0; i < m_checkBoxes.size(); ++i) {
                if (isFlagSet(i))
                    value |= m_values[i];
            }
            return value;
        }
        
        int FlagsEditor::getMixedFlagValue() const {
            int value = 0;
            for (size_t i = 0; i < m_checkBoxes.size(); ++i) {
                if (isFlagMixed(i))
                    value |= m_values[i];
            }
            return value;
        }

        wxString FlagsEditor::getFlagLabel(const size_t index) const {
            assert(index < m_checkBoxes.size());
            return m_checkBoxes[index]->GetLabel();
        }

        int FlagsEditor::lineHeight() const {
            assert(!m_checkBoxes.empty());
            return m_checkBoxes.front()->GetSize().y;
        }

        void FlagsEditor::OnCheckBoxClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const size_t index = getIndexFromEvent(event);
            assert(index < m_checkBoxes.size());
            
            FlagChangedCommand command;
            command.setValues(index, getSetFlagValue(), getMixedFlagValue());
            command.SetEventObject(this);
            command.SetId(GetId());
            ProcessEvent(command);
        }

        void FlagsEditor::setCheckBoxCount(const size_t count) {
            while (count > m_checkBoxes.size()) {
                wxCheckBox* checkBox = new wxCheckBox(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxCHK_3STATE);
                checkBox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &FlagsEditor::OnCheckBoxClicked, this);
                m_checkBoxes.push_back(checkBox);
            }
            while (count < m_checkBoxes.size())
                delete m_checkBoxes.back(), m_checkBoxes.pop_back();
            m_values.resize(count);
        }

        size_t FlagsEditor::getIndexFromEvent(const wxCommandEvent& event) const {
            for (size_t i = 0; i < m_checkBoxes.size(); ++i)
                if (event.GetEventObject() == m_checkBoxes[i])
                    return i;
            return m_checkBoxes.size();
        }
    }
}
