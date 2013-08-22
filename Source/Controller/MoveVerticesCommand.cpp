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

#include "MoveVerticesCommand.h"

#include "Controller/VertexHandleManager.h"
#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Utility/Console.h"
#include "Utility/List.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool MoveVerticesCommand::performDo() {
            if (!canDo())
                return false;
            
            m_handleManager.remove(m_brushes);
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);
            m_verticesAfter.clear();

            BrushVerticesMap::const_iterator mapIt, mapEnd;
            for (mapIt = m_brushVertices.begin(), mapEnd = m_brushVertices.end(); mapIt != mapEnd; ++mapIt) {
                Model::Brush* brush = mapIt->first;
                const Vec3f::List& oldVertexPositions = mapIt->second;
                const Vec3f::List newVertexPositions = brush->moveVertices(oldVertexPositions, m_delta);
                
                m_verticesAfter.insert(newVertexPositions.begin(), newVertexPositions.end());
            }
            
            document().brushesDidChange(m_brushes);
            m_handleManager.add(m_brushes);
            m_handleManager.selectVertexHandles(m_verticesAfter);

            return true;
        }
        
        bool MoveVerticesCommand::performUndo() {
            m_handleManager.remove(m_brushes);
            document().brushesWillChange(m_brushes);
            restoreSnapshots(m_brushes);
            document().brushesDidChange(m_brushes);
            m_handleManager.add(m_brushes);
            m_handleManager.selectVertexHandles(m_verticesBefore);

            return true;
        }
        
        MoveVerticesCommand::MoveVerticesCommand(Model::MapDocument& document, const wxString& name, VertexHandleManager& handleManager, const Vec3f& delta) :
        SnapshotCommand(Command::MoveVertices, document, name),
        m_handleManager(handleManager),
        m_delta(delta) {
            const Model::VertexToBrushesMap& vertices = m_handleManager.selectedVertexHandles();
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                const Vec3f& position = vIt->first;
                const Model::BrushList& vertexBrushes = vIt->second;
                Model::BrushList::const_iterator brushIt, brushEnd;
                for (brushIt = vertexBrushes.begin(), brushEnd = vertexBrushes.end(); brushIt != brushEnd; ++brushIt) {
                    Model::Brush* brush = *brushIt;
                    BrushVerticesMapInsertResult result = m_brushVertices.insert(BrushVerticesMapEntry(brush, Vec3f::List()));
                    if (result.second)
                        m_brushes.push_back(brush);
                    result.first->second.push_back(position);
                    m_verticesBefore.insert(position);
                }
            }
            
            assert(!m_brushes.empty());
            assert(m_brushes.size() == m_brushVertices.size());
        }

        MoveVerticesCommand* MoveVerticesCommand::moveVertices(Model::MapDocument& document, VertexHandleManager& handleManager, const Vec3f& delta) {
            return new MoveVerticesCommand(document, handleManager.selectedVertexHandles().size() == 1 ? wxT("Move Vertex") : wxT("Move Vertices"), handleManager, delta);
        }

        bool MoveVerticesCommand::canDo() const {
            BrushVerticesMap::const_iterator it, end;
            for (it = m_brushVertices.begin(), end = m_brushVertices.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Vec3f::List& vertices = it->second;
                if (!brush->canMoveVertices(vertices, m_delta))
                    return false;
            }
            return true;
        }

        bool MoveVerticesCommand::hasRemainingVertices() const {
            if (state() == Done)
                return !m_verticesAfter.empty();
            return !m_verticesBefore.empty();
        }
    }
}
