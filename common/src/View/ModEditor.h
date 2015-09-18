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

#ifndef TrenchBroom_ModEditor
#define TrenchBroom_ModEditor

#include "StringUtils.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxBitmapButton;
class wxListBox;
class wxSearchCtrl;
class wxWindow;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace Model {
        class Object;
    }
    
    namespace View {
        class ModEditor : public wxPanel {
        private:
            MapDocumentWPtr m_document;
            
            wxListBox* m_availableModList;
            wxListBox* m_enabledModList;
            wxSearchCtrl* m_filterBox;
            
            StringList m_availableMods;
            bool m_ignoreNotifier;
        public:
            ModEditor(wxWindow* parent, MapDocumentWPtr document);
            ~ModEditor();
            
            void OnAddModClicked(wxCommandEvent& event);
            void OnRemoveModClicked(wxCommandEvent& event);
            void OnMoveModUpClicked(wxCommandEvent& event);
            void OnMoveModDownClicked(wxCommandEvent& event);
            void OnUpdateAddButtonUI(wxUpdateUIEvent& event);
            void OnUpdateRemoveButtonUI(wxUpdateUIEvent& event);
            void OnUpdateMoveUpButtonUI(wxUpdateUIEvent& event);
            void OnUpdateMoveDownButtonUI(wxUpdateUIEvent& event);
            void OnFilterBoxChanged(wxCommandEvent& event);
        private:
            void createGui();

            void bindObservers();
            void unbindObservers();
            
            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);
            void modsDidChange();
            void preferenceDidChange(const IO::Path& path);

            void updateAvailableMods();
            void updateMods();
        };
    }
}

#endif /* defined(TrenchBroom_ModEditor) */
