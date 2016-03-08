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

#ifndef TrenchBroom_EntityDefinitionFileChooser
#define TrenchBroom_EntityDefinitionFileChooser

#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxButton;
class wxListBox;
class wxStaticText;

namespace TrenchBroom {
    namespace View {
        class EntityDefinitionFileChooser : public wxPanel {
        private:
            MapDocumentWPtr m_document;
            
            wxListBox* m_builtin;
            wxStaticText* m_external;
            wxButton* m_chooseExternal;
            wxButton* m_reloadExternal;
        public:
            EntityDefinitionFileChooser(wxWindow* parent, MapDocumentWPtr document);
            ~EntityDefinitionFileChooser();
            
            void OnBuiltinSelectionChanged(wxCommandEvent& event);
            void OnChooseExternalClicked(wxCommandEvent& event);
            void OnReloadExternalClicked(wxCommandEvent& event);
            void OnUpdateReloadExternal(wxUpdateUIEvent& event);
        private:
            void createGui();
            void bindEvents();
            
            void bindObservers();
            void unbindObservers();
            
            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);
            void entityDefinitionsDidChange();
            
            void updateControls();
        };
    }
}

#endif /* defined(TrenchBroom_EntityDefinitionFileChooser) */
