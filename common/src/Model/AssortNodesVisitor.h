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
        class EntityNode;
        class GroupNode;
        class LayerNode;
        class WorldNode;

        class CollectLayersStrategy {
        private:
            std::vector<LayerNode*> m_layers;
        public:
            const std::vector<LayerNode*>& layers() const;
        protected:
            void addLayer(LayerNode* layer);
        };

        class SkipLayersStrategy {
        private:
            static const std::vector<LayerNode*> m_layers;
        public:
            const std::vector<LayerNode*>& layers() const;
        protected:
            void addLayer(LayerNode* layer);
        };

        class CollectGroupsStrategy {
        private:
            std::vector<GroupNode*> m_groups;
        public:
            const std::vector<GroupNode*>& groups() const;
        protected:
            void addGroup(GroupNode* group);
        };

        class SkipGroupsStrategy {
        private:
            static const std::vector<GroupNode*> m_groups;
        public:
            const std::vector<GroupNode*>& groups() const;
        protected:
            void addGroup(GroupNode* group);
        };

        class CollectEntitiesStrategy {
        private:
            std::vector<EntityNode*> m_entities;
        public:
            const std::vector<EntityNode*>& entities() const;
        protected:
            void addEntity(EntityNode* entity);
        };

        class SkipEntitiesStrategy {
        private:
            static const std::vector<EntityNode*> m_entities;
        public:
            const std::vector<EntityNode*>& entities() const;
        protected:
            void addEntity(EntityNode* entity);
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
            void doVisit(WorldNode* /* world */)   override {}
            void doVisit(LayerNode* layer)   override {  LayerStrategy::addLayer(layer); }
            void doVisit(GroupNode* group)   override {  GroupStrategy::addGroup(group); }
            void doVisit(EntityNode* entity) override { EntityStrategy::addEntity(entity); }
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
