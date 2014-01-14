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

#include "RebuildBrushGeometryCommand.h"

#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "View/MapDocument.h"
#include "View/VertexHandleManager.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType RebuildBrushGeometryCommand::Type = Command::freeType();
        const FloatType RebuildBrushGeometryCommand::MaxDistance = 0.01;
        
        RebuildBrushGeometryCommand::Ptr RebuildBrushGeometryCommand::rebuildBrushGeometry(View::MapDocumentWPtr document, const Model::BrushList& brushes) {
            return Ptr(new RebuildBrushGeometryCommand(document, brushes));
        }

        RebuildBrushGeometryCommand::RebuildBrushGeometryCommand(View::MapDocumentWPtr document, const Model::BrushList& brushes) :
        BrushVertexHandleCommand(Type, "Rebuild Brush Geometry", false, true),
        m_document(document),
        m_brushes(brushes) {}
        
        bool RebuildBrushGeometryCommand::doPerformDo() {
            View::MapDocumentSPtr document = lock(m_document);
            const BBox3& worldBounds = document->worldBounds();
            
            document->objectWillChangeNotifier(m_brushes.begin(), m_brushes.end());

            Model::BrushList::iterator it, end;
            for (it = m_brushes.begin(), end = m_brushes.end(); it != end; ++it) {
                Model::Brush* brush = *it;
                brush->rebuildGeometry(worldBounds);
            }
            
            document->objectDidChangeNotifier(m_brushes.begin(), m_brushes.end());
            return true;
        }
        
        void RebuildBrushGeometryCommand::doRemoveBrushes(View::VertexHandleManager& manager) {
            m_selectedVertexHandles = getSelectedHandles(manager.selectedVertexHandles());
            m_selectedEdgeHandles = getSelectedHandles(manager.selectedEdgeHandles());
            m_selectedFaceHandles = getSelectedHandles(manager.selectedFaceHandles());
            
            assert(( m_selectedVertexHandles.empty() && m_selectedEdgeHandles.empty() && m_selectedFaceHandles.empty()) ||
                   (!m_selectedVertexHandles.empty() ^ !m_selectedEdgeHandles.empty() ^ !m_selectedFaceHandles.empty()));
            
            manager.removeBrushes(m_brushes);
        }

        void RebuildBrushGeometryCommand::doAddBrushes(View::VertexHandleManager& manager) {
            manager.addBrushes(m_brushes);
        }
        
        void RebuildBrushGeometryCommand::doSelectNewHandlePositions(View::VertexHandleManager& manager) {
            assert(( m_selectedVertexHandles.empty() && m_selectedEdgeHandles.empty() && m_selectedFaceHandles.empty()) ||
                   (!m_selectedVertexHandles.empty() ^ !m_selectedEdgeHandles.empty() ^ !m_selectedFaceHandles.empty()));
            
            if (!m_selectedVertexHandles.empty())
                selectNewVertexHandlePositions(manager);
            else if (!m_selectedEdgeHandles.empty())
                selectNewEdgeHandlePositions(manager);
            else if (!m_selectedFaceHandles.empty())
                selectNewFaceHandlePositions(manager);
        }
        
        void RebuildBrushGeometryCommand::selectNewVertexHandlePositions(View::VertexHandleManager& manager) const {
            Vec3::List::const_iterator oIt, oEnd, nIt, nEnd;
            for (oIt = m_selectedVertexHandles.begin(), oEnd = m_selectedVertexHandles.end(); oIt != oEnd; ++oIt) {
                const Vec3& oldPosition = *oIt;
                const Vec3::List newPositions = findVertexHandlePositions(oldPosition);
                for (nIt = newPositions.begin(), nEnd = newPositions.end(); nIt != nEnd; ++nIt) {
                    const Vec3& newPosition = *nIt;
                    manager.selectVertexHandle(newPosition);
                }
            }
        }

        Vec3::List RebuildBrushGeometryCommand::findVertexHandlePositions(const Vec3& original) const {
            Vec3::List result;
            Model::BrushList::const_iterator bIt, bEnd;
            Model::BrushVertexList::const_iterator vIt, vEnd;
            
            for (bIt = m_brushes.begin(), bEnd = m_brushes.end(); bIt != bEnd; ++bIt) {
                const Model::Brush* brush = *bIt;
                const Model::BrushVertexList& vertices = brush->vertices();
                for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                    const Model::BrushVertex* vertex = *vIt;
                    if (original.squaredDistanceTo(vertex->position) <= MaxDistance * MaxDistance)
                        result.push_back(vertex->position);
                }
            }
            
            return result;
        }
        
        void RebuildBrushGeometryCommand::selectNewEdgeHandlePositions(View::VertexHandleManager& manager) const {
            Vec3::List::const_iterator oIt, oEnd, nIt, nEnd;
            for (oIt = m_selectedEdgeHandles.begin(), oEnd = m_selectedEdgeHandles.end(); oIt != oEnd; ++oIt) {
                const Vec3& oldPosition = *oIt;
                const Vec3::List newPositions = findEdgeHandlePositions(oldPosition);
                for (nIt = newPositions.begin(), nEnd = newPositions.end(); nIt != nEnd; ++nIt) {
                    const Vec3& newPosition = *nIt;
                    manager.selectEdgeHandle(newPosition);
                }
            }
        }

        Vec3::List RebuildBrushGeometryCommand::findEdgeHandlePositions(const Vec3& original) const {
            Vec3::List result;
            Model::BrushList::const_iterator bIt, bEnd;
            Model::BrushEdgeList::const_iterator eIt, eEnd;
            
            for (bIt = m_brushes.begin(), bEnd = m_brushes.end(); bIt != bEnd; ++bIt) {
                const Model::Brush* brush = *bIt;
                const Model::BrushEdgeList& edges = brush->edges();
                for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd; ++eIt) {
                    const Model::BrushEdge* edge = *eIt;
                    const Vec3 center = edge->center();
                    if (original.squaredDistanceTo(center) <= MaxDistance * MaxDistance)
                        result.push_back(center);
                }
            }
            
            return result;
        }
        
        void RebuildBrushGeometryCommand::selectNewFaceHandlePositions(View::VertexHandleManager& manager) const {
            Vec3::List::const_iterator oIt, oEnd, nIt, nEnd;
            for (oIt = m_selectedFaceHandles.begin(), oEnd = m_selectedFaceHandles.end(); oIt != oEnd; ++oIt) {
                const Vec3& oldPosition = *oIt;
                const Vec3::List newPositions = findFaceHandlePositions(oldPosition);
                for (nIt = newPositions.begin(), nEnd = newPositions.end(); nIt != nEnd; ++nIt) {
                    const Vec3& newPosition = *nIt;
                    manager.selectFaceHandle(newPosition);
                }
            }
        }

        Vec3::List RebuildBrushGeometryCommand::findFaceHandlePositions(const Vec3& original) const {
            Vec3::List result;
            Model::BrushList::const_iterator bIt, bEnd;
            Model::BrushFaceList::const_iterator fIt, fEnd;
            
            for (bIt = m_brushes.begin(), bEnd = m_brushes.end(); bIt != bEnd; ++bIt) {
                const Model::Brush* brush = *bIt;
                const Model::BrushFaceList& faces = brush->faces();
                for (fIt = faces.begin(), fEnd = faces.end(); fIt != fEnd; ++fIt) {
                    const Model::BrushFace* face = *fIt;
                    const Vec3 center = face->center();
                    if (original.squaredDistanceTo(center) <= MaxDistance * MaxDistance)
                        result.push_back(center);
                }
            }
            
            return result;
        }

        void RebuildBrushGeometryCommand::doSelectOldHandlePositions(View::VertexHandleManager& manager) {
        }
    }
}
