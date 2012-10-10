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
        Map::Map(const BBox& worldBounds) :
        m_worldBounds(worldBounds),
        m_worldspawn(NULL) {}

        Map::~Map() {
            clear();
        }

        void Map::addEntity(Entity& entity) {
            if (!entity.worldspawn() || worldspawn() == NULL) {
                m_entities.push_back(&entity);
                entity.setMap(this);
            }
        }
        
        void Map::removeEntity(Entity& entity) {
            if (entity.worldspawn())
                m_worldspawn = NULL;
            m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), &entity), m_entities.end());
        }

        Entity* Map::worldspawn() {
            for (unsigned int i = 0; i < m_entities.size() && m_worldspawn == NULL; i++) {
                Entity* entity = m_entities[i];
                if (entity->worldspawn())
                    m_worldspawn = entity;
            }
            
            return m_worldspawn;
        }

        void Map::clear() {
            while (!m_entities.empty()) delete m_entities.back(), m_entities.pop_back();
            m_worldspawn = NULL;
        }
    }
}
