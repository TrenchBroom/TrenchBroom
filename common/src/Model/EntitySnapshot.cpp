/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "EntitySnapshot.h"

#include "Model/Entity.h"

namespace TrenchBroom {
    namespace Model {
        EntitySnapshot::EntitySnapshot(Entity* entity, const EntityAttribute& origin, const EntityAttribute& rotation) :
        m_entity(entity),
        m_origin(origin),
        m_rotation(rotation) {}

        static void restoreAttribute(Entity* entity, const EntityAttribute& attribute) {
            if (attribute.name().empty())
                return;
            
            if (attribute.value().empty()) {
                // If the entity has an attribute with this name, clear the value, but otherwise don't insert {"name" ""}.
                if (entity->hasAttribute(attribute.name())) {
                    entity->addOrUpdateAttribute(attribute.name(), "");
                }
                return;
            }
            
            // normal case
            entity->addOrUpdateAttribute(attribute.name(), attribute.value());
        }
        
        void EntitySnapshot::doRestore(const bbox3& worldBounds) {
            restoreAttribute(m_entity, m_origin);
            restoreAttribute(m_entity, m_rotation);
        }
    }
}
