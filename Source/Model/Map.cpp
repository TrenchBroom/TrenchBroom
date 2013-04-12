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

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Utility/List.h"

namespace TrenchBroom {
    namespace Model {
        Map::Map(const BBox& worldBounds, bool forceIntegerFacePoints) :
        m_worldBounds(worldBounds),
        m_forceIntegerFacePoints(forceIntegerFacePoints),
        m_worldspawn(NULL) {}

        Map::~Map() {
            clear();
        }

        void Map::setForceIntegerFacePoints(bool forceIntegerFacePoints) {
            EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                const Model::BrushList& brushes = entity.brushes();
                BrushList::const_iterator brushIt, brushEnd;
                for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                    Model::Brush& brush = **brushIt;
                    brush.setForceIntegerFacePoints(forceIntegerFacePoints);
                }
            }
            
            m_forceIntegerFacePoints = forceIntegerFacePoints;
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
            entity.setMap(NULL);
            Utility::erase(m_entities, &entity);
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
            Utility::deleteAll(m_entities);
            m_worldspawn = NULL;
        }
    }
}
