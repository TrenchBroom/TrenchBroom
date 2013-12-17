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

#include "SmartChoiceEditor.h"
#include "Assets/PropertyDefinition.h"
#include "Model/ModelUtils.h"
#include "View/ControllerFacade.h"
#include "View/LayoutConstants.h"

#include <wx/combobox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        SmartChoiceEditor::SmartChoiceEditor(View::MapDocumentPtr document, View::ControllerPtr controller) :
        SmartPropertyEditor(document, controller),
        m_panel(NULL),
        m_comboBox(NULL) {}

        void SmartChoiceEditor::OnComboBox(wxCommandEvent& event) {
            const String valueDescStr = m_comboBox->GetValue().ToStdString();
            const String valueStr = valueDescStr.substr(0, valueDescStr.find_first_of(':') - 1);
            controller()->setEntityProperty(entities(), key(), valueStr);
        }
        
        void SmartChoiceEditor::OnTextEnter(wxCommandEvent& event) {
            controller()->setEntityProperty(entities(), key(), m_comboBox->GetValue().ToStdString());
        }

        wxWindow* SmartChoiceEditor::doCreateVisual(wxWindow* parent) {
            assert(m_panel == NULL);
            assert(m_comboBox == NULL);
            
            m_panel = new wxPanel(parent);
            wxStaticText* infoText = new wxStaticText(m_panel, wxID_ANY, _("Select a choice option:"));
#if defined __APPLE__
            infoText->SetFont(*wxSMALL_FONT);
#endif
            m_comboBox = new wxComboBox(m_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxTE_PROCESS_ENTER);
            m_comboBox->Bind(wxEVT_COMBOBOX, &SmartChoiceEditor::OnComboBox, this);
            m_comboBox->Bind(wxEVT_TEXT_ENTER, &SmartChoiceEditor::OnTextEnter, this);
            
            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(infoText);
            sizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            sizer->Add(m_comboBox, 0, wxEXPAND);
            sizer->AddStretchSpacer();
            
            m_panel->SetSizer(sizer);
            return m_panel;
        }
        
        void SmartChoiceEditor::doDestroyVisual() {
            assert(m_panel != NULL);
            assert(m_comboBox != NULL);
            
            m_panel->Destroy();
            m_panel = NULL;
            m_comboBox= NULL;
        }
        
        void SmartChoiceEditor::doUpdateVisual(const Model::EntityList& entities) {
            assert(m_panel != NULL);
            assert(m_comboBox != NULL);
            
            m_comboBox->Clear();

            const Assets::PropertyDefinition* propDef = Model::selectPropertyDefinition(key(), entities);
            if (propDef == NULL || propDef->type() != Assets::PropertyDefinition::ChoiceProperty) {
                m_comboBox->Disable();
            } else {
                const Assets::ChoicePropertyDefinition* choiceDef = static_cast<const Assets::ChoicePropertyDefinition*>(propDef);
                const Assets::ChoicePropertyOption::List& options = choiceDef->options();
                
                Assets::ChoicePropertyOption::List::const_iterator it, end;
                for (it = options.begin(), end = options.end(); it != end; ++it) {
                    const Assets::ChoicePropertyOption& option = *it;
                    m_comboBox->Append(option.value() + " : " + option.description());
                }
                
                const Model::PropertyValue value = Model::selectPropertyValue(key(), entities);
                m_comboBox->SetValue(value);
            }
        }
    }
}
