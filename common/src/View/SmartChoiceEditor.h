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

#ifndef TrenchBroom_SmartChoiceEditor
#define TrenchBroom_SmartChoiceEditor

#include "Model/ModelTypes.h"
#include "View/SmartAttributeEditor.h"
#include "View/ViewTypes.h"

class wxComboBox;
class wxCommandEvent;
class wxPanel;
class wxStaticText;
class wxWindow;

namespace TrenchBroom {
    namespace Assets {
        class ChoicePropertyDefinition;
    }
    
    namespace View {
        class SmartChoiceEditor : public SmartAttributeEditor {
        private:
            wxPanel* m_panel;
            wxComboBox* m_comboBox;
        public:
            SmartChoiceEditor(View::MapDocumentWPtr document);
            
            void OnComboBox(wxCommandEvent& event);
            void OnTextEnter(wxCommandEvent& event);
        private:
            wxWindow* doCreateVisual(wxWindow* parent);
            void doDestroyVisual();
            void doUpdateVisual(const Model::AttributableNodeList& attributables);
        };
    }
}

#endif /* defined(TrenchBroom_SmartChoiceEditor) */
