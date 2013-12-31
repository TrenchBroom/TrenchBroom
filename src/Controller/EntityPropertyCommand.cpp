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
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType EntityPropertyCommand::Type = Command::freeType();

        void EntityPropertyCommand::setKey(const Model::PropertyKey& key) {
            m_oldKey = key;
        }
        
        void EntityPropertyCommand::setNewKey(const Model::PropertyKey& newKey) {
            m_newKey = newKey;
        }
        
        void EntityPropertyCommand::setNewValue(const Model::PropertyValue& newValue) {
            m_newValue = newValue;
        }
        
        EntityPropertyCommand::EntityPropertyCommand(View::MapDocumentWPtr document, const PropertyCommand command, const Model::EntityList& entities, const bool force) :
        Command(Type, makeName(command), true, true),
        m_command(command),
        m_document(document),
        m_entities(entities),
        m_force(force) {}

        Command::Ptr EntityPropertyCommand::renameEntityProperty(View::MapDocumentWPtr document, const Model::EntityList& entities, const Model::PropertyKey& oldKey, const Model::PropertyKey& newKey, const bool force) {
            EntityPropertyCommand::Ptr command(new EntityPropertyCommand(document, PCRenameProperty, entities, force));
            command->setKey(oldKey);
            command->setNewKey(newKey);
            return command;
        }
        
        Command::Ptr EntityPropertyCommand::setEntityProperty(View::MapDocumentWPtr document, const Model::EntityList& entities, const Model::PropertyKey& key, const Model::PropertyKey& newValue, const bool force) {
            EntityPropertyCommand::Ptr command(new EntityPropertyCommand(document, PCSetProperty, entities, force));
            command->setKey(key);
            command->setNewValue(newValue);
            return command;
        }
        
        Command::Ptr EntityPropertyCommand::removeEntityProperty(View::MapDocumentWPtr document, const Model::EntityList& entities, const Model::PropertyKey& key, const bool force) {
            EntityPropertyCommand::Ptr command(new EntityPropertyCommand(document, PCRemoveProperty, entities, force));
            command->setKey(key);
            return command;
        }

        const Model::PropertyKey& EntityPropertyCommand::key() const {
            return m_oldKey;
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
            return m_newKey == key || m_oldKey == key;
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
            
            View::MapDocumentSPtr document = lock(m_document);
            m_snapshot.clear();
            
            document->objectWillChangeNotifier(m_entities.begin(), m_entities.end());
            switch (m_command) {
                case PCRenameProperty:
                    doRename(document);
                    break;
                case PCSetProperty:
                    doSetValue(document);
                    break;
                case PCRemoveProperty:
                    doRemove(document);
                    break;
                default:
                    break;
            }
            document->objectDidChangeNotifier(m_entities.begin(), m_entities.end());
            
            return true;
        }
        
        bool EntityPropertyCommand::doPerformUndo() {
            View::MapDocumentSPtr document = lock(m_document);
            
            document->objectWillChangeNotifier(m_entities.begin(), m_entities.end());
            switch(m_command) {
                case PCRenameProperty:
                    undoRename(document);
                    break;
                case PCSetProperty:
                    undoSetValue(document);
                    break;
                case PCRemoveProperty:
                    undoRemove(document);
                    break;
                default:
                    break;
            };
            document->objectDidChangeNotifier(m_entities.begin(), m_entities.end());
            m_snapshot.clear();
            
            return true;
        }

        void EntityPropertyCommand::doRename(View::MapDocumentSPtr document) {
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                if (entity->hasProperty(key())) {
                    const Model::PropertyValue& value = entity->property(key());
                    const Model::EntityProperty before(key(), value);
                    const Model::EntityProperty after(newKey(), value);
                    
                    m_snapshot[entity] = before;
                    entity->renameProperty(after.key, after.value);
                    document->entityPropertyDidChangeNotifier(entity, before, after);
                }
            }
        }
        
        void EntityPropertyCommand::doSetValue(View::MapDocumentSPtr document) {
            m_definitionAffected = (key() == Model::PropertyKeys::Classname);
            const Model::EntityProperty empty;
            const Model::EntityProperty after(key(), newValue());
            
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                if (entity->hasProperty(key())) {
                    const Model::PropertyValue& oldValue = entity->property(key());
                    const Model::EntityProperty before(key(), oldValue);
                    
                    m_snapshot[entity] = before;
                    entity->addOrUpdateProperty(after.key, after.value);
                    document->entityPropertyDidChangeNotifier(entity, before, after);
                } else {
                    entity->addOrUpdateProperty(after.key, after.value);
                    document->entityPropertyDidChangeNotifier(entity, empty, after);
                }
            }
        }
        
        void EntityPropertyCommand::doRemove(View::MapDocumentSPtr document) {
            const Model::EntityProperty empty;

            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                if (entity->hasProperty(key())) {
                    const Model::PropertyValue& oldValue = entity->property(key());
                    const Model::EntityProperty before(key(), oldValue);

                    m_snapshot[entity] = before;
                    document->entityPropertyDidChangeNotifier(entity, before, empty);
                    entity->removeProperty(key());
                }
            }
        }

        void EntityPropertyCommand::undoRename(View::MapDocumentSPtr document) {
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                PropertySnapshot::iterator snapshotIt = m_snapshot.find(entity);
                if (snapshotIt != m_snapshot.end()) {
                    const Model::PropertyValue& value = snapshotIt->second.value;
                    const Model::EntityProperty before(key(), value);
                    const Model::EntityProperty after(newKey(), value);
                    
                    entity->renameProperty(newKey(), key());
                    document->entityPropertyDidChangeNotifier(entity, after, before);
                }
            }
        }
        
        void EntityPropertyCommand::undoSetValue(View::MapDocumentSPtr document) {
            const Model::EntityProperty empty;
            const Model::EntityProperty after(key(), newValue());

            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                PropertySnapshot::iterator snapshotIt = m_snapshot.find(entity);
                if (snapshotIt == m_snapshot.end()) {
                    document->entityPropertyDidChangeNotifier(entity, after, empty);
                    entity->removeProperty(key());
                } else {
                    const Model::EntityProperty& before = snapshotIt->second;
                    entity->addOrUpdateProperty(before.key, before.value);
                    document->entityPropertyDidChangeNotifier(entity, after, before);
                }
            }
        }
        
        void EntityPropertyCommand::undoRemove(View::MapDocumentSPtr document) {
            const Model::EntityProperty empty;

            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                PropertySnapshot::iterator snapshotIt = m_snapshot.find(entity);
                if (snapshotIt != m_snapshot.end()) {
                    const Model::EntityProperty& before = snapshotIt->second;
                    entity->addOrUpdateProperty(before.key, before.value);
                    document->entityPropertyDidChangeNotifier(entity, empty, before);
                }
            }
        }

        bool EntityPropertyCommand::affectsImmutablePropertyKey() const {
            return (!Model::isPropertyKeyMutable(newKey()) ||
                    !Model::isPropertyKeyMutable(key()));
        }
        
        bool EntityPropertyCommand::affectsImmutablePropertyValue() const {
            return !Model::isPropertyValueMutable(key());
        }

        bool EntityPropertyCommand::canSetKey() const {
            return (key() != m_newKey &&
                    !anyEntityHasProperty(newKey()));
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
