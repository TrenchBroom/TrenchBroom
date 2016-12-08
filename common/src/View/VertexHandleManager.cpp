/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
            return m_selectedVertexHandles.find(position) != std::end(m_selectedVertexHandles);
        }
        
        bool VertexHandleManager::isEdgeHandleSelected(const Vec3& position) const {
            return m_selectedEdgeHandles.find(position) != std::end(m_selectedEdgeHandles);
        }
        
        bool VertexHandleManager::isFaceHandleSelected(const Vec3& position) const {
            return m_selectedFaceHandles.find(position) != std::end(m_selectedFaceHandles);
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
            for (vMapIt = std::begin(m_selectedVertexHandles), vMapEnd = std::end(m_selectedVertexHandles); vMapIt != vMapEnd; ++vMapIt) {
                const Model::BrushSet& brushes = vMapIt->second;
                brushSet.insert(std::begin(brushes), std::end(brushes));
            }
            
            Model::VertexToEdgesMap::const_iterator eMapIt, eMapEnd;
            Model::BrushEdgeSet::const_iterator eIt, eEnd;
            for (eMapIt = std::begin(m_selectedEdgeHandles), eMapEnd = std::end(m_selectedEdgeHandles); eMapIt != eMapEnd; ++eMapIt) {
                const Model::BrushEdgeSet& edges = eMapIt->second;
                
                for (eIt = std::begin(edges), eEnd = std::end(edges); eIt != eEnd; ++eIt) {
                    Model::BrushEdge* edge = *eIt;
                    Model::Brush* brush = edge->firstFace()->payload()->brush();
                    brushSet.insert(brush);
                }
            }
            
            Model::VertexToFacesMap::const_iterator fMapIt, fMapEnd;
            Model::BrushFaceSet::const_iterator fIt, fEnd;
            for (fMapIt = std::begin(m_selectedFaceHandles), fMapEnd = std::end(m_selectedFaceHandles); fMapIt != fMapEnd; ++fMapIt) {
                const Model::BrushFaceSet& faces = fMapIt->second;
                
                for (fIt = std::begin(faces), fEnd = std::end(faces); fIt != fEnd; ++fIt) {
                    Model::BrushFace* face = *fIt;
                    brushSet.insert(face->brush());
                }
            }
            
            return brushSet;
        }

        const Model::BrushSet& VertexHandleManager::brushes(const Vec3& handlePosition) const {
            Model::VertexToBrushesMap::const_iterator mapIt = m_selectedVertexHandles.find(handlePosition);
            if (mapIt != std::end(m_selectedVertexHandles))
                return mapIt->second;
            mapIt = m_unselectedVertexHandles.find(handlePosition);
            if (mapIt != std::end(m_unselectedVertexHandles))
                return mapIt->second;
            return Model::EmptyBrushSet;
        }
        
        const Model::BrushEdgeSet& VertexHandleManager::edges(const Vec3& handlePosition) const {
            Model::VertexToEdgesMap::const_iterator mapIt = m_selectedEdgeHandles.find(handlePosition);
            if (mapIt != std::end(m_selectedEdgeHandles))
                return mapIt->second;
            mapIt = m_unselectedEdgeHandles.find(handlePosition);
            if (mapIt != std::end(m_unselectedEdgeHandles))
                return mapIt->second;
            return Model::EmptyBrushEdgeSet;
        }
        
        const Model::BrushFaceSet& VertexHandleManager::faces(const Vec3& handlePosition) const {
            Model::VertexToFacesMap::const_iterator mapIt = m_selectedFaceHandles.find(handlePosition);
            if (mapIt != std::end(m_selectedFaceHandles))
                return mapIt->second;
            mapIt = m_unselectedFaceHandles.find(handlePosition);
            if (mapIt != std::end(m_unselectedFaceHandles))
                return mapIt->second;
            return Model::EmptyBrushFaceSet;
        }
        
        void VertexHandleManager::addBrush(Model::Brush* brush) {
            ensure(brush != NULL, "brush is null");
            
            const Model::Brush::VertexList brushVertices = brush->vertices();
            Model::Brush::VertexList::const_iterator vIt, vEnd;
            for (vIt = std::begin(brushVertices), vEnd = std::end(brushVertices); vIt != vEnd; ++vIt) {
                const Model::BrushVertex* vertex = *vIt;
                Model::VertexToBrushesMap::iterator mapIt = m_selectedVertexHandles.find(vertex->position());
                if (mapIt != std::end(m_selectedVertexHandles)) {
                    mapIt->second.insert(brush);
                    m_selectedVertexCount++;
                } else {
                    m_unselectedVertexHandles[vertex->position()].insert(brush);
                }
            }
            m_totalVertexCount += brushVertices.size();
            
            const Model::Brush::EdgeList brushEdges = brush->edges();
            Model::Brush::EdgeList::const_iterator eIt, eEnd;
            for (eIt = std::begin(brushEdges), eEnd = std::end(brushEdges); eIt != eEnd; ++eIt) {
                Model::BrushEdge* edge = *eIt;
                const Vec3 position = edge->center();
                Model::VertexToEdgesMap::iterator mapIt = m_selectedEdgeHandles.find(position);
                if (mapIt != std::end(m_selectedEdgeHandles)) {
                    mapIt->second.insert(edge);
                    m_selectedEdgeCount++;
                } else {
                    m_unselectedEdgeHandles[position].insert(edge);
                }
            }
            m_totalEdgeCount+= brushEdges.size();
            
            const Model::BrushFaceList& brushFaces = brush->faces();
            Model::BrushFaceList::const_iterator fIt, fEnd;
            for (fIt = std::begin(brushFaces), fEnd = std::end(brushFaces); fIt != fEnd; ++fIt) {
                Model::BrushFace* face = *fIt;
                const Vec3 position = face->center();
                Model::VertexToFacesMap::iterator mapIt = m_selectedFaceHandles.find(position);
                if (mapIt != std::end(m_selectedFaceHandles)) {
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
            for (vIt = std::begin(brushVertices), vEnd = std::end(brushVertices); vIt != vEnd; ++vIt) {
                const Model::BrushVertex* vertex = *vIt;
                if (removeHandle(vertex->position(), brush, m_selectedVertexHandles)) {
                    ensure(m_selectedVertexCount > 0, "no selected vertices");
                    m_selectedVertexCount--;
                } else {
                    removeHandle(vertex->position(), brush, m_unselectedVertexHandles);
                }
            }
            ensure(m_totalVertexCount >= brushVertices.size(), "brush vertices exceed total vertices");
            m_totalVertexCount -= brushVertices.size();
            
            const Model::Brush::EdgeList brushEdges = brush->edges();
            Model::Brush::EdgeList::const_iterator eIt, eEnd;
            for (eIt = std::begin(brushEdges), eEnd = std::end(brushEdges); eIt != eEnd; ++eIt) {
                Model::BrushEdge* edge = *eIt;
                const Vec3 position = edge->center();
                if (removeHandle(position, edge, m_selectedEdgeHandles)) {
                    ensure(m_selectedEdgeCount > 0, "no selected edges");
                    m_selectedEdgeCount--;
                } else {
                    removeHandle(position, edge, m_unselectedEdgeHandles);
                }
            }
            ensure(m_totalEdgeCount >= brushEdges.size(), "brush edges exceed total edges");
            m_totalEdgeCount -= brushEdges.size();
            
            const Model::BrushFaceList& brushFaces = brush->faces();
            Model::BrushFaceList::const_iterator fIt, fEnd;
            for (fIt = std::begin(brushFaces), fEnd = std::end(brushFaces); fIt != fEnd; ++fIt) {
                Model::BrushFace* face = *fIt;
                const Vec3 position = face->center();
                if (removeHandle(position, face, m_selectedFaceHandles)) {
                    ensure(m_selectedFaceCount > 0, "no selected faces");
                    m_selectedFaceCount--;
                } else {
                    removeHandle(position, face, m_unselectedFaceHandles);
                }
            }
            ensure(m_totalFaceCount >= brushFaces.size(), "brush faces exceed total faces");
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
                ensure(m_selectedVertexCount >= count, "deselected vertices exceeds selected vertices");
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
                ensure(m_selectedVertexCount >= count, "deselected vertices exceeds selected vertices");
                m_selectedVertexCount -= count;
                m_renderStateValid = false;
            }
        }

        void VertexHandleManager::selectVertexHandles(const Vec3::List& positions) {
            Vec3::List::const_iterator it, end;
            for (it = std::begin(positions), end = std::end(positions); it != end; ++it)
                selectVertexHandle(*it);
        }
        
        void VertexHandleManager::deselectAllVertexHandles() {
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            for (vIt = std::begin(m_selectedVertexHandles), vEnd = std::end(m_selectedVertexHandles); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                const Model::BrushSet& selectedBrushes = vIt->second;
                Model::BrushSet& unselectedBrushes = m_unselectedVertexHandles[position];
                unselectedBrushes.insert(std::begin(selectedBrushes), std::end(selectedBrushes));
            }
            m_selectedVertexHandles.clear();
            m_selectedVertexCount = 0;
            m_renderStateValid = false;
        }
        
        void VertexHandleManager::toggleVertexHandles(const Vec3::List& positions) {
            Vec3::List::const_iterator it, end;
            for (it = std::begin(positions), end = std::end(positions); it != end; ++it)
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
                ensure(m_selectedEdgeCount >= count, "deselected edges exceeds selected edges");
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
                ensure(m_selectedEdgeCount >= count, "deselected edges exceeds selected edges");
                m_selectedEdgeCount -= count;
                m_renderStateValid = false;
            }
        }

        void VertexHandleManager::selectEdgeHandles(const Edge3::List& edges) {
            Edge3::List::const_iterator it, end;
            for (it = std::begin(edges), end = std::end(edges); it != end; ++it) {
                const Edge3& edge = *it;
                selectEdgeHandle(edge.center());
            }
        }
        
        void VertexHandleManager::deselectAllEdgeHandles() {
            Model::VertexToEdgesMap::const_iterator eIt, eEnd;
            for (eIt = std::begin(m_selectedEdgeHandles), eEnd = std::end(m_selectedEdgeHandles); eIt != eEnd; ++eIt) {
                const Vec3& position = eIt->first;
                const Model::BrushEdgeSet& selectedEdges = eIt->second;
                Model::BrushEdgeSet& unselectedEdges = m_unselectedEdgeHandles[position];
                unselectedEdges.insert(std::begin(selectedEdges), std::end(selectedEdges));
            }
            m_selectedEdgeHandles.clear();
            m_selectedEdgeCount = 0;
            m_renderStateValid = false;
        }
        
        void VertexHandleManager::toggleEdgeHandles(const Vec3::List& positions) {
            Vec3::List::const_iterator it, end;
            for (it = std::begin(positions), end = std::end(positions); it != end; ++it)
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
                ensure(m_selectedFaceCount >= count, "deselected faces exceeds selected faces");
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
                ensure(m_selectedFaceCount >= count, "deselected faces exceeds selected faces");
                m_selectedFaceCount -= count;
                m_renderStateValid = false;
            }
        }

        void VertexHandleManager::selectFaceHandles(const Polygon3::List& faces) {
            Polygon3::List::const_iterator it, end;
            for (it = std::begin(faces), end = std::end(faces); it != end; ++it) {
                const Polygon3& face = *it;
                selectFaceHandle(face.center());
            }
        }
        
        void VertexHandleManager::deselectAllFaceHandles() {
            Model::VertexToFacesMap::const_iterator fIt, fEnd;
            for (fIt = std::begin(m_selectedFaceHandles), fEnd = std::end(m_selectedFaceHandles); fIt != fEnd; ++fIt) {
                const Vec3& position = fIt->first;
                const Model::BrushFaceSet& selectedFaces = fIt->second;
                Model::BrushFaceSet& unselectedFaces = m_unselectedFaceHandles[position];
                unselectedFaces.insert(std::begin(selectedFaces), std::end(selectedFaces));
            }
            m_selectedFaceHandles.clear();
            m_selectedFaceCount = 0;
            m_renderStateValid = false;
        }
        
        void VertexHandleManager::toggleFaceHandles(const Vec3::List& positions) {
            Vec3::List::const_iterator it, end;
            for (it = std::begin(positions), end = std::end(positions); it != end; ++it)
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
            for (oIt = std::begin(positions), oEnd = std::end(positions); oIt != oEnd; ++oIt) {
                const Vec3& oldPosition = *oIt;
                const Vec3::List newPositions = findVertexHandlePositions(brushes, oldPosition, maxDistance);
                for (nIt = std::begin(newPositions), nEnd = std::end(newPositions); nIt != nEnd; ++nIt) {
                    const Vec3& newPosition = *nIt;
                    selectVertexHandle(newPosition);
                }
            }
        }
        
        void VertexHandleManager::reselectEdgeHandles(const Model::BrushSet& brushes, const Vec3::List& positions, const FloatType maxDistance) {
            Vec3::List::const_iterator oIt, oEnd, nIt, nEnd;
            for (oIt = std::begin(positions), oEnd = std::end(positions); oIt != oEnd; ++oIt) {
                const Vec3& oldPosition = *oIt;
                const Vec3::List newPositions = findEdgeHandlePositions(brushes, oldPosition, maxDistance);
                for (nIt = std::begin(newPositions), nEnd = std::end(newPositions); nIt != nEnd; ++nIt) {
                    const Vec3& newPosition = *nIt;
                    selectEdgeHandle(newPosition);
                }
            }
        }
        
        void VertexHandleManager::reselectFaceHandles(const Model::BrushSet& brushes, const Vec3::List& positions, const FloatType maxDistance) {
            Vec3::List::const_iterator oIt, oEnd, nIt, nEnd;
            for (oIt = std::begin(positions), oEnd = std::end(positions); oIt != oEnd; ++oIt) {
                const Vec3& oldPosition = *oIt;
                const Vec3::List newPositions = findFaceHandlePositions(brushes, oldPosition, maxDistance);
                for (nIt = std::begin(newPositions), nEnd = std::end(newPositions); nIt != nEnd; ++nIt) {
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
                for (vIt = std::begin(m_unselectedVertexHandles), vEnd = std::end(m_unselectedVertexHandles); vIt != vEnd; ++vIt) {
                    const Vec3& position = vIt->first;
                    const Model::Hit hit = pickHandle(ray, camera, position, VertexHandleHit);
                    if (hit.isMatch())
                        pickResult.addHit(hit);
                }
            }
            
            for (vIt = std::begin(m_selectedVertexHandles), vEnd = std::end(m_selectedVertexHandles); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                const Model::Hit hit = pickHandle(ray, camera, position, VertexHandleHit);
                if (hit.isMatch())
                    pickResult.addHit(hit);
            }
            
            if (m_selectedVertexHandles.empty() && m_selectedFaceHandles.empty() && !splitMode) {
                for (eIt = std::begin(m_unselectedEdgeHandles), eEnd = std::end(m_unselectedEdgeHandles); eIt != eEnd; ++eIt) {
                    const Vec3& position = eIt->first;
                    const Model::Hit hit = pickHandle(ray, camera, position, EdgeHandleHit);
                    if (hit.isMatch())
                        pickResult.addHit(hit);
                }
            }
            
            for (eIt = std::begin(m_selectedEdgeHandles), eEnd = std::end(m_selectedEdgeHandles); eIt != eEnd; ++eIt) {
                const Vec3& position = eIt->first;
                const Model::Hit hit = pickHandle(ray, camera, position, EdgeHandleHit);
                if (hit.isMatch())
                    pickResult.addHit(hit);
            }
            
            if (m_selectedVertexHandles.empty() && m_selectedEdgeHandles.empty() && !splitMode) {
                for (fIt = std::begin(m_unselectedFaceHandles), fEnd = std::end(m_unselectedFaceHandles); fIt != fEnd; ++fIt) {
                    const Vec3& position = fIt->first;
                    const Model::Hit hit = pickHandle(ray, camera, position, FaceHandleHit);
                    if (hit.isMatch())
                        pickResult.addHit(hit);
                }
            }
            
            for (fIt = std::begin(m_selectedFaceHandles), fEnd = std::end(m_selectedFaceHandles); fIt != fEnd; ++fIt) {
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
            renderService.renderString(position.asString(), position);
        }

        void VertexHandleManager::renderEdgeHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handlePosition) {
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::HandleColor));
            
            Model::VertexToEdgesMap::const_iterator it = m_unselectedEdgeHandles.find(handlePosition);
            if (it != std::end(m_unselectedEdgeHandles)) {
                const Model::BrushEdgeSet& edges = it->second;
                ensure(!edges.empty(), "no unselected edge handles");
                
                const Model::BrushEdge* edge = *std::begin(edges);
                renderService.renderLine(edge->firstVertex()->position(), edge->secondVertex()->position());
            }
        }
        
        void VertexHandleManager::renderFaceHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handlePosition) {
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::HandleColor));
            
            Model::VertexToFacesMap::const_iterator it = m_unselectedFaceHandles.find(handlePosition);
            if (it != std::end(m_unselectedFaceHandles)) {
                const Model::BrushFaceSet& faces = it->second;
                ensure(!faces.empty(), "no unselected face handles");
                
                const Model::BrushFace* face = *std::begin(faces);
                const Model::BrushFace::VertexList& vertices = face->vertices();

                Vec3f::List vertexPositions;
                vertexPositions.reserve(vertices.size());
                Vec3f::toList(std::begin(vertices), std::end(vertices), Model::BrushGeometry::GetVertexPosition(), vertexPositions);
                
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
            
            for (bIt = std::begin(brushes), bEnd = std::end(brushes); bIt != bEnd; ++bIt) {
                const Model::Brush* brush = *bIt;
                const Model::Brush::VertexList vertices = brush->vertices();
                for (vIt = std::begin(vertices), vEnd = std::end(vertices); vIt != vEnd; ++vIt) {
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
            
            for (bIt = std::begin(brushes), bEnd = std::end(brushes); bIt != bEnd; ++bIt) {
                const Model::Brush* brush = *bIt;
                const Model::Brush::EdgeList edges = brush->edges();
                for (eIt = std::begin(edges), eEnd = std::end(edges); eIt != eEnd; ++eIt) {
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
            
            for (bIt = std::begin(brushes), bEnd = std::end(brushes); bIt != bEnd; ++bIt) {
                const Model::Brush* brush = *bIt;
                const Model::BrushFaceList& faces = brush->faces();
                for (fIt = std::begin(faces), fEnd = std::end(faces); fIt != fEnd; ++fIt) {
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
            ensure(!m_renderStateValid, "render state already valid");
            
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

            for (vIt = std::begin(m_unselectedVertexHandles), vEnd = std::end(m_unselectedVertexHandles); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                m_unselectedVertexHandlePositions.push_back(position);
            }

            for (eIt = std::begin(m_unselectedEdgeHandles), eEnd = std::end(m_unselectedEdgeHandles); eIt != eEnd; ++eIt) {
                const Vec3& position = eIt->first;
                m_unselectedEdgeHandlePositions.push_back(position);
            }

            for (fIt = std::begin(m_unselectedFaceHandles), fEnd = std::end(m_unselectedFaceHandles); fIt != fEnd; ++fIt) {
                const Vec3& position = fIt->first;
                m_unselectedFaceHandlePositions.push_back(position);
            }

            for (vIt = std::begin(m_selectedVertexHandles), vEnd = std::end(m_selectedVertexHandles); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                m_selectedHandlePositions.push_back(position);
            }
            
            
            for (eIt = std::begin(m_selectedEdgeHandles), eEnd = std::end(m_selectedEdgeHandles); eIt != eEnd; ++eIt) {
                const Vec3& position = eIt->first;
                m_selectedHandlePositions.push_back(position);
                
                const Model::BrushEdgeSet& edges = eIt->second;
                Model::BrushEdgeSet::const_iterator edgeIt, edgeEnd;
                for (edgeIt = std::begin(edges), edgeEnd = std::end(edges); edgeIt != edgeEnd; ++edgeIt) {
                    const Model::BrushEdge* edge = *edgeIt;
                    m_edgeVertices.push_back(Vec3f(edge->firstVertex()->position()));
                    m_edgeVertices.push_back(Vec3f(edge->secondVertex()->position()));
                }
            }
            
            for (fIt = std::begin(m_selectedFaceHandles), fEnd = std::end(m_selectedFaceHandles); fIt != fEnd; ++fIt) {
                const Vec3f& position = fIt->first;
                m_selectedHandlePositions.push_back(Vec3f(position));
                
                const Model::BrushFaceSet& faces = fIt->second;
                Model::BrushFaceSet::const_iterator faceIt, faceEnd;
                for (faceIt = std::begin(faces), faceEnd = std::end(faces); faceIt != faceEnd; ++faceIt) {
                    const Model::BrushFace* face = *faceIt;
                    const Model::BrushFace::EdgeList edges = face->edges();
                    
                    Model::BrushFace::EdgeList::const_iterator edgeIt, edgeEnd;
                    for (edgeIt = std::begin(edges), edgeEnd = std::end(edges); edgeIt != edgeEnd; ++edgeIt) {
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
