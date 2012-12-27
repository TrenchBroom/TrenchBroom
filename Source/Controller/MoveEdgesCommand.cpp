/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MoveEdgesCommand.h"
#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Face.h"

namespace TrenchBroom {
    namespace Controller {
        bool MoveEdgesCommand::performDo() {
            if (!canDo())
                return false;
            
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);
            
            BrushEdgesMap::const_iterator it, end;
            for (it = m_brushEdges.begin(), end = m_brushEdges.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Model::EdgeList& edges = it->second;
                brush->moveEdges(edges, m_delta);
            }

            document().brushesDidChange(m_brushes);
            return true;
        }
        
        bool MoveEdgesCommand::performUndo() {
            document().brushesWillChange(m_brushes);
            restoreSnapshots(m_brushes);
            document().brushesDidChange(m_brushes);
            return true;
        }
        
        MoveEdgesCommand::MoveEdgesCommand(Model::MapDocument& document, const wxString& name, const Model::VertexToEdgesMap& brushEdges, const Vec3f& delta) :
        SnapshotCommand(Command::MoveVertices, document, name),
        m_delta(delta) {
            Model::VertexToEdgesMap::const_iterator mapIt, mapEnd;
            for (mapIt = brushEdges.begin(), mapEnd = brushEdges.end(); mapIt != mapEnd; ++mapIt) {
                const Model::EdgeList& edges = mapIt->second;
                Model::EdgeList::const_iterator edgeIt, edgeEnd;
                for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                    Model::Edge* edge = *edgeIt;
                    Model::Brush* brush = edge->left->face->brush();
                    
                    BrushEdgesMapInsertResult result = m_brushEdges.insert(BrushEdgesMapEntry(brush, Model::EdgeList()));
                    if (result.second)
                        m_brushes.push_back(brush);
                    result.first->second.push_back(edge);
                    m_edges.push_back(edge);
                }
            }
            
            assert(!m_brushes.empty());
            assert(m_brushes.size() == m_brushEdges.size());
        }

        MoveEdgesCommand* MoveEdgesCommand::moveEdges(Model::MapDocument& document, const Model::VertexToEdgesMap& brushEdges, const Vec3f& delta) {
            return new MoveEdgesCommand(document, brushEdges.size() == 1 ? wxT("Move Edge") : wxT("Move Edges"), brushEdges, delta);
        }

        bool MoveEdgesCommand::canDo() const {
            BrushEdgesMap::const_iterator it, end;
            for (it = m_brushEdges.begin(), end = m_brushEdges.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Model::EdgeList& edges = it->second;
                if (!brush->canMoveEdges(edges, m_delta))
                    return false;
            }
            return true;
        }
    }
}
