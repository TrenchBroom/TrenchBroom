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

#include "SplitEdgesCommand.h"

#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Face.h"

namespace TrenchBroom {
    namespace Controller {
        bool SplitEdgesCommand::performDo() {
            if (!canDo())
                return false;
            
            m_vertices.clear();
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);

            BrushEdgeMap::const_iterator it, end;
            for (it = m_brushEdges.begin(), end = m_brushEdges.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Model::EdgeInfo& edgeInfo = it->second;
                Vec3f newVertexPosition = brush->splitEdge(edgeInfo, m_delta);
                m_vertices.insert(newVertexPosition);
            }

            document().brushesDidChange(m_brushes);
            return true;
        }
        
        bool SplitEdgesCommand::performUndo() {
            document().brushesWillChange(m_brushes);
            restoreSnapshots(m_brushes);
            document().brushesDidChange(m_brushes);
            
            m_vertices.clear();
            return true;
        }

        SplitEdgesCommand::SplitEdgesCommand(Model::MapDocument& document, const wxString& name, const Model::EdgeList& edges, const Vec3f& delta) :
        SnapshotCommand(Command::MoveVertices, document, name),
        m_delta(delta) {
            Model::EdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                const Model::Edge& edge = **it;
                Model::EdgeInfo edgeInfo = edge.info();
                Model::Brush* brush = edge.left->face->brush();
                
                BrushEdgeMapInsertResult result = m_brushEdges.insert(BrushEdgeMapEntry(brush, edgeInfo));
                assert(result.second);
                m_brushes.push_back(brush);
            }
        }

        SplitEdgesCommand* SplitEdgesCommand::splitEdges(Model::MapDocument& document, const Model::EdgeList& edges, const Vec3f& delta) {
            return new SplitEdgesCommand(document, edges.size() == 1 ? wxT("Split Edge") : wxT("Split Edges"), edges, delta);
        }

        bool SplitEdgesCommand::canDo() const {
            BrushEdgeMap::const_iterator it, end;
            for (it = m_brushEdges.begin(), end = m_brushEdges.end(); it != end; ++it) {
                const Model::Brush* brush = it->first;
                const Model::EdgeInfo& edgeInfo = it->second;
                if (!brush->canSplitEdge(edgeInfo, m_delta))
                    return false;
            }
            return true;
        }
    }
}
