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

#include "RemoveObjectsQuery.h"

#include "CollectionUtils.h"
#include "AddObjectsQuery.h"
#include "Model/Brush.h"
#include "Model/Entity.h"

namespace TrenchBroom {
    namespace Model {
        RemoveObjectsQuery::RemoveObjectsQuery() {}
        
        RemoveObjectsQuery::RemoveObjectsQuery(const AddObjectsQuery& addQuery) {
            const EntityList& addedEntities = addQuery.entities();
            removeEntities(addedEntities.begin(), addedEntities.end());
            
            const EntityBrushesMap& addedBrushes = addQuery.brushes();
            EntityBrushesMap::const_iterator it, end;
            for (it = addedBrushes.begin(), end = addedBrushes.end(); it != end; ++it) {
                const Model::BrushList& entityBrushes = it->second;
                removeBrushes(entityBrushes.begin(), entityBrushes.end());
            }
        }

        const ObjectList& RemoveObjectsQuery::parents() const {
            return m_parents;
        }
        
        const ObjectList& RemoveObjectsQuery::objects() const {
            return m_objects;
        }
        
        size_t RemoveObjectsQuery::objectCount() const {
            return m_objects.size();
        }
        
        const EntityList& RemoveObjectsQuery::entities() const {
            return m_entities;
        }
        
        const BrushList& RemoveObjectsQuery::brushes() const {
            return m_brushes;
        }
        
        void RemoveObjectsQuery::removeEntity(Entity* entity) {
            assert(entity != NULL);
            assert(!VectorUtils::contains(m_entities, entity));
            assert(!VectorUtils::contains(m_objects, entity));
            m_entities.push_back(entity);
            m_objects.push_back(entity);
        }
        
        void RemoveObjectsQuery::removeBrush(Brush* brush) {
            assert(brush != NULL);
            Entity* entity = brush->parent();
            assert(entity != NULL);
            
            BrushCountMap::iterator it = MapUtils::findOrInsert(m_brushCounts, entity, 0);
            size_t& brushCount = it->second;
            if (brushCount == 0) {
                assert(!VectorUtils::contains(m_parents, entity));
                m_parents.push_back(entity);
            }
            
            const BrushList& entityBrushes = entity->brushes();
            if (brushCount + 1 == entityBrushes.size() && !entity->worldspawn()) { // always remove empty brush entities
                VectorUtils::removeAll(m_brushes.begin(), m_brushes.end(), entityBrushes.begin(), entityBrushes.end());
                if (!VectorUtils::contains(m_entities, entity))
                    removeEntity(entity);
                m_brushCounts.erase(it);
                
                assert(VectorUtils::contains(m_parents, entity));
                VectorUtils::erase(m_parents, entity);
            } else {
                m_brushes.push_back(brush);
                m_objects.push_back(brush);
                ++brushCount;
            }
        }

        void RemoveObjectsQuery::clear() {
            m_parents.clear();
            m_objects.clear();
            m_entities.clear();
            m_brushes.clear();
            m_brushCounts.clear();
        }

        void RemoveObjectsQuery::clearAndDelete() {
            m_parents.clear();
            m_objects.clear();
            VectorUtils::clearAndDelete(m_entities);
            VectorUtils::clearAndDelete(m_brushes);
            m_brushCounts.clear();
        }

        void RemoveObjectsQuery::doVisit(Entity* entity) {
            removeEntity(entity);
        }
        
        void RemoveObjectsQuery::doVisit(Brush* brush) {
            removeBrush(brush);
        }
    }
}
