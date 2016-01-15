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

#include "MoveBrushVerticesCommand.h"

#include "Model/Brush.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexHandleManager.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType MoveBrushVerticesCommand::Type = Command::freeType();

        MoveBrushVerticesCommand::Ptr MoveBrushVerticesCommand::move(const Model::VertexToBrushesMap& vertices, const Vec3& delta) {
            Model::BrushList brushes;
            Model::BrushVerticesMap brushVertices;
            Vec3::List vertexPositions;
            extractVertexMap(vertices, brushes, brushVertices, vertexPositions);
            
            return Ptr(new MoveBrushVerticesCommand(brushes, brushVertices, vertexPositions, delta));
        }

        bool MoveBrushVerticesCommand::hasRemainingVertices() const {
            return !m_newVertexPositions.empty();
        }

        MoveBrushVerticesCommand::MoveBrushVerticesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const Vec3::List& vertexPositions, const Vec3& delta) :
        VertexCommand(Type, "Move vertices", brushes),
        m_vertices(vertices),
        m_oldVertexPositions(vertexPositions),
        m_delta(delta) {
            assert(!m_delta.null());
        }

        bool MoveBrushVerticesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const BBox3& worldBounds = document->worldBounds();
            Model::BrushVerticesMap::const_iterator it, end;
            for (it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Vec3::List& vertices = it->second;
                if (!brush->canMoveVertices(worldBounds, vertices, m_delta))
                    return false;
            }
            return true;
        }

        bool MoveBrushVerticesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newVertexPositions = document->performMoveVertices(m_vertices, m_delta);
            return true;
        }

        void MoveBrushVerticesCommand::doSelectNewHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes) {
            manager.selectVertexHandles(m_newVertexPositions);
        }
        
        void MoveBrushVerticesCommand::doSelectOldHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes) {
            manager.selectVertexHandles(m_oldVertexPositions);
        }

        bool MoveBrushVerticesCommand::doCollateWith(UndoableCommand::Ptr command) {
            MoveBrushVerticesCommand* other = static_cast<MoveBrushVerticesCommand*>(command.get());
            
            if (!VectorUtils::equals(m_newVertexPositions, other->m_oldVertexPositions))
                return false;
            
            m_newVertexPositions = other->m_newVertexPositions;
            m_delta += other->m_delta;
            
            return true;
        }
    }
}
