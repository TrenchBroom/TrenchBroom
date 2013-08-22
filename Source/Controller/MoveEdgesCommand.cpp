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

#include "Controller/VertexHandleManager.h"
#include "MoveEdgesCommand.h"
#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Face.h"
#include "Utility/Console.h"

namespace TrenchBroom {
    namespace Controller {
        bool MoveEdgesCommand::performDo() {
            if (!canDo())
                return false;

            m_handleManager.remove(m_brushes);
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);
            m_edgesAfter.clear();
            
            Model::BrushEdgesMap::const_iterator it, end;
            for (it = m_brushEdges.begin(), end = m_brushEdges.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Model::EdgeInfoList& edgeInfos = it->second;
                const Model::EdgeInfoList newEdgeInfos = brush->moveEdges(edgeInfos, m_delta);
                m_edgesAfter.insert(m_edgesAfter.end(), newEdgeInfos.begin(), newEdgeInfos.end());
            }

            document().brushesDidChange(m_brushes);
            m_handleManager.add(m_brushes);
            m_handleManager.selectEdgeHandles(m_edgesAfter);

            return true;
        }

        bool MoveEdgesCommand::performUndo() {
            m_handleManager.remove(m_brushes);
            document().brushesWillChange(m_brushes);
            restoreSnapshots(m_brushes);
            document().brushesDidChange(m_brushes);
            m_handleManager.add(m_brushes);
            m_handleManager.selectEdgeHandles(m_edgesBefore);
            
            return true;
        }

        MoveEdgesCommand::MoveEdgesCommand(Model::MapDocument& document, const wxString& name, VertexHandleManager& handleManager, const Vec3f& delta) :
        SnapshotCommand(Command::MoveVertices, document, name),
        m_handleManager(handleManager),
        m_delta(delta) {
            const Model::VertexToEdgesMap& brushEdges = m_handleManager.selectedEdgeHandles();
            Model::VertexToEdgesMap::const_iterator mapIt, mapEnd;
            for (mapIt = brushEdges.begin(), mapEnd = brushEdges.end(); mapIt != mapEnd; ++mapIt) {
                const Model::EdgeList& edges = mapIt->second;
                Model::EdgeList::const_iterator edgeIt, edgeEnd;
                for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                    Model::Edge* edge = *edgeIt;
                    Model::Brush* brush = edge->left->face->brush();
                    const Model::EdgeInfo edgeInfo = edge->info();

                    Model::BrushEdgesMapInsertResult result = m_brushEdges.insert(Model::BrushEdgesMapEntry(brush, Model::EdgeInfoList()));
                    if (result.second)
                        m_brushes.push_back(brush);
                    result.first->second.push_back(edgeInfo);
                    m_edgesBefore.push_back(edgeInfo);
                }
            }

            assert(!m_brushes.empty());
            assert(m_brushes.size() == m_brushEdges.size());
        }

        MoveEdgesCommand* MoveEdgesCommand::moveEdges(Model::MapDocument& document, VertexHandleManager& handleManager, const Vec3f& delta) {
            return new MoveEdgesCommand(document, handleManager.selectedEdgeHandles().size() == 1 ? wxT("Move Edge") : wxT("Move Edges"), handleManager, delta);
        }

        bool MoveEdgesCommand::canDo() const {
            Model::BrushEdgesMap::const_iterator it, end;
            for (it = m_brushEdges.begin(), end = m_brushEdges.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Model::EdgeInfoList& edges = it->second;
                if (!brush->canMoveEdges(edges, m_delta))
                    return false;
            }
            return true;
        }
    }
}
