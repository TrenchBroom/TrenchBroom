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

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushEdge.h"
#include "Model/BrushVertex.h"
#include "Model/Picker.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Hit::HitType VertexHandleManager::VertexHandleHit = Hit::freeHitType();
        const Hit::HitType VertexHandleManager::EdgeHandleHit = Hit::freeHitType();
        const Hit::HitType VertexHandleManager::FaceHandleHit = Hit::freeHitType();
        
        VertexHandleManager::VertexHandleManager(View::MapDocumentWPtr document, Renderer::TextureFont& font) :
        m_vbo(0xFF),
        m_totalVertexCount(0),
        m_selectedVertexCount(0),
        m_totalEdgeCount(0),
        m_selectedEdgeCount(0),
        m_totalFaceCount(0),
        m_selectedFaceCount(0),
        m_handleRenderer(m_vbo),
        m_guideRenderer(document),
        m_textRenderer(font),
        m_textColorProvider(Preferences::InfoOverlayTextColor, Preferences::InfoOverlayBackgroundColor),
        m_renderStateValid(false) {
            m_textRenderer.setFadeDistance(1000.0f);
        }
        
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
        
        Vec3::List VertexHandleManager::selectedVertexHandlePositions() const {
            return handlePositions(m_selectedVertexHandles);
        }
        
        Vec3::List VertexHandleManager::selectedEdgeHandlePositions() const {
            return handlePositions(m_selectedEdgeHandles);
        }
        
        Vec3::List VertexHandleManager::selectedFaceHandlePositions() const {
            return handlePositions(m_selectedFaceHandles);
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
        
        Model::BrushList VertexHandleManager::selectedBrushes() const {
            Model::BrushSet brushSet;
            
            Model::VertexToBrushesMap::const_iterator vMapIt, vMapEnd;
            for (vMapIt = m_selectedVertexHandles.begin(), vMapEnd = m_selectedVertexHandles.end(); vMapIt != vMapEnd; ++vMapIt) {
                const Model::BrushList& brushes = vMapIt->second;
                brushSet.insert(brushes.begin(), brushes.end());
            }
            
            Model::VertexToEdgesMap::const_iterator eMapIt, eMapEnd;
            Model::BrushEdgeList::const_iterator eIt, eEnd;
            for (eMapIt = m_selectedEdgeHandles.begin(), eMapEnd = m_selectedEdgeHandles.end(); eMapIt != eMapEnd; ++eMapIt) {
                const Model::BrushEdgeList& edges = eMapIt->second;
                
                for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd; ++eIt) {
                    Model::BrushEdge* edge = *eIt;
                    Model::Brush* brush = edge->leftFace()->parent();
                    brushSet.insert(brush);
                }
            }
            
            Model::VertexToFacesMap::const_iterator fMapIt, fMapEnd;
            Model::BrushFaceList::const_iterator fIt, fEnd;
            for (fMapIt = m_selectedFaceHandles.begin(), fMapEnd = m_selectedFaceHandles.end(); fMapIt != fMapEnd; ++fMapIt) {
                const Model::BrushFaceList& faces = fMapIt->second;
                
                for (fIt = faces.begin(), fEnd = faces.end(); fIt != fEnd; ++fIt) {
                    Model::BrushFace* face = *fIt;
                    brushSet.insert(face->parent());
                }
            }
            
            return Model::BrushList(brushSet.begin(), brushSet.end());
        }

        const Model::BrushList& VertexHandleManager::brushes(const Vec3& handlePosition) const {
            Model::VertexToBrushesMap::const_iterator mapIt = m_selectedVertexHandles.find(handlePosition);
            if (mapIt != m_selectedVertexHandles.end())
                return mapIt->second;
            mapIt = m_unselectedVertexHandles.find(handlePosition);
            if (mapIt != m_unselectedVertexHandles.end())
                return mapIt->second;
            return Model::EmptyBrushList;
        }
        
        const Model::BrushEdgeList& VertexHandleManager::edges(const Vec3& handlePosition) const {
            Model::VertexToEdgesMap::const_iterator mapIt = m_selectedEdgeHandles.find(handlePosition);
            if (mapIt != m_selectedEdgeHandles.end())
                return mapIt->second;
            mapIt = m_unselectedEdgeHandles.find(handlePosition);
            if (mapIt != m_unselectedEdgeHandles.end())
                return mapIt->second;
            return Model::EmptyBrushEdgeList;
        }
        
        const Model::BrushFaceList& VertexHandleManager::faces(const Vec3& handlePosition) const {
            Model::VertexToFacesMap::const_iterator mapIt = m_selectedFaceHandles.find(handlePosition);
            if (mapIt != m_selectedFaceHandles.end())
                return mapIt->second;
            mapIt = m_unselectedFaceHandles.find(handlePosition);
            if (mapIt != m_unselectedFaceHandles.end())
                return mapIt->second;
            return Model::EmptyBrushFaceList;
        }
        
        void VertexHandleManager::addBrush(Model::Brush* brush) {
            assert(brush != NULL);
            
            const Model::BrushVertexList& brushVertices = brush->vertices();
            Model::BrushVertexList::const_iterator vIt, vEnd;
            for (vIt = brushVertices.begin(), vEnd = brushVertices.end(); vIt != vEnd; ++vIt) {
                const Model::BrushVertex* vertex = *vIt;
                Model::VertexToBrushesMap::iterator mapIt = m_selectedVertexHandles.find(vertex->position);
                if (mapIt != m_selectedVertexHandles.end()) {
                    mapIt->second.push_back(brush);
                    m_selectedVertexCount++;
                } else {
                    m_unselectedVertexHandles[vertex->position].push_back(brush);
                }
            }
            m_totalVertexCount += brushVertices.size();
            
            const Model::BrushEdgeList& brushEdges = brush->edges();
            Model::BrushEdgeList::const_iterator eIt, eEnd;
            for (eIt = brushEdges.begin(), eEnd = brushEdges.end(); eIt != eEnd; ++eIt) {
                Model::BrushEdge* edge = *eIt;
                const Vec3 position = edge->center();
                Model::VertexToEdgesMap::iterator mapIt = m_selectedEdgeHandles.find(position);
                if (mapIt != m_selectedEdgeHandles.end()) {
                    mapIt->second.push_back(edge);
                    m_selectedEdgeCount++;
                } else {
                    m_unselectedEdgeHandles[position].push_back(edge);
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
                    mapIt->second.push_back(face);
                    m_selectedFaceCount++;
                } else {
                    m_unselectedFaceHandles[position].push_back(face);
                }
            }
            m_totalFaceCount += brushFaces.size();
            m_renderStateValid = false;
        }
        
        void VertexHandleManager::addBrushes(const Model::BrushList& brushes) {
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                addBrush(*it);
        }
        
        void VertexHandleManager::removeBrush(Model::Brush* brush) {
            const Model::BrushVertexList& brushVertices = brush->vertices();
            Model::BrushVertexList::const_iterator vIt, vEnd;
            for (vIt = brushVertices.begin(), vEnd = brushVertices.end(); vIt != vEnd; ++vIt) {
                const Model::BrushVertex* vertex = *vIt;
                if (removeHandle(vertex->position, brush, m_selectedVertexHandles)) {
                    assert(m_selectedVertexCount > 0);
                    m_selectedVertexCount--;
                } else {
                    removeHandle(vertex->position, brush, m_unselectedVertexHandles);
                }
            }
            assert(m_totalVertexCount >= brushVertices.size());
            m_totalVertexCount -= brushVertices.size();
            
            const Model::BrushEdgeList& brushEdges = brush->edges();
            Model::BrushEdgeList::const_iterator eIt, eEnd;
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
        
        void VertexHandleManager::removeBrushes(const Model::BrushList& brushes) {
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                removeBrush(*it);
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
        
        void VertexHandleManager::selectVertexHandles(const Vec3::List& positions) {
            Vec3::List::const_iterator it, end;
            for (it = positions.begin(), end = positions.end(); it != end; ++it)
                selectVertexHandle(*it);
        }
        
        void VertexHandleManager::deselectAllVertexHandles() {
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            for (vIt = m_selectedVertexHandles.begin(), vEnd = m_selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                const Model::BrushList& selectedBrushes = vIt->second;
                Model::BrushList& unselectedBrushes = m_unselectedVertexHandles[position];
                VectorUtils::append(unselectedBrushes, selectedBrushes);
            }
            m_selectedVertexHandles.clear();
            m_selectedVertexCount = 0;
            m_renderStateValid = false;
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
                const Model::BrushEdgeList& selectedEdges = eIt->second;
                Model::BrushEdgeList& unselectedEdges = m_unselectedEdgeHandles[position];
                VectorUtils::append(unselectedEdges, selectedEdges);
            }
            m_selectedEdgeHandles.clear();
            m_selectedEdgeCount = 0;
            m_renderStateValid = false;
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
                const Model::BrushFaceList& selectedFaces = fIt->second;
                Model::BrushFaceList& unselectedFaces = m_unselectedFaceHandles[position];
                VectorUtils::append(unselectedFaces, selectedFaces);
            }
            m_selectedFaceHandles.clear();
            m_selectedFaceCount = 0;
            m_renderStateValid = false;
        }
        
        bool VertexHandleManager::hasSelectedHandles() const {
            return !m_selectedVertexHandles.empty() || !m_selectedEdgeHandles.empty() || !m_selectedFaceHandles.empty();
        }

        void VertexHandleManager::deselectAllHandles() {
            deselectAllVertexHandles();
            deselectAllEdgeHandles();
            deselectAllFaceHandles();
        }
        
        void VertexHandleManager::pick(const Ray3& ray, Hits& hits, bool splitMode) const {
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            Model::VertexToEdgesMap::const_iterator eIt, eEnd;
            Model::VertexToFacesMap::const_iterator fIt, fEnd;
            
            if ((m_selectedEdgeHandles.empty() && m_selectedFaceHandles.empty()) || splitMode) {
                for (vIt = m_unselectedVertexHandles.begin(), vEnd = m_unselectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                    const Vec3& position = vIt->first;
                    const Hit hit = pickHandle(ray, position, VertexHandleHit);
                    if (hit.isMatch())
                        hits.addHit(hit);
                }
            }
            
            for (vIt = m_selectedVertexHandles.begin(), vEnd = m_selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                const Hit hit = pickHandle(ray, position, VertexHandleHit);
                if (hit.isMatch())
                    hits.addHit(hit);
            }
            
            if (m_selectedVertexHandles.empty() && m_selectedFaceHandles.empty() && !splitMode) {
                for (eIt = m_unselectedEdgeHandles.begin(), eEnd = m_unselectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                    const Vec3& position = eIt->first;
                    const Hit hit = pickHandle(ray, position, EdgeHandleHit);
                    if (hit.isMatch())
                        hits.addHit(hit);
                }
            }
            
            for (eIt = m_selectedEdgeHandles.begin(), eEnd = m_selectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                const Vec3& position = eIt->first;
                const Hit hit = pickHandle(ray, position, EdgeHandleHit);
                if (hit.isMatch())
                    hits.addHit(hit);
            }
            
            if (m_selectedVertexHandles.empty() && m_selectedEdgeHandles.empty() && !splitMode) {
                for (fIt = m_unselectedFaceHandles.begin(), fEnd = m_unselectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                    const Vec3& position = fIt->first;
                    const Hit hit = pickHandle(ray, position, FaceHandleHit);
                    if (hit.isMatch())
                        hits.addHit(hit);
                }
            }
            
            for (fIt = m_selectedFaceHandles.begin(), fEnd = m_selectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                const Vec3& position = fIt->first;
                const Hit hit = pickHandle(ray, position, FaceHandleHit);
                if (hit.isMatch())
                    hits.addHit(hit);
            }
        }

        void VertexHandleManager::render(Renderer::RenderContext& renderContext, const bool splitMode) {
            if (!m_renderStateValid)
                validateRenderState(splitMode);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            m_handleRenderer.setColor(prefs.get(Preferences::HandleColor));
            m_handleRenderer.setOccludedColor(prefs.get(Preferences::OccludedHandleColor));
            m_handleRenderer.setRenderOccluded(true);

            if (m_selectedEdgeHandles.empty() && m_selectedFaceHandles.empty() && !splitMode)
                m_handleRenderer.renderMultipleHandles(renderContext, VectorUtils::cast<Vec3f>(m_unselectedVertexHandlePositions));
            
            if (m_selectedVertexHandles.empty() && m_selectedFaceHandles.empty() && !splitMode)
                m_handleRenderer.renderMultipleHandles(renderContext, VectorUtils::cast<Vec3f>(m_unselectedEdgeHandlePositions));
            
            if (m_selectedVertexHandles.empty() && m_selectedEdgeHandles.empty() && !splitMode)
                m_handleRenderer.renderMultipleHandles(renderContext, VectorUtils::cast<Vec3f>(m_unselectedFaceHandlePositions));
            
            if ((!m_selectedEdgeHandles.empty() || !m_selectedFaceHandles.empty()) && !splitMode) {
                Renderer::glSetEdgeOffset(0.025f);
                m_edgeRenderer.setUseColor(true);
                glDisable(GL_DEPTH_TEST);
                m_edgeRenderer.setColor(prefs.get(Preferences::OccludedHandleColor));
                m_edgeRenderer.render(renderContext);
                glEnable(GL_DEPTH_TEST);
                m_edgeRenderer.setColor(prefs.get(Preferences::HandleColor));
                m_edgeRenderer.render(renderContext);
                Renderer::glResetEdgeOffset();
            }
            
            m_handleRenderer.setColor(prefs.get(Preferences::SelectedHandleColor));
            m_handleRenderer.setOccludedColor(prefs.get(Preferences::OccludedSelectedHandleColor));
            m_handleRenderer.renderMultipleHandles(renderContext, VectorUtils::cast<Vec3f>(m_selectedHandlePositions));
        }

        void VertexHandleManager::renderHighlight(Renderer::RenderContext& renderContext, const Vec3& position) {
            PreferenceManager& prefs = PreferenceManager::instance();
            m_handleRenderer.setHighlightColor(prefs.get(Preferences::SelectedHandleColor));
            m_handleRenderer.renderHandleHighlight(renderContext, Vec3f(position));

            m_guideRenderer.setPosition(position);
            m_guideRenderer.setColor(prefs.get(Preferences::HandleColor));
            m_guideRenderer.render(renderContext);
            
            Renderer::TextAnchor::Ptr anchor(new Renderer::SimpleTextAnchor(position, Renderer::Alignment::Bottom, Vec2f(0.0f, 16.0f)));
            m_textRenderer.renderOnce(position, position.asString(), anchor);
            
            glDisable(GL_DEPTH_TEST);
            m_textRenderer.render(renderContext, m_textFilter, m_textColorProvider, Renderer::Shaders::ColoredTextShader, Renderer::Shaders::TextBackgroundShader);
            glEnable(GL_DEPTH_TEST);
        }

        Hit VertexHandleManager::pickHandle(const Ray3& ray, const Vec3& position, Hit::HitType type) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            const FloatType handleRadius = prefs.get(Preferences::HandleRadius);
            const FloatType scalingFactor = prefs.get(Preferences::HandleScalingFactor);
            const FloatType maxDistance = prefs.get(Preferences::MaximumHandleDistance);
            
            const FloatType distance = ray.intersectWithSphere(position, 2.0 * handleRadius, scalingFactor, maxDistance);
            if (!Math::isnan(distance)) {
                const Vec3 hitPoint = ray.pointAtDistance(distance);
                return Hit::hit<Vec3>(type, distance, hitPoint, position);
            }
            
            return Hit::NoHit;
        }
        
        void VertexHandleManager::validateRenderState(const bool splitMode) {
            assert(!m_renderStateValid);
            
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            Model::VertexToEdgesMap::const_iterator eIt, eEnd;
            Model::VertexToFacesMap::const_iterator fIt, fEnd;

            typedef Renderer::VertexSpecs::P3::Vertex EdgeVertex;
            EdgeVertex::List edgeVertices;

            m_unselectedVertexHandlePositions.clear();
            m_unselectedEdgeHandlePositions.clear();
            m_unselectedFaceHandlePositions.clear();
            m_selectedHandlePositions.clear();
            
            m_unselectedVertexHandlePositions.reserve(m_unselectedVertexHandles.size());
            m_unselectedEdgeHandlePositions.reserve(m_unselectedEdgeHandles.size());
            m_unselectedFaceHandlePositions.reserve(m_unselectedFaceHandles.size());
            m_selectedHandlePositions.reserve(m_selectedVertexHandles.size() + m_selectedEdgeHandles.size() + m_selectedFaceHandles.size());

            for (vIt = m_unselectedVertexHandles.begin(), vEnd = m_unselectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                m_unselectedVertexHandlePositions.push_back(Vec3f(position));
            }

            for (eIt = m_unselectedEdgeHandles.begin(), eEnd = m_unselectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                const Vec3& position = eIt->first;
                m_unselectedEdgeHandlePositions.push_back(Vec3f(position));
            }

            for (fIt = m_unselectedFaceHandles.begin(), fEnd = m_unselectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                const Vec3& position = fIt->first;
                m_unselectedFaceHandlePositions.push_back(Vec3f(position));
            }

            for (vIt = m_selectedVertexHandles.begin(), vEnd = m_selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                m_selectedHandlePositions.push_back(Vec3f(position));
            }
            
            
            for (eIt = m_selectedEdgeHandles.begin(), eEnd = m_selectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                const Vec3& position = eIt->first;
                m_selectedHandlePositions.push_back(Vec3f(position));
                
                const Model::BrushEdgeList& edges = eIt->second;
                Model::BrushEdgeList::const_iterator edgeIt, edgeEnd;
                for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                    const Model::BrushEdge* edge = *edgeIt;
                    edgeVertices.push_back(Vec3f(edge->start->position));
                    edgeVertices.push_back(Vec3f(edge->end->position));
                }
            }
            
            for (fIt = m_selectedFaceHandles.begin(), fEnd = m_selectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                const Vec3f& position = fIt->first;
                m_selectedHandlePositions.push_back(Vec3f(position));
                
                const Model::BrushFaceList& faces = fIt->second;
                Model::BrushFaceList::const_iterator faceIt, faceEnd;
                for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                    const Model::BrushFace* face = *faceIt;
                    const Model::BrushEdgeList& edges = face->edges();
                    
                    Model::BrushEdgeList::const_iterator edgeIt, edgeEnd;
                    for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                        const Model::BrushEdge* edge = *edgeIt;
                        edgeVertices.push_back(Vec3f(edge->start->position));
                        edgeVertices.push_back(Vec3f(edge->end->position));
                    }
                }
            }

            m_edgeRenderer = Renderer::EdgeRenderer(Renderer::VertexArray::swap(GL_LINES, edgeVertices));
            m_renderStateValid = true;
        }
    }
}
