/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__ModEditor__
#define __TrenchBroom__ModEditor__

#include "StringUtils.h"
#include "View/ViewTypes.h"

#include <wx/collpane.h>

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
        class ModEditor : public wxCollapsiblePane {
        private:
            MapDocumentPtr m_document;
            ControllerPtr m_controller;
            
            wxListBox* m_availableModList;
            wxListBox* m_enabledModList;
            wxSearchCtrl* m_filterBox;
            wxBitmapButton* m_addModsButton;
            wxBitmapButton* m_removeModsButton;
            wxBitmapButton* m_moveModUpButton;
            wxBitmapButton* m_moveModDownButton;
            
            StringList m_availableMods;
        public:
            ModEditor(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller);
            ~ModEditor();
            
            void OnPaneChanged(wxCollapsiblePaneEvent& event);
            void OnUpdateButtonUI(wxUpdateUIEvent& event);
            void OnFilterBoxChanged(wxCommandEvent& event);
        private:
            void createGui();
            void bindEvents();

            void bindObservers();
            void unbindObservers();
            
            void documentWasNewed();
            void documentWasLoaded();
            void objectDidChange(Model::Object* object);
            void preferenceDidChange(const IO::Path& path);

            void updateAvailableMods();
            void updateMods();
        };
    }
}

#endif /* defined(__TrenchBroom__ModEditor__) */
