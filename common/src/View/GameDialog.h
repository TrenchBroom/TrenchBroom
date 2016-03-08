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

#ifndef TrenchBroom_GameDialog
#define TrenchBroom_GameDialog

#include "StringUtils.h"
#include "Model/MapFormat.h"

#include <wx/dialog.h>

class wxButton;
class wxChoice;
class wxWindow;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    namespace View {
        class GameListBox;
        class GameSelectionCommand;

        class GameDialog : public wxDialog {
        protected:
            GameListBox* m_gameListBox;
            wxChoice* m_mapFormatChoice;
            wxButton* m_openPreferencesButton;
        public:
            virtual ~GameDialog();
            
            static bool showNewDocumentDialog(wxWindow* parent, String& gameName, Model::MapFormat::Type& mapFormat);
            static bool showOpenDocumentDialog(wxWindow* parent, String& gameName, Model::MapFormat::Type& mapFormat);
            
            String selectedGameName() const;
            Model::MapFormat::Type selectedMapFormat() const;

            void OnGameSelectionChanged(GameSelectionCommand& command);
            void OnGameSelected(GameSelectionCommand& command);
            void OnUpdateMapFormatChoice(wxUpdateUIEvent& event);
            
            void OnOpenPreferencesClicked(wxCommandEvent& event);
            void OnUpdateOkButton(wxUpdateUIEvent& event);
        protected:
            GameDialog();
            void createDialog(wxWindow* parent, const wxString& title, const wxString& infoText);

            void createGui(const wxString& title, const wxString& infoText);
            virtual wxWindow* createInfoPanel(wxWindow* parent, const wxString& title, const wxString& infoText);
            virtual wxWindow* createSelectionPanel(wxWindow* parent);
        private:
            bool isOkEnabled() const;
            void gameSelectionChanged(const String& gameName);
            void updateMapFormats(const String& gameName);
            void gameSelected(const String& gameName);
            
            void bindObservers();
            void unbindObservers();
            void preferenceDidChange(const IO::Path& path);
        };
    }
}

#endif /* defined(TrenchBroom_GameDialog) */
