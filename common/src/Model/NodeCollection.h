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

#ifndef TrenchBroom_NodeCollection
#define TrenchBroom_NodeCollection

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
        class EntityNode;
        class GroupNode;
        class LayerNode;
        class Node;

        class NodeCollection {
        private:
            class AddNode;
            class RemoveNode;
        private:
            std::vector<Node*> m_nodes;
            std::vector<LayerNode*> m_layers;
            std::vector<GroupNode*> m_groups;
            std::vector<EntityNode*> m_entities;
            std::vector<BrushNode*> m_brushes;
        public:
            bool empty() const;
            size_t nodeCount() const;
            size_t layerCount() const;
            size_t groupCount() const;
            size_t entityCount() const;
            size_t brushCount() const;

            bool hasLayers() const;
            bool hasOnlyLayers() const;
            bool hasGroups() const;
            bool hasOnlyGroups() const;
            bool hasEntities() const;
            bool hasOnlyEntities() const;
            bool hasBrushes() const;
            bool hasOnlyBrushes() const;

            std::vector<Node*>::iterator begin();
            std::vector<Node*>::iterator end();
            std::vector<Node*>::const_iterator begin() const;
            std::vector<Node*>::const_iterator end() const;

            const std::vector<Node*>& nodes() const;
            const std::vector<LayerNode*>& layers() const;
            const std::vector<GroupNode*>& groups() const;
            const std::vector<EntityNode*>& entities() const;
            const std::vector<BrushNode*>& brushes() const;

            void addNodes(const std::vector<Node*>& nodes);
            void addNode(Node* node);

            void removeNodes(const std::vector<Node*>& nodes);
            void removeNode(Node* node);

            void clear();
        };
    }
}

#endif /* defined(TrenchBroom_NodeCollection) */
