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

#include "Model/Brush.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexHandleManagerOld.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType RemoveBrushEdgesCommand::Type = Command::freeType();

        RemoveBrushEdgesCommand::Ptr RemoveBrushEdgesCommand::remove(const Model::VertexToEdgesMap& edges) {
            Model::BrushList brushes;
            Model::BrushEdgesMap brushEdges;
            Edge3::List edgePositions;
            
            extractEdgeMap(edges, brushes, brushEdges, edgePositions);
            Model::BrushVerticesMap brushVertices = brushVertexMap(brushEdges);
            
            return Ptr(new RemoveBrushEdgesCommand(brushes, brushVertices, edgePositions));
        }

        RemoveBrushEdgesCommand::RemoveBrushEdgesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const Edge3::List& edgePositions) :
        RemoveBrushElementsCommand(Type, "Remove Brush Edges", brushes, vertices),
        m_oldEdgePositions(edgePositions) {}

        void RemoveBrushEdgesCommand::doSelectOldHandlePositions(VertexHandleManagerOld& manager, const Model::BrushList& brushes) {
            manager.selectEdgeHandles(m_oldEdgePositions);
        }
    }
}
