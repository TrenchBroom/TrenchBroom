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

#include "AddObjectsQuery.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/RemoveObjectsQuery.h"

namespace TrenchBroom {
    namespace Model {
        AddObjectsQuery::AddObjectsQuery() {}
        
        AddObjectsQuery::AddObjectsQuery(const RemoveObjectsQuery& removeQuery) {
            const EntityList& removedEntities = removeQuery.entities();
            EntityList::const_iterator eIt, eEnd;
            for (eIt = removedEntities.begin(), eEnd = removedEntities.end(); eIt != eEnd; ++eIt) {
                Entity* entity = *eIt;
                Layer* layer = entity->layer();
                addEntity(entity, layer);
            }
            
            const BrushList& removedBrushes = removeQuery.brushes();
            BrushList::const_iterator bIt, bEnd;
            for (bIt = removedBrushes.begin(), bEnd = removedBrushes.end(); bIt != bEnd; ++bIt) {
                Brush* brush = *bIt;
                Entity* entity = brush->parent();
                Layer* layer = brush->layer();
                addBrush(brush, entity, layer);
            }
        }

        const ObjectList& AddObjectsQuery::parents() const {
            return m_parents;
        }
        
        const ObjectList& AddObjectsQuery::objects() const {
            return m_objects;
        }
        
        size_t AddObjectsQuery::objectCount() const {
            return m_objects.size();
        }

        const EntityList& AddObjectsQuery::entities() const {
            return m_entities;
        }
        
        const EntityBrushesMap& AddObjectsQuery::brushes() const {
            return m_brushes;
        }
        
        const ObjectLayerMap& AddObjectsQuery::layers() const {
            return m_layers;
        }

        void AddObjectsQuery::addEntity(Entity* entity, Layer* layer) {
            assert(entity != NULL);
            assert(layer != NULL);
            assert(!VectorUtils::contains(m_entities, entity));
            assert(!VectorUtils::contains(m_objects, entity));
            
            setLayer(entity, layer);
            m_entities.push_back(entity);
            m_objects.push_back(entity);
        }
        
        void AddObjectsQuery::addBrushes(const EntityBrushesMap& brushes, const ObjectLayerMap& layers) {
            EntityBrushesMap::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                addBrushes(it->second, it->first, layers);
        }
        
        void AddObjectsQuery::addBrushes(const BrushList& brushes, Entity* entity, const ObjectLayerMap& layers) {
            BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                Brush* brush = *it;
                Layer* layer = MapUtils::find(layers, brush, static_cast<Layer*>(NULL));
                addBrush(brush, entity, layer);
            }
        }

        void AddObjectsQuery::addBrushes(const BrushList& brushes, Entity* entity, Model::Layer* layer) {
            BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                Brush* brush = *it;
                addBrush(brush, entity, layer);
            }
        }

        void AddObjectsQuery::addBrush(Brush* brush, Entity* entity, Layer* layer) {
            assert(brush != NULL);
            assert(entity != NULL);
            assert(layer != NULL);
            assert(checkBrushLayer(brush, entity, layer));
            assert(!VectorUtils::contains(m_objects, brush));
            
            EntityBrushesMap::iterator it = MapUtils::findOrInsert(m_brushes, entity);
            BrushList& brushes = it->second;
            assert(!VectorUtils::contains(brushes, brush));

            if (brushes.empty()) {
                assert(!VectorUtils::contains(m_parents, entity));
                m_parents.push_back(entity);
            }
            
            setLayer(brush, layer);
            brushes.push_back(brush);
            m_objects.push_back(brush);
        }

        void AddObjectsQuery::setLayer(Object* object, Layer* layer) {
            assert(m_layers.count(object) == 0);
            m_layers[object] = layer;
        }

        bool AddObjectsQuery::checkBrushLayer(Brush* brush, Entity* entity, Layer* layer) const {
            if (entity->worldspawn())
                return true;
            if (entity->layer() != NULL)
                return entity->layer() == layer;
            return (VectorUtils::contains(m_entities, entity) &&
                    layer == MapUtils::find(m_layers, entity, static_cast<Layer*>(NULL)));
        }
        
        void AddObjectsQuery::clear() {
            m_parents.clear();
            m_objects.clear();
            m_entities.clear();
            m_brushes.clear();
            m_layers.clear();
        }
        
        void AddObjectsQuery::clearAndDelete() {
            m_parents.clear();
            m_objects.clear();
            VectorUtils::clearAndDelete(m_entities);
            MapUtils::clearAndDelete(m_brushes);
            m_layers.clear();
        }

        AddObjectsQueryBuilder::AddObjectsQueryBuilder(Layer* layer, Entity* entity) :
        m_layer(layer),
        m_entity(entity) {}
        
        const AddObjectsQuery& AddObjectsQueryBuilder::query() const {
            return m_query;
        }
        
        void AddObjectsQueryBuilder::setLayer(Layer* layer) {
            assert(layer != NULL);
            m_layer = layer;
        }
        
        void AddObjectsQueryBuilder::setEntity(Entity* entity) {
            m_entity = entity;
        }

        void AddObjectsQueryBuilder::doVisit(Entity* entity) {
            m_query.addEntity(entity, m_layer);
        }
        
        void AddObjectsQueryBuilder::doVisit(Brush* brush) {
            m_query.addBrush(brush, m_entity, m_layer);
        }
    }
}
