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

#ifndef __TrenchBroom__ChooseGameDialog__
#define __TrenchBroom__ChooseGameDialog__

#include "StringUtils.h"

#include <wx/dialog.h>

class wxButton;
class wxFrame;
class wxPanel;
class wxWindow;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace View {
        class GameListBox;
        class GameSelectedCommand;
        
        class ChooseGameDialog : public wxDialog {
        private:
            GameListBox* m_gameListBox;
            wxButton* m_openPreferencesButton;
        public:
            static String ShowNewDocument(wxWindow* parent);
            static String ShowOpenDocument(wxWindow* parent);
            
            ~ChooseGameDialog();
            
            const String selectedGameName() const;

            void OnGameSelected(GameSelectedCommand& event);
            void OnOpenPreferencesClicked(wxCommandEvent& event);
            void OnUpdateOkButton(wxUpdateUIEvent& event);
        private:
            ChooseGameDialog(wxWindow* parent, const wxString& title, const wxString& infoText);

            void createGui(const wxString& title, const wxString& infoText);
            wxPanel* createInfoPanel(wxWindow* parent, const wxString& title, const wxString& infoText);
            wxPanel* createGameList(wxWindow* parent);
            void bindEvents();

            void bindObservers();
            void unbindObservers();
            void preferenceDidChange(const IO::Path& path);
        };
    }
}

#endif /* defined(__TrenchBroom__ChooseGameDialog__) */
