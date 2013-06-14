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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Entity.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"

namespace TrenchBroom {
    namespace Model {
        const String Entity::DefaultPropertyValue = "";
        const Entity::List Entity::EmptyList = Entity::List();
        
        Entity::Entity() {}

        Entity::Ptr Entity::newEntity() {
            return Entity::Ptr(new Entity());
        }

        const EntityProperty::List& Entity::properties() const {
            return m_properties.properties();
        }
        
        bool Entity::hasProperty(const PropertyKey& key) const {
            return m_properties.hasProperty(key);
        }

        const PropertyValue& Entity::property(const PropertyKey& key, const PropertyValue& defaultValue) const {
            const PropertyValue* value = m_properties.property(key);
            if (value == NULL)
                return defaultValue;
            return *value;
        }
        
        void Entity::addOrUpdateProperty(const PropertyKey& key, const PropertyValue& value) {
            m_properties.addOrUpdateProperty(key, value);
        }

        const PropertyValue& Entity::classname(const PropertyValue& defaultClassname) const {
            return property(PropertyKeys::Classname, defaultClassname);
        }

        void Entity::addBrush(Brush::Ptr brush) {
            m_brushes.push_back(brush);
        }

        void Entity::removeBrush(Brush::Ptr brush) {
            VectorUtils::remove(m_brushes, brush);
        }
    }
}
