/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "VertexHandleManagerOld.h"

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

#include <algorithm>
#include <cassert>
#include <iterator>

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType VertexHandleManagerOld::VertexHandleHit = Model::Hit::freeHitType();
        const Model::Hit::HitType VertexHandleManagerOld::EdgeHandleHit   = Model::Hit::freeHitType();
        const Model::Hit::HitType VertexHandleManagerOld::FaceHandleHit   = Model::Hit::freeHitType();
        
        VertexHandleManagerOld::VertexHandleManagerOld(View::MapDocumentWPtr document) :
        m_totalVertexCount(0),
        m_selectedVertexCount(0),
        m_totalEdgeCount(0),
        m_selectedEdgeCount(0),
        m_totalFaceCount(0),
        m_selectedFaceCount(0),
        m_guideRenderer(document),
        m_renderStateValid(false) {}
        
        const Model::VertexToBrushesMap& VertexHandleManagerOld::unselectedVertexHandles() const {
            return m_unselectedVertexHandles;
        }
        
        const Model::VertexToBrushesMap& VertexHandleManagerOld::selectedVertexHandles() const {
            return m_selectedVertexHandles;
        }
        
        const Model::VertexToEdgesMap& VertexHandleManagerOld::unselectedEdgeHandles() const {
            return m_unselectedEdgeHandles;
        }
        
        const Model::VertexToEdgesMap& VertexHandleManagerOld::selectedEdgeHandles() const {
            return m_selectedEdgeHandles;
        }
        
        const Model::VertexToFacesMap& VertexHandleManagerOld::unselectedFaceHandles() const {
            return m_unselectedFaceHandles;
        }
        
        const Model::VertexToFacesMap& VertexHandleManagerOld::selectedFaceHandles() const {
            return m_selectedFaceHandles;
        }
        
        Vec3::List VertexHandleManagerOld::vertexHandlePositions() const {
            Vec3::List result;
            result.reserve(m_selectedVertexHandles.size() + m_unselectedVertexHandlePositions.size());
            handlePositions(m_unselectedVertexHandles, result);
            handlePositions(m_selectedVertexHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManagerOld::edgeHandlePositions() const {
            Vec3::List result;
            result.reserve(m_selectedEdgeHandles.size() + m_unselectedEdgeHandlePositions.size());
            handlePositions(m_unselectedEdgeHandles, result);
            handlePositions(m_selectedEdgeHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManagerOld::faceHandlePositions() const {
            Vec3::List result;
            result.reserve(m_selectedFaceHandles.size() + m_unselectedFaceHandlePositions.size());
            handlePositions(m_unselectedFaceHandles, result);
            handlePositions(m_selectedFaceHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManagerOld::unselectedVertexHandlePositions() const {
            Vec3::List result;
            handlePositions(m_unselectedVertexHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManagerOld::unselectedEdgeHandlePositions() const {
            Vec3::List result;
            handlePositions(m_unselectedEdgeHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManagerOld::unselectedFaceHandlePositions() const {
            Vec3::List result;
            handlePositions(m_unselectedFaceHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManagerOld::selectedVertexHandlePositions() const {
            Vec3::List result;
            handlePositions(m_selectedVertexHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManagerOld::selectedEdgeHandlePositions() const {
            Vec3::List result;
            handlePositions(m_selectedEdgeHandles, result);
            return result;
        }
        
        Vec3::List VertexHandleManagerOld::selectedFaceHandlePositions() const {
            Vec3::List result;
            handlePositions(m_selectedFaceHandles, result);
            return result;
        }

        bool VertexHandleManagerOld::isHandleSelected(const Vec3& position) const {
            return (isVertexHandleSelected(position) ||
                    isEdgeHandleSelected(position) ||
                    isFaceHandleSelected(position));
        }

        bool VertexHandleManagerOld::isVertexHandleSelected(const Vec3& position) const {
            return m_selectedVertexHandles.find(position) != std::end(m_selectedVertexHandles);
        }
        
        bool VertexHandleManagerOld::isEdgeHandleSelected(const Vec3& position) const {
            return m_selectedEdgeHandles.find(position) != std::end(m_selectedEdgeHandles);
        }
        
        bool VertexHandleManagerOld::isFaceHandleSelected(const Vec3& position) const {
            return m_selectedFaceHandles.find(position) != std::end(m_selectedFaceHandles);
        }
        
        size_t VertexHandleManagerOld::selectedVertexCount() const {
            return m_selectedVertexCount;
        }
        
        size_t VertexHandleManagerOld::totalVertexCount() const {
            return m_totalVertexCount;
        }
        
        size_t VertexHandleManagerOld::selectedEdgeCount() const {
            return m_selectedEdgeCount;
        }
        
        size_t VertexHandleManagerOld::totalEdgeCount() const {
            return m_totalEdgeCount;
        }
        
        size_t VertexHandleManagerOld::selectedFaceCount() const {
            return m_selectedFaceCount;
        }
        
        size_t VertexHandleManagerOld::totalSelectedFaceCount() const {
            return m_totalFaceCount;
        }
        
        Model::BrushSet VertexHandleManagerOld::selectedBrushes() const {
            Model::BrushSet brushSet;
            
            for (const auto& entry : m_selectedVertexHandles) {
                const Model::BrushSet& brushes = entry.second;
                brushSet.insert(std::begin(brushes), std::end(brushes));
            }
            
            for (const auto& entry : m_selectedEdgeHandles) {
                const Model::BrushEdgeSet& edges = entry.second;
                for (Model::BrushEdge* edge : edges) {
                    Model::Brush* brush = edge->firstFace()->payload()->brush();
                    brushSet.insert(brush);
                }
            }
            
            for (const auto& entry : m_selectedFaceHandles) {
                const Model::BrushFaceSet& faces = entry.second;
                for (Model::BrushFace* face : faces)
                    brushSet.insert(face->brush());
            }
            
            return brushSet;
        }

        const Model::BrushSet& VertexHandleManagerOld::brushes(const Vec3& handlePosition) const {
            Model::VertexToBrushesMap::const_iterator mapIt = m_selectedVertexHandles.find(handlePosition);
            if (mapIt != std::end(m_selectedVertexHandles))
                return mapIt->second;
            mapIt = m_unselectedVertexHandles.find(handlePosition);
            if (mapIt != std::end(m_unselectedVertexHandles))
                return mapIt->second;
            return Model::EmptyBrushSet;
        }
        
        const Model::BrushEdgeSet& VertexHandleManagerOld::edges(const Vec3& handlePosition) const {
            Model::VertexToEdgesMap::const_iterator mapIt = m_selectedEdgeHandles.find(handlePosition);
            if (mapIt != std::end(m_selectedEdgeHandles))
                return mapIt->second;
            mapIt = m_unselectedEdgeHandles.find(handlePosition);
            if (mapIt != std::end(m_unselectedEdgeHandles))
                return mapIt->second;
            return Model::EmptyBrushEdgeSet;
        }
        
        const Model::BrushFaceSet& VertexHandleManagerOld::faces(const Vec3& handlePosition) const {
            Model::VertexToFacesMap::const_iterator mapIt = m_selectedFaceHandles.find(handlePosition);
            if (mapIt != std::end(m_selectedFaceHandles))
                return mapIt->second;
            mapIt = m_unselectedFaceHandles.find(handlePosition);
            if (mapIt != std::end(m_unselectedFaceHandles))
                return mapIt->second;
            return Model::EmptyBrushFaceSet;
        }
        
        void VertexHandleManagerOld::addBrush(Model::Brush* brush) {
            ensure(brush != NULL, "brush is null");
            
            for (const Model::BrushVertex* vertex : brush->vertices()) {
                const auto mapIt = m_selectedVertexHandles.find(vertex->position());
                if (mapIt != std::end(m_selectedVertexHandles)) {
                    mapIt->second.insert(brush);
                    ++m_selectedVertexCount;
                } else {
                    m_unselectedVertexHandles[vertex->position()].insert(brush);
                }
            }
            m_totalVertexCount += brush->vertexCount();
            
            for (Model::BrushEdge* edge : brush->edges()) {
                const Vec3 position = edge->center();
                const auto mapIt = m_selectedEdgeHandles.find(position);
                if (mapIt != std::end(m_selectedEdgeHandles)) {
                    mapIt->second.insert(edge);
                    ++m_selectedEdgeCount;
                } else {
                    m_unselectedEdgeHandles[position].insert(edge);
                }
            }
            m_totalEdgeCount += brush->edgeCount();

            for (Model::BrushFace* face : brush->faces()) {
                const Vec3 position = face->center();
                const auto mapIt = m_selectedFaceHandles.find(position);
                if (mapIt != std::end(m_selectedFaceHandles)) {
                    mapIt->second.insert(face);
                    ++m_selectedFaceCount;
                } else {
                    m_unselectedFaceHandles[position].insert(face);
                }
            }
            m_totalFaceCount += brush->faceCount();
            
            m_renderStateValid = false;
        }
        
        void VertexHandleManagerOld::removeBrush(Model::Brush* brush) {
            for (const Model::BrushVertex* vertex : brush->vertices()) {
                if (removeHandle(vertex->position(), brush, m_selectedVertexHandles)) {
                    ensure(m_selectedVertexCount > 0, "no selected vertices");
                    --m_selectedVertexCount;
                } else {
                    removeHandle(vertex->position(), brush, m_unselectedVertexHandles);
                }
            }
            ensure(m_totalVertexCount >= brush->vertexCount(), "brush vertices exceed total vertices");
            m_totalVertexCount -= brush->vertexCount();
            
            for (Model::BrushEdge* edge : brush->edges()) {
                const Vec3 position = edge->center();
                if (removeHandle(position, edge, m_selectedEdgeHandles)) {
                    ensure(m_selectedEdgeCount > 0, "no selected edges");
                    --m_selectedEdgeCount;
                } else {
                    removeHandle(position, edge, m_unselectedEdgeHandles);
                }
            }
            ensure(m_totalEdgeCount >= brush->edgeCount(), "brush edges exceed total edges");
            m_totalEdgeCount -= brush->edgeCount();
            
            for (Model::BrushFace* face : brush->faces()) {
                const Vec3 position = face->center();
                if (removeHandle(position, face, m_selectedFaceHandles)) {
                    ensure(m_selectedFaceCount > 0, "no selected faces");
                    --m_selectedFaceCount;
                } else {
                    removeHandle(position, face, m_unselectedFaceHandles);
                }
            }
            ensure(m_totalFaceCount >= brush->faceCount(), "brush faces exceed total faces");
            m_totalFaceCount -= brush->faceCount();
            
            m_renderStateValid = false;
        }
        
        void VertexHandleManagerOld::clear() {
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
        
        void VertexHandleManagerOld::selectVertexHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_unselectedVertexHandles, m_selectedVertexHandles)) > 0) {
                m_selectedVertexCount += count;
                m_renderStateValid = false;
            }
        }
        
        void VertexHandleManagerOld::deselectVertexHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_selectedVertexHandles, m_unselectedVertexHandles)) > 0) {
                ensure(m_selectedVertexCount >= count, "deselected vertices exceeds selected vertices");
                m_selectedVertexCount -= count;
                m_renderStateValid = false;
            }
        }
        
        void VertexHandleManagerOld::toggleVertexHandle(const Vec3& position) {
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

        void VertexHandleManagerOld::selectVertexHandles(const Vec3::List& positions) {
            for (const Vec3& position : positions)
                selectVertexHandle(position);
        }
        
        void VertexHandleManagerOld::deselectAllVertexHandles() {
            for (const auto& entry : m_selectedVertexHandles) {
                const Vec3& position = entry.first;
                const Model::BrushSet& selectedBrushes = entry.second;
                Model::BrushSet& unselectedBrushes = m_unselectedVertexHandles[position];
                unselectedBrushes.insert(std::begin(selectedBrushes), std::end(selectedBrushes));
            }
            m_selectedVertexHandles.clear();
            m_selectedVertexCount = 0;
            m_renderStateValid = false;
        }
        
        void VertexHandleManagerOld::toggleVertexHandles(const Vec3::List& positions) {
            for (const Vec3& position : positions)
                toggleVertexHandle(position);
        }
        
        void VertexHandleManagerOld::selectEdgeHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_unselectedEdgeHandles, m_selectedEdgeHandles)) > 0) {
                m_selectedEdgeCount += count;
                m_renderStateValid = false;
            }
        }
        
        void VertexHandleManagerOld::deselectEdgeHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_selectedEdgeHandles, m_unselectedEdgeHandles)) > 0) {
                ensure(m_selectedEdgeCount >= count, "deselected edges exceeds selected edges");
                m_selectedEdgeCount -= count;
                m_renderStateValid = false;
            }
        }
        
        void VertexHandleManagerOld::toggleEdgeHandle(const Vec3& position) {
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

        void VertexHandleManagerOld::selectEdgeHandles(const Edge3::List& edges) {
            for (const Edge3& edge : edges)
                selectEdgeHandle(edge.center());
        }
        
        void VertexHandleManagerOld::deselectAllEdgeHandles() {
            for (const auto& entry : m_selectedEdgeHandles) {
                const Vec3& position = entry.first;
                const Model::BrushEdgeSet& selectedEdges = entry.second;
                Model::BrushEdgeSet& unselectedEdges = m_unselectedEdgeHandles[position];
                unselectedEdges.insert(std::begin(selectedEdges), std::end(selectedEdges));
            }
            m_selectedEdgeHandles.clear();
            m_selectedEdgeCount = 0;
            m_renderStateValid = false;
        }
        
        void VertexHandleManagerOld::toggleEdgeHandles(const Vec3::List& positions) {
            for (const Vec3& position : positions)
                toggleEdgeHandle(position);
        }

        void VertexHandleManagerOld::selectFaceHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_unselectedFaceHandles, m_selectedFaceHandles)) > 0) {
                m_selectedFaceCount += count;
                m_renderStateValid = false;
            }
        }
        
        void VertexHandleManagerOld::deselectFaceHandle(const Vec3& position) {
            size_t count = 0;
            if ((count = moveHandle(position, m_selectedFaceHandles, m_unselectedFaceHandles)) > 0) {
                ensure(m_selectedFaceCount >= count, "deselected faces exceeds selected faces");
                m_selectedFaceCount -= count;
                m_renderStateValid = false;
            }
        }
        
        void VertexHandleManagerOld::toggleFaceHandle(const Vec3& position) {
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

        void VertexHandleManagerOld::selectFaceHandles(const Polygon3::List& faces) {
            for (const Polygon3& face : faces)
                selectFaceHandle(face.center());
        }
        
        void VertexHandleManagerOld::deselectAllFaceHandles() {
            for (const auto& entry : m_selectedFaceHandles) {
                const Vec3& position = entry.first;
                const Model::BrushFaceSet& selectedFaces = entry.second;
                Model::BrushFaceSet& unselectedFaces = m_unselectedFaceHandles[position];
                unselectedFaces.insert(std::begin(selectedFaces), std::end(selectedFaces));
            }
            m_selectedFaceHandles.clear();
            m_selectedFaceCount = 0;
            m_renderStateValid = false;
        }
        
        void VertexHandleManagerOld::toggleFaceHandles(const Vec3::List& positions) {
            for (const Vec3& position : positions)
                toggleFaceHandle(position);
        }

        bool VertexHandleManagerOld::hasSelectedHandles() const {
            return !m_selectedVertexHandles.empty() || !m_selectedEdgeHandles.empty() || !m_selectedFaceHandles.empty();
        }

        void VertexHandleManagerOld::deselectAllHandles() {
            deselectAllVertexHandles();
            deselectAllEdgeHandles();
            deselectAllFaceHandles();
        }
        
        void VertexHandleManagerOld::reselectVertexHandles(const Model::BrushSet& brushes, const Vec3::List& positions, const FloatType maxDistance) {
            for (const Vec3& oldPosition : positions) {
                for (const Vec3& newPosition : findVertexHandlePositions(brushes, oldPosition, maxDistance))
                    selectVertexHandle(newPosition);
            }
        }
        
        void VertexHandleManagerOld::reselectEdgeHandles(const Model::BrushSet& brushes, const Vec3::List& positions, const FloatType maxDistance) {
            for (const Vec3& oldPosition : positions) {
                for (const Vec3& newPosition : findEdgeHandlePositions(brushes, oldPosition, maxDistance))
                    selectEdgeHandle(newPosition);
            }
        }
        
        void VertexHandleManagerOld::reselectFaceHandles(const Model::BrushSet& brushes, const Vec3::List& positions, const FloatType maxDistance) {
            for (const Vec3& oldPosition : positions) {
                for (const Vec3& newPosition : findFaceHandlePositions(brushes, oldPosition, maxDistance))
                    selectFaceHandle(newPosition);
            }
        }

        void VertexHandleManagerOld::pick(const Ray3& ray, const Renderer::Camera& camera, Model::PickResult& pickResult, bool splitMode) const {
            if ((m_selectedEdgeHandles.empty() && m_selectedFaceHandles.empty()) || splitMode) {
                for (const auto& entry : m_unselectedVertexHandles) {
                    const Vec3& position = entry.first;
                    const Model::Hit hit = pickHandle(ray, camera, position, VertexHandleHit);
                    if (hit.isMatch())
                        pickResult.addHit(hit);
                }
            }

            for (const auto& entry : m_selectedVertexHandles) {
                const Vec3& position = entry.first;
                const Model::Hit hit = pickHandle(ray, camera, position, VertexHandleHit);
                if (hit.isMatch())
                    pickResult.addHit(hit);
            }
            
            if (m_selectedVertexHandles.empty() && m_selectedFaceHandles.empty() && !splitMode) {
                for (const auto& entry : m_unselectedEdgeHandles) {
                    const Vec3& position = entry.first;
                    const Model::Hit hit = pickHandle(ray, camera, position, EdgeHandleHit);
                    if (hit.isMatch())
                        pickResult.addHit(hit);
                }
            }
            
            for (const auto& entry : m_selectedEdgeHandles) {
                const Vec3& position = entry.first;
                const Model::Hit hit = pickHandle(ray, camera, position, EdgeHandleHit);
                if (hit.isMatch())
                    pickResult.addHit(hit);
            }
            
            if (m_selectedVertexHandles.empty() && m_selectedEdgeHandles.empty() && !splitMode) {
                for (const auto& entry : m_unselectedFaceHandles) {
                    const Vec3& position = entry.first;
                    const Model::Hit hit = pickHandle(ray, camera, position, FaceHandleHit);
                    if (hit.isMatch())
                        pickResult.addHit(hit);
                }
            }
            
            for (const auto& entry : m_selectedFaceHandles) {
                const Vec3& position = entry.first;
                const Model::Hit hit = pickHandle(ray, camera, position, FaceHandleHit);
                if (hit.isMatch())
                    pickResult.addHit(hit);
            }
        }

        void VertexHandleManagerOld::render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const bool splitMode) {
            if (!m_renderStateValid)
                validateRenderState(splitMode);
            
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::HandleColor));
            if (m_selectedEdgeHandles.empty() && m_selectedFaceHandles.empty() && !splitMode)
                renderService.renderHandles(VectorUtils::cast<Vec3f>(m_unselectedVertexHandlePositions));
            
            if (m_selectedVertexHandles.empty() && m_selectedFaceHandles.empty() && !splitMode)
                renderService.renderHandles(VectorUtils::cast<Vec3f>(m_unselectedEdgeHandlePositions));
            
            if (m_selectedVertexHandles.empty() && m_selectedEdgeHandles.empty() && !splitMode)
                renderService.renderHandles(VectorUtils::cast<Vec3f>(m_unselectedFaceHandlePositions));
            
            if ((!m_selectedEdgeHandles.empty() || !m_selectedFaceHandles.empty()) && !splitMode)
                renderService.renderLines(m_edgeVertices);
            
            renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
            renderService.renderHandles(VectorUtils::cast<Vec3f>(m_selectedHandlePositions));
        }

        void VertexHandleManagerOld::renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& position) {
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
            renderService.renderHandleHighlight(position);
            
            renderService.setForegroundColor(pref(Preferences::SelectedInfoOverlayTextColor));
            renderService.setBackgroundColor(pref(Preferences::SelectedInfoOverlayBackgroundColor));
            renderService.renderString(position.asString(), position);
        }

        void VertexHandleManagerOld::renderEdgeHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handlePosition) {
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
        
        void VertexHandleManagerOld::renderFaceHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handlePosition) {
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

        void VertexHandleManagerOld::renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& position) {
            Renderer::RenderService renderService(renderContext, renderBatch);
            m_guideRenderer.setPosition(position);
            m_guideRenderer.setColor(Color(pref(Preferences::HandleColor), 0.5f));
            renderBatch.add(&m_guideRenderer);
        }
        
        Vec3::List VertexHandleManagerOld::findVertexHandlePositions(const Model::BrushSet& brushes, const Vec3& query, const FloatType maxDistance) {
            Vec3::List result;
            
            for (const Model::Brush* brush : brushes) {
                for (const Model::BrushVertex* vertex : brush->vertices()) {
                    if (query.squaredDistanceTo(vertex->position()) <= maxDistance * maxDistance)
                        result.push_back(vertex->position());
                }
            }
            
            return result;
        }

        Vec3::List VertexHandleManagerOld::findEdgeHandlePositions(const Model::BrushSet& brushes, const Vec3& query, const FloatType maxDistance) {
            Vec3::List result;

            for (const Model::Brush* brush : brushes) {
                for (const Model::BrushEdge* edge : brush->edges()) {
                    const Vec3 center = edge->center();
                    if (query.squaredDistanceTo(center) <= maxDistance * maxDistance)
                        result.push_back(center);
                }
            }
            
            return result;
        }
        
        Vec3::List VertexHandleManagerOld::findFaceHandlePositions(const Model::BrushSet& brushes, const Vec3& query, const FloatType maxDistance) {
            Vec3::List result;

            for (const Model::Brush* brush : brushes) {
                for (const Model::BrushFace* face : brush->faces()) {
                    const Vec3 center = face->center();
                    if (query.squaredDistanceTo(center) <= maxDistance * maxDistance)
                        result.push_back(center);
                }
            }
            
            return result;
        }

        Model::Hit VertexHandleManagerOld::pickHandle(const Ray3& ray, const Renderer::Camera& camera, const Vec3& position, Model::Hit::HitType type) const {
            const FloatType distance = camera.pickPointHandle(ray, position, pref(Preferences::HandleRadius));
            if (!Math::isnan(distance)) {
                const Vec3 hitPoint = ray.pointAtDistance(distance);
                return Model::Hit::hit<Vec3>(type, distance, hitPoint, position);
            }
            
            return Model::Hit::NoHit;
        }
        
        void VertexHandleManagerOld::validateRenderState(const bool splitMode) {
            ensure(!m_renderStateValid, "render state already valid");

            m_unselectedVertexHandlePositions.clear();
            m_unselectedEdgeHandlePositions.clear();
            m_unselectedFaceHandlePositions.clear();
            m_selectedHandlePositions.clear();
            m_edgeVertices.clear();
            
            m_unselectedVertexHandlePositions.reserve(m_unselectedVertexHandles.size());
            m_unselectedEdgeHandlePositions.reserve(m_unselectedEdgeHandles.size());
            m_unselectedFaceHandlePositions.reserve(m_unselectedFaceHandles.size());
            m_selectedHandlePositions.reserve(m_selectedVertexHandles.size() + m_selectedEdgeHandles.size() + m_selectedFaceHandles.size());

            for (const auto& entry : m_unselectedVertexHandles) {
                const Vec3& position = entry.first;
                m_unselectedVertexHandlePositions.push_back(position);
            }

            for (const auto& entry : m_unselectedEdgeHandles) {
                const Vec3& position = entry.first;
                m_unselectedEdgeHandlePositions.push_back(position);
            }

            for (const auto& entry : m_unselectedFaceHandles) {
                const Vec3& position = entry.first;
                m_unselectedFaceHandlePositions.push_back(position);
            }

            for (const auto& entry : m_selectedVertexHandles) {
                const Vec3& position = entry.first;
                m_selectedHandlePositions.push_back(position);
            }
            
            
            for (const auto& entry : m_selectedEdgeHandles) {
                const Vec3& position = entry.first;
                m_selectedHandlePositions.push_back(position);
                
                const Model::BrushEdgeSet& edges = entry.second;
                for (const Model::BrushEdge* edge : edges) {
                    m_edgeVertices.push_back(Vec3f(edge->firstVertex()->position()));
                    m_edgeVertices.push_back(Vec3f(edge->secondVertex()->position()));
                }
            }
            
            for (const auto& entry : m_selectedFaceHandles) {
                const Vec3f& position = entry.first;
                m_selectedHandlePositions.push_back(Vec3f(position));
                
                const Model::BrushFaceSet& faces = entry.second;
                for (const Model::BrushFace* face : faces) {
                    const Model::BrushFace::EdgeList edges = face->edges();
                    for (const Model::BrushEdge* edge : edges) {
                        m_edgeVertices.push_back(Vec3f(edge->firstVertex()->position()));
                        m_edgeVertices.push_back(Vec3f(edge->secondVertex()->position()));
                    }
                }
            }

            m_renderStateValid = true;
        }
    }
}
