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

#ifndef __TrenchBroom__PreferenceDialog__
#define __TrenchBroom__PreferenceDialog__

#include <wx/dialog.h>

class wxPanel;
class wxSimplebook;
class wxToolBar;
class wxToolBarToolBase;

namespace TrenchBroom {
    namespace View {
        class PreferencePane;
        
        class PreferenceDialog : public wxDialog {
        private:
            typedef enum {
                PrefPane_First = 0,
                PrefPane_Games = 0,
                PrefPane_General = 1,
                PrefPane_Keyboard = 2,
                PrefPane_Last = 2
            } PrefPane;

            wxToolBar* m_toolBar;
            wxSimplebook* m_book;
        public:
            PreferenceDialog();
            bool Create();
            
            void OnToolClicked(wxCommandEvent& event);
            void OnOKClicked(wxCommandEvent& event);
            void OnApplyClicked(wxCommandEvent& event);
            void OnCancelClicked(wxCommandEvent& event);
            void OnClose(wxCloseEvent& event);
            void OnFileClose(wxCommandEvent& event);

            DECLARE_DYNAMIC_CLASS(PreferenceDialog)
        private:
            void createGui();
            void bindEvents();
            
            void switchToPane(const PrefPane pane);
            void toggleTools(const PrefPane pane);

            PreferencePane* currentPane() const;
            PrefPane currentPaneId() const;

            void updateAcceleratorTable(const PrefPane pane);
        };
    }
}

#endif /* defined(__TrenchBroom__PreferenceDialog__) */
