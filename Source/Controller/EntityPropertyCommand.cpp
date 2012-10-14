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

#include "EntityPropertyCommand.h"

#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/EntityDefinitionManager.h"
#include "Model/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        EntityPropertyCommand::EntityPropertyCommand(Type type, Model::MapDocument& document, const wxString& name) :
        SnapshotCommand(type, document, name),
        m_definitionChanged(false) {}

        bool EntityPropertyCommand::performDo() {
            const Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            if (entities.empty())
                return false;
            
            Model::EntityDefinitionManager& definitionManager = document().definitionManager();
            if (type() == SetEntityPropertyKey) {
                makeSnapshots(entities);
                m_definitionChanged = (key() == Model::Entity::ClassnameKey || m_newKey == Model::Entity::ClassnameKey);
                for (unsigned int i = 0; i < entities.size(); i++) {
                    Model::Entity& entity = *entities[i];
                    Model::PropertyValue value = *entity.propertyForKey(key());
                    entity.deleteProperty(key());
                    entity.setProperty(m_newKey, value);
                    
                    if (m_definitionChanged) {
                        entity.setDefinition(definitionManager.definition(value));
                    }
                }
                return true;
            }
            if (type() == SetEntityPropertyValue) {
                makeSnapshots(entities);
                m_definitionChanged = (key() == Model::Entity::ClassnameKey);
                for (unsigned int i = 0; i < entities.size(); i++) {
                    Model::Entity& entity = *entities[i];
                    entity.setProperty(key(), m_newValue);
                    if (m_definitionChanged)
                        entity.setDefinition(definitionManager.definition(m_newValue));
                }
                return true;
            }
            if (type() == RemoveEntityProperty) {
                makeSnapshots(entities);
                for (unsigned int i = 0; i < m_keys.size(); i++) {
                    const Model::PropertyKey& key = m_keys[i];
                    m_definitionChanged |= (key == Model::Entity::ClassnameKey);
                    
                    for (unsigned int j = 0; j < entities.size(); j++) {
                        Model::Entity& entity = *entities[j];
                        entity.deleteProperty(key);
                        if (key == Model::Entity::ClassnameKey)
                            entity.setDefinition(NULL);
                    }
                }
                return true;
            }
            
            return false;
        }
        
        bool EntityPropertyCommand::performUndo() {
            const Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            assert(!entities.empty());
            
            restoreSnapshots(entities);
            if (m_definitionChanged) {
                Model::EntityDefinitionManager& definitionManager = document().definitionManager();
                for (unsigned int i = 0; i < entities.size(); i++) {
                    Model::Entity& entity = *entities[i];
                    const String* classname = entity.classname();
                    if (classname != NULL)
                        entity.setDefinition(definitionManager.definition(*classname));
                    else
                        entity.setDefinition(NULL);
                }
            }
            return true;
        }

        EntityPropertyCommand* EntityPropertyCommand::setEntityPropertyKey(Model::MapDocument& document, const Model::PropertyKey& oldKey, const Model::PropertyKey& newKey) {
            EntityPropertyCommand* command = new EntityPropertyCommand(SetEntityPropertyKey, document, wxT("Set Property Key"));
            command->setKey(oldKey);
            command->setNewKey(newKey);
            return command;
        }
        
        EntityPropertyCommand* EntityPropertyCommand::setEntityPropertyValue(Model::MapDocument& document, const Model::PropertyKey& key, const Model::PropertyValue& newValue) {
            EntityPropertyCommand* command = new EntityPropertyCommand(SetEntityPropertyValue, document, wxT("Set Property Value"));
            command->setKey(key);
            command->setNewValue(newValue);
            return command;
        }
        
        EntityPropertyCommand* EntityPropertyCommand::removeEntityProperty(Model::MapDocument& document, const Model::PropertyKey& key) {
            EntityPropertyCommand* command = new EntityPropertyCommand(RemoveEntityProperty, document, wxT("Delete Property"));
            command->setKey(key);
            return command;
        }

        EntityPropertyCommand* EntityPropertyCommand::removeEntityProperties(Model::MapDocument& document, const Model::PropertyKeyList& keys) {
            EntityPropertyCommand* command = new EntityPropertyCommand(RemoveEntityProperty, document, wxT("Delete Properties"));
            command->setKeys(keys);
            return command;
        }
    }
}