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

#ifndef __TrenchBroom__AddObjectsQuery__
#define __TrenchBroom__AddObjectsQuery__

#include "Model/Object.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class RemoveObjectsQuery;
        
        class AddObjectsQuery {
        private:
            ObjectList m_parents;
            ObjectList m_objects;
            EntityList m_entities;
            EntityBrushesMap m_brushes;
            ObjectLayerMap m_layers;
        public:
            AddObjectsQuery();
            AddObjectsQuery(const RemoveObjectsQuery& removeQuery);
            
            const ObjectList& parents() const;
            const ObjectList& objects() const;
            size_t objectCount() const;
            
            const EntityList& entities() const;
            const EntityBrushesMap& brushes() const;
            const ObjectLayerMap& layers() const;
            
            void addEntity(Entity* entity, Layer* layer);
            void addBrushes(const EntityBrushesMap& brushes, const ObjectLayerMap& layers);
            void addBrushes(const BrushList& brushes, Entity* entity, const ObjectLayerMap& layers);
            void addBrushes(const BrushList& brushes, Entity* entity, Layer* layer);
            void addBrush(Brush* brush, Entity* entity, Layer* layer);
        private:
            void setLayer(Object* object, Layer* layer);
            bool checkBrushLayer(Brush* brush, Entity* entity, Layer* layer) const;
        public:
            void clear();
            void clearAndDelete();
        };
        
        class AddObjectsQueryBuilder {
        private:
            AddObjectsQuery m_query;
            Layer* m_layer;
            Entity* m_entity;
        public:
            AddObjectsQueryBuilder(Layer* layer, Entity* entity = NULL);
            const AddObjectsQuery& query() const;
            
            void setLayer(Layer* layer);
            void setEntity(Entity* entity);
        private:
            void doVisit(Entity* entity);
            void doVisit(Brush* brush);
        };
    }
}

#endif /* defined(__TrenchBroom__AddObjectsQuery__) */
