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
            
        }
        
        bool MoveVerticesCommand::performUndo() {
        }
        
        MoveVerticesCommand::MoveVerticesCommand(Model::MapDocument& document, const wxString& name, const BrushVerticesMap& brushVertices, const Vec3f& delta) :
        SnapshotCommand(Command::MoveVertices, document, name),
        m_brushVertices(brushVertices),
        m_delta(delta) {
        }

        MoveVerticesCommand* MoveVerticesCommand::moveVertices(Model::MapDocument& document, const BrushVerticesMap& brushVertices, const Vec3f& delta) {
            assert(!brushVertices.empty());
            
            bool oneVertex = true;
            Vec3f first = brushVertices.begin()->second.front();

            BrushVerticesMap::const_iterator mapIt, mapEnd;
            for (mapIt = brushVertices.begin(), mapEnd = brushVertices.end(); mapIt != mapEnd && oneVertex; ++mapIt) {
                const Vec3f::List& vertices = mapIt->second;
                Vec3f::List::const_iterator vIt, vEnd;
                for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                    const Vec3f& vertex = *vIt;
                    if (!vertex.equals(first))
                        oneVertex = false;
                }
            }
            
            return new MoveVerticesCommand(document, oneVertex ? wxT("Move Vertex") : wxT("Move Vertices"), brushVertices, delta);
        }
    }
}