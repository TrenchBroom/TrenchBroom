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

#include "ClipTool.h"

#include "CollectionUtils.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/HitQuery.h"
#include "Model/World.h"
#include "Model/PickResult.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"
#include "Renderer/TextAnchor.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType ClipTool::ClipPointHit = Model::Hit::freeHitType();

        ClipTool::ClipPlaneStrategy::~ClipPlaneStrategy() {}
        
        Vec3 ClipTool::ClipPlaneStrategy::snapClipPoint(const Grid& grid, const Vec3& point) const {
            return doSnapClipPoint(grid, point);
        }

        ClipTool::ClipTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_numClipPoints(0),
        m_dragIndex(4),
        m_clipSide(ClipSide_Front),
        m_remainingBrushRenderer(new Renderer::BrushRenderer(false)),
        m_clippedBrushRenderer(new Renderer::BrushRenderer(true)) {}
        
        ClipTool::~ClipTool() {
            deleteBrushes();
            delete m_remainingBrushRenderer;
            delete m_clippedBrushRenderer;
        }
        
        void ClipTool::toggleClipSide() {
            switch (m_clipSide) {
                case ClipSide_Front:
                    m_clipSide = ClipSide_Both;
                    break;
                case ClipSide_Both:
                    m_clipSide = ClipSide_Back;
                    break;
                case ClipSide_Back:
                    m_clipSide = ClipSide_Front;
                    break;
            }
            update();
        }
        
        void ClipTool::performClip() {

            reset();
        }
        
        void ClipTool::pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            for (size_t i = 0; i < m_numClipPoints; ++i) {
                const Vec3& point = m_clipPoints[i];
                const FloatType distance = camera.pickPointHandle(pickRay, point, pref(Preferences::HandleRadius));
                if (!Math::isnan(distance)) {
                    const Vec3 hitPoint = pickRay.pointAtDistance(distance);
                    pickResult.addHit(Model::Hit(ClipPointHit, distance, hitPoint, i));
                }
            }
        }
        
        Vec3 ClipTool::defaultClipPointPos() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectionBounds().center();
        }
        
        bool ClipTool::addClipPoint(const Vec3& point, const ClipPlaneStrategy& strategy) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            
            const Vec3 snappedPoint = strategy.snapClipPoint(grid, point);
            if (m_numClipPoints == 2 && linearlyDependent(m_clipPoints[0], m_clipPoints[1], snappedPoint))
                return false;
            
            m_clipPoints[m_numClipPoints] = snappedPoint;
            ++m_numClipPoints;
            
            update();
            
            return true;
        }

        bool ClipTool::beginDragClipPoint(const Model::PickResult& pickResult) {
            const Model::Hit& hit = pickResult.query().type(ClipPointHit).occluded().first();
            if (!hit.isMatch())
                return false;
            m_dragIndex = hit.target<size_t>();
            return true;
        }

        Vec3 ClipTool::draggedPointPosition() const {
            assert(m_dragIndex < m_numClipPoints);
            return m_clipPoints[m_dragIndex];
        }

        bool ClipTool::dragClipPoint(const Vec3& newPosition, const ClipPlaneStrategy& strategy) {
            assert(m_dragIndex < m_numClipPoints);
            const Vec3 oldPosition = m_clipPoints[m_dragIndex];
            
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            m_clipPoints[m_dragIndex] = strategy.snapClipPoint(grid, newPosition);

            if (m_numClipPoints == 3 && linearlyDependent(m_clipPoints[0], m_clipPoints[1], m_clipPoints[2])) {
                m_clipPoints[m_dragIndex] = oldPosition;
                return false;
            }
            
            update();
            return true;
        }
        
        bool ClipTool::hasClipPoints() const {
            return m_numClipPoints > 0;
        }
        
        void ClipTool::deleteLastClipPoint() {
            if (m_numClipPoints > 0) {
                --m_numClipPoints;
                update();
            }
        }
        
        void ClipTool::reset() {
            resetClipPoints();
            resetClipSide();
            update();
        }
        
        void ClipTool::resetClipPoints() {
            m_numClipPoints = 0;
        }
        
        void ClipTool::resetClipSide() {
            m_clipSide = ClipSide_Front;
        }

        void ClipTool::renderBrushes(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_remainingBrushRenderer->setFaceColor(pref(Preferences::FaceColor));
            m_remainingBrushRenderer->setEdgeColor(pref(Preferences::SelectedEdgeColor));
            m_remainingBrushRenderer->setShowOccludedEdges(true);
            m_remainingBrushRenderer->setOccludedEdgeColor(pref(Preferences::OccludedSelectedEdgeColor));
            m_remainingBrushRenderer->setTint(true);
            m_remainingBrushRenderer->setTintColor(pref(Preferences::SelectedFaceColor));
            m_remainingBrushRenderer->render(renderContext, renderBatch);
            
            m_clippedBrushRenderer->setFaceColor(pref(Preferences::FaceColor));
            m_clippedBrushRenderer->setEdgeColor(pref(Preferences::SelectedEdgeColor));
            m_clippedBrushRenderer->setShowOccludedEdges(true);
            m_clippedBrushRenderer->setOccludedEdgeColor(pref(Preferences::OccludedSelectedEdgeColor));
            m_clippedBrushRenderer->setTint(false);
            m_clippedBrushRenderer->setTransparencyAlpha(0.5f);
            m_clippedBrushRenderer->render(renderContext, renderBatch);
        }
        
        void ClipTool::renderClipPoints(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            Renderer::RenderService renderService(renderContext, renderBatch);
            
            if (m_numClipPoints > 1) {
                renderService.renderLine(pref(Preferences::HandleColor), m_clipPoints[0], m_clipPoints[1]);
                if (m_numClipPoints > 2) {
                    renderService.renderLine(pref(Preferences::HandleColor), m_clipPoints[1], m_clipPoints[2]);
                    renderService.renderLine(pref(Preferences::HandleColor), m_clipPoints[2], m_clipPoints[0]);
                }
            }
            
            for (size_t i = 0; i < m_numClipPoints; ++i) {
                const Vec3& point = m_clipPoints[i];
                renderService.renderPointHandle(point);
                
                StringStream str;
                str << (i+1) << ": " << point.asString();
                
                const Renderer::SimpleTextAnchor anchor(point, Renderer::TextAlignment::Bottom, Vec2f(0.0f, 10.0f));
                renderService.renderStringOnTop(pref(Preferences::HandleColor), pref(Preferences::InfoOverlayBackgroundColor), str.str(), anchor);
            }
        }
        
        void ClipTool::renderHighlight(const bool dragging, const Model::PickResult& pickResult, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (dragging) {
                renderHighlight(m_dragIndex, renderContext, renderBatch);
            } else {
                const Model::Hit& hit = pickResult.query().type(ClipPointHit).occluded().first();
                if (hit.isMatch()) {
                    const size_t index = hit.target<size_t>();
                    renderHighlight(index, renderContext, renderBatch);
                }
            }
        }

        void ClipTool::renderHighlight(const size_t index, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            assert(index < m_numClipPoints);
            
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.renderPointHandleHighlight(m_clipPoints[index]);
        }

        bool ClipTool::doActivate() {
            MapDocumentSPtr document = lock(m_document);
            if (!document->selectedNodes().hasOnlyBrushes())
                return false;
            bindObservers();
            reset();
            return true;
        }
        
        bool ClipTool::doDeactivate() {
            reset();
            unbindObservers();
            return true;
        }
        
        void ClipTool::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->selectionDidChangeNotifier.addObserver(this, &ClipTool::selectionDidChange);
            document->nodesWillChangeNotifier.addObserver(this, &ClipTool::nodesWillChange);
            document->nodesDidChangeNotifier.addObserver(this, &ClipTool::nodesDidChange);
        }
        
        void ClipTool::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->selectionDidChangeNotifier.removeObserver(this, &ClipTool::selectionDidChange);
                document->nodesWillChangeNotifier.removeObserver(this, &ClipTool::nodesWillChange);
                document->nodesDidChangeNotifier.removeObserver(this, &ClipTool::nodesDidChange);
            }
        }
        
        void ClipTool::selectionDidChange(const Selection& selection) {
            updateBrushes();
        }
        
        void ClipTool::nodesWillChange(const Model::NodeList& nodes) {
            updateBrushes();
        }
        
        void ClipTool::nodesDidChange(const Model::NodeList& nodes) {
            updateBrushes();
        }

        void ClipTool::update() {
            clearRenderers();
            updateBrushes();
            updateRenderers();
            refreshViews();
        }

        void ClipTool::updateBrushes() {
            deleteBrushes();
            clipBrushes();
        }
        
        void ClipTool::clipBrushes() {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushList& brushes = document->selectedNodes().brushes();
            const BBox3& worldBounds = document->worldBounds();
            
            if (m_numClipPoints == 3) {
                Model::World* world = document->world();
                Model::BrushList::const_iterator bIt, bEnd;
                for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                    Model::Brush* brush = *bIt;
                    Model::Node* parent = brush->parent();
                    
                    Model::BrushFace* frontFace = world->createFace(m_clipPoints[0], m_clipPoints[1], m_clipPoints[2], document->currentTextureName());
                    Model::BrushFace* backFace = world->createFace(m_clipPoints[0], m_clipPoints[2], m_clipPoints[1], document->currentTextureName());
                    setFaceAttributes(brush->faces(), frontFace, backFace);
                    
                    Model::Brush* frontBrush = brush->clone(worldBounds);
                    if (frontBrush->clip(worldBounds, frontFace))
                        m_frontBrushes[parent].push_back(frontBrush);
                    else
                        delete frontBrush;
                    
                    Model::Brush* backBrush = brush->clone(worldBounds);
                    if (backBrush->clip(worldBounds, backFace))
                        m_backBrushes[parent].push_back(backBrush);
                    else
                        delete backBrush;
                }
            } else {
                Model::BrushList::const_iterator bIt, bEnd;
                for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                    Model::Brush* brush = *bIt;
                    Model::Node* parent = brush->parent();
                    
                    Model::Brush* frontBrush = brush->clone(worldBounds);
                    m_frontBrushes[parent].push_back(frontBrush);
                }
            }
        }
        
        void ClipTool::deleteBrushes() {
            MapUtils::clearAndDelete(m_frontBrushes);
            MapUtils::clearAndDelete(m_backBrushes);
        }

        void ClipTool::setFaceAttributes(const Model::BrushFaceList& faces, Model::BrushFace* frontFace, Model::BrushFace* backFace) const {
            assert(!faces.empty());
            
            Model::BrushFaceList::const_iterator faceIt = faces.begin();
            Model::BrushFaceList::const_iterator faceEnd = faces.end();
            const Model::BrushFace* bestFrontFace = *faceIt++;
            const Model::BrushFace* bestBackFace = bestFrontFace;
            
            while (faceIt != faceEnd) {
                const Model::BrushFace* face = *faceIt;
                
                const Vec3 bestFrontDiff = bestFrontFace->boundary().normal - frontFace->boundary().normal;
                const Vec3 frontDiff = face->boundary().normal - frontFace->boundary().normal;
                if (frontDiff.squaredLength() < bestFrontDiff.squaredLength())
                    bestFrontFace = face;
                
                const Vec3f bestBackDiff = bestBackFace->boundary().normal - backFace->boundary().normal;
                const Vec3f backDiff = face->boundary().normal - backFace->boundary().normal;
                if (backDiff.squaredLength() < bestBackDiff.squaredLength())
                    bestBackFace = face;
                ++faceIt;
            }
            
            assert(bestFrontFace != NULL);
            assert(bestBackFace != NULL);
            frontFace->setAttributes(bestFrontFace);
            backFace->setAttributes(bestBackFace);
        }

        void ClipTool::clearRenderers() {
            m_remainingBrushRenderer->clear();
            m_clippedBrushRenderer->clear();
        }
        
        void ClipTool::updateRenderers() {
            if (keepFrontBrushes())
                addBrushesToRenderer(m_frontBrushes, m_remainingBrushRenderer);
            else
                addBrushesToRenderer(m_frontBrushes, m_clippedBrushRenderer);
            
            if (keepBackBrushes())
                addBrushesToRenderer(m_backBrushes, m_remainingBrushRenderer);
            else
                addBrushesToRenderer(m_backBrushes, m_clippedBrushRenderer);
        }

        bool ClipTool::keepFrontBrushes() const {
            return m_clipSide != ClipSide_Back;
        }
        
        bool ClipTool::keepBackBrushes() const {
            return m_clipSide != ClipSide_Front;
        }

        class ClipTool::AddBrushesToRendererVisitor : public Model::NodeVisitor {
        private:
            Renderer::BrushRenderer* m_renderer;
        public:
            AddBrushesToRendererVisitor(Renderer::BrushRenderer* renderer) : m_renderer(renderer) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Entity* entity) {}
            void doVisit(Model::Brush* brush)   { m_renderer->addBrush(brush); }
        };
        
        void ClipTool::addBrushesToRenderer(const Model::ParentChildrenMap& map, Renderer::BrushRenderer* renderer) {
            AddBrushesToRendererVisitor visitor(renderer);
            
            Model::ParentChildrenMap::const_iterator it, end;
            for (it = map.begin(), end = map.end(); it != end; ++it) {
                const Model::NodeList& brushes = it->second;
                Model::Node::accept(brushes.begin(), brushes.end(), visitor);
            }
        }
    }
}
