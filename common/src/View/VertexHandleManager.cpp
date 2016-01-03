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

#include "VertexHandleManager.h"

#include "AttrString.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/PickResult.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderService.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shaders.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType VertexHandleManager::VertexHandleHit = Model::Hit::freeHitType();
        const Model::Hit::HitType VertexHandleManager::EdgeHandleHit   = Model::Hit::freeHitType();
        const Model::Hit::HitType VertexHandleManager::FaceHandleHit   = Model::Hit::freeHitType();
        
        VertexHandleManager::VertexHandleManager(View::MapDocumentWPtr document) :
        m_totalVertexCount(0),
        m_selectedVertexCount(0),
        m_totalEdgeCount(0),
        m_selectedEdgeCount(0),
        m_totalFaceCount(0),
        m_selectedFaceCount(0),
        m_guideRenderer(document),
        m_renderStateValid(false) {}
        
        const Model::VertexToBrushesMap& VertexHandleManager::unselectedVertexHandles() const {
            return m_unselectedVertexHandles;
        }
        
        const Model::VertexToBrushesMap& VertexHandleManager::selectedVertexHandles() const {
            return m_selectedVertexHandles;
        }
        
        const Model::VertexToEdgesMap& VertexHandleManager::unselectedEdgeHandles() const {
            return m_unselectedEdgeHandles;
        }
        
        const Model::VertexToEdgesMap& VertexHandleManager::selectedEdgeHandles() const {
            return m_selectedEdgeHandles;
        }
        
        const Model::VertexToFacesMap& VertexHandleManager::unselectedFaceHandles() const {
            return m_unselectedFaceHandles;
        }
        
        const Model::VertexToFacesMap& VertexHandleManager::selectedFaceHandles() const {
            return m_selectedFaceHandles;
        }
        
        Vec3::List VertexHandleManager::vertexHandlePositions() const {
            Vec3::List result;
            result.reserve(m_selectedVertexHandles.size() + m_unselectedVertexHandlePositions.size());
            handlePositions(m_unselectedVertexHandles, result);
            handlePositions(m_selectedVertexHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManager::edgeHandlePositions() const {
            Vec3::List result;
            result.reserve(m_selectedEdgeHandles.size() + m_unselectedEdgeHandlePositions.size());
            handlePositions(m_unselectedEdgeHandles, result);
            handlePositions(m_selectedEdgeHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManager::faceHandlePositions() const {
            Vec3::List result;
            result.reserve(m_selectedFaceHandles.size() + m_unselectedFaceHandlePositions.size());
            handlePositions(m_unselectedFaceHandles, result);
            handlePositions(m_selectedFaceHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManager::unselectedVertexHandlePositions() const {
            Vec3::List result;
            handlePositions(m_unselectedVertexHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManager::unselectedEdgeHandlePositions() const {
            Vec3::List result;
            handlePositions(m_unselectedEdgeHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManager::unselectedFaceHandlePositions() const {
            Vec3::List result;
            handlePositions(m_unselectedFaceHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManager::selectedVertexHandlePositions() const {
            Vec3::List result;
            handlePositions(m_selectedVertexHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManager::selectedEdgeHandlePositions() const {
            Vec3::List result;
            handlePositions(m_selectedEdgeHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManager::selectedFaceHandlePositions() const {
            Vec3::List result;
            handlePositions(m_selectedFaceHandles, result);
            return result;
        }

        bool VertexHandleManager::isHandleSelected(const Vec3& position) const {
            return (isVertexHandleSelected(position) ||
                    isEdgeHandleSelected(position) ||
                    isFaceHandleSelected(position));
        }

        bool VertexHandleManager::isVertexHandleSelected(const Vec3& position) const {
            return m_selectedVertexHandles.find(position) != m_selectedVertexHandles.end();
        }
        
        bool VertexHandleManager::isEdgeHandleSelected(const Vec3& position) const {
            return m_selectedEdgeHandles.find(position) != m_selectedEdgeHandles.end();
        }
        
        bool VertexHandleManager::isFaceHandleSelected(const Vec3& position) const {
            return m_selectedFaceHandles.find(position) != m_selectedFaceHandles.end();
        }
        
        size_t VertexHandleManager::selectedVertexCount() const {
            return m_selectedVertexCount;
        }
        
        size_t VertexHandleManager::totalVertexCount() const {
            return m_totalVertexCount;
        }
        
        size_t VertexHandleManager::selectedEdgeCount() const {
            return m_selectedEdgeCount;
        }
        
        size_t VertexHandleManager::totalEdgeCount() const {
            return m_totalEdgeCount;
        }
        
        size_t VertexHandleManager::selectedFaceCount() const {
            return m_selectedFaceCount;
        }
        
        size_t VertexHandleManager::totalSelectedFaceCount() const {
            return m_totalFaceCount;
        }
        
        Model::BrushSet VertexHandleManager::selectedBrushes() const {
            Model::BrushSet brushSet;
            
            Model::VertexToBrushesMap::const_iterator vMapIt, vMapEnd;
            for (vMapIt = m_selectedVertexHandles.begin(), vMapEnd = m_selectedVertexHandles.end(); vMapIt != vMapEnd; ++vMapIt) {
                const Model::BrushSet& brushes = vMapIt->second;
                brushSet.insert(brushes.begin(), brushes.end());
            }
            
            Model::VertexToEdgesMap::const_iterator eMapIt, eMapEnd;
            Model::BrushEdgeSet::const_iterator eIt, eEnd;
            for (eMapIt = m_selectedEdgeHandles.begin(), eMapEnd = m_selectedEdgeHandles.end(); eMapIt != eMapEnd; ++eMapIt) {
                const Model::BrushEdgeSet& edges = eMapIt->second;
                
                for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd; ++eIt) {
                    Model::BrushEdge* edge = *eIt;
                    Model::Brush* brush = edge->firstFace()->payload()->brush();
                    brushSet.insert(brush);
                }
            }
            
            Model::VertexToFacesMap::const_iterator fMapIt, fMapEnd;
            Model::BrushFaceSet::const_iterator fIt, fEnd;
            for (fMapIt = m_selectedFaceHandles.begin(), fMapEnd = m_selectedFaceHandles.end(); fMapIt != fMapEnd; ++fMapIt) {
                const Model::BrushFaceSet& faces = fMapIt->second;
                
                for (fIt = faces.begin(), fEnd = faces.end(); fIt != fEnd; ++fIt) {
                    Model::BrushFace* face = *fIt;
                    brushSet.insert(face->brush());
                }
            }
            
            return brushSet;
        }

        const Model::BrushSet& VertexHandleManager::brushes(const Vec3& handlePosition) const {
            Model::VertexToBrushesMap::const_iterator mapIt = m_selectedVertexHandles.find(handlePosition);
            if (mapIt != m_selectedVertexHandles.end())
                return mapIt->second;
            mapIt = m_unselectedVertexHandles.find(handlePosition);
            if (mapIt != m_unselectedVertexHandles.end())
                return mapIt->second;
            return Model::EmptyBrushSet;
        }
        
        const Model::BrushEdgeSet& VertexHandleManager::edges(const Vec3& handlePosition) const {
            Model::VertexToEdgesMap::const_iterator mapIt = m_selectedEdgeHandles.find(handlePosition);
            if (mapIt != m_selectedEdgeHandles.end())
                return mapIt->second;
            mapIt = m_unselectedEdgeHandles.find(handlePosition);
            if (mapIt != m_unselectedEdgeHandles.end())
                return mapIt->second;
            return Model::EmptyBrushEdgeSet;
        }
        
        const Model::BrushFaceSet& VertexHandleManager::faces(const Vec3& handlePosition) const {
            Model::VertexToFacesMap::const_iterator mapIt = m_selectedFaceHandles.find(handlePosition);
            if (mapIt != m_selectedFaceHandles.end())
                return mapIt->second;
            mapIt = m_unselectedFaceHandles.find(handlePosition);
            if (mapIt != m_unselectedFaceHandles.end())
                return mapIt->second;
            return Model::EmptyBrushFaceSet;
        }
        
        void VertexHandleManager::addBrush(Model::Brush* brush) {
            assert(brush != NULL);
            
            const Model::Brush::VertexList brushVertices = brush->vertices();
            Model::Brush::VertexList::const_iterator vIt, vEnd;
            for (vIt = brushVertices.begin(), vEnd = brushVertices.end(); vIt != vEnd; ++vIt) {
                const Model::BrushVertex* vertex = *vIt;
                Model::VertexToBrushesMap::iterator mapIt = m_selectedVertexHandles.find(vertex->position());
                if (mapIt != m_selectedVertexHandles.end()) {
                    mapIt->second.insert(brush);
                    m_selectedVertexCount++;
                } else {
                    m_unselectedVertexHandles[vertex->position()].insert(brush);
                }
            }
            m_totalVertexCount += brushVertices.size();
            
            const Model::Brush::EdgeList brushEdges = brush->edges();
            Model::Brush::EdgeList::const_iterator eIt, eEnd;
            for (eIt = brushEdges.begin(), eEnd = brushEdges.end(); eIt != eEnd; ++eIt) {
                Model::BrushEdge* edge = *eIt;
                const Vec3 position = edge->center();
                Model::VertexToEdgesMap::iterator mapIt = m_selectedEdgeHandles.find(position);
                if (mapIt != m_selectedEdgeHandles.end()) {
                    mapIt->second.insert(edge);
                    m_selectedEdgeCount++;
                } else {
                    m_unselectedEdgeHandles[position].insert(edge);
                }
            }
            m_totalEdgeCount+= brushEdges.size();
            
            const Model::BrushFaceList& brushFaces = brush->faces();
            Model::BrushFaceList::const_iterator fIt, fEnd;
            for (fIt = brushFaces.begin(), fEnd = brushFaces.end(); fIt != fEnd; ++fIt) {
                Model::BrushFace* face = *fIt;
                const Vec3 position = face->center();
                Model::VertexToFacesMap::iterator mapIt = m_selectedFaceHandles.find(position);
                if (mapIt != m_selectedFaceHandles.end()) {
                    mapIt->second.insert(face);
                    m_selectedFaceCount++;
                } else {
                    m_unselectedFaceHandles[position].insert(face);
                }
            }
            m_totalFaceCount += brushFaces.size();
            m_renderStateValid = false;
        }
        
        void VertexHandleManager::removeBrush(Model::Brush* brush) {
            const Model::Brush::VertexList brushVertices = brush->vertices();
            Model::Brush::VertexList::const_iterator vIt, vEnd;
            for (vIt = brushVertices.begin(), vEnd = brushVertices.end(); vIt != vEnd; ++vIt) {
                const Model::BrushVertex* vertex = *vIt;
                if (removeHandle(vertex->position(), brush, m_selectedVertexHandles)) {
                    assert(m_selectedVertexCount > 0);
                    m_selectedVertexCount--;
                } else {
                    removeHandle(vertex->position(), brush, m_unselectedVertexHandles);
                }
            }
            assert(m_totalVertexCount >= brushVertices.size());
            m_totalVertexCount -= brushVertices.size();
            
            const Model::Brush::EdgeList brushEdges = brush->edges();
            Model::Brush::EdgeList::const_iterator eIt, eEnd;
            for (eIt = brushEdges.begin(), eEnd = brushEdges.end(); eIt != eEnd; ++eIt) {
                Model::BrushEdge* edge = *eIt;
                const Vec3 position = edge->center();
                if (removeHandle(position, edge, m_selectedEdgeHandles)) {
                    assert(m_selectedEdgeCount > 0);
                    m_selectedEdgeCount--;
                } else {
                    removeHandle(position, edge, m_unselectedEdgeHandles);
                }
            }
            assert(m_totalEdgeCount >= brushEdges.size());
            m_totalEdgeCount -= brushEdges.size();
            
            const Model::BrushFaceList& brushFaces = brush->faces();
            Model::BrushFaceList::const_iterator fIt, fEnd;
            for (fIt = brushFaces.begin(), fEnd = brushFaces.end(); fIt != fEnd; ++fIt) {
                Model::BrushFace* face = *fIt;
                const Vec3 position = face->center();
                if (removeHandle(position, face, m_selectedFaceHandles)) {
                    assert(m_selectedFaceCount > 0);
                    m_selectedFaceCount--;
                } else {
                    removeHandle(position, face, m_unselectedFaceHandles);
                }
            }
            assert(m_totalFaceCount >= brushFaces.size());
            m_totalFaceCount -= brushFaces.size();
            m_renderStateValid = false;
        }
        
        void VertexHandleManager::clear() {
            m_unselectedVertexHandles.clear();
            m_selectedVertexHandles.clear();
            m_totalVertexCount = 0;
            m_selectedVertexCount = 0;
            m_unselectedEdgeHandles.clear();
            m_selectedEdgeHandles.clear();
            m_totalEdgeCount = 0;
            m_selectedEdgeCount = 0;
            m_unselectedFaceHandles.clear();
            m_selectedFaceHandles.clear();
            m_totalFaceCount = 0;
            m_selectedFaceCount = 0;
            m_renderStateValid = false;
        }
        
        void VertexHandleManager::selectVertexHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_unselectedVertexHandles, m_selectedVertexHandles)) > 0) {
                m_selectedVertexCount += count;
                m_renderStateValid = false;
            }
        }
        
        void VertexHandleManager::deselectVertexHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_selectedVertexHandles, m_unselectedVertexHandles)) > 0) {
                assert(m_selectedVertexCount >= count);
                m_selectedVertexCount -= count;
                m_renderStateValid = false;
            }
        }
        
        void VertexHandleManager::toggleVertexHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_unselectedVertexHandles, m_selectedVertexHandles)) > 0) {
                m_selectedVertexCount += count;
                m_renderStateValid = false;
            } else if ((count = moveHandle(position, m_selectedVertexHandles, m_unselectedVertexHandles)) > 0) {
                assert(m_selectedVertexCount >= count);
                m_selectedVertexCount -= count;
                m_renderStateValid = false;
            }
        }

        void VertexHandleManager::selectVertexHandles(const Vec3::List& positions) {
            Vec3::List::const_iterator it, end;
            for (it = positions.begin(), end = positions.end(); it != end; ++it)
                selectVertexHandle(*it);
        }
        
        void VertexHandleManager::deselectAllVertexHandles() {
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            for (vIt = m_selectedVertexHandles.begin(), vEnd = m_selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                const Model::BrushSet& selectedBrushes = vIt->second;
                Model::BrushSet& unselectedBrushes = m_unselectedVertexHandles[position];
                unselectedBrushes.insert(selectedBrushes.begin(), selectedBrushes.end());
            }
            m_selectedVertexHandles.clear();
            m_selectedVertexCount = 0;
            m_renderStateValid = false;
        }
        
        void VertexHandleManager::toggleVertexHandles(const Vec3::List& positions) {
            Vec3::List::const_iterator it, end;
            for (it = positions.begin(), end = positions.end(); it != end; ++it)
                toggleVertexHandle(*it);
        }
        
        void VertexHandleManager::selectEdgeHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_unselectedEdgeHandles, m_selectedEdgeHandles)) > 0) {
                m_selectedEdgeCount += count;
                m_renderStateValid = false;
            }
        }
        
        void VertexHandleManager::deselectEdgeHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_selectedEdgeHandles, m_unselectedEdgeHandles)) > 0) {
                assert(m_selectedEdgeCount >= count);
                m_selectedEdgeCount -= count;
                m_renderStateValid = false;
            }
        }
        
        void VertexHandleManager::toggleEdgeHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_unselectedEdgeHandles, m_selectedEdgeHandles)) > 0) {
                m_selectedEdgeCount += count;
                m_renderStateValid = false;
            } else if ((count = moveHandle(position, m_selectedEdgeHandles, m_unselectedEdgeHandles)) > 0) {
                assert(m_selectedEdgeCount >= count);
                m_selectedEdgeCount -= count;
                m_renderStateValid = false;
            }
        }

        void VertexHandleManager::selectEdgeHandles(const Edge3::List& edges) {
            Edge3::List::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                const Edge3& edge = *it;
                selectEdgeHandle(edge.center());
            }
        }
        
        void VertexHandleManager::deselectAllEdgeHandles() {
            Model::VertexToEdgesMap::const_iterator eIt, eEnd;
            for (eIt = m_selectedEdgeHandles.begin(), eEnd = m_selectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                const Vec3& position = eIt->first;
                const Model::BrushEdgeSet& selectedEdges = eIt->second;
                Model::BrushEdgeSet& unselectedEdges = m_unselectedEdgeHandles[position];
                unselectedEdges.insert(selectedEdges.begin(), selectedEdges.end());
            }
            m_selectedEdgeHandles.clear();
            m_selectedEdgeCount = 0;
            m_renderStateValid = false;
        }
        
        void VertexHandleManager::toggleEdgeHandles(const Vec3::List& positions) {
            Vec3::List::const_iterator it, end;
            for (it = positions.begin(), end = positions.end(); it != end; ++it)
                toggleEdgeHandle(*it);
        }

        void VertexHandleManager::selectFaceHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_unselectedFaceHandles, m_selectedFaceHandles)) > 0) {
                m_selectedFaceCount += count;
                m_renderStateValid = false;
            }
        }
        
        void VertexHandleManager::deselectFaceHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_selectedFaceHandles, m_unselectedFaceHandles)) > 0) {
                assert(m_selectedFaceCount >= count);
                m_selectedFaceCount -= count;
                m_renderStateValid = false;
            }
        }
        
        void VertexHandleManager::toggleFaceHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_unselectedFaceHandles, m_selectedFaceHandles)) > 0) {
                m_selectedFaceCount += count;
                m_renderStateValid = false;
            } else if ((count = moveHandle(position, m_selectedFaceHandles, m_unselectedFaceHandles)) > 0) {
                assert(m_selectedFaceCount >= count);
                m_selectedFaceCount -= count;
                m_renderStateValid = false;
            }
        }

        void VertexHandleManager::selectFaceHandles(const Polygon3::List& faces) {
            Polygon3::List::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                const Polygon3& face = *it;
                selectFaceHandle(face.center());
            }
        }
        
        void VertexHandleManager::deselectAllFaceHandles() {
            Model::VertexToFacesMap::const_iterator fIt, fEnd;
            for (fIt = m_selectedFaceHandles.begin(), fEnd = m_selectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                const Vec3& position = fIt->first;
                const Model::BrushFaceSet& selectedFaces = fIt->second;
                Model::BrushFaceSet& unselectedFaces = m_unselectedFaceHandles[position];
                unselectedFaces.insert(selectedFaces.begin(), selectedFaces.end());
            }
            m_selectedFaceHandles.clear();
            m_selectedFaceCount = 0;
            m_renderStateValid = false;
        }
        
        void VertexHandleManager::toggleFaceHandles(const Vec3::List& positions) {
            Vec3::List::const_iterator it, end;
            for (it = positions.begin(), end = positions.end(); it != end; ++it)
                toggleFaceHandle(*it);
        }

        bool VertexHandleManager::hasSelectedHandles() const {
            return !m_selectedVertexHandles.empty() || !m_selectedEdgeHandles.empty() || !m_selectedFaceHandles.empty();
        }

        void VertexHandleManager::deselectAllHandles() {
            deselectAllVertexHandles();
            deselectAllEdgeHandles();
            deselectAllFaceHandles();
        }
        
        void VertexHandleManager::reselectVertexHandles(const Model::BrushSet& brushes, const Vec3::List& positions, const FloatType maxDistance) {
            Vec3::List::const_iterator oIt, oEnd, nIt, nEnd;
            for (oIt = positions.begin(), oEnd = positions.end(); oIt != oEnd; ++oIt) {
                const Vec3& oldPosition = *oIt;
                const Vec3::List newPositions = findVertexHandlePositions(brushes, oldPosition, maxDistance);
                for (nIt = newPositions.begin(), nEnd = newPositions.end(); nIt != nEnd; ++nIt) {
                    const Vec3& newPosition = *nIt;
                    selectVertexHandle(newPosition);
                }
            }
        }
        
        void VertexHandleManager::reselectEdgeHandles(const Model::BrushSet& brushes, const Vec3::List& positions, const FloatType maxDistance) {
            Vec3::List::const_iterator oIt, oEnd, nIt, nEnd;
            for (oIt = positions.begin(), oEnd = positions.end(); oIt != oEnd; ++oIt) {
                const Vec3& oldPosition = *oIt;
                const Vec3::List newPositions = findEdgeHandlePositions(brushes, oldPosition, maxDistance);
                for (nIt = newPositions.begin(), nEnd = newPositions.end(); nIt != nEnd; ++nIt) {
                    const Vec3& newPosition = *nIt;
                    selectEdgeHandle(newPosition);
                }
            }
        }
        
        void VertexHandleManager::reselectFaceHandles(const Model::BrushSet& brushes, const Vec3::List& positions, const FloatType maxDistance) {
            Vec3::List::const_iterator oIt, oEnd, nIt, nEnd;
            for (oIt = positions.begin(), oEnd = positions.end(); oIt != oEnd; ++oIt) {
                const Vec3& oldPosition = *oIt;
                const Vec3::List newPositions = findFaceHandlePositions(brushes, oldPosition, maxDistance);
                for (nIt = newPositions.begin(), nEnd = newPositions.end(); nIt != nEnd; ++nIt) {
                    const Vec3& newPosition = *nIt;
                    selectFaceHandle(newPosition);
                }
            }
        }

        void VertexHandleManager::pick(const Ray3& ray, const Renderer::Camera& camera, Model::PickResult& pickResult, bool splitMode) const {
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            Model::VertexToEdgesMap::const_iterator eIt, eEnd;
            Model::VertexToFacesMap::const_iterator fIt, fEnd;
            
            if ((m_selectedEdgeHandles.empty() && m_selectedFaceHandles.empty()) || splitMode) {
                for (vIt = m_unselectedVertexHandles.begin(), vEnd = m_unselectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                    const Vec3& position = vIt->first;
                    const Model::Hit hit = pickHandle(ray, camera, position, VertexHandleHit);
                    if (hit.isMatch())
                        pickResult.addHit(hit);
                }
            }
            
            for (vIt = m_selectedVertexHandles.begin(), vEnd = m_selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                const Model::Hit hit = pickHandle(ray, camera, position, VertexHandleHit);
                if (hit.isMatch())
                    pickResult.addHit(hit);
            }
            
            if (m_selectedVertexHandles.empty() && m_selectedFaceHandles.empty() && !splitMode) {
                for (eIt = m_unselectedEdgeHandles.begin(), eEnd = m_unselectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                    const Vec3& position = eIt->first;
                    const Model::Hit hit = pickHandle(ray, camera, position, EdgeHandleHit);
                    if (hit.isMatch())
                        pickResult.addHit(hit);
                }
            }
            
            for (eIt = m_selectedEdgeHandles.begin(), eEnd = m_selectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                const Vec3& position = eIt->first;
                const Model::Hit hit = pickHandle(ray, camera, position, EdgeHandleHit);
                if (hit.isMatch())
                    pickResult.addHit(hit);
            }
            
            if (m_selectedVertexHandles.empty() && m_selectedEdgeHandles.empty() && !splitMode) {
                for (fIt = m_unselectedFaceHandles.begin(), fEnd = m_unselectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                    const Vec3& position = fIt->first;
                    const Model::Hit hit = pickHandle(ray, camera, position, FaceHandleHit);
                    if (hit.isMatch())
                        pickResult.addHit(hit);
                }
            }
            
            for (fIt = m_selectedFaceHandles.begin(), fEnd = m_selectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                const Vec3& position = fIt->first;
                const Model::Hit hit = pickHandle(ray, camera, position, FaceHandleHit);
                if (hit.isMatch())
                    pickResult.addHit(hit);
            }
        }

        void VertexHandleManager::render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const bool splitMode) {
            if (!m_renderStateValid)
                validateRenderState(splitMode);
            
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::HandleColor));
            if (m_selectedEdgeHandles.empty() && m_selectedFaceHandles.empty() && !splitMode)
                renderService.renderPointHandles(VectorUtils::cast<Vec3f>(m_unselectedVertexHandlePositions));
            
            if (m_selectedVertexHandles.empty() && m_selectedFaceHandles.empty() && !splitMode)
                renderService.renderPointHandles(VectorUtils::cast<Vec3f>(m_unselectedEdgeHandlePositions));
            
            if (m_selectedVertexHandles.empty() && m_selectedEdgeHandles.empty() && !splitMode)
                renderService.renderPointHandles(VectorUtils::cast<Vec3f>(m_unselectedFaceHandlePositions));
            
            if ((!m_selectedEdgeHandles.empty() || !m_selectedFaceHandles.empty()) && !splitMode)
                renderService.renderLines(m_edgeVertices);
            
            renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
            renderService.renderPointHandles(VectorUtils::cast<Vec3f>(m_selectedHandlePositions));
        }

        void VertexHandleManager::renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& position) {
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
            renderService.renderPointHandleHighlight(position);
            
            renderService.setForegroundColor(pref(Preferences::SelectedInfoOverlayTextColor));
            renderService.setBackgroundColor(pref(Preferences::SelectedInfoOverlayBackgroundColor));
            renderService.renderStringOnTop(position.asString(), position);
        }

        void VertexHandleManager::renderEdgeHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handlePosition) {
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::HandleColor));
            
            Model::VertexToEdgesMap::const_iterator it = m_unselectedEdgeHandles.find(handlePosition);
            if (it != m_unselectedEdgeHandles.end()) {
                const Model::BrushEdgeSet& edges = it->second;
                assert(!edges.empty());
                
                const Model::BrushEdge* edge = *edges.begin();
                renderService.renderLine(edge->firstVertex()->position(), edge->secondVertex()->position());
            }
        }
        
        void VertexHandleManager::renderFaceHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handlePosition) {
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::HandleColor));
            
            Model::VertexToFacesMap::const_iterator it = m_unselectedFaceHandles.find(handlePosition);
            if (it != m_unselectedFaceHandles.end()) {
                const Model::BrushFaceSet& faces = it->second;
                assert(!faces.empty());
                
                const Model::BrushFace* face = *faces.begin();
                const Model::BrushFace::VertexList& vertices = face->vertices();

                Vec3f::List vertexPositions;
                vertexPositions.reserve(vertices.size());
                Vec3f::toList(vertices.begin(), vertices.end(), Model::BrushGeometry::GetVertexPosition(), vertexPositions);
                
                renderService.renderPolygonOutline(vertexPositions);
            }
        }

        void VertexHandleManager::renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& position) {
            Renderer::RenderService renderService(renderContext, renderBatch);
            m_guideRenderer.setPosition(position);
            m_guideRenderer.setColor(Color(pref(Preferences::HandleColor), 0.5f));
            renderBatch.add(&m_guideRenderer);
        }
        
        Vec3::List VertexHandleManager::findVertexHandlePositions(const Model::BrushSet& brushes, const Vec3& query, const FloatType maxDistance) {
            Vec3::List result;
            Model::BrushSet::const_iterator bIt, bEnd;
            Model::Brush::VertexList::const_iterator vIt, vEnd;
            
            for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                const Model::Brush* brush = *bIt;
                const Model::Brush::VertexList vertices = brush->vertices();
                for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                    const Model::BrushVertex* vertex = *vIt;
                    if (query.squaredDistanceTo(vertex->position()) <= maxDistance * maxDistance)
                        result.push_back(vertex->position());
                }
            }
            
            return result;
        }

        Vec3::List VertexHandleManager::findEdgeHandlePositions(const Model::BrushSet& brushes, const Vec3& query, const FloatType maxDistance) {
            Vec3::List result;
            Model::BrushSet::const_iterator bIt, bEnd;
            Model::Brush::EdgeList::const_iterator eIt, eEnd;
            
            for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                const Model::Brush* brush = *bIt;
                const Model::Brush::EdgeList edges = brush->edges();
                for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd; ++eIt) {
                    const Model::BrushEdge* edge = *eIt;
                    const Vec3 center = edge->center();
                    if (query.squaredDistanceTo(center) <= maxDistance * maxDistance)
                        result.push_back(center);
                }
            }
            
            return result;
        }
        
        Vec3::List VertexHandleManager::findFaceHandlePositions(const Model::BrushSet& brushes, const Vec3& query, const FloatType maxDistance) {
            Vec3::List result;
            Model::BrushSet::const_iterator bIt, bEnd;
            Model::BrushFaceList::const_iterator fIt, fEnd;
            
            for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                const Model::Brush* brush = *bIt;
                const Model::BrushFaceList& faces = brush->faces();
                for (fIt = faces.begin(), fEnd = faces.end(); fIt != fEnd; ++fIt) {
                    const Model::BrushFace* face = *fIt;
                    const Vec3 center = face->center();
                    if (query.squaredDistanceTo(center) <= maxDistance * maxDistance)
                        result.push_back(center);
                }
            }
            
            return result;
        }

        Model::Hit VertexHandleManager::pickHandle(const Ray3& ray, const Renderer::Camera& camera, const Vec3& position, Model::Hit::HitType type) const {
            const FloatType distance = camera.pickPointHandle(ray, position, pref(Preferences::HandleRadius));
            if (!Math::isnan(distance)) {
                const Vec3 hitPoint = ray.pointAtDistance(distance);
                return Model::Hit::hit<Vec3>(type, distance, hitPoint, position);
            }
            
            return Model::Hit::NoHit;
        }
        
        void VertexHandleManager::validateRenderState(const bool splitMode) {
            assert(!m_renderStateValid);
            
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            Model::VertexToEdgesMap::const_iterator eIt, eEnd;
            Model::VertexToFacesMap::const_iterator fIt, fEnd;

            m_unselectedVertexHandlePositions.clear();
            m_unselectedEdgeHandlePositions.clear();
            m_unselectedFaceHandlePositions.clear();
            m_selectedHandlePositions.clear();
            m_edgeVertices.clear();
            
            m_unselectedVertexHandlePositions.reserve(m_unselectedVertexHandles.size());
            m_unselectedEdgeHandlePositions.reserve(m_unselectedEdgeHandles.size());
            m_unselectedFaceHandlePositions.reserve(m_unselectedFaceHandles.size());
            m_selectedHandlePositions.reserve(m_selectedVertexHandles.size() + m_selectedEdgeHandles.size() + m_selectedFaceHandles.size());

            for (vIt = m_unselectedVertexHandles.begin(), vEnd = m_unselectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                m_unselectedVertexHandlePositions.push_back(position);
            }

            for (eIt = m_unselectedEdgeHandles.begin(), eEnd = m_unselectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                const Vec3& position = eIt->first;
                m_unselectedEdgeHandlePositions.push_back(position);
            }

            for (fIt = m_unselectedFaceHandles.begin(), fEnd = m_unselectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                const Vec3& position = fIt->first;
                m_unselectedFaceHandlePositions.push_back(position);
            }

            for (vIt = m_selectedVertexHandles.begin(), vEnd = m_selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                m_selectedHandlePositions.push_back(position);
            }
            
            
            for (eIt = m_selectedEdgeHandles.begin(), eEnd = m_selectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                const Vec3& position = eIt->first;
                m_selectedHandlePositions.push_back(position);
                
                const Model::BrushEdgeSet& edges = eIt->second;
                Model::BrushEdgeSet::const_iterator edgeIt, edgeEnd;
                for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                    const Model::BrushEdge* edge = *edgeIt;
                    m_edgeVertices.push_back(Vec3f(edge->firstVertex()->position()));
                    m_edgeVertices.push_back(Vec3f(edge->secondVertex()->position()));
                }
            }
            
            for (fIt = m_selectedFaceHandles.begin(), fEnd = m_selectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                const Vec3f& position = fIt->first;
                m_selectedHandlePositions.push_back(Vec3f(position));
                
                const Model::BrushFaceSet& faces = fIt->second;
                Model::BrushFaceSet::const_iterator faceIt, faceEnd;
                for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                    const Model::BrushFace* face = *faceIt;
                    const Model::BrushFace::EdgeList edges = face->edges();
                    
                    Model::BrushFace::EdgeList::const_iterator edgeIt, edgeEnd;
                    for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                        const Model::BrushEdge* edge = *edgeIt;
                        m_edgeVertices.push_back(Vec3f(edge->firstVertex()->position()));
                        m_edgeVertices.push_back(Vec3f(edge->secondVertex()->position()));
                    }
                }
            }

            m_renderStateValid = true;
        }
    }
}
