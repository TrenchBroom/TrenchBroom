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

#include "Model/Brush.h"
#include "Model/BrushGeometry.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool MoveVerticesCommand::performDo() {
            BrushVerticesMap::const_iterator it, end;
            for (it = m_brushVertices.begin(), end = m_brushVertices.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Vec3f::List& vertices = it->second;
                if (!brush->canMoveVertices(vertices, m_delta))
                    return false;
            }
            
            m_vertices.clear();
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);

            for (it = m_brushVertices.begin(), end = m_brushVertices.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Vec3f::List& oldVertexPositions = it->second;
                Vec3f::List newVertexPositions = brush->moveVertices(oldVertexPositions, m_delta);
                m_vertices.insert(newVertexPositions.begin(), newVertexPositions.end());
            }
            
            document().brushesDidChange(m_brushes);
            return true;
        }
        
        bool MoveVerticesCommand::performUndo() {
            document().brushesWillChange(m_brushes);
            restoreSnapshots(m_brushes);
            document().brushesDidChange(m_brushes);

            m_vertices.clear();
            BrushVerticesMap::const_iterator it, end;
            for (it = m_brushVertices.begin(), end = m_brushVertices.end(); it != end; ++it) {
                const Vec3f::List& vertices = it->second;
                m_vertices.insert(vertices.begin(), vertices.end());
            }
            
            return true;
        }
        
        MoveVerticesCommand::MoveVerticesCommand(Model::MapDocument& document, const wxString& name, const Model::VertexToBrushesMap& brushVertices, const Vec3f& delta) :
        SnapshotCommand(Command::MoveVertices, document, name),
        m_delta(delta) {
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            for (vIt = brushVertices.begin(), vEnd = brushVertices.end(); vIt != vEnd; ++vIt) {
                const Vec3f& position = vIt->first;
                const Model::BrushList& brushes = vIt->second;
                Model::BrushList::const_iterator bIt, bEnd;
                for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                    Model::Brush* brush = *bIt;
                    BrushVerticesMapInsertResult result = m_brushVertices.insert(BrushVerticesMapEntry(brush, Vec3f::EmptyList));
                    if (result.second)
                        m_brushes.push_back(brush);
                    result.first->second.push_back(position);
                }
                m_vertices.insert(position);
            }
            assert(!m_brushVertices.empty());
            assert(m_brushes.size() == m_brushVertices.size());
        }

        MoveVerticesCommand* MoveVerticesCommand::moveVertices(Model::MapDocument& document, const Model::VertexToBrushesMap& brushVertices, const Vec3f& delta) {
            return new MoveVerticesCommand(document, brushVertices.size() == 1 ? wxT("Move Vertex") : wxT("Move Vertices"), brushVertices, delta);
        }
    }
}
