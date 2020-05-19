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

#ifndef TrenchBroom_NodeWriter
#define TrenchBroom_NodeWriter

#include "IO/NodeSerializer.h"

#include <cstdio> // FILE*
#include <map>
#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
        class BrushFace;
        class EntityNode;
        class LayerNode;
        class Node;
        class WorldNode;
    }

    namespace IO {
        class NodeWriter {
        private:
            using EntityBrushesMap = std::map<Model::EntityNode*, std::vector<Model::BrushNode*>>;
            class CollectEntityBrushesStrategy;
            class WriteNode;

            const Model::WorldNode& m_world;
            std::unique_ptr<NodeSerializer> m_serializer;
        public:
            NodeWriter(const Model::WorldNode& world, FILE* stream);
            NodeWriter(const Model::WorldNode& world, std::ostream& stream);
            NodeWriter(const Model::WorldNode& world, NodeSerializer* serializer);

            void writeMap();
        private:
            void writeDefaultLayer();
            void writeCustomLayers();
            void writeCustomLayer(const Model::LayerNode* layer);
        public:
            void writeNodes(const std::vector<Model::Node*>& nodes);
        private:
            void writeWorldBrushes(const std::vector<Model::BrushNode*>& brushes);
            void writeEntityBrushes(const EntityBrushesMap& entityBrushes);
        public:
            void writeBrushFaces(const std::vector<Model::BrushFace>& faces);
        };
    }
}

#endif /* defined(TrenchBroom_NodeWriter) */
