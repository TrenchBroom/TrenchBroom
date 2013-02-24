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

#include "SpawnFlagsEditor.h"

#include "Controller/EntityPropertyCommand.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/PropertyDefinition.h"
#include "Utility/CommandProcessor.h"

#include <wx/checkbox.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        wxWindow* SpawnFlagsEditor::createVisual(wxWindow* parent) {
            assert(m_scrolledWindow == NULL);
            
            m_scrolledWindow = new wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, static_cast<long>(wxVSCROLL | wxBORDER_SUNKEN));
            m_scrolledWindow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));

            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            for (unsigned int i = 0; i < 24; i++) {
                m_flags[i] = new wxCheckBox(m_scrolledWindow, wxID_ANY, wxT("                                 "), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE);
                m_flags[i]->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &SpawnFlagsEditor::OnCheckBoxClicked, this);
                sizer->Add(m_flags[i]);
            }
            
            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(m_scrolledWindow, 1, wxEXPAND);
            parent->SetSizer(outerSizer);
            m_scrolledWindow->SetSizerAndFit(sizer);
            m_scrolledWindow->SetScrollRate(0, m_flags[0]->GetSize().y);
            m_scrolledWindow->Scroll(m_lastScrollPos);

            return m_scrolledWindow;
        }
        
        void SpawnFlagsEditor::destroyVisual() {
            assert(m_scrolledWindow != NULL);
            m_lastScrollPos = m_scrolledWindow->GetViewStart();
            m_scrolledWindow->Destroy();
            m_scrolledWindow = NULL;
        }
        
        void SpawnFlagsEditor::updateVisual() {
            const Model::EntityList entities = selectedEntities();
            FlagValue values[24];
            for (size_t i = 0; i < 24; i++)
                values[i] = Unset;
            
            Model::EntityDefinition* definition = NULL;
            if (!entities.empty()) {
                definition = entities[0]->definition();
                for (size_t i = 1; i < entities.size() && definition != NULL; i++) {
                    if (definition != entities[i]->definition())
                        definition = NULL;
                }
                
                for (size_t i = 0; i < entities.size(); i++) {
                    const Model::Entity& entity = *entities[i];
                    const Model::PropertyValue* value = entity.propertyForKey(property());
                    if (value != NULL) {
                        int intValue = std::atoi(value->c_str());
                        for (size_t j = 0; j < 24; j++) {
                            bool set = (intValue & (1 << j)) != 0;
                            switch (values[j]) {
                                case Unset:
                                    values[j] = set ? On : Off;
                                    break;
                                case On:
                                    if (!set)
                                        values[j] = Mixed;
                                    break;
                                case Off:
                                    if (set)
                                        values[j] = Mixed;
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
            }
            
            for (unsigned int i = 0; i < 24; i++) {

                wxColour colour = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT);
                wxString label;
                if (i == 8) {
                    label << "!Easy";
                } else if (i == 9) {
                    label << "!Normal";
                } else if (i == 10) {
                    label << "!Hard";
                } else if (i == 11) {
                    label << "!DM";
                } else if (definition != NULL) {
                    const Model::FlagsPropertyOption* spawnflag = definition->spawnflags().option(static_cast<int>(1 << i));
                    if (spawnflag != NULL) {
                        label << spawnflag->description();
                    } else {
                        label << (1 << i);
                        colour = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
                    }
                } else {
                    label << (1 << i);
                    colour = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
                }
                
                m_flags[i]->SetLabel(label);
                m_flags[i]->SetForegroundColour(colour);
                
                switch (values[i]) {
                    case On:
                        m_flags[i]->Set3StateValue(wxCHK_CHECKED);
                        break;
                    case Mixed:
                        m_flags[i]->Set3StateValue(wxCHK_UNDETERMINED);
                        break;
                    default:
                        m_flags[i]->Set3StateValue(wxCHK_UNCHECKED);
                        break;
                }
            }
        }

        SpawnFlagsEditor::SpawnFlagsEditor(SmartPropertyEditorManager& manager) :
        SmartPropertyEditor(manager),
        m_scrolledWindow(NULL),
        m_lastScrollPos(wxPoint(0,0)) {}

        void SpawnFlagsEditor::OnCheckBoxClicked(wxCommandEvent& event) {
            const Model::EntityList entities = selectedEntities();
            if (entities.empty())
                return;
            
            int flag = 0;
            for (size_t i = 0; i < 24 && flag == 0; i++) {
                if (m_flags[i]->GetId() == event.GetId())
                    flag = (1 << i);
            }
            
            if (flag != 0) {
                CommandProcessor::BeginGroup(document().GetCommandProcessor(), wxT("Set Spawnflags"));
                
                Model::EntityList::const_iterator it, end;
                for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                    Model::Entity& entity = **it;
                    const Model::PropertyValue* value = entity.propertyForKey(property());
                    int intValue = value == NULL ? 0 : std::atoi(value->c_str());
                    
                    if (event.IsChecked())
                        intValue |= flag;
                    else
                        intValue &= ~flag;
                    
                    StringStream newValue;
                    newValue << intValue;
                    
                    Controller::EntityPropertyCommand* command = Controller::EntityPropertyCommand::setEntityPropertyValue(document(), entity, property(), newValue.str());
                    document().GetCommandProcessor()->Submit(command);
                }

                CommandProcessor::EndGroup(document().GetCommandProcessor());
            }
        }
    }
}
