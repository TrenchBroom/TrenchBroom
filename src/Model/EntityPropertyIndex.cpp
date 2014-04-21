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

#include "EntityPropertyIndex.h"

#include "CollectionUtils.h"
#include "Model/Entity.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        EntityPropertyQuery EntityPropertyQuery::exact(const String& pattern) {
            return EntityPropertyQuery(Type_Exact, pattern);
        }
        
        EntityPropertyQuery EntityPropertyQuery::prefix(const String& pattern) {
            return EntityPropertyQuery(Type_Prefix, pattern);
        }
        
        EntityPropertyQuery EntityPropertyQuery::numbered(const String& pattern) {
            return EntityPropertyQuery(Type_Numbered, pattern);
        }
        
        EntityPropertyQuery EntityPropertyQuery::any() {
            return EntityPropertyQuery(Type_Any);
        }
        
        EntitySet EntityPropertyQuery::execute(const StringIndex<Entity*>& index) const {
            switch (m_type) {
                case Type_Exact:
                    return index.queryExactMatches(m_pattern);
                case Type_Prefix:
                    return index.queryPrefixMatches(m_pattern);
                case Type_Numbered:
                    return index.queryNumberedMatches(m_pattern);
                case Type_Any:
                    return EmptyEntitySet;
            }
        }
        
        EntityPropertyQuery::EntityPropertyQuery(const Type type, const String& pattern) :
        m_type(type),
        m_pattern(pattern) {}

        void EntityPropertyIndex::addEntity(Entity* entity) {
            const EntityProperty::List& properties = entity->properties();
            EntityProperty::List::const_iterator it, end;
            for (it = properties.begin(), end = properties.end(); it != end; ++it) {
                const EntityProperty& property = *it;
                addEntityProperty(entity, property);
            }
        }
        
        void EntityPropertyIndex::removeEntity(Entity* entity) {
            const EntityProperty::List& properties = entity->properties();
            EntityProperty::List::const_iterator it, end;
            for (it = properties.begin(), end = properties.end(); it != end; ++it) {
                const EntityProperty& property = *it;
                removeEntityProperty(entity, property);
            }
        }

        void EntityPropertyIndex::addEntityProperty(Entity* entity, const EntityProperty& property) {
            m_keyIndex.insert(property.key, entity);
            m_valueIndex.insert(property.value, entity);
        }
        
        void EntityPropertyIndex::removeEntityProperty(Entity* entity, const EntityProperty& property) {
            m_keyIndex.remove(property.key, entity);
            m_valueIndex.remove(property.value, entity);
        }

        EntityList EntityPropertyIndex::findEntities(const EntityPropertyQuery& keyQuery, const EntityPropertyQuery& valueQuery) const {
            const EntitySet keyResult = keyQuery.execute(m_keyIndex);
            const EntitySet valueResult = valueQuery.execute(m_valueIndex);
            
            if (keyResult.empty() || valueResult.empty())
                return EmptyEntityList;

            EntityList result;
            SetUtils::intersection(keyResult, valueResult, result);
            return result;
        }
    }
}
