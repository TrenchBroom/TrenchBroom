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

#include "Model/Brush.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType MoveBrushEdgesCommand::Type = Command::freeType();
        
        MoveBrushEdgesCommand::Ptr MoveBrushEdgesCommand::move(const Model::EdgeToBrushesMap& edges, const vec3& delta) {
            Model::BrushList brushes;
            Model::BrushEdgesMap brushEdges;
            segment3::List edgePositions;
            extractEdgeMap(edges, brushes, brushEdges, edgePositions);
            
            return Ptr(new MoveBrushEdgesCommand(brushes, brushEdges, edgePositions, delta));
        }

        MoveBrushEdgesCommand::MoveBrushEdgesCommand(const Model::BrushList& brushes, const Model::BrushEdgesMap& edges, const segment3::List& edgePositions, const vec3& delta) :
        VertexCommand(Type, "Move Brush Edges", brushes),
        m_edges(edges),
        m_oldEdgePositions(edgePositions),
        m_delta(delta) {
            assert(!isZero(m_delta));
        }
        
        bool MoveBrushEdgesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const bbox3& worldBounds = document->worldBounds();
            for (const auto& entry : m_edges) {
                Model::Brush* brush = entry.first;
                const segment3::List& edges = entry.second;
                if (!brush->canMoveEdges(worldBounds, edges, m_delta))
                    return false;
            }
            return true;
        }
        
        bool MoveBrushEdgesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newEdgePositions = document->performMoveEdges(m_edges, m_delta);
            return true;
        }

        bool MoveBrushEdgesCommand::doCollateWith(UndoableCommand::Ptr command) {
            MoveBrushEdgesCommand* other = static_cast<MoveBrushEdgesCommand*>(command.get());

            if (!VectorUtils::equals(m_newEdgePositions, other->m_oldEdgePositions))
                return false;

            m_newEdgePositions = other->m_newEdgePositions;
            m_delta = m_delta + other->m_delta;

            return true;
        }

        void MoveBrushEdgesCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<segment3>& manager) const {
            manager.select(std::begin(m_newEdgePositions), std::end(m_newEdgePositions));
        }
        
        void MoveBrushEdgesCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<segment3>& manager) const {
            manager.select(std::begin(m_oldEdgePositions), std::end(m_oldEdgePositions));
        }
    }
}
