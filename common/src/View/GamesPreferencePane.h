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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_GamesPreferencePane
#define TrenchBroom_GamesPreferencePane

#include <iostream>

#include "View/PreferencePane.h"

class wxButton;
class wxStaticText;

namespace TrenchBroom {
    namespace View {
        class GameListBox;
        class GameSelectionCommand;
        
        class GamesPreferencePane : public PreferencePane {
        private:
            GameListBox* m_gameListBox;
            wxStaticText* m_gamePathValueLabel;
            wxButton* m_chooseGamePathButton;
        public:
            GamesPreferencePane(wxWindow* parent);

            void OnGameSelectionChanged(GameSelectionCommand& event);
            void OnChooseGamePathClicked(wxCommandEvent& event);
        private:
            void createGui();
            wxWindow* createGamePreferences();
            
            void bindEvents();
            
            bool doCanResetToDefaults();
            void doResetToDefaults();
            void doUpdateControls();
            bool doValidate();
        };
    }
}

#endif /* defined(TrenchBroom_GamesPreferencePane) */
