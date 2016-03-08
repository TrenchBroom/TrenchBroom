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

#ifndef TrenchBroom_AssortNodesVisitor
#define TrenchBroom_AssortNodesVisitor

#include "Model/ModelTypes.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        class CollectLayersStrategy {
        private:
            LayerList m_layers;
        public:
            const LayerList& layers() const;
        protected:
            void addLayer(Layer* layer);
        };
        
        class SkipLayersStrategy {
        public:
            const LayerList& layers() const;
        protected:
            void addLayer(Layer* layer);
        };

        class CollectGroupsStrategy {
        private:
            GroupList m_groups;
        public:
            const GroupList& groups() const;
        protected:
            void addGroup(Group* group);
        };
        
        class SkipGroupsStrategy {
        public:
            const GroupList& groups() const;
        protected:
            void addGroup(Group* group);
        };
        
        class CollectEntitiesStrategy {
        private:
            EntityList m_entities;
        public:
            const EntityList& entities() const;
        protected:
            void addEntity(Entity* entity);
        };
        
        class SkipEntitiesStrategy {
        public:
            const EntityList& entities() const;
        protected:
            void addEntity(Entity* entity);
        };

        class CollectBrushesStrategy {
        private:
            BrushList m_brushes;
        public:
            const BrushList& brushes() const;
        protected:
            void addBrush(Brush* brush);
        };
        
        class SkipBrushesStrategy {
        public:
            const BrushList& brushes() const;
        protected:
            void addBrush(Brush* brush);
        };

        template <class LayerStrategy, class GroupStrategy, class EntityStrategy, class BrushStrategy>
        class AssortNodesVisitorT : public NodeVisitor, public LayerStrategy, public GroupStrategy, public EntityStrategy, public BrushStrategy {
        private:
            void doVisit(World* world)   {}
            void doVisit(Layer* layer)   {  LayerStrategy::addLayer(layer); }
            void doVisit(Group* group)   {  GroupStrategy::addGroup(group); }
            void doVisit(Entity* entity) { EntityStrategy::addEntity(entity); }
            void doVisit(Brush* brush)   {  BrushStrategy::addBrush(brush); }
        };

        typedef AssortNodesVisitorT<CollectLayersStrategy, CollectGroupsStrategy, CollectEntitiesStrategy, CollectBrushesStrategy> AssortNodesVisitor;
        typedef AssortNodesVisitorT<CollectLayersStrategy, SkipGroupsStrategy,    SkipEntitiesStrategy,    SkipBrushesStrategy>    CollectLayersVisitor;
        typedef AssortNodesVisitorT<SkipLayersStrategy,    CollectGroupsStrategy, SkipEntitiesStrategy,    SkipBrushesStrategy>    CollectGroupsVisitor;
        typedef AssortNodesVisitorT<SkipLayersStrategy,    CollectGroupsStrategy, CollectEntitiesStrategy, CollectBrushesStrategy>    CollectObjectsVisitor;
        typedef AssortNodesVisitorT<SkipLayersStrategy,    SkipGroupsStrategy,    SkipEntitiesStrategy,    CollectBrushesStrategy>    CollectBrushesVisitor;
    }
}

#endif /* defined(TrenchBroom_AssortNodesVisitor) */
