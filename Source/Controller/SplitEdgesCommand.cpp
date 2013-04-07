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

#include "Controller/VertexHandleManager.h"
#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Face.h"

namespace TrenchBroom {
    namespace Controller {
        bool SplitEdgesCommand::performDo() {
            if (!canDo())
                return false;
            
            m_handleManager.remove(m_brushes);
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);
            m_verticesAfter.clear();

            Model::BrushEdgesMap::const_iterator bIt, bEnd;
            for (bIt = m_brushEdges.begin(), bEnd = m_brushEdges.end(); bIt != bEnd; ++bIt) {
                Model::Brush* brush = bIt->first;
                const Model::EdgeInfoList& edgeInfos = bIt->second;
                Model::EdgeInfoList::const_iterator eIt, eEnd;
                for (eIt = edgeInfos.begin(), eEnd = edgeInfos.end(); eIt != eEnd; ++eIt) {
                    const Model::EdgeInfo& edgeInfo = *eIt;
                    const Vec3f newVertexPosition = brush->splitEdge(edgeInfo, m_delta);
                    m_verticesAfter.insert(newVertexPosition);
                }
            }

            document().brushesDidChange(m_brushes);
            m_handleManager.add(m_brushes);
            m_handleManager.selectVertexHandles(m_verticesAfter);

            return true;
        }
        
        bool SplitEdgesCommand::performUndo() {
            m_handleManager.remove(m_brushes);
            document().brushesWillChange(m_brushes);
            restoreSnapshots(m_brushes);
            document().brushesDidChange(m_brushes);
            m_handleManager.add(m_brushes);
            m_handleManager.selectEdgeHandles(m_edgesBefore);

            return true;
        }

        SplitEdgesCommand::SplitEdgesCommand(Model::MapDocument& document, const wxString& name, VertexHandleManager& handleManager, const Vec3f& delta) :
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
        }

        SplitEdgesCommand* SplitEdgesCommand::splitEdges(Model::MapDocument& document, VertexHandleManager& handleManager, const Vec3f& delta) {
            return new SplitEdgesCommand(document, handleManager.selectedEdgeHandles().size() == 1 ? wxT("Split Edge") : wxT("Split Edges"), handleManager, delta);
        }

        bool SplitEdgesCommand::canDo() const {
            Model::BrushEdgesMap::const_iterator bIt, bEnd;
            for (bIt = m_brushEdges.begin(), bEnd = m_brushEdges.end(); bIt != bEnd; ++bIt) {
                Model::Brush* brush = bIt->first;
                const Model::EdgeInfoList& edgeInfos = bIt->second;
                Model::EdgeInfoList::const_iterator eIt, eEnd;
                for (eIt = edgeInfos.begin(), eEnd = edgeInfos.end(); eIt != eEnd; ++eIt) {
                    const Model::EdgeInfo& edgeInfo = *eIt;
                    if (!brush->canSplitEdge(edgeInfo, m_delta))
                        return false;
                }
            }
            return true;
        }
    }
}
