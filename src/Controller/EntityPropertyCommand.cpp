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

#include "EntityPropertyCommand.h"

#include "CollectionUtils.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType EntityPropertyCommand::Type = Command::freeType();

        void EntityPropertyCommand::setKey(const Model::PropertyKey& key) {
            assert(m_keys.size() <= 1);
            if (m_keys.empty())
                m_keys.push_back(key);
            else
                m_keys[0] = key;
        }
        
        void EntityPropertyCommand::setKeys(const Model::PropertyKeyList& keys) {
            m_keys = keys;
        }
        
        void EntityPropertyCommand::setNewKey(const Model::PropertyKey& newKey) {
            m_newKey = newKey;
        }
        
        void EntityPropertyCommand::setNewValue(const Model::PropertyValue& newValue) {
            m_newValue = newValue;
        }
        
        EntityPropertyCommand::EntityPropertyCommand(View::MapDocumentPtr document, const PropertyCommand command, const Model::EntityList& entities, const bool force) :
        Command(Type, makeName(command), true, true),
        m_command(command),
        m_document(document),
        m_entities(entities),
        m_force(force) {}

        Command::Ptr EntityPropertyCommand::renameEntityProperty(View::MapDocumentPtr document, const Model::EntityList& entities, const Model::PropertyKey& oldKey, const Model::PropertyKey& newKey, const bool force) {
            EntityPropertyCommand::Ptr command(new EntityPropertyCommand(document, PCRenameProperty, entities, force));
            command->setKey(oldKey);
            command->setNewKey(newKey);
            return command;
        }
        
        Command::Ptr EntityPropertyCommand::setEntityProperty(View::MapDocumentPtr document, const Model::EntityList& entities, const Model::PropertyKey& key, const Model::PropertyKey& newValue, const bool force) {
            EntityPropertyCommand::Ptr command(new EntityPropertyCommand(document, PCSetProperty, entities, force));
            command->setKey(key);
            command->setNewValue(newValue);
            return command;
        }
        
        Command::Ptr EntityPropertyCommand::removeEntityProperty(View::MapDocumentPtr document, const Model::EntityList& entities, const Model::PropertyKey& key, const bool force) {
            EntityPropertyCommand::Ptr command(new EntityPropertyCommand(document, PCRemoveProperty, entities, force));
            command->setKey(key);
            return command;
        }

        const Model::PropertyKey& EntityPropertyCommand::key() const {
            assert(m_keys.size() == 1);
            return m_keys.front();
        }
        
        const Model::PropertyKeyList& EntityPropertyCommand::keys() const {
            return m_keys;
        }
        
        const Model::PropertyKey& EntityPropertyCommand::newKey() const {
            return m_newKey;
        }
        
        const Model::PropertyValue& EntityPropertyCommand::newValue() const {
            return m_newValue;
        }

        bool EntityPropertyCommand::definitionAffected() const {
            return m_definitionAffected;
        }
        
        bool EntityPropertyCommand::propertyAffected(const Model::PropertyKey& key) {
            return m_newKey == key || std::find(m_keys.begin(), m_keys.end(), key) != m_keys.end();
        }
        
        bool EntityPropertyCommand::entityAffected(const Model::Entity* entity) {
            return std::find(m_entities.begin(), m_entities.end(), entity) != m_entities.end();
        }
        
        const Model::EntityList& EntityPropertyCommand::affectedEntities() const {
            return m_entities;
        }

        String EntityPropertyCommand::makeName(const PropertyCommand command) {
            switch (command) {
                case PCRenameProperty:
                    return "Rename entity property";
                case PCSetProperty:
                    return "Set entity property";
                case PCRemoveProperty:
                    return "Remove entity property";
                default:
                    assert(false);
                    return "";
            }
        }

        bool EntityPropertyCommand::doPerformDo() {
            if (!m_force && affectsImmutablePropertyValue())
                return false;
            if (m_command == PCRenameProperty)
                if (!canSetKey() || (!m_force && affectsImmutablePropertyKey()))
                    return false;
            
            m_snapshot = Model::Snapshot(m_entities);
            
            m_document->objectWillChangeNotifier(m_entities.begin(), m_entities.end());
            switch (m_command) {
                case PCRenameProperty:
                    rename();
                    break;
                case PCSetProperty:
                    setValue();
                    break;
                case PCRemoveProperty:
                    remove();
                    break;
                default:
                    break;
            }
            m_document->objectDidChangeNotifier(m_entities.begin(), m_entities.end());
            
            return true;
        }
        
        bool EntityPropertyCommand::doPerformUndo() {
            m_document->objectWillChangeNotifier(m_entities.begin(), m_entities.end());
            m_snapshot.restore(m_document->worldBounds());
            m_document->objectDidChangeNotifier(m_entities.begin(), m_entities.end());
            return true;
        }

        void EntityPropertyCommand::rename() {
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                if (entity.hasProperty(key()))
                    entity.renameProperty(key(), m_newKey);
            }
        }
        
        void EntityPropertyCommand::setValue() {
            m_definitionAffected = (key() == Model::PropertyKeys::Classname);
            
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                entity.addOrUpdateProperty(key(), m_newValue);
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

        bool EntityPropertyCommand::affectsImmutablePropertyKey() const {
            return (!Model::isPropertyKeyMutable(m_newKey) ||
                    !Model::isPropertyKeyMutable(key()));
        }
        
        bool EntityPropertyCommand::affectsImmutablePropertyValue() const {
            Model::PropertyKeyList::const_iterator it, end;
            for (it = m_keys.begin(), end = m_keys.end(); it != end; ++it) {
                const Model::PropertyKey& key = *it;
                if (!Model::isPropertyValueMutable(key))
                    return true;
            }
            return false;
        }

        bool EntityPropertyCommand::canSetKey() const {
            return (key() != m_newKey &&
                    !anyEntityHasProperty(m_newKey));
        }

        bool EntityPropertyCommand::anyEntityHasProperty(const Model::PropertyKey& key) const {
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                if (entity.hasProperty(key))
                    return true;
            }
            return false;
        }
    }
}
