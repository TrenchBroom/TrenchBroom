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

#ifndef TrenchBroom_FlagsEditor
#define TrenchBroom_FlagsEditor

#include <wx/panel.h>

#include <vector>

class wxCheckBox;
class wxCommandEvent;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class FlagsEditor : public wxPanel {
        private:
            typedef std::vector<wxCheckBox*> CheckBoxList;
            typedef std::vector<int> ValueList;

            size_t m_numCols;
            CheckBoxList m_checkBoxes;
            ValueList m_values;
        public:
            FlagsEditor(wxWindow* parent, size_t numCols);
            
            void setFlags(const wxArrayString& labels, const wxArrayString& tooltips = wxArrayString(0));
            void setFlags(const wxArrayInt& values, const wxArrayString& labels, const wxArrayString& tooltips = wxArrayString(0));
            void setFlagValue(int set, int mixed = 0);

            size_t getNumFlags() const;
            bool isFlagSet(size_t index) const;
            bool isFlagMixed(size_t index) const;
            int getSetFlagValue() const;
            int getMixedFlagValue() const;
            wxString getFlagLabel(size_t index) const;
            
            int lineHeight() const;
            
            void OnCheckBoxClicked(wxCommandEvent& event);
        private:
            void setCheckBoxCount(size_t count);
            size_t getIndexFromEvent(const wxCommandEvent& event) const;
        };
    }
}

#endif /* defined(TrenchBroom_FlagsEditor) */
