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

#ifndef __TrenchBroom__NodeWriter__
#define __TrenchBroom__NodeWriter__

#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace IO {
        class NodeSerializer;
        class NodeWriter {
        private:
            typedef std::map<Model::Entity*, Model::BrushList> EntityBrushesMap;
            class CollectEntityBrushesStrategy;
            
            NodeSerializer& m_serializer;
        public:
            NodeWriter(NodeSerializer& serializer);
            void writeNodes(const Model::NodeList& nodes);
            
            static void writeNodesToStream(const Model::NodeList& nodes, Model::MapFormat::Type format, std::ostream& stream);
        private:
            void writeGroups(const Model::GroupList& groups);
            void writeEntities(const Model::EntityList& entities);
            void writeWorldBrushes(const Model::BrushList& brushes);
            void writeEntityBrushes(const EntityBrushesMap& entityBrushes);
        };
    }
}

#endif /* defined(__TrenchBroom__NodeWriter__) */
