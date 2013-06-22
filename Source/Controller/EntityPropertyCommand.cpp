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
        bool EntityPropertyCommand::performDo() {
            if (!m_force && affectsImmutableProperty())
                return false;
            if (type() == SetEntityPropertyKey)
                if (!canSetKey() || (!m_force && affectsImmutableKey()))
                    return false;
            
            makeSnapshots(m_entities);
            document().entitiesWillChange(m_entities);
            switch (type()) {
                case Command::SetEntityPropertyKey:
                    setKey();
                    break;
                case SetEntityPropertyValue:
                    setValue();
                    break;
                case RemoveEntityProperty:
                    remove();
                    break;
                default:
                    break;
            }
            document().entitiesDidChange(m_entities);
            
            return true;
        }
        
        bool EntityPropertyCommand::affectsImmutableProperty() const {
            for (size_t i = 0; i < m_keys.size(); i++) {
                const Model::PropertyKey& key = m_keys[i];
                Model::EntityList::const_iterator entityIt, entityEnd;
                for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                    Model::Entity& entity = **entityIt;
                    if (!entity.propertyIsMutable(key))
                        return true;
                }
            }
            return false;
        }

        bool EntityPropertyCommand::affectsImmutableKey() const {
            return (!Model::Entity::propertyKeyIsMutable(m_newKey) ||
                    !Model::Entity::propertyKeyIsMutable(key()));
        }

        bool EntityPropertyCommand::canSetKey() const {
            return (key() != m_newKey &&
                    !anyEntityHasProperty(m_newKey));
        }

        bool EntityPropertyCommand::anyEntityHasProperty(const Model::PropertyKey& key) const {
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                if (entity.propertyForKey(key) != NULL)
                    return true;
            }
            return false;
        }
        
        void EntityPropertyCommand::setKey() {
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                if (entity.propertyForKey(key()) != NULL)
                    entity.renameProperty(key(), m_newKey);
            }
        }
        
        void EntityPropertyCommand::setValue() {
            Model::EntityDefinitionManager& definitionManager = document().definitionManager();
            m_definitionChanged = (key() == Model::Entity::ClassnameKey);

            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                entity.setProperty(key(), m_newValue);
                if (m_definitionChanged)
                    entity.setDefinition(definitionManager.definition(m_newValue));
            }
        }

        void EntityPropertyCommand::remove() {
            for (size_t i = 0; i < m_keys.size(); i++) {
                const Model::PropertyKey& key = m_keys[i];
                Model::EntityList::const_iterator entityIt, entityEnd;
                for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                    Model::Entity& entity = **entityIt;
                    entity.removeProperty(key);
                }
            }
        }

        bool EntityPropertyCommand::performUndo() {
            restoreSnapshots(m_entities);
            if (m_definitionChanged)
                restoreEntityDefinitions();
            return true;
        }

        void EntityPropertyCommand::restoreEntityDefinitions() {
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

        EntityPropertyCommand::EntityPropertyCommand(Type type, Model::MapDocument& document, const Model::EntityList& entities, const wxString& name) :
        SnapshotCommand(type, document, name),
        m_entities(entities),
        m_definitionChanged(false),
        m_force(false) {}

        EntityPropertyCommand* EntityPropertyCommand::setEntityPropertyKey(Model::MapDocument& document, const Model::EntityList& entities, const Model::PropertyKey& oldKey, const Model::PropertyKey& newKey, const bool force) {
            EntityPropertyCommand* command = new EntityPropertyCommand(SetEntityPropertyKey, document, entities, wxT("Set Property Key"));
            command->setKey(oldKey);
            command->setNewKey(newKey);
            command->setForce(force);
            return command;
        }
        
        EntityPropertyCommand* EntityPropertyCommand::setEntityPropertyValue(Model::MapDocument& document, const Model::EntityList& entities, const Model::PropertyKey& key, const Model::PropertyValue& newValue, const bool force) {
            EntityPropertyCommand* command = new EntityPropertyCommand(SetEntityPropertyValue, document, entities, wxT("Set Property Value"));
            command->setKey(key);
            command->setNewValue(newValue);
            command->setForce(force);
            return command;
        }
        
        EntityPropertyCommand* EntityPropertyCommand::setEntityPropertyValue(Model::MapDocument& document, Model::Entity& entity, const Model::PropertyKey& key, const Model::PropertyValue& newValue, const bool force) {
            Model::EntityList entities;
            entities.push_back(&entity);
            EntityPropertyCommand* command = new EntityPropertyCommand(SetEntityPropertyValue, document, entities, wxT("Set Property Value"));
            command->setKey(key);
            command->setNewValue(newValue);
            command->setForce(force);
            return command;
        }

        EntityPropertyCommand* EntityPropertyCommand::removeEntityProperty(Model::MapDocument& document, const Model::EntityList& entities, const Model::PropertyKey& key, const bool force) {
            EntityPropertyCommand* command = new EntityPropertyCommand(RemoveEntityProperty, document, entities, wxT("Delete Property"));
            command->setKey(key);
            command->setForce(force);
            return command;
        }

        EntityPropertyCommand* EntityPropertyCommand::removeEntityProperties(Model::MapDocument& document, const Model::EntityList& entities, const Model::PropertyKeyList& keys, const bool force) {
            EntityPropertyCommand* command = new EntityPropertyCommand(RemoveEntityProperty, document, entities, wxT("Delete Properties"));
            command->setKeys(keys);
            command->setForce(force);
            return command;
        }
    }
}
