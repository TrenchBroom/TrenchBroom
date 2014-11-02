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

#include "SnapBrushVerticesCommand.h"

#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexHandleManager.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SnapBrushVerticesCommand::Type = Command::freeType();

        SnapBrushVerticesCommand* SnapBrushVerticesCommand::snap(VertexHandleManager& handleManager, const size_t snapTo) {
            return new SnapBrushVerticesCommand(handleManager, snapTo);
        }

        SnapBrushVerticesCommand::~SnapBrushVerticesCommand() {
            deleteSnapshot();
        }

        SnapBrushVerticesCommand::SnapBrushVerticesCommand(VertexHandleManager& handleManager, const size_t snapTo) :
        DocumentCommand(Type, "Snap brush vertices"),
        m_handleManager(handleManager),
        m_snapTo(snapTo),
        m_snapshot(NULL) {}
        
        bool SnapBrushVerticesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            const VertexInfo info = buildInfo();
            
            takeSnapshot(info.brushes);
            m_brushes = info.brushes;
            m_oldVertexPositions = info.vertexPositions;
            m_handleManager.removeBrushes(m_brushes);
            
            const Vec3::List newVertexPositions = document->performSnapVertices(info.vertices, m_snapTo);
            
            m_handleManager.addBrushes(m_brushes);
            m_handleManager.reselectVertexHandles(m_brushes, newVertexPositions, 0.01);
            
            return true;
        }
        
        bool SnapBrushVerticesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            assert(m_snapshot != NULL);

            m_handleManager.removeBrushes(m_brushes);
            document->restoreSnapshot(m_snapshot);
            deleteSnapshot();
            m_handleManager.addBrushes(m_brushes);
            m_handleManager.reselectVertexHandles(m_brushes, m_oldVertexPositions, 0.01);
            
            VectorUtils::clearToZero(m_brushes);
            VectorUtils::clearToZero(m_oldVertexPositions);
            
            return true;
        }
        
        SnapBrushVerticesCommand::VertexInfo SnapBrushVerticesCommand::buildInfo() const {
            VertexInfo result;
            if (m_handleManager.selectedVertexCount() > 0)
                buildInfo(m_handleManager.selectedVertexHandles(), result);
            else
                buildInfo(m_handleManager.unselectedVertexHandles(), result);
            return result;
        }

        void SnapBrushVerticesCommand::buildInfo(const Model::VertexToBrushesMap& vertices, VertexInfo& info) const {
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

        void SnapBrushVerticesCommand::takeSnapshot(const Model::BrushList& brushes) {
            assert(m_snapshot == NULL);
            m_snapshot = new Model::Snapshot(brushes.begin(), brushes.end());
        }

        void SnapBrushVerticesCommand::deleteSnapshot() {
            delete m_snapshot;
        }
        
        bool SnapBrushVerticesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
        
        bool SnapBrushVerticesCommand::doCollateWith(UndoableCommand* command) {
            return false;
        }
    }
}
