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

#ifndef TrenchBroom_NodeCollection
#define TrenchBroom_NodeCollection

#include "Model/ModelTypes.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        class NodeCollection {
        private:
            class AddNode;
            class RemoveNode;
        private:
            NodeList m_nodes;
            LayerList m_layers;
            GroupList m_groups;
            EntityList m_entities;
            BrushList m_brushes;
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

            NodeList::iterator begin();
            NodeList::iterator end();
            NodeList::const_iterator begin() const;
            NodeList::const_iterator end() const;
            
            const NodeList& nodes() const;
            const LayerList& layers() const;
            const GroupList& groups() const;
            const EntityList& entities() const;
            const BrushList& brushes() const;

            void addNodes(const NodeList& nodes);
            void addNode(Node* node);

            void removeNodes(const NodeList& nodes);
            void removeNode(Node* node);
            
            void clear();
        };
    }
}

#endif /* defined(TrenchBroom_NodeCollection) */
