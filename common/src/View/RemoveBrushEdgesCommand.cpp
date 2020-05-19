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

#include "RemoveBrushEdgesCommand.h"

#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/VertexHandleManager.h"

#include <vecmath/segment.h> // do not remove
#include <vecmath/polygon.h> // do not remove

#include <vector>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType RemoveBrushEdgesCommand::Type = Command::freeType();

        std::unique_ptr<RemoveBrushEdgesCommand> RemoveBrushEdgesCommand::remove(const EdgeToBrushesMap& edges) {
            std::vector<Model::BrushNode*> brushes;
            BrushEdgesMap brushEdges;
            std::vector<vm::segment3> edgePositions;

            extractEdgeMap(edges, brushes, brushEdges, edgePositions);
            BrushVerticesMap brushVertices = brushVertexMap(brushEdges);

            return std::make_unique<RemoveBrushEdgesCommand>(brushes, brushVertices, edgePositions);
        }

        RemoveBrushEdgesCommand::RemoveBrushEdgesCommand(const std::vector<Model::BrushNode*>& brushes, const BrushVerticesMap& vertices, const std::vector<vm::segment3>& edgePositions) :
        RemoveBrushElementsCommand(Type, "Remove Brush Edges", brushes, vertices),
        m_oldEdgePositions(edgePositions) {}

        RemoveBrushEdgesCommand::~RemoveBrushEdgesCommand() = default;

        void RemoveBrushEdgesCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const {
            manager.select(std::begin(m_oldEdgePositions), std::end(m_oldEdgePositions));
        }
    }
}
