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

#include "SmartSpawnflagsEditor.h"

#include "Assets/EntityDefinition.h"
#include "Assets/PropertyDefinition.h"
#include "Model/Entity.h"
#include "Model/ModelUtils.h"
#include "View/ControllerFacade.h"

#include <wx/checkbox.h>
#include <wx/settings.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        struct UpdateSpawnflag {
        private:
            ControllerPtr m_controller;
            const Model::PropertyKey& m_key;
            int m_flagIndex;
            bool m_setFlag;
        public:
            UpdateSpawnflag(ControllerPtr controller, const Model::PropertyKey& key, const int flagIndex, const bool setFlag) :
            m_controller(controller),
            m_key(key),
            m_flagIndex(flagIndex),
            m_setFlag(setFlag) {}
            
            void operator()(Model::Entity* entity) const {
                const Model::PropertyValue value = propertyValue(entity);
                m_controller->setEntityProperty(*entity, m_key, value);
            }
            
            Model::PropertyValue propertyValue(Model::Entity* entity) const {
                int intValue = entity->hasProperty(m_key) ? std::atoi(entity->property(m_key).c_str()) : 0;
                
                if (m_setFlag)
                    intValue |= m_flagIndex;
                else
                    intValue &= ~m_flagIndex;
                
                StringStream str;
                str << intValue;
                return str.str();
            }
        };
        
        SmartSpawnflagsEditor::SmartSpawnflagsEditor(View::MapDocumentPtr document, View::ControllerPtr controller) :
        SmartPropertyEditor(document, controller),
        m_scrolledWindow(NULL) {}

        void SmartSpawnflagsEditor::OnCheckBoxClicked(wxCommandEvent& event) {
            const Model::EntityList& updateEntities = entities();
            if (updateEntities.empty())
                return;
            
            const int flag = getFlagFromEvent(event);
            if (flag == 0)
                return;
            
            controller()->beginUndoableGroup("Set Spawnflags");
            Model::each(updateEntities.begin(), updateEntities.end(),
                        UpdateSpawnflag(controller(), key(), flag, event.IsChecked()),
                        Model::MatchAll());
            controller()->closeGroup();
        }
        
        wxWindow* SmartSpawnflagsEditor::doCreateVisual(wxWindow* parent) {
            assert(m_scrolledWindow == NULL);
            
            m_scrolledWindow = new wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, static_cast<long>(wxHSCROLL | wxVSCROLL | wxBORDER_SUNKEN));
            m_scrolledWindow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            
            const size_t numRows = NumFlags / NumCols;
            wxFlexGridSizer* sizer = new wxFlexGridSizer(static_cast<int>(numRows),
                                                         static_cast<int>(NumCols),
                                                         0, 0);
            m_flags = CheckBoxList(NumFlags, NULL);
            for (size_t row = 0; row < numRows; ++row) {
                for (size_t col = 0; col < NumCols; ++col) {
                    const size_t index = col * numRows + row;
                    m_flags[index] = new wxCheckBox(m_scrolledWindow, wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE);
                    m_flags[index]->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &SmartSpawnflagsEditor::OnCheckBoxClicked, this);
                    sizer->Add(m_flags[index]);
                }
            }
            
            m_scrolledWindow->SetSizerAndFit(sizer);
            m_scrolledWindow->SetScrollRate(1, m_flags[0]->GetSize().y);
            
            return m_scrolledWindow;
        }
        
        void SmartSpawnflagsEditor::doDestroyVisual() {
            assert(m_scrolledWindow != NULL);
            m_lastScrollPos = m_scrolledWindow->GetViewStart();
            m_scrolledWindow->Destroy();
            m_scrolledWindow = NULL;
            m_flags.clear();
        }

        void SmartSpawnflagsEditor::doUpdateVisual() {
            assert(!entities().empty());
            const FlagList flags = getFlagValuesFromEntities(entities());
            const Assets::EntityDefinition* definition = Model::selectEntityDefinition(entities());
            
            for (size_t i = 0; i < NumFlags; ++i)
                setFlagCheckBox(i, flags, definition);
            
            m_scrolledWindow->Layout();
            m_scrolledWindow->FitInside();
            m_scrolledWindow->Refresh();
            resetScrollPos();
        }
        
        void SmartSpawnflagsEditor::resetScrollPos() {
            // TODO: the y position is not properly set (at least on OS X)
            int xRate, yRate;
            m_scrolledWindow->GetScrollPixelsPerUnit(&xRate, &yRate);
            m_scrolledWindow->Scroll(m_lastScrollPos.x * xRate, m_lastScrollPos.y * yRate);
        }

        SmartSpawnflagsEditor::FlagList SmartSpawnflagsEditor::getFlagValuesFromEntities(const Model::EntityList& entities) const {
            FlagList result(NumFlags, Unset);
            
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                const Model::Entity* entity = *it;
                setFlagValue(*entity, result);
            }
            
            return result;
        }
        
        void SmartSpawnflagsEditor::setFlagValue(const Model::Entity& entity, FlagList& flags) const {
            for (size_t i = 0; i < NumFlags; ++i) {
                const bool set = isFlagSetOnEntity(entity, i);
                switch (flags[i]) {
                    case Unset:
                        flags[i] = set ? On : Off;
                        break;
                    case On:
                        if (!set)
                            flags[i] = Mixed;
                        break;
                    case Off:
                        if (set)
                            flags[i] = Mixed;
                        break;
                    default:
                        break;
                }
            }
        }

        bool SmartSpawnflagsEditor::isFlagSetOnEntity(const Model::Entity& entity, const size_t index) const {
            const bool hasFlag = entity.hasProperty(key());
            if (!hasFlag)
                return false;
            
            const Model::PropertyValue& propertyValue = entity.property(key());
            const int flagValue = std::atoi(propertyValue.c_str());
            return (flagValue & (1 << index)) != 0;
        }

        void SmartSpawnflagsEditor::setFlagCheckBox(const size_t index, const FlagList& flags, const Assets::EntityDefinition* definition) {
            wxColour colour;
            wxString label;
            getColorAndLabelForFlag(index, definition, colour, label);
            
            wxCheckBox* checkBox = m_flags[index];
            checkBox->SetLabel(label);
            checkBox->SetForegroundColour(colour);

            const FlagValue flag = flags[index];
            switch (flag) {
                case On:
                    checkBox->Set3StateValue(wxCHK_CHECKED);
                    break;
                case Mixed:
                    checkBox->Set3StateValue(wxCHK_UNDETERMINED);
                    break;
                default:
                    checkBox->Set3StateValue(wxCHK_UNCHECKED);
                    break;
            }
        }
        
        void SmartSpawnflagsEditor::getColorAndLabelForFlag(const size_t flag, const Assets::EntityDefinition* definition, wxColour& colour, wxString& label) const {
            if (definition != NULL) {
                const Assets::FlagsPropertyDefinition* flagDefs = definition->spawnflags();
                
                const Assets::FlagsPropertyOption* flagDef = flagDefs != NULL ? flagDefs->option(static_cast<int>(1 << flag)) : NULL;
                if (flagDef != NULL) {
                    colour = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT);
                    label << flagDef->description();
                } else {
                    colour = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
                    label << (1 << flag);
                }
            } else {
                colour = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
                label << (1 << flag);
            }
        }

        int SmartSpawnflagsEditor::getFlagFromEvent(wxCommandEvent& event) const {
            int flag = 0;
            for (size_t i = 0; i < NumFlags && flag == 0; i++) {
                if (m_flags[i]->GetId() == event.GetId())
                    flag = (1 << i);
            }
            return flag;
        }
    }
}
