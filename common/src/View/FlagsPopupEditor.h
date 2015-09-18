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

#ifndef TrenchBroom_FlagsPopupEditor
#define TrenchBroom_FlagsPopupEditor

#include <wx/panel.h>

class wxStaticText;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class FlagsEditor;
        class FlagChangedCommand;
        class PopupButton;
        
        class FlagsPopupEditor : public wxPanel {
        private:
            wxStaticText* m_flagsTxt;
            PopupButton* m_button;
            FlagsEditor* m_editor;
        public:
            FlagsPopupEditor(wxWindow* parent, size_t numCols, const wxString& buttonLabel = "...", bool showFlagsText = true);

            void setFlags(const wxArrayString& labels, const wxArrayString& tooltips = wxArrayString(0));
            void setFlags(const wxArrayInt& values, const wxArrayString& labels, const wxArrayString& tooltips = wxArrayString(0));
            void setFlagValue(int set, int mixed = 0);
            
            void OnFlagChanged(FlagChangedCommand& event);
            bool Enable(bool enable = true);
        private:
            void updateFlagsText();
        };
    }
}

#endif /* defined(TrenchBroom_FlagsPopupEditor) */
