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

#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxBitmapButton;
class wxStaticText;
class wxWindow;

namespace TrenchBroom {
    namespace Model {
        class Object;
    }
    
    namespace View {
        class ModEditor : public wxPanel {
        private:
            MapDocumentPtr m_document;
            ControllerPtr m_controller;
            
            wxStaticText* m_modList;
            wxBitmapButton* m_editModsButton;
        public:
            ModEditor(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller);
            ~ModEditor();
        private:
            void createGui();
            void bindEvents();

            void bindObservers();
            void unbindObservers();
            
            void documentWasNewed();
            void documentWasLoaded();
            void objectDidChange(Model::Object* object);

            void updateMods();
        };
    }
}

#endif /* defined(__TrenchBroom__ModEditor__) */
