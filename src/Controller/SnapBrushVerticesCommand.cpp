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

#include "SnapBrushVerticesCommand.h"

#include "Model/Brush.h"
#include "Model/BrushVertex.h"
#include "View/MapDocument.h"
#include "View/VertexHandleManager.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType SnapBrushVerticesCommand::Type = Command::freeType();

        SnapBrushVerticesCommand::Ptr SnapBrushVerticesCommand::snapVertices(View::MapDocumentWPtr document, const Model::VertexToBrushesMap& vertices, const size_t snapTo) {
            return Ptr(new SnapBrushVerticesCommand(document, vertices, snapTo));
        }

        SnapBrushVerticesCommand::Ptr SnapBrushVerticesCommand::snapAllVertices(View::MapDocumentWPtr document, const Model::BrushList& brushes, const size_t snapTo) {
            return Ptr(new SnapBrushVerticesCommand(document, brushes, snapTo));
        }

        SnapBrushVerticesCommand::SnapBrushVerticesCommand(View::MapDocumentWPtr document, const Model::VertexToBrushesMap& vertices, const size_t snapTo) :
        BrushVertexHandleCommand(Type, vertices.size() == 1 ? "Snap Brush Vertex" : "Snap Brush Vertices", true, true),
        m_document(document),
        m_snapTo(snapTo) {
            extractVertices(vertices);
        }
        
        SnapBrushVerticesCommand::SnapBrushVerticesCommand(View::MapDocumentWPtr document, const Model::BrushList& brushes, const size_t snapTo) :
        BrushVertexHandleCommand(Type, "Snap Brush Vertices", true, true),
        m_document(document),
        m_brushes(brushes),
        m_snapTo(snapTo) {
            extractAllVertices();
        }
        
        
        bool SnapBrushVerticesCommand::doPerformDo() {
            View::MapDocumentSPtr document = lock(m_document);
            const BBox3& worldBounds = document->worldBounds();
            m_snapshot = Model::Snapshot(m_brushes);
            m_newVertexPositions.clear();
            
            BrushVerticesMap::const_iterator mapIt, mapEnd;
            for (mapIt = m_brushVertices.begin(), mapEnd = m_brushVertices.end(); mapIt != mapEnd; ++mapIt) {
                Model::Brush* brush = mapIt->first;
                document->objectWillChangeNotifier(brush);
                const Vec3::List& oldVertexPositions = mapIt->second;
                const Vec3::List newVertexPositions = brush->snapVertices(worldBounds, oldVertexPositions, m_snapTo);
                VectorUtils::append(m_newVertexPositions, newVertexPositions);
                document->objectDidChangeNotifier(brush);
            }
            
            return true;
        }
        
        bool SnapBrushVerticesCommand::doPerformUndo() {
            View::MapDocumentSPtr document = lock(m_document);
            const BBox3& worldBounds = document->worldBounds();
            
            document->objectWillChangeNotifier(m_brushes.begin(), m_brushes.end());
            m_snapshot.restore(worldBounds);
            document->objectDidChangeNotifier(m_brushes.begin(), m_brushes.end());
            
            return true;
        }
        
        void SnapBrushVerticesCommand::doRemoveBrushes(View::VertexHandleManager& manager) {
            manager.removeBrushes(m_brushes);
        }
        
        void SnapBrushVerticesCommand::doAddBrushes(View::VertexHandleManager& manager) {
            manager.addBrushes(m_brushes);
        }
        
        void SnapBrushVerticesCommand::doSelectNewHandlePositions(View::VertexHandleManager& manager) {
            manager.selectVertexHandles(m_newVertexPositions);
        }
        
        void SnapBrushVerticesCommand::doSelectOldHandlePositions(View::VertexHandleManager& manager) {
            manager.selectVertexHandles(m_oldVertexPositions);
        }
        
        void SnapBrushVerticesCommand::extractVertices(const Model::VertexToBrushesMap& vertices) {
            typedef std::pair<BrushVerticesMap::iterator, bool> BrushVerticesMapInsertResult;
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            Model::BrushList::const_iterator bIt, bEnd;
            
            for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                const Model::BrushList& vertexBrushes = vIt->second;
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
        
        void SnapBrushVerticesCommand::extractAllVertices() {
            Model::BrushList::const_iterator bIt, bEnd;
            Model::BrushVertexList::const_iterator vIt, vEnd;
            
            for (bIt = m_brushes.begin(), bEnd = m_brushes.end(); bIt != bEnd; ++bIt) {
                Model::Brush* brush = *bIt;
                const Model::BrushVertexList& vertices = brush->vertices();
                for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                    const Model::BrushVertex* vertex = *vIt;
                    m_brushVertices[brush].push_back(vertex->position);
                }
            }

            assert(m_brushes.size() == m_brushVertices.size());
        }
    }
}
