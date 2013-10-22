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

#ifndef __TrenchBroom__GamesPreferencePane__
#define __TrenchBroom__GamesPreferencePane__

#include <iostream>

#include "View/PreferencePane.h"

class wxButton;
class wxCheckBox;
class wxChoice;
class wxSlider;
class wxStaticText;

namespace TrenchBroom {
    namespace View {
        class GamesPreferencePane : public PreferencePane {
        private:
            wxChoice* m_gameSelectionChoice;
            wxStaticText* m_gamePathValueLabel;
            wxButton* m_chooseGamePathButton;
        public:
            GamesPreferencePane(wxWindow* parent);
        private:
            void createGui();
            wxWindow* createGameSelectionBox();
            wxWindow* createGamePreferences();
            
            void bindEvents();
            
            void updateControls();
            bool doValidate();
        };
    }
}

#endif /* defined(__TrenchBroom__GamesPreferencePane__) */
