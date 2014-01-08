/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "MoveVerticesCommand.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType MoveVerticesCommand::Type = Command::freeType();

        MoveVerticesCommand::Ptr MoveVerticesCommand::moveVertices(View::MapDocumentWPtr document, const Model::VertexToBrushesMap& vertices, const Vec3& delta) {
            return Ptr(new MoveVerticesCommand(document, vertices, delta));
        }

        const Model::BrushList& MoveVerticesCommand::brushes() const {
            return m_brushes;
        }
        
        const Vec3::List& MoveVerticesCommand::oldVertexPositions() const {
            return m_oldVertexPositions;
        }
        
        const Vec3::List& MoveVerticesCommand::newVertexPositions() const {
            return m_newVertexPositions;
        }

        MoveVerticesCommand::MoveVerticesCommand(View::MapDocumentWPtr document, const Model::VertexToBrushesMap& vertices, const Vec3& delta) :
        Command(Type, makeName(vertices), true, true),
        m_document(document),
        m_delta(delta) {
            assert(!delta.null());
            extractVertices(vertices);
        }

        bool MoveVerticesCommand::doPerformDo() {
            View::MapDocumentSPtr document = lock(m_document);
            if (!canPerformDo(document))
                return false;
            
            const BBox3& worldBounds = document->worldBounds();
            m_snapshot = Model::Snapshot(m_brushes);
            m_newVertexPositions.clear();
            
            BrushVerticesMap::const_iterator mapIt, mapEnd;
            for (mapIt = m_brushVertices.begin(), mapEnd = m_brushVertices.end(); mapIt != mapEnd; ++mapIt) {
                Model::Brush* brush = mapIt->first;
                document->objectWillChangeNotifier(brush);
                const Vec3::List& oldVertexPositions = mapIt->second;
                const Vec3::List newVertexPositions = brush->moveVertices(worldBounds, oldVertexPositions, m_delta);
                VectorUtils::append(m_newVertexPositions, newVertexPositions);
                document->objectDidChangeNotifier(brush);
            }
            
            return true;
        }
        
        bool MoveVerticesCommand::canPerformDo(View::MapDocumentSPtr document) const {
            const BBox3& worldBounds = document->worldBounds();
            BrushVerticesMap::const_iterator it, end;
            for (it = m_brushVertices.begin(), end = m_brushVertices.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Vec3::List& vertices = it->second;
                if (!brush->canMoveVertices(worldBounds, vertices, m_delta))
                    return false;
            }
            return true;
        }

        bool MoveVerticesCommand::doPerformUndo() {
            View::MapDocumentSPtr document = lock(m_document);
            const BBox3& worldBounds = document->worldBounds();

            document->objectWillChangeNotifier(m_brushes.begin(), m_brushes.end());
            m_snapshot.restore(worldBounds);
            document->objectDidChangeNotifier(m_brushes.begin(), m_brushes.end());

            return true;
        }
        
        String MoveVerticesCommand::makeName(const Model::VertexToBrushesMap& vertices) {
            return String("Move ") + (vertices.size() == 1 ? "Vertex" : "Vertices");
        }
        
        void MoveVerticesCommand::extractVertices(const Model::VertexToBrushesMap& vertices) {
            typedef std::pair<BrushVerticesMap::iterator, bool> BrushVerticesMapInsertResult;
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                const Model::BrushList& vertexBrushes = vIt->second;
                Model::BrushList::const_iterator bIt, bEnd;
                for (bIt = vertexBrushes.begin(), bEnd = vertexBrushes.end(); bIt != bEnd; ++bIt) {
                    Model::Brush* brush = *bIt;
                    BrushVerticesMapInsertResult result = m_brushVertices.insert(std::make_pair(brush, Vec3::List()));
                    if (result.second)
                        m_brushes.push_back(brush);
                    result.first->second.push_back(position);
                }
                m_oldVertexPositions.push_back(position);
            }
            
            assert(!m_brushes.empty());
            assert(m_brushes.size() == m_brushVertices.size());
        }
    }
}
