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

#ifndef __TrenchBroom__MapPropertiesDialog__
#define __TrenchBroom__MapPropertiesDialog__

#include "Utility/String.h"

#include <wx/dialog.h>

class wxButton;
class wxChoice;
class wxListBox;

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
    }
    
    namespace View {
        class MapPropertiesDialog : public wxDialog {
        protected:
            Model::MapDocument& m_document;
            
            wxChoice* m_modChoice;
            wxChoice* m_defChoice;
            wxListBox* m_wadList;
            wxButton* m_addWadButton;
            wxButton* m_removeWadsButton;
            wxButton* m_changeWadPathsButton;
            wxButton* m_moveWadUpButton;
            wxButton* m_moveWadDownButton;
            
            void populateDefChoice(String def);
            void populateModChoice(String mod);
            void populateWadList();
            
            void init();
        public:
            MapPropertiesDialog(Model::MapDocument& document);
            
            DECLARE_EVENT_TABLE();
        };
    }
}

#endif /* defined(__TrenchBroom__MapPropertiesDialog__) */
