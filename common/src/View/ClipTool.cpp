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
#include "Model/BrushVertex.h"
#include "Model/PickResult.h"
#include "Model/World.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"
#include "View/MapDocument.h"
#include "View/Selection.h"

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType ClipTool::PointHit = Model::Hit::freeHitType();
        
        ClipTool::PointSnapper::~PointSnapper() {}
        
        bool ClipTool::PointSnapper::snap(const Vec3& point, Vec3& result) const {
            return doSnap(point, result);
        }
        
        ClipTool::PointStrategy::~PointStrategy() {}
        
        bool ClipTool::PointStrategy::computeThirdPoint(const Vec3& point1, const Vec3& point2, Vec3& point3) const {
            return doComputeThirdPoint(point1, point2, point3);
        }
        
        ClipTool::PointStrategyFactory::~PointStrategyFactory() {}
        ClipTool::PointStrategy* ClipTool::PointStrategyFactory::createStrategy() const {
            return doCreateStrategy();
        }
        
        ClipTool::PointStrategy* ClipTool::DefaultPointStrategyFactory::doCreateStrategy() const {
            return NULL;
        }
        
        ClipTool::ClipStrategy::~ClipStrategy() {}
        
        void ClipTool::ClipStrategy::pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            doPick(pickRay, camera, pickResult);
        }
        
        void ClipTool::ClipStrategy::render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) {
            doRender(renderContext, renderBatch, pickResult);
        }
        
        bool ClipTool::ClipStrategy::canClip() const {
            return doCanClip();
        }
        
        bool ClipTool::ClipStrategy::canAddPoint(const Vec3& point, const PointSnapper& snapper) const {
            return doCanAddPoint(point, snapper);
        }
        
        void ClipTool::ClipStrategy::addPoint(const Vec3& point, const PointSnapper& snapper, const PointStrategyFactory& factory) {
            assert(canAddPoint(point, snapper));
            return doAddPoint(point, snapper, factory);
        }
        
        void ClipTool::ClipStrategy::removeLastPoint() {
            doRemoveLastPoint();
        }
        
        bool ClipTool::ClipStrategy::beginDragPoint(const Model::PickResult& pickResult, Vec3& initialPosition) {
            return doBeginDragPoint(pickResult, initialPosition);
        }
        
        bool ClipTool::ClipStrategy::dragPoint(const Vec3& newPosition, const PointSnapper& snapper, Vec3& snappedPosition) {
            return doDragPoint(newPosition, snapper, snappedPosition);
        }
        
        bool ClipTool::ClipStrategy::setFace(const Model::BrushFace* face) {
            return doSetFace(face);
        }
        
        void ClipTool::ClipStrategy::reset() {
            doReset();
        }
        
        size_t ClipTool::ClipStrategy::getPoints(Vec3& point1, Vec3& point2, Vec3& point3) const {
            return doGetPoints(point1, point2, point3);
        }
        
        class ClipTool::PointClipStrategy : public ClipTool::ClipStrategy {
        private:
            Vec3 m_points[3];
            size_t m_numPoints;
            size_t m_dragIndex;
            PointStrategy* m_pointStrategy;
        public:
            PointClipStrategy() :
            m_numPoints(0),
            m_dragIndex(4),
            m_pointStrategy(NULL) {}
            
            ~PointClipStrategy() {
                resetPointStrategy();
            }
            
            void doPick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
                for (size_t i = 0; i < m_numPoints; ++i) {
                    const Vec3& point = m_points[i];
                    const FloatType distance = camera.pickPointHandle(pickRay, point, pref(Preferences::HandleRadius));
                    if (!Math::isnan(distance)) {
                        const Vec3 hitPoint = pickRay.pointAtDistance(distance);
                        pickResult.addHit(Model::Hit(PointHit, distance, hitPoint, i));
                    }
                }
            }
            
            void doRender(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) {
                renderPoints(renderContext, renderBatch);
                renderHighlight(renderContext, renderBatch, pickResult);
            }
            
            bool doCanClip() const {
                if (m_numPoints < 2)
                    return false;
                if (m_numPoints == 2 && m_pointStrategy == NULL)
                    return false;
                if (m_numPoints == 2 && m_pointStrategy != NULL) {
                    Vec3 point3;
                    if (!m_pointStrategy->computeThirdPoint(m_points[0], m_points[1], point3))
                        return false;
                }
                return true;
            }
            
            bool doCanAddPoint(const Vec3& point, const PointSnapper& snapper) const {
                Vec3 snapped;
                if (!snapper.snap(point, snapped))
                    return false;
                if (m_numPoints == 2 && linearlyDependent(m_points[0], m_points[1], snapped))
                    return false;
                return true;
            }
            
            void doAddPoint(const Vec3& point, const PointSnapper& snapper, const PointStrategyFactory& factory) {
                if (m_numPoints == 1)
                    m_pointStrategy = factory.createStrategy();
                
                Vec3 snapped;
                CHECK_BOOL(snapper.snap(point, snapped));
                
                m_points[m_numPoints] = snapped;
                ++m_numPoints;
            }
            
            void doRemoveLastPoint() {
                if (m_numPoints > 0) {
                    --m_numPoints;
                    if (m_numPoints < 2)
                        resetPointStrategy();
                }
            }
            
            bool doBeginDragPoint(const Model::PickResult& pickResult, Vec3& initialPosition) {
                const Model::Hit& hit = pickResult.query().type(PointHit).occluded().first();
                if (!hit.isMatch())
                    return false;
                m_dragIndex = hit.target<size_t>();
                initialPosition = m_points[m_dragIndex];
                return true;
            }
            
            bool doDragPoint(const Vec3& newPosition, const PointSnapper& snapper, Vec3& snappedPosition) {
                assert(m_dragIndex < m_numPoints);
                
                if (!snapper.snap(newPosition, snappedPosition))
                    return false;
                
                if (m_numPoints == 2 && linearlyDependent(m_points[0], m_points[1], snappedPosition))
                    return false;
                
                m_points[m_dragIndex] = snappedPosition;
                return true;
            }
            
            bool doSetFace(const Model::BrushFace* face) {
                return false;
            }
            
            void doReset() {
                m_numPoints = 0;
                resetPointStrategy();
            }
            
            size_t doGetPoints(Vec3& point1, Vec3& point2, Vec3& point3) const {
                switch (m_numPoints) {
                    case 0:
                        return 0;
                    case 1:
                        point1 = m_points[0];
                        return 1;
                    case 2:
                        point1 = m_points[0];
                        point2 = m_points[1];
                        if (m_pointStrategy != NULL && m_pointStrategy->computeThirdPoint(point1, point2, point3))
                            return 3;
                        return 2;
                    case 3:
                        point1 = m_points[0];
                        point2 = m_points[1];
                        point3 = m_points[2];
                        return 3;
                    default:
                        assert(false);
                }
            }
        private:
            void resetPointStrategy() {
                delete m_pointStrategy;
                m_pointStrategy = NULL;
            }
            
            void renderPoints(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::HandleColor));
                
                if (m_numPoints > 1) {
                    renderService.renderLine(m_points[0], m_points[1]);
                    if (m_numPoints > 2) {
                        renderService.renderLine(m_points[1], m_points[2]);
                        renderService.renderLine(m_points[2], m_points[0]);
                    }
                }
                
                renderService.setForegroundColor(pref(Preferences::HandleColor));
                renderService.setBackgroundColor(pref(Preferences::InfoOverlayBackgroundColor));
                
                for (size_t i = 0; i < m_numPoints; ++i) {
                    const Vec3& point = m_points[i];
                    renderService.renderPointHandle(point);
                    
                    StringStream str;
                    str << (i+1) << ": " << point.asString();
                    
                    renderService.renderStringOnTop(str.str(), point);
                }
            }
            
            void renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) {
                if (m_dragIndex < m_numPoints) {
                    renderHighlight(renderContext, renderBatch, m_dragIndex);
                } else {
                    const Model::Hit& hit = pickResult.query().type(PointHit).occluded().first();
                    if (hit.isMatch()) {
                        const size_t index = hit.target<size_t>();
                        renderHighlight(renderContext, renderBatch, index);
                    }
                }
            }
            
            void renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const size_t index) {
                assert(index < m_numPoints);
                
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
                renderService.renderPointHandleHighlight(m_points[index]);
            }
        };
        
        class ClipTool::FaceClipStrategy : public ClipTool::ClipStrategy {
        private:
            const Model::BrushFace* m_face;
        public:
            FaceClipStrategy() :
            m_face(NULL) {}
        private:
            void doPick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {}
            
            void doRender(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) {
                if (m_face != NULL) {
                    Renderer::RenderService renderService(renderContext, renderBatch);
                    renderService.setForegroundColor(pref(Preferences::HandleColor));
                    
                    const Model::BrushVertexList& vertices = m_face->vertices();
                    
                    Vec3f::List positions;
                    positions.reserve(vertices.size());
                    
                    Model::BrushVertexList::const_iterator it, end;
                    for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                        const Model::BrushVertex* vertex = *it;
                        positions.push_back(vertex->position);
                    }
                    
                    renderService.renderPolygonOutline(positions);
                }
            }
            
            bool doCanClip() const { return m_face != NULL; }
            bool doCanAddPoint(const Vec3& point, const PointSnapper& snapper) const { return false; }
            void doAddPoint(const Vec3& point, const PointSnapper& snapper, const PointStrategyFactory& factory) {}
            void doRemoveLastPoint() {}
            bool doBeginDragPoint(const Model::PickResult& pickResult, Vec3& initialPosition ) { return false; }
            bool doDragPoint(const Vec3& newPosition, const PointSnapper& snapper, Vec3& snappedPosition) { return false; }
            
            bool doSetFace(const Model::BrushFace* face) {
                assert(face != NULL);
                m_face = face;
                return true; }
            
            void doReset() {
                m_face = NULL;
            }
            
            size_t doGetPoints(Vec3& point1, Vec3& point2, Vec3& point3) const {
                if (m_face == NULL)
                    return 0;
                
                const Model::BrushFace::Points& points = m_face->points();
                point1 = points[0];
                point2 = points[1];
                point3 = points[2];
                return 3;
            }
        };
        
        ClipTool::ClipTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_clipSide(ClipSide_Front),
        m_strategy(NULL),
        m_remainingBrushRenderer(new Renderer::BrushRenderer(false)),
        m_clippedBrushRenderer(new Renderer::BrushRenderer(true)),
        m_ignoreNotifications(false) {}
        
        ClipTool::~ClipTool() {
            delete m_strategy;
            delete m_remainingBrushRenderer;
            delete m_clippedBrushRenderer;
            MapUtils::clearAndDelete(m_frontBrushes);
            MapUtils::clearAndDelete(m_backBrushes);
        }
        
        void ClipTool::toggleSide() {
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
        
        void ClipTool::resetSide() {
            m_clipSide = ClipSide_Front;
            update();
        }
        
        void ClipTool::pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            if (m_strategy != NULL)
                m_strategy->pick(pickRay, camera, pickResult);
        }
        
        void ClipTool::render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) {
            renderBrushes(renderContext, renderBatch);
            renderStrategy(renderContext, renderBatch, pickResult);
        }
        
        void ClipTool::renderBrushes(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_remainingBrushRenderer->setFaceColor(pref(Preferences::FaceColor));
            m_remainingBrushRenderer->setEdgeColor(pref(Preferences::SelectedEdgeColor));
            m_remainingBrushRenderer->setShowEdges(true);
            m_remainingBrushRenderer->setShowOccludedEdges(true);
            m_remainingBrushRenderer->setOccludedEdgeColor(pref(Preferences::OccludedSelectedEdgeColor));
            m_remainingBrushRenderer->setTint(true);
            m_remainingBrushRenderer->setTintColor(pref(Preferences::SelectedFaceColor));
            m_remainingBrushRenderer->render(renderContext, renderBatch);
            
            m_clippedBrushRenderer->setFaceColor(pref(Preferences::FaceColor));
            m_clippedBrushRenderer->setShowEdges(false);
            m_clippedBrushRenderer->setTint(false);
            m_clippedBrushRenderer->setTransparencyAlpha(0.5f);
            m_clippedBrushRenderer->render(renderContext, renderBatch);
        }
        
        void ClipTool::renderStrategy(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) {
            if (m_strategy != NULL)
                m_strategy->render(renderContext, renderBatch, pickResult);
        }
        
        bool ClipTool::canClip() const {
            return m_strategy != NULL && m_strategy->canClip();
        }
        
        void ClipTool::performClip() {
            assert(canClip());
            
            MapDocumentSPtr document = lock(m_document);
            const Transaction transaction(document, "Clip Brushes");
            
            // need to make a copies here so that we are not affected by the deselection
            const Model::ParentChildrenMap toAdd = clipBrushes();
            const Model::NodeList toRemove = document->selectedNodes().nodes();
            const Model::NodeList addedNodes = document->addNodes(toAdd);
            
            document->deselectAll();
            document->removeNodes(toRemove);
            document->select(addedNodes);
        }
        
        Model::ParentChildrenMap ClipTool::clipBrushes() {
            Model::ParentChildrenMap result;
            if (!m_frontBrushes.empty()) {
                if (keepFrontBrushes()) {
                    result.insert(m_frontBrushes.begin(), m_frontBrushes.end());
                    m_frontBrushes.clear();
                } else {
                    MapUtils::clearAndDelete(m_frontBrushes);
                }
            }
            
            if (!m_backBrushes.empty()) {
                if (keepBackBrushes()) {
                    result.insert(m_backBrushes.begin(), m_backBrushes.end());
                    m_backBrushes.clear();
                } else {
                    MapUtils::clearAndDelete(m_backBrushes);
                }
            }
            
            reset();
            return result;
        }
        
        Vec3 ClipTool::defaultClipPointPos() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectionBounds().center();
        }
        
        bool ClipTool::canAddPoint(const Vec3& point, const PointSnapper& snapper) const {
            return m_strategy == NULL || m_strategy->canAddPoint(point, snapper);
        }
        
        void ClipTool::addPoint(const Vec3& point, const PointSnapper& snapper, const PointStrategyFactory& factory) {
            assert(canAddPoint(point, snapper));
            if (m_strategy == NULL)
                m_strategy = new PointClipStrategy();
            
            m_strategy->addPoint(point, snapper, factory);
            update();
        }
        
        void ClipTool::removeLastPoint() {
            if (m_strategy != NULL) {
                m_strategy->removeLastPoint();
                update();
            }
        }
        
        bool ClipTool::beginDragPoint(const Model::PickResult& pickResult, Vec3& initialPosition) {
            if (m_strategy == NULL)
                return false;
            return m_strategy->beginDragPoint(pickResult, initialPosition);
        }
        
        bool ClipTool::dragPoint(const Vec3& newPosition, const PointSnapper& snapper, Vec3& snappedPosition) {
            assert(m_strategy != NULL);
            return m_strategy->dragPoint(newPosition, snapper, snappedPosition);
        }
        
        void ClipTool::setFace(const Model::BrushFace* face) {
            delete m_strategy;
            m_strategy = new FaceClipStrategy();
            m_strategy->setFace(face);
            update();
        }
        
        bool ClipTool::reset() {
            const bool result = (m_strategy != NULL);
            if (m_strategy != NULL)
                resetStrategy();
            resetSide();
            return result;
        }
        
        void ClipTool::resetStrategy() {
            delete m_strategy;
            m_strategy = NULL;
        }
        
        void ClipTool::update() {
            clearRenderers();
            clearBrushes();
            
            updateBrushes();
            updateRenderers();
            
            refreshViews();
        }
        
        void ClipTool::clearBrushes() {
            MapUtils::clearAndDelete(m_frontBrushes);
            MapUtils::clearAndDelete(m_backBrushes);
        }
        
        void ClipTool::updateBrushes() {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushList& brushes = document->selectedNodes().brushes();
            const BBox3& worldBounds = document->worldBounds();
            
            if (canClip()) {
                Vec3 point1, point2, point3;
                const size_t numPoints = m_strategy->getPoints(point1, point2, point3);
                assert(numPoints == 3);
                
                Model::World* world = document->world();
                Model::BrushList::const_iterator bIt, bEnd;
                for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                    Model::Brush* brush = *bIt;
                    Model::Node* parent = brush->parent();
                    
                    Model::BrushFace* frontFace = world->createFace(point1, point2, point3, document->currentTextureName());
                    Model::BrushFace* backFace = world->createFace(point1, point3, point2, document->currentTextureName());
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
        
        bool ClipTool::keepFrontBrushes() const {
            return m_clipSide != ClipSide_Back;
        }
        
        bool ClipTool::keepBackBrushes() const {
            return m_clipSide != ClipSide_Front;
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
            if (!m_ignoreNotifications)
                update();
        }
        
        void ClipTool::nodesWillChange(const Model::NodeList& nodes) {
            if (!m_ignoreNotifications)
                update();
        }
        
        void ClipTool::nodesDidChange(const Model::NodeList& nodes) {
            if (!m_ignoreNotifications)
                update();
        }
    }
}
