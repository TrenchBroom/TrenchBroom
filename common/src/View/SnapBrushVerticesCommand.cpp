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

#include "SnapBrushVerticesCommand.h"

#include "Model/BrushNode.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SnapBrushVerticesCommand::Type = Command::freeType();

        std::unique_ptr<SnapBrushVerticesCommand> SnapBrushVerticesCommand::snap(const FloatType snapTo) {
            return std::make_unique<SnapBrushVerticesCommand>(snapTo);
        }

        SnapBrushVerticesCommand::SnapBrushVerticesCommand(const FloatType snapTo) :
        SnapshotCommand(Type, "Snap Brush Vertices"),
        m_snapTo(snapTo) {}

        std::unique_ptr<CommandResult> SnapBrushVerticesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            const auto success = document->performSnapVertices(std::nullopt, m_snapTo);
            return std::make_unique<CommandResult>(success);
        }

        bool SnapBrushVerticesCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool SnapBrushVerticesCommand::doCollateWith(UndoableCommand* command) {
            SnapBrushVerticesCommand* other = static_cast<SnapBrushVerticesCommand*>(command);
            return other->m_snapTo == m_snapTo;
        }

        // SnapSpecificBrushVerticesCommand

        const Command::CommandType SnapSpecificBrushVerticesCommand::Type = Command::freeType();

        std::unique_ptr<SnapSpecificBrushVerticesCommand> SnapSpecificBrushVerticesCommand::snap(const FloatType snapTo, const VertexToBrushesMap& vertices) {
            std::vector<Model::BrushNode*> brushes;
            BrushVerticesMap brushVertices;
            std::vector<vm::vec3> vertexPositions;
            extractVertexMap(vertices, brushes, brushVertices, vertexPositions);

            return std::make_unique<SnapSpecificBrushVerticesCommand>(snapTo, brushes, brushVertices);
        }

        SnapSpecificBrushVerticesCommand::SnapSpecificBrushVerticesCommand(const FloatType snapTo, const std::vector<Model::BrushNode*>& brushes, const BrushVerticesMap& vertices) :
        VertexCommand(Type, "Snap Brush Vertices", brushes),
        m_snapTo(snapTo),
        m_vertices(vertices) {}

        bool SnapSpecificBrushVerticesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const vm::bbox3& worldBounds = document->worldBounds();
            for (const auto& entry : m_vertices) {
                const Model::BrushNode* brushNode = entry.first;
                const Model::Brush& brush = brushNode->brush();
                const std::vector<vm::vec3>& vertices = entry.second;
                if (!brush.canSnapVertices(worldBounds, m_snapTo, std::make_optional(vertices))) {
                    return false;
                }
            }
            return true;
        }

        bool SnapSpecificBrushVerticesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            document->performSnapVertices(std::make_optional(m_vertices), m_snapTo);
            return true;
        }

        bool SnapSpecificBrushVerticesCommand::doCollateWith(UndoableCommand*) {
            return false;
        }
    }
}
