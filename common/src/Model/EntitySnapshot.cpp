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

        void EntitySnapshot::doRestore(const BBox3& worldBounds) {
            if (!m_origin.name().empty())
                m_entity->addOrUpdateAttribute(m_origin.name(), m_origin.value());
            if (!m_rotation.name().empty())
                m_entity->addOrUpdateAttribute(m_rotation.name(), m_rotation.value());
        }
    }
}
