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

#ifndef __TrenchBroom__EntityPropertyIndex__
#define __TrenchBroom__EntityPropertyIndex__

#include "Model/EntityProperties.h"
#include "Model/ModelTypes.h"

#include <map>

namespace TrenchBroom {
    namespace Model {
        class EntityPropertyIndex {
        private:
            typedef std::pair<PropertyKey, PropertyValue> PropertyPair;
            typedef std::map<PropertyPair, EntityList> EntityPropertyMap;
            EntityPropertyMap m_propertyMap;
            EntityPropertyMap m_numberedPropertyMap;
        public:
            void addEntity(Entity* entity);
            void removeEntity(Entity* entity);
            
            void addEntityProperty(Entity* entity, const EntityProperty& property);
            void removeEntityProperty(Entity* entity, const EntityProperty& property);
            
            const EntityList& findEntitiesWithProperty(const PropertyKey& key, const PropertyValue& value) const;
            const EntityList& findEntitiesWithNumberedProperty(const PropertyKey& unnumberedKey, const PropertyValue& value) const;
        };
    }
}

#endif /* defined(__TrenchBroom__EntityPropertyIndex__) */
