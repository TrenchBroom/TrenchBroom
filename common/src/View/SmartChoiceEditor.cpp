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

#include "SmartChoiceEditor.h"
#include "Assets/AttributeDefinition.h"
#include "Model/AttributableNode.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"

#include <wx/combobox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        SmartChoiceEditor::SmartChoiceEditor(View::MapDocumentWPtr document) :
        SmartAttributeEditor(document),
        m_panel(NULL),
        m_comboBox(NULL) {}

        void SmartChoiceEditor::OnComboBox(wxCommandEvent& event) {
            if (m_panel->IsBeingDeleted()) return;

            const String valueDescStr = m_comboBox->GetValue().ToStdString();
            const String valueStr = valueDescStr.substr(0, valueDescStr.find_first_of(':') - 1);
            document()->setAttribute(name(), valueStr);
        }
        
        void SmartChoiceEditor::OnTextEnter(wxCommandEvent& event) {
            if (m_panel->IsBeingDeleted()) return;

            document()->setAttribute(name(), m_comboBox->GetValue().ToStdString());
        }

        wxWindow* SmartChoiceEditor::doCreateVisual(wxWindow* parent) {
            assert(m_panel == NULL);
            assert(m_comboBox == NULL);
            
            m_panel = new wxPanel(parent);
            wxStaticText* infoText = new wxStaticText(m_panel, wxID_ANY, "Select a choice option:");
#if defined __APPLE__
            infoText->SetFont(*wxSMALL_FONT);
#endif
            m_comboBox = new wxComboBox(m_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxTE_PROCESS_ENTER);
            m_comboBox->Bind(wxEVT_COMBOBOX, &SmartChoiceEditor::OnComboBox, this);
            m_comboBox->Bind(wxEVT_TEXT_ENTER, &SmartChoiceEditor::OnTextEnter, this);
            
            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->AddSpacer(LayoutConstants::WideVMargin);
            sizer->Add(infoText, 0, wxLEFT | wxRIGHT, LayoutConstants::WideHMargin);
            sizer->AddSpacer(LayoutConstants::WideVMargin);
            sizer->Add(m_comboBox, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::WideHMargin);
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
        
        void SmartChoiceEditor::doUpdateVisual(const Model::AttributableNodeList& attributables) {
            assert(m_panel != NULL);
            assert(m_comboBox != NULL);
            
            m_comboBox->Clear();

            const Assets::AttributeDefinition* attrDef = Model::AttributableNode::selectAttributeDefinition(name(), attributables);
            if (attrDef == NULL || attrDef->type() != Assets::AttributeDefinition::Type_ChoiceAttribute) {
                m_comboBox->Disable();
            } else {
                const Assets::ChoiceAttributeDefinition* choiceDef = static_cast<const Assets::ChoiceAttributeDefinition*>(attrDef);
                const Assets::ChoiceAttributeOption::List& options = choiceDef->options();
                
                Assets::ChoiceAttributeOption::List::const_iterator it, end;
                for (it = options.begin(), end = options.end(); it != end; ++it) {
                    const Assets::ChoiceAttributeOption& option = *it;
                    m_comboBox->Append(option.value() + " : " + option.description());
                }
                
                const Model::AttributeValue value = Model::AttributableNode::selectAttributeValue(name(), attributables);
                m_comboBox->SetValue(value);
            }
        }
    }
}
