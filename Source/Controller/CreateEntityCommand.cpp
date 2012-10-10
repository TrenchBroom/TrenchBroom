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

#include "CreateEntityCommand.h"

#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/EntityDefinitionManager.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        CreateEntityCommand::CreateEntityCommand(Model::MapDocument& document, const Model::Properties& properties) :
        DocumentCommand(CreateEntity, document, true, wxT("Create Entity")),
        m_properties(properties),
        m_entity(NULL) {}
        
        CreateEntityCommand::~CreateEntityCommand() {
            if (state() == Undone && m_entity != NULL) {
                delete m_entity;
                m_entity = NULL;
            }
        }
        
        bool CreateEntityCommand::performDo() {
            if (m_entity == NULL) {
                m_entity = new Model::Entity(document().map().worldBounds());
                m_entity->setProperties(m_properties, true);
                m_properties.clear();
                
                const String* classname = m_entity->classname();
                if (classname != NULL) {
                    Model::EntityDefinition* definition = document().definitionManager().definition(*classname);
                    m_entity->setDefinition(definition);
                }
            }
            document().addEntity(*m_entity);
            document().UpdateAllViews(NULL, this);
            return true;
        }
        
        bool CreateEntityCommand::performUndo() {
            assert(m_entity != NULL);
            document().removeEntity(*m_entity);
            document().UpdateAllViews(NULL, this);
            return true;
        }

        CreateEntityCommand* CreateEntityCommand::createFromTemplate(Model::MapDocument& document, const Model::Entity& entityTemplate) {
            return new CreateEntityCommand(document, entityTemplate.properties());
        }
    }
}