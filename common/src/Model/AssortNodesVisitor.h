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

#ifndef TrenchBroom_AssortNodesVisitor
#define TrenchBroom_AssortNodesVisitor

#include "Model/NodeVisitor.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
        class Entity;
        class Group;
        class Layer;
        class World;

        class CollectLayersStrategy {
        private:
            std::vector<Layer*> m_layers;
        public:
            const std::vector<Layer*>& layers() const;
        protected:
            void addLayer(Layer* layer);
        };

        class SkipLayersStrategy {
        private:
            static const std::vector<Layer*> m_layers;
        public:
            const std::vector<Layer*>& layers() const;
        protected:
            void addLayer(Layer* layer);
        };

        class CollectGroupsStrategy {
        private:
            std::vector<Group*> m_groups;
        public:
            const std::vector<Group*>& groups() const;
        protected:
            void addGroup(Group* group);
        };

        class SkipGroupsStrategy {
        private:
            static const std::vector<Group*> m_groups;
        public:
            const std::vector<Group*>& groups() const;
        protected:
            void addGroup(Group* group);
        };

        class CollectEntitiesStrategy {
        private:
            std::vector<Entity*> m_entities;
        public:
            const std::vector<Entity*>& entities() const;
        protected:
            void addEntity(Entity* entity);
        };

        class SkipEntitiesStrategy {
        private:
            static const std::vector<Entity*> m_entities;
        public:
            const std::vector<Entity*>& entities() const;
        protected:
            void addEntity(Entity* entity);
        };

        class CollectBrushesStrategy {
        private:
            std::vector<BrushNode*> m_brushes;
        public:
            const std::vector<BrushNode*>& brushes() const;
        protected:
            void addBrush(BrushNode* brush);
        };

        class SkipBrushesStrategy {
        private:
            static const std::vector<BrushNode*> m_brushes;
        public:
            const std::vector<BrushNode*>& brushes() const;
        protected:
            void addBrush(BrushNode* brush);
        };

        template <class LayerStrategy, class GroupStrategy, class EntityStrategy, class BrushStrategy>
        class AssortNodesVisitorT : public NodeVisitor, public LayerStrategy, public GroupStrategy, public EntityStrategy, public BrushStrategy {
        private:
            void doVisit(World* /* world */)   override {}
            void doVisit(Layer* layer)   override {  LayerStrategy::addLayer(layer); }
            void doVisit(Group* group)   override {  GroupStrategy::addGroup(group); }
            void doVisit(Entity* entity) override { EntityStrategy::addEntity(entity); }
            void doVisit(BrushNode* brush)   override {  BrushStrategy::addBrush(brush); }
        };

        using AssortNodesVisitor = AssortNodesVisitorT<CollectLayersStrategy, CollectGroupsStrategy, CollectEntitiesStrategy, CollectBrushesStrategy>;
        using CollectLayersVisitor = AssortNodesVisitorT<CollectLayersStrategy, SkipGroupsStrategy,    SkipEntitiesStrategy,    SkipBrushesStrategy>   ;
        using CollectGroupsVisitor = AssortNodesVisitorT<SkipLayersStrategy,    CollectGroupsStrategy, SkipEntitiesStrategy,    SkipBrushesStrategy>   ;
        using CollectObjectsVisitor = AssortNodesVisitorT<SkipLayersStrategy,    CollectGroupsStrategy, CollectEntitiesStrategy, CollectBrushesStrategy>   ;
        using CollectBrushesVisitor = AssortNodesVisitorT<SkipLayersStrategy,    SkipGroupsStrategy,    SkipEntitiesStrategy,    CollectBrushesStrategy>   ;
    }
}

#endif /* defined(TrenchBroom_AssortNodesVisitor) */
