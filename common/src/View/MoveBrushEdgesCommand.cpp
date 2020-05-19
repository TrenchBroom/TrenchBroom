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

#include "MoveBrushEdgesCommand.h"

#include "FloatType.h"
#include "Model/BrushNode.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexHandleManager.h"

#include <vecmath/segment.h> // do not remove
#include <vecmath/polygon.h> // do not remove

#include <vector>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType MoveBrushEdgesCommand::Type = Command::freeType();

        std::unique_ptr<MoveBrushEdgesCommand> MoveBrushEdgesCommand::move(const EdgeToBrushesMap& edges, const vm::vec3& delta) {
            std::vector<Model::BrushNode*> brushes;
            BrushEdgesMap brushEdges;
            std::vector<vm::segment3> edgePositions;
            extractEdgeMap(edges, brushes, brushEdges, edgePositions);

            return std::make_unique<MoveBrushEdgesCommand>(brushes, brushEdges, edgePositions, delta);
        }

        MoveBrushEdgesCommand::MoveBrushEdgesCommand(const std::vector<Model::BrushNode*>& brushes, const BrushEdgesMap& edges, const std::vector<vm::segment3>& edgePositions, const vm::vec3& delta) :
        VertexCommand(Type, "Move Brush Edges", brushes),
        m_edges(edges),
        m_oldEdgePositions(edgePositions),
        m_delta(delta) {
            assert(!vm::is_zero(m_delta, vm::C::almost_zero()));
        }

        MoveBrushEdgesCommand::~MoveBrushEdgesCommand() = default;

        bool MoveBrushEdgesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const vm::bbox3& worldBounds = document->worldBounds();
            for (const auto& entry : m_edges) {
                const Model::BrushNode* brushNode = entry.first;
                const std::vector<vm::segment3>& edges = entry.second;
                const Model::Brush& brush = brushNode->brush();
                if (!brush.canMoveEdges(worldBounds, edges, m_delta)) {
                    return false;
                }
            }
            return true;
        }

        bool MoveBrushEdgesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newEdgePositions = document->performMoveEdges(m_edges, m_delta);
            return true;
        }

        bool MoveBrushEdgesCommand::doCollateWith(UndoableCommand* command) {
            MoveBrushEdgesCommand* other = static_cast<MoveBrushEdgesCommand*>(command);

            if (!canCollateWith(*other)) {
                return false;
            }

            if (m_newEdgePositions != other->m_oldEdgePositions) {
                return false;
            }

            m_newEdgePositions = other->m_newEdgePositions;
            m_delta = m_delta + other->m_delta;

            return true;
        }

        void MoveBrushEdgesCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const {
            manager.select(std::begin(m_newEdgePositions), std::end(m_newEdgePositions));
        }

        void MoveBrushEdgesCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const {
            manager.select(std::begin(m_oldEdgePositions), std::end(m_oldEdgePositions));
        }
    }
}
