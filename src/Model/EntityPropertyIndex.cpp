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

#include "EntityPropertyIndex.h"

#include "CollectionUtils.h"
#include "Model/Entity.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
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
            EntityList& entities = m_propertyMap[std::make_pair(property.key, property.value)];
            VectorUtils::setInsert(entities, entity);

            const String unnumberedKey = numberedPropertyPrefix(property.key);
            if (!unnumberedKey.empty()) {
                EntityList& entities = m_numberedPropertyMap[std::make_pair(unnumberedKey, property.value)];
                VectorUtils::setInsert(entities, entity);
            }
        }
        
        void EntityPropertyIndex::removeEntityProperty(Entity* entity, const EntityProperty& property) {
            EntityPropertyMap::iterator it = m_propertyMap.find(std::make_pair(property.key, property.value));
            assert(it != m_propertyMap.end());
            
            EntityList& entities = it->second;
            VectorUtils::setRemove(entities, entity);
            if (entities.empty())
                m_propertyMap.erase(it);
            
            const String unnumberedKey = numberedPropertyPrefix(property.key);
            if (!unnumberedKey.empty()) {
                EntityPropertyMap::iterator it = m_numberedPropertyMap.find(std::make_pair(unnumberedKey, property.value));
                assert(it != m_numberedPropertyMap.end());
                
                EntityList& entities = it->second;
                VectorUtils::setRemove(entities, entity);
                if (entities.empty())
                    m_numberedPropertyMap.erase(it);
            }
        }

        const EntityList& EntityPropertyIndex::findEntitiesWithProperty(const PropertyKey& key, const PropertyValue& value) const {
            EntityPropertyMap::const_iterator it = m_propertyMap.find(std::make_pair(key, value));
            if (it == m_propertyMap.end())
                return EmptyEntityList;
            return it->second;
        }
        
        const EntityList& EntityPropertyIndex::findEntitiesWithNumberedProperty(const PropertyKey& unnumberedKey, const PropertyValue& value) const {
            EntityPropertyMap::const_iterator it = m_numberedPropertyMap.find(std::make_pair(unnumberedKey, value));
            if (it == m_numberedPropertyMap.end())
                return EmptyEntityList;
            return it->second;
        }
    }
}
