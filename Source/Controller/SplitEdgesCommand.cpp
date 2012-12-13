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
            m_vertices.clear();
            Model::EdgeList::const_iterator eIt, eEnd;
            for (eIt = m_edges.begin(), eEnd = m_edges.end(); eIt != eEnd; ++eIt) {
                Model::Edge* edge = *eIt;
                Model::Brush* brush = edge->left->face->brush();
                if (!brush->canSplitEdge(edge, m_delta))
                    return false;
            }
            
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);

            for (eIt = m_edges.begin(), eEnd = m_edges.end(); eIt != eEnd; ++eIt) {
                Model::Edge* edge = *eIt;
                Model::Brush* brush = edge->left->face->brush();
                Vec3f newVertexPosition = brush->splitEdge(edge, m_delta);
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
        m_edges(edges),
        m_delta(delta) {
            Model::EdgeList::const_iterator it, end;
            for (it = m_edges.begin(), end = m_edges.end(); it != end; ++it) {
                const Model::Edge& edge = **it;
                m_brushes.push_back(edge.left->face->brush());
            }
        }

        SplitEdgesCommand* SplitEdgesCommand::splitEdges(Model::MapDocument& document, const Model::EdgeList& edges, const Vec3f& delta) {
            return new SplitEdgesCommand(document, edges.size() == 1 ? wxT("Split Edge") : wxT("Split Edges"), edges, delta);
        }
    }
}