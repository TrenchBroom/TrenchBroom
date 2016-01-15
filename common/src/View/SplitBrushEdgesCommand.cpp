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

#include "SplitBrushEdgesCommand.h"

#include "Model/Brush.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexHandleManager.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SplitBrushEdgesCommand::Type = Command::freeType();
        
        SplitBrushEdgesCommand::Ptr SplitBrushEdgesCommand::split(const Model::VertexToEdgesMap& edges, const Vec3& delta) {
            Model::BrushList brushes;
            Model::BrushEdgesMap brushEdges;
            Edge3::List edgePositions;
            extractEdgeMap(edges, brushes, brushEdges, edgePositions);
            
            return Ptr(new SplitBrushEdgesCommand(brushes, brushEdges, edgePositions, delta));
        }
        
        SplitBrushEdgesCommand::SplitBrushEdgesCommand(const Model::BrushList& brushes, const Model::BrushEdgesMap& edges, const Edge3::List& edgePositions, const Vec3& delta) :
        VertexCommand(Type, "Split edges", brushes),
        m_edges(edges),
        m_oldEdgePositions(edgePositions),
        m_delta(delta) {
            assert(!m_delta.null());
        }
        
        bool SplitBrushEdgesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const BBox3& worldBounds = document->worldBounds();
            Model::BrushEdgesMap::const_iterator it, end;
            for (it = m_edges.begin(), end = m_edges.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Edge3::List& edges = it->second;
                for (size_t i = 0; i < edges.size(); ++i) {
                    const Edge3& edge = edges[i];
                    if (!brush->canSplitEdge(worldBounds, edge, m_delta))
                        return false;
                }
            }
            return true;
        }
        
        bool SplitBrushEdgesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newVertexPositions = document->performSplitEdges(m_edges, m_delta);
            return true;
        }
        
        void SplitBrushEdgesCommand::doSelectNewHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes) {
            manager.selectVertexHandles(m_newVertexPositions);
        }
        
        void SplitBrushEdgesCommand::doSelectOldHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes) {
            manager.selectEdgeHandles(m_oldEdgePositions);
        }
        
        bool SplitBrushEdgesCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
