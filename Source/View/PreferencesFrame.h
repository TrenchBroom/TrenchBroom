/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__PreferencesFrame__
#define __TrenchBroom__PreferencesFrame__

#include <wx/frame.h>

class wxPanel;
class wxToolBar;

namespace TrenchBroom {
    namespace View {
        class PreferencesFrame : public wxFrame {
        protected:
            wxToolBar* m_toolBar;
            wxPanel* m_generalPreferencePane;
        public:
            PreferencesFrame();

            void OnOkClicked(wxCommandEvent& event);
			void OnCancelClicked(wxCommandEvent& event);
			void OnClose(wxCloseEvent& event);
            void OnFileExit(wxCommandEvent& event);

            DECLARE_EVENT_TABLE();
        };
    }
}

#endif /* defined(__TrenchBroom__PreferencesFrame__) */
