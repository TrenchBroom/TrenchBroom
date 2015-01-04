/*
 Copyright (C) 2010-2014 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY {} without even the implied warranty of
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
#include "Model/NodeVisitor.h"
#include "Model/PickResult.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"
#include "Renderer/TextAnchor.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType ClipTool::HandleHit = Model::Hit::freeHitType();

        ClipTool::ClipTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_remainingBrushRenderer(new Renderer::BrushRenderer(false)),
        m_clippedBrushRenderer(new Renderer::BrushRenderer(true)){}
        
        ClipTool::~ClipTool() {
            clearClipResult();
            delete m_remainingBrushRenderer;
            delete m_clippedBrushRenderer;
        }

        bool ClipTool::canToggleClipSide() const {
            return m_clipper.numPoints() > 0;
        }
        
        void ClipTool::toggleClipSide() {
            m_clipper.toggleClipSide();
            updateBrushes();
        }
        
        bool ClipTool::canPerformClip() const {
            return m_clipper.numPoints() > 0;
        }
        
        void ClipTool::performClip() {
            MapDocumentSPtr document = lock(m_document);
            
            ClipResult clipResult;
            using std::swap;
            swap(clipResult, m_clipResult);

            // need to make a copies here so that we are not affected by the deselection
            const Model::NodeList toRemove = document->selectedNodes().nodes();
            Model::NodeList addedNodes;

            const Transaction transaction(document, "Clip Brushes");
            if (!clipResult.frontBrushes.empty() && m_clipper.keepFrontBrushes()) {
                const Model::NodeList addedFrontNodes = document->addNodes(clipResult.frontBrushes);
                VectorUtils::append(addedNodes, addedFrontNodes);
                clipResult.frontBrushes.clear();
            } else {
                MapUtils::clearAndDelete(clipResult.frontBrushes);
            }
            if (!clipResult.backBrushes.empty() && m_clipper.keepBackBrushes()) {
                const Model::NodeList addedBackNodes = document->addNodes(clipResult.backBrushes);
                VectorUtils::append(addedNodes, addedBackNodes);
                clipResult.backBrushes.clear();
            } else {
                MapUtils::clearAndDelete(clipResult.backBrushes);
            }

            document->deselectAll();
            document->removeNodes(toRemove);
            document->select(addedNodes);
            
            m_clipper.reset();
            updateBrushes();
        }
        
        bool ClipTool::canDeleteLastClipPoint() const {
            return m_clipper.numPoints() > 0;
        }
        
        void ClipTool::deleteLastClipPoint() {
            m_clipper.deleteLastClipPoint();
            updateBrushes();
        }

        void ClipTool::pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            const Vec3::List clipPoints = m_clipper.clipPointPositions();
            for (size_t i = 0; i < clipPoints.size(); ++i) {
                const FloatType distance = camera.pickPointHandle(pickRay, clipPoints[i], pref(Preferences::HandleRadius));
                if (!Math::isnan(distance)) {
                    const Vec3 hitPoint = pickRay.pointAtDistance(distance);
                    pickResult.addHit(Model::Hit(HandleHit, distance, hitPoint, i));
                }
            }
        }
        
        bool ClipTool::addClipPoint(const Vec3& clickPoint, const Model::BrushFace* face, const Renderer::Camera& camera) {
            assert(face != NULL);
            
            const Vec3 clipPoint = computeClipPoint(clickPoint, face->boundary());
            if (m_clipper.canAddClipPoint(clipPoint)) {
                m_clipper.addClipPoint(clipPoint, face, camera);
                updateBrushes();
                return true;
            }
            return false;
        }
        
        void ClipTool::setClipPoints(const Vec3& clickPoint, const Model::BrushFace* face, const Renderer::Camera& camera) {
            m_clipper.setPoints(clickPoint, face, camera);
            updateBrushes();
        }
        
        bool ClipTool::resetClipper() {
            if (m_clipper.numPoints() > 0) {
                m_clipper.reset();
                updateBrushes();
                return true;
            }
            return false;
        }
        
        bool ClipTool::beginDragClipPoint(const Model::PickResult& pickResult) {
            const Model::Hit& hit = pickResult.query().type(HandleHit).occluded().first();
            if (!hit.isMatch())
                return false;
            m_dragPointIndex = hit.target<size_t>();
            return true;
        }
        
        void ClipTool::dragClipPoint(const Vec3& hitPoint, const Model::BrushFace* face) {
            const Vec3 clipPoint = computeClipPoint(hitPoint, face->boundary());
            if (m_clipper.canUpdateClipPoint(m_dragPointIndex, clipPoint)) {
                m_clipper.updateClipPoint(m_dragPointIndex, clipPoint, face);
                updateBrushes();
            }
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
            
            const Vec3::List& points = m_clipper.clipPointPositions();
            if (points.size() > 1) {
                renderService.renderLine(pref(Preferences::HandleColor), points[0], points[1]);
                if (points.size() > 2) {
                    renderService.renderLine(pref(Preferences::HandleColor), points[1], points[2]);
                    renderService.renderLine(pref(Preferences::HandleColor), points[2], points[0]);
                }
            }
            
            for (size_t i = 0; i < points.size(); ++i) {
                const Vec3& point = points[i];
                renderService.renderPointHandle(point);
                
                StringStream str;
                str << i << ": " << point.asString();
                
                const Renderer::SimpleTextAnchor anchor(point, Renderer::TextAlignment::Bottom, Vec2f(0.0f, 10.0f));
                renderService.renderStringOnTop(pref(Preferences::HandleColor), pref(Preferences::InfoOverlayBackgroundColor), str.str(), anchor);
            }
        }
        
        void ClipTool::renderCurrentClipPoint(const Model::PickResult& pickResult, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            const Model::Hit& handleHit = pickResult.query().type(HandleHit).occluded().first();
            const Model::Hit& brushHit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
            
            if (handleHit.isMatch()) {
                const size_t index = handleHit.target<size_t>();
                renderHighlight(renderContext, renderBatch, index);
            } else if (brushHit.isMatch()) {
                const Model::BrushFace* face = brushHit.target<Model::BrushFace*>();
                const Vec3 point = computeClipPoint(brushHit.hitPoint(), face->boundary());
                
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.renderPointHandle(point);
            }
        }

        void ClipTool::renderDragHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderHighlight(renderContext, renderBatch, m_dragPointIndex);
        }

        void ClipTool::renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const size_t index) {
            const Vec3::List& points = m_clipper.clipPointPositions();
            assert(index < points.size());
            
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.renderPointHandleHighlight(points[index]);
        }

        bool ClipTool::doActivate() {
            MapDocumentSPtr document = lock(m_document);
            if (!document->selectedNodes().hasOnlyBrushes())
                return false;
            resetClipper();
            updateBrushes();
            bindObservers();
            return true;
        }

        bool ClipTool::doDeactivate() {
            clearClipResult();
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

        Vec3 ClipTool::computeClipPoint(const Vec3& point, const Plane3& boundary) const {
            MapDocumentSPtr document = lock(m_document);
            return document->grid().snap(point, boundary);
        }
        
        void ClipTool::updateBrushes() {
            clearClipResult();
            
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushList& brushes = document->selectedNodes().brushes();
            if (!brushes.empty())
                m_clipResult = m_clipper.clip(brushes, document);
            else
                m_clipper.reset();
            
            if (m_clipper.keepFrontBrushes())
                addBrushesToRenderer(m_clipResult.frontBrushes, m_remainingBrushRenderer);
            else
                addBrushesToRenderer(m_clipResult.frontBrushes, m_clippedBrushRenderer);
            
            if (m_clipper.keepBackBrushes())
                addBrushesToRenderer(m_clipResult.backBrushes, m_remainingBrushRenderer);
            else
                addBrushesToRenderer(m_clipResult.backBrushes, m_clippedBrushRenderer);
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

        void ClipTool::clearClipResult() {
            m_remainingBrushRenderer->clear();
            m_clippedBrushRenderer->clear();
            MapUtils::clearAndDelete(m_clipResult.frontBrushes);
            MapUtils::clearAndDelete(m_clipResult.backBrushes);
        }
    }
}
