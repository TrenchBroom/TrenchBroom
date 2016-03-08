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

#ifndef TrenchBroom_NodeWriter
#define TrenchBroom_NodeWriter

#include "IO/NodeSerializer.h"
#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"

#include <cstdio>

namespace TrenchBroom {
    namespace IO {
        class Path;
        class NodeSerializer;
        
        class NodeWriter {
        private:
            typedef std::map<Model::Entity*, Model::BrushList> EntityBrushesMap;
            class CollectEntityBrushesStrategy;
            class WriteNode;
            
            Model::World* m_world;
            NodeSerializer::Ptr m_serializer;
        public:
            NodeWriter(Model::World* world, FILE* stream);
            NodeWriter(Model::World* world, std::ostream& stream);
            
            void writeMap();
        private:
            void writeDefaultLayer();
            void writeCustomLayers();
            void writeCustomLayer(Model::Layer* layer);
        public:
            void writeNodes(const Model::NodeList& nodes);
        private:
            void writeWorldBrushes(const Model::BrushList& brushes);
            void writeEntityBrushes(const EntityBrushesMap& entityBrushes);
        public:
            void writeBrushFaces(const Model::BrushFaceList& faces);
        };
    }
}

#endif /* defined(TrenchBroom_NodeWriter) */
