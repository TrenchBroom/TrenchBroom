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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__NewDocumentDialog__
#define __TrenchBroom__NewDocumentDialog__

#include "StringUtils.h"

#include <wx/dialog.h>

class wxFrame;
class wxPanel;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class GameListBox;
        class GameSelectedCommand;
        
        class NewDocumentDialog : public wxDialog {
        private:
            GameListBox* m_gameListBox;
        public:
            NewDocumentDialog(wxWindow* parent);

            const String selectedGameName() const;

            void OnGameSelected(GameSelectedCommand& event);
            void OnUpdateOkButton(wxUpdateUIEvent& event);
        private:
            void createGui();
            wxPanel* createInfoPanel(wxWindow* parent);
            wxPanel* createGameList(wxWindow* parent);
            void bindEvents();
        };
    }
}

#endif /* defined(__TrenchBroom__NewDocumentDialog__) */
