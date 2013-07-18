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

#include "Map.h"

#include "CollectionUtils.h"

namespace TrenchBroom {
    namespace Model {
        MapPtr Map::newMap() {
            return MapPtr(new Map());
        }

        Map::~Map() {
            m_worldspawn = EntityPtr();
        }

        const EntityList& Map::entities() const {
            return m_entities;
        }

        void Map::addEntity(EntityPtr entity) {
            m_entities.push_back(entity);
        }

        EntityPtr Map::worldspawn() {
            if (m_worldspawn == NULL)
                m_worldspawn = findWorldspawn();
            return m_worldspawn;
        }
        
        Map::Map() :
        m_worldspawn(EntityPtr()) {}
        
        EntityPtr Map::findWorldspawn() const {
            EntityList::const_iterator it, end;
            for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                EntityPtr entity = *it;
                if (entity->classname() == PropertyValues::WorldspawnClassname)
                    return entity;
            }
            return EntityPtr();
        }
    }
}
