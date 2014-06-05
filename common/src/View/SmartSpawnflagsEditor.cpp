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

#include "SmartSpawnflagsEditor.h"

#include "Assets/EntityDefinition.h"
#include "Assets/PropertyDefinition.h"
#include "Model/Entity.h"
#include "Model/ModelUtils.h"
#include "View/ControllerFacade.h"
#include "View/FlagChangedCommand.h"
#include "View/FlagsEditor.h"
#include "View/ViewUtils.h"

#include <wx/settings.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        struct UpdateSpawnflag {
        private:
            ControllerSPtr m_controller;
            const Model::PropertyKey& m_key;
            size_t m_flagIndex;
            bool m_setFlag;
        public:
            UpdateSpawnflag(ControllerSPtr controller, const Model::PropertyKey& key, const size_t flagIndex, const bool setFlag) :
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
                const int flagValue = (1 << m_flagIndex);
                
                if (m_setFlag)
                    intValue |= flagValue;
                else
                    intValue &= ~flagValue;
                
                StringStream str;
                str << intValue;
                return str.str();
            }
        };
        
        SmartSpawnflagsEditor::SmartSpawnflagsEditor(View::MapDocumentWPtr document, View::ControllerWPtr controller) :
        SmartPropertyEditor(document, controller),
        m_scrolledWindow(NULL),
        m_ignoreUpdates(false) {}

        void SmartSpawnflagsEditor::OnFlagChanged(FlagChangedCommand& event) {
            const Model::EntityList& updateEntities = entities();
            if (updateEntities.empty())
                return;

            const size_t index = event.index();
            const bool set = event.flagSet();
            
            const SetBool ignoreUpdates(m_ignoreUpdates);
            
            controller()->beginUndoableGroup("Set Spawnflags");
            Model::each(updateEntities.begin(), updateEntities.end(),
                        UpdateSpawnflag(controller(), key(), index, set),
                        Model::MatchAll());
            controller()->closeGroup();
        }
        
        wxWindow* SmartSpawnflagsEditor::doCreateVisual(wxWindow* parent) {
            assert(m_scrolledWindow == NULL);
            
            m_scrolledWindow = new wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, static_cast<long>(wxHSCROLL | wxVSCROLL | wxBORDER_NONE));
            m_scrolledWindow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            
            m_flagsEditor = new FlagsEditor(m_scrolledWindow, NumCols);
            m_flagsEditor->Bind(EVT_FLAG_CHANGED_EVENT, EVT_FLAG_CHANGED_HANDLER(SmartSpawnflagsEditor::OnFlagChanged), this);
            
            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_flagsEditor, 1, wxEXPAND);
            m_scrolledWindow->SetSizerAndFit(sizer);
            
            return m_scrolledWindow;
        }
        
        void SmartSpawnflagsEditor::doDestroyVisual() {
            assert(m_scrolledWindow != NULL);
            m_lastScrollPos = m_scrolledWindow->GetViewStart();
            m_scrolledWindow->Destroy();
            m_scrolledWindow = NULL;
            m_flagsEditor = NULL;
        }

        void SmartSpawnflagsEditor::doUpdateVisual(const Model::EntityList& entities) {
            assert(!entities.empty());
            if (m_ignoreUpdates)
                return;
            
            wxArrayString labels;
            getFlagsFromEntities(entities, labels);
            m_flagsEditor->setFlags(labels);
            
            int set, mixed;
            getFlagValuesFromEntities(entities, set, mixed);
            m_flagsEditor->setFlagValue(set, mixed);
            
            m_scrolledWindow->SetScrollRate(1, m_flagsEditor->lineHeight());
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

        void SmartSpawnflagsEditor::getFlagsFromEntities(const Model::EntityList& entities, wxArrayString& labels) const {
            const Assets::EntityDefinition* definition = Model::selectEntityDefinition(entities);
            
            for (size_t i = 0; i < NumFlags; ++i) {
                wxString label;
                if (definition != NULL) {
                    const Assets::FlagsPropertyDefinition* flagDefs = definition->spawnflags();
                    
                    const Assets::FlagsPropertyOption* flagDef = flagDefs != NULL ? flagDefs->option(static_cast<int>(1 << i)) : NULL;
                    if (flagDef != NULL) {
                        label << flagDef->description();
                    } else {
                        label << (1 << i);
                    }
                } else {
                    label << (1 << i);
                }
                labels.push_back(label);
            }
        }

        void SmartSpawnflagsEditor::getFlagValuesFromEntities(const Model::EntityList& entities, int& setFlags, int& mixedFlags) const {
            if (entities.empty()) {
                setFlags = 0;
                mixedFlags = 0;
                return;
            }
            
            Model::EntityList::const_iterator it = entities.begin();
            Model::EntityList::const_iterator end = entities.end();
            setFlags = getFlagValueFromEntity(**it);
            mixedFlags = 0;
            
            while (++it != end)
                combineFlags(NumFlags, getFlagValueFromEntity(**it), setFlags, mixedFlags);
        }

        int SmartSpawnflagsEditor::getFlagValueFromEntity(const Model::Entity& entity) const {
            if (!entity.hasProperty(key()))
                return 0;

            const Model::PropertyValue& propertyValue = entity.property(key());
            return std::atoi(propertyValue.c_str());
        }
    }
}
