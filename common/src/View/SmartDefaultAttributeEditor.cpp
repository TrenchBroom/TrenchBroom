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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SmartDefaultAttributeEditor.h"

#include "Assets/EntityDefinition.h"
#include "Model/ModelUtils.h"

#include <wx/sizer.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        SmartDefaultAttributeEditor::SmartDefaultAttributeEditor(View::MapDocumentWPtr document, View::ControllerWPtr controller) :
        SmartAttributeEditor(document, controller) {}

        wxWindow* SmartDefaultAttributeEditor::doCreateVisual(wxWindow* parent) {
            m_descriptionTxt = new wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_BESTWRAP | wxBORDER_NONE);
            return m_descriptionTxt;
        }
        
        void SmartDefaultAttributeEditor::doDestroyVisual() {
            m_descriptionTxt->Destroy();
            m_descriptionTxt = NULL;
        }

        void SmartDefaultAttributeEditor::doUpdateVisual(const Model::AttributableList& attributables) {
            m_descriptionTxt->Clear();

            const Assets::EntityDefinition* entityDefinition = Model::selectEntityDefinition(entities);
            if (entityDefinition != NULL)
                m_descriptionTxt->AppendText(entityDefinition->description());
        }
    }
}
