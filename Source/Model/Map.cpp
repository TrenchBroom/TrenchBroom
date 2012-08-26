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

#include "Map.h"

#include "Model/Entity.h"

namespace TrenchBroom {
    namespace Model {
        void Map::setEntityDefinition(Entity* entity) {
        }
        
        void Map::addEntity(Entity* entity) {
            assert(entity != NULL);
            if (!entity->worldspawn() || worldspawn(false) == NULL) {
                m_entities.push_back(entity);
                entity->setMap(this);
                setEntityDefinition(entity);
            }
        }
        
        Entity* Map::createEntity(const PropertyValue& classname) {
            return NULL;
        }

        Entity* Map::worldspawn(bool create) {
            for (unsigned int i = 0; i < m_entities.size() && m_worldspawn == NULL; i++) {
                Entity* entity = m_entities[i];
                if (entity->worldspawn())
                    m_worldspawn = entity;
            }

            if (m_worldspawn == NULL && create)
                m_worldspawn = createEntity(Entity::WorldspawnClassname);
            
            return m_worldspawn;
        }
    }
}
