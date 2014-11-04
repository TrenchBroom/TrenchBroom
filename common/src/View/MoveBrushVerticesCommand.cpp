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
        MoveBrushVerticesCommand* MoveBrushVerticesCommand::move(VertexHandleManager& handleManager, const Vec3& delta) {
            return new MoveBrushVerticesCommand(handleManager, delta);
        }
        
        MoveBrushVerticesCommand::~MoveBrushVerticesCommand() {
            if (m_snapshot != NULL)
                deleteSnapshot();
        }

        bool MoveBrushVerticesCommand::hasRemainingVertices() const {
            return !m_newVertexPositions.empty();
        }

        MoveBrushVerticesCommand::MoveBrushVerticesCommand(VertexHandleManager& handleManager, const Vec3& delta) :
        DocumentCommand(Type, "Move vertices"),
        m_handleManager(handleManager),
        m_delta(delta),
        m_snapshot(NULL) {
            assert(!m_delta.null());
        }

        bool MoveBrushVerticesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            const BBox3& worldBounds = document->worldBounds();
            const VertexInfo info = buildInfo();
            if (!canMoveVertices(worldBounds, info.vertices))
                return false;
            
            takeSnapshot(info.brushes);
            m_brushes = info.brushes;
            m_oldVertexPositions = info.vertexPositions;
            m_handleManager.removeBrushes(m_brushes);
            
            m_newVertexPositions = document->performMoveVertices(info.vertices, m_delta);
            
            m_handleManager.addBrushes(m_brushes);
            m_handleManager.selectVertexHandles(m_newVertexPositions);
            
            return true;
        }
        
        bool MoveBrushVerticesCommand::canMoveVertices(const BBox3& worldBounds, const Model::BrushVerticesMap& brushVertices) const {
            Model::BrushVerticesMap::const_iterator it, end;
            for (it = brushVertices.begin(), end = brushVertices.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Vec3::List& vertices = it->second;
                if (!brush->canMoveVertices(worldBounds, vertices, m_delta))
                    return false;
            }
            return true;
        }

        bool MoveBrushVerticesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            assert(m_snapshot != NULL);
            
            m_handleManager.removeBrushes(m_brushes);
            document->restoreSnapshot(m_snapshot);
            deleteSnapshot();
            m_handleManager.addBrushes(m_brushes);
            m_handleManager.reselectVertexHandles(m_brushes, m_oldVertexPositions, 0.01);
            
            VectorUtils::clearToZero(m_brushes);
            VectorUtils::clearToZero(m_oldVertexPositions);
            VectorUtils::clearToZero(m_newVertexPositions);
            
            return true;
        }
        
        MoveBrushVerticesCommand::VertexInfo MoveBrushVerticesCommand::buildInfo() const {
            VertexInfo result;
            if (m_handleManager.selectedVertexCount() > 0)
                buildInfo(m_handleManager.selectedVertexHandles(), result);
            else
                buildInfo(m_handleManager.unselectedVertexHandles(), result);
            return result;
        }
        
        void MoveBrushVerticesCommand::buildInfo(const Model::VertexToBrushesMap& vertices, VertexInfo& info) const {
            typedef std::pair<Model::BrushVerticesMap::iterator, bool> BrushVerticesMapInsertResult;
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            Model::BrushList::const_iterator bIt, bEnd;
            
            for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                const Model::BrushList& vertexBrushes = vIt->second;
                for (bIt = vertexBrushes.begin(), bEnd = vertexBrushes.end(); bIt != bEnd; ++bIt) {
                    Model::Brush* brush = *bIt;
                    BrushVerticesMapInsertResult result = info.vertices.insert(std::make_pair(brush, Vec3::List()));
                    if (result.second)
                        info.brushes.push_back(brush);
                    result.first->second.push_back(position);
                }
                info.vertexPositions.push_back(position);
            }
        }
        
        void MoveBrushVerticesCommand::takeSnapshot(const Model::BrushList& brushes) {
            assert(m_snapshot == NULL);
            m_snapshot = new Model::Snapshot(brushes.begin(), brushes.end());
        }
        
        void MoveBrushVerticesCommand::deleteSnapshot() {
            delete m_snapshot;
            m_snapshot = NULL;
        }
        
        bool MoveBrushVerticesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return true;
        }
        
        UndoableCommand* MoveBrushVerticesCommand::doRepeat(MapDocumentCommandFacade* document) const {
            return new MoveBrushVerticesCommand(m_handleManager, m_delta);
        }

        bool MoveBrushVerticesCommand::doCollateWith(UndoableCommand* command) {
            MoveBrushVerticesCommand* other = static_cast<MoveBrushVerticesCommand*>(command);
            
            if (!VectorUtils::equals(m_newVertexPositions, other->m_oldVertexPositions))
                return false;
            
            m_newVertexPositions = other->m_newVertexPositions;
            m_delta += other->m_delta;
            
            return true;
        }
    }
}
