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

#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/EntityDefinitionManager.h"
#include "Model/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        EntityPropertyCommand::EntityPropertyCommand(Type type, Model::MapDocument& document, const Model::EntityList& entities, const wxString& name) :
        SnapshotCommand(type, document, name),
        m_entities(entities),
        m_definitionChanged(false) {}

        bool EntityPropertyCommand::performDo() {
            Model::EntityList::const_iterator entityIt, entityEnd;
            Model::EntityDefinitionManager& definitionManager = document().definitionManager();
            if (type() == SetEntityPropertyKey && key() != m_newKey) {
                for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                    Model::Entity& entity = **entityIt;
                    if (!entity.propertyKeyIsMutable(key()) ||
                        entity.propertyForKey(m_newKey) != NULL)
                        return false;
                }
                
                makeSnapshots(m_entities);
                
                for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                    Model::Entity& entity = **entityIt;
                    if (entity.propertyForKey(key()) != NULL)
                        entity.renameProperty(key(), m_newKey);
                }
                return true;
            }
            
            if (type() == SetEntityPropertyValue) {
                makeSnapshots(m_entities);
                m_definitionChanged = (key() == Model::Entity::ClassnameKey);

                for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                    Model::Entity& entity = **entityIt;
                    entity.setProperty(key(), m_newValue);
                    if (m_definitionChanged)
                        entity.setDefinition(definitionManager.definition(m_newValue));
                }
                return true;
            }
            
            if (type() == RemoveEntityProperty) {
                makeSnapshots(m_entities);
                for (unsigned int i = 0; i < m_keys.size(); i++) {
                    for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                        Model::Entity& entity = **entityIt;
                        if (!entity.propertyKeyIsMutable(key()))
                            return false;
                    }

                    const Model::PropertyKey& key = m_keys[i];
                    for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                        Model::Entity& entity = **entityIt;
                        entity.removeProperty(key);
                    }
                }
                return true;
            }
            
            return false;
        }
        
        bool EntityPropertyCommand::performUndo() {
            restoreSnapshots(m_entities);
            if (m_definitionChanged) {
                Model::EntityDefinitionManager& definitionManager = document().definitionManager();

                Model::EntityList::const_iterator entityIt, entityEnd;
                for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                    Model::Entity& entity = **entityIt;
                    const String* classname = entity.classname();
                    if (classname != NULL)
                        entity.setDefinition(definitionManager.definition(*classname));
                    else
                        entity.setDefinition(NULL);
                }
            }

            return true;
        }

        EntityPropertyCommand* EntityPropertyCommand::setEntityPropertyKey(Model::MapDocument& document, const Model::EntityList& entities, const Model::PropertyKey& oldKey, const Model::PropertyKey& newKey) {
            EntityPropertyCommand* command = new EntityPropertyCommand(SetEntityPropertyKey, document, entities, wxT("Set Property Key"));
            command->setKey(oldKey);
            command->setNewKey(newKey);
            return command;
        }
        
        EntityPropertyCommand* EntityPropertyCommand::setEntityPropertyValue(Model::MapDocument& document, const Model::EntityList& entities, const Model::PropertyKey& key, const Model::PropertyValue& newValue) {
            EntityPropertyCommand* command = new EntityPropertyCommand(SetEntityPropertyValue, document, entities, wxT("Set Property Value"));
            command->setKey(key);
            command->setNewValue(newValue);
            return command;
        }
        
        EntityPropertyCommand* EntityPropertyCommand::removeEntityProperty(Model::MapDocument& document, const Model::EntityList& entities, const Model::PropertyKey& key) {
            EntityPropertyCommand* command = new EntityPropertyCommand(RemoveEntityProperty, document, entities, wxT("Delete Property"));
            command->setKey(key);
            return command;
        }

        EntityPropertyCommand* EntityPropertyCommand::removeEntityProperties(Model::MapDocument& document, const Model::EntityList& entities, const Model::PropertyKeyList& keys) {
            EntityPropertyCommand* command = new EntityPropertyCommand(RemoveEntityProperty, document, entities, wxT("Delete Properties"));
            command->setKeys(keys);
            return command;
        }
    }
}
