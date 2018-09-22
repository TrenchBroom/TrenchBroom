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

#include "RemoveBrushVerticesCommand.h"

#include "Model/Brush.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType RemoveBrushVerticesCommand::Type = Command::freeType();

        RemoveBrushVerticesCommand::Ptr RemoveBrushVerticesCommand::remove(const Model::VertexToBrushesMap& vertices) {
            Model::BrushList brushes;
            Model::BrushVerticesMap brushVertices;
            std::vector<vm::vec3> vertexPositions;
            extractVertexMap(vertices, brushes, brushVertices, vertexPositions);
            
            return Ptr(new RemoveBrushVerticesCommand(brushes, brushVertices, vertexPositions));
        }

        RemoveBrushVerticesCommand::RemoveBrushVerticesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const std::vector<vm::vec3>& vertexPositions) :
        RemoveBrushElementsCommand(Type, "Remove Brush Vertices", brushes, vertices),
        m_oldVertexPositions(vertexPositions) {}
    }
}
