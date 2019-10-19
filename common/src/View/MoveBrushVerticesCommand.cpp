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

#include "MoveBrushVerticesCommand.h"

#include "Constants.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType MoveBrushVerticesCommand::Type = Command::freeType();

        MoveBrushVerticesCommand::Ptr MoveBrushVerticesCommand::move(const Model::VertexToBrushesMap& vertices, const vm::vec3& delta) {
            Model::BrushList brushes;
            Model::BrushVerticesMap brushVertices;
            std::vector<vm::vec3> vertexPositions;
            extractVertexMap(vertices, brushes, brushVertices, vertexPositions);

            return Ptr(new MoveBrushVerticesCommand(brushes, brushVertices, vertexPositions, delta));
        }

        bool MoveBrushVerticesCommand::hasRemainingVertices() const {
            return !m_newVertexPositions.empty();
        }

        MoveBrushVerticesCommand::MoveBrushVerticesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const std::vector<vm::vec3>& vertexPositions, const vm::vec3& delta) :
        VertexCommand(Type, "Move Brush Vertices", brushes),
        m_vertices(vertices),
        m_oldVertexPositions(vertexPositions),
        m_delta(delta) {
            assert(!vm::is_zero(m_delta, vm::C::almost_zero()));
        }

        bool MoveBrushVerticesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const vm::bbox3& worldBounds = document->worldBounds();
            for (const auto& entry : m_vertices) {
                Model::Brush* brush = entry.first;
                const std::vector<vm::vec3>& vertices = entry.second;
                if (!brush->canMoveVertices(worldBounds, vertices, m_delta))
                    return false;
            }
            return true;
        }

        bool MoveBrushVerticesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newVertexPositions = document->performMoveVertices(m_vertices, m_delta);
            return true;
        }

        bool MoveBrushVerticesCommand::doCollateWith(UndoableCommand::Ptr command) {
            MoveBrushVerticesCommand* other = static_cast<MoveBrushVerticesCommand*>(command.get());

            if (!canCollateWith(*other)) {
                return false;
            }

            if (!VectorUtils::equals(m_newVertexPositions, other->m_oldVertexPositions)) {
                return false;
            }

            m_newVertexPositions = other->m_newVertexPositions;
            m_delta = m_delta + other->m_delta;

            return true;
        }

        void MoveBrushVerticesCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const {
            manager.select(std::begin(m_newVertexPositions), std::end(m_newVertexPositions));
        }

        void MoveBrushVerticesCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const {
            manager.select(std::begin(m_oldVertexPositions), std::end(m_oldVertexPositions));
        }
    }
}
