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

#include "ClipTool.h"

#include "CollectionUtils.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Macros.h"
#include "Model/AssortNodesVisitor.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
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
        
        ClipTool::ClipStrategy::~ClipStrategy() {}
        
        void ClipTool::ClipStrategy::pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            doPick(pickRay, camera, pickResult);
        }
        
        void ClipTool::ClipStrategy::render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) {
            doRender(renderContext, renderBatch, pickResult);
        }
        
        void ClipTool::ClipStrategy::renderFeedback(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& point) const {
            doRenderFeedback(renderContext, renderBatch, point);
        }

        bool ClipTool::ClipStrategy::computeThirdPoint(Vec3& point) const {
            return doComputeThirdPoint(point);
        }

        bool ClipTool::ClipStrategy::canClip() const {
            return doCanClip();
        }
        
        bool ClipTool::ClipStrategy::hasPoints() const {
            return doHasPoints();
        }

        bool ClipTool::ClipStrategy::canAddPoint(const Vec3& point) const {
            return doCanAddPoint(point);
        }
        
        void ClipTool::ClipStrategy::addPoint(const Vec3& point, const Vec3::List& helpVectors) {
            assert(canAddPoint(point));
            return doAddPoint(point, helpVectors);
        }
        
        bool ClipTool::ClipStrategy::canRemoveLastPoint() const {
            return doCanRemoveLastPoint();
        }

        void ClipTool::ClipStrategy::removeLastPoint() {
            doRemoveLastPoint();
        }
        
        bool ClipTool::ClipStrategy::canDragPoint(const Model::PickResult& pickResult, Vec3& initialPosition) const {
            return doCanDragPoint(pickResult, initialPosition);
        }

        void ClipTool::ClipStrategy::beginDragPoint(const Model::PickResult& pickResult) {
            doBeginDragPoint(pickResult);
        }
        
        void ClipTool::ClipStrategy::beginDragLastPoint() {
            doBeginDragLastPoint();
        }

        bool ClipTool::ClipStrategy::dragPoint(const Vec3& newPosition, const Vec3::List& helpVectors) {
            return doDragPoint(newPosition, helpVectors);
        }
        
        void ClipTool::ClipStrategy::endDragPoint() {
            doEndDragPoint();
        }

        void ClipTool::ClipStrategy::cancelDragPoint() {
            doCancelDragPoint();
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
            struct ClipPoint {
                Vec3 point;
                Vec3::List helpVectors;
                
                ClipPoint() {}
                
                ClipPoint(const Vec3& i_point, const Vec3::List& i_helpVectors) :
                point(i_point),
                helpVectors(i_helpVectors) {}
            };
            
            ClipPoint m_points[3];
            size_t m_numPoints;
            size_t m_dragIndex;
            ClipPoint m_originalPoint;
        public:
            PointClipStrategy() :
            m_numPoints(0),
            m_dragIndex(4) {}
        private:
            void doPick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const override {
                for (size_t i = 0; i < m_numPoints; ++i) {
                    const Vec3& point = m_points[i].point;
                    const FloatType distance = camera.pickPointHandle(pickRay, point, pref(Preferences::HandleRadius));
                    if (!Math::isnan(distance)) {
                        const Vec3 hitPoint = pickRay.pointAtDistance(distance);
                        pickResult.addHit(Model::Hit(PointHit, distance, hitPoint, i));
                    }
                }
            }
            
            void doRender(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) override {
                renderPoints(renderContext, renderBatch);
                renderHighlight(renderContext, renderBatch, pickResult);
            }
            
            void doRenderFeedback(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& point) const override {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
                renderService.renderHandle(point);
            }

            bool doComputeThirdPoint(Vec3& point) const override {
                ensure(m_numPoints == 2, "invalid numPoints");
                point = m_points[1].point + 128.0 * computeHelpVector();
                return !linearlyDependent(m_points[0].point, m_points[1].point, point);
            }

            Vec3 computeHelpVector() const {
                size_t counts[3];
                counts[0] = counts[1] = counts[2] = 0;
                
                const Vec3::List helpVectors = combineHelpVectors();
                for (size_t i = 0; i < std::min(m_numPoints, helpVectors.size()); ++i) {
                    const Math::Axis::Type axis = helpVectors[i].firstComponent();
                    counts[axis]++;
                }
                
                if (counts[0] > counts[1] && counts[0] > counts[2])
                    return Vec3::PosX;
                if (counts[1] > counts[0] && counts[1] > counts[2])
                    return Vec3::PosY;
                if (counts[2] > counts[0] && counts[2] > counts[1])
                    return Vec3::PosZ;

                // two counts are equal
                // prefer the Z axis if possible:
                if (counts[2] == counts[0] || counts[2] == counts[1])
                    return Vec3::PosZ;
                // Z axis cannot win, so X and Y axis are a tie, prefer the X axis:
                return Vec3::PosX;
            }
            
            Vec3::List combineHelpVectors() const {
                Vec3::List result;
                for (size_t i = 0; i < m_numPoints; ++i) {
                    const Vec3::List& helpVectors = m_points[i].helpVectors;
                    VectorUtils::append(result, helpVectors);
                }
                
                VectorUtils::sortAndRemoveDuplicates(result);
                return result;
            }
            
            bool doCanClip() const override {
                if (m_numPoints < 2)
                    return false;
                if (m_numPoints == 2) {
                    Vec3 point3;
                    if (!computeThirdPoint(point3))
                        return false;
                }
                return true;
            }
            
            bool doHasPoints() const override {
                return m_numPoints > 0;
            }

            bool doCanAddPoint(const Vec3& point) const override {
                if (m_numPoints == 3)
                    return false;
                
                if (m_numPoints == 2 && linearlyDependent(m_points[0].point, m_points[1].point, point))
                    return false;
                return true;
            }
            
            void doAddPoint(const Vec3& point, const Vec3::List& helpVectors) override {
                m_points[m_numPoints] = ClipPoint(point, helpVectors);
                ++m_numPoints;
            }
            
            bool doCanRemoveLastPoint() const override {
                return m_numPoints > 0;
            }

            void doRemoveLastPoint() override {
                ensure(canRemoveLastPoint(), "can't remove last point");
                --m_numPoints;
            }
            
            bool doCanDragPoint(const Model::PickResult& pickResult, Vec3& initialPosition) const override {
                const Model::Hit& hit = pickResult.query().type(PointHit).occluded().first();
                if (!hit.isMatch())
                    return false;
                size_t index = hit.target<size_t>();
                initialPosition = m_points[index].point;
                return true;
            }
            
            void doBeginDragPoint(const Model::PickResult& pickResult) override {
                const Model::Hit& hit = pickResult.query().type(PointHit).occluded().first();
                assert(hit.isMatch());
                m_dragIndex = hit.target<size_t>();
                m_originalPoint = m_points[m_dragIndex];
            }
            
            void doBeginDragLastPoint() override {
                ensure(m_numPoints > 0, "invalid numPoints");
                m_dragIndex = m_numPoints - 1;
                m_originalPoint = m_points[m_dragIndex];
            }

            bool doDragPoint(const Vec3& newPosition, const Vec3::List& helpVectors) override {
                ensure(m_dragIndex < m_numPoints, "drag index out of range");
                
                if (m_numPoints == 2 && linearlyDependent(m_points[0].point, m_points[1].point, newPosition))
                    return false;

                if (m_numPoints == 3) {
                    size_t index0, index1;
                    switch (m_dragIndex) {
                        case 0:
                            index0 = 1;
                            index1 = 2;
                            break;
                        case 1:
                            index0 = 0;
                            index1 = 2;
                            break;
                        case 2:
                        default:
                            index0 = 0;
                            index1 = 1;
                            break;
                    }
                    
                    if (linearlyDependent(m_points[index0].point, m_points[index1].point, newPosition))
                        return false;
                }
                
                if (helpVectors.empty())
                    m_points[m_dragIndex] = ClipPoint(newPosition, m_points[m_dragIndex].helpVectors);
                else
                    m_points[m_dragIndex] = ClipPoint(newPosition, helpVectors);
                return true;
            }
            
            void doEndDragPoint() override {
                m_dragIndex = 4;
            }

            void doCancelDragPoint() override {
                ensure(m_dragIndex < m_numPoints, "drag index out of range");
                m_points[m_dragIndex] = m_originalPoint;
                m_dragIndex = 4;
            }

            bool doSetFace(const Model::BrushFace* face) override {
                return false;
            }
            
            void doReset() override {
                m_numPoints = 0;
            }
            
            size_t doGetPoints(Vec3& point1, Vec3& point2, Vec3& point3) const override {
                switch (m_numPoints) {
                    case 0:
                        return 0;
                    case 1:
                        point1 = m_points[0].point;
                        return 1;
                    case 2:
                        point1 = m_points[0].point;
                        point2 = m_points[1].point;
                        if (computeThirdPoint(point3))
                            return 3;
                        return 2;
                    case 3:
                        point1 = m_points[0].point;
                        point2 = m_points[1].point;
                        point3 = m_points[2].point;
                        return 3;
                    default:
                        ensure(false, "invalid numPoints");
                        return 0;
                }
            }
        private:
            void renderPoints(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
                renderService.setShowOccludedObjects();
                
                if (m_numPoints > 1) {
                    renderService.renderLine(m_points[0].point, m_points[1].point);
                    if (m_numPoints > 2) {
                        renderService.renderLine(m_points[1].point, m_points[2].point);
                        renderService.renderLine(m_points[2].point, m_points[0].point);
                    }
                }
                
                renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
                renderService.setBackgroundColor(pref(Preferences::InfoOverlayBackgroundColor));
                
                for (size_t i = 0; i < m_numPoints; ++i) {
                    const Vec3& point = m_points[i].point;
                    renderService.renderHandle(point);
                    
                    StringStream str;
                    str << (i+1) << ": " << point.asString();
                    
                    renderService.renderString(str.str(), point);
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
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
                renderService.renderHandleHighlight(m_points[index].point);
            }
        };
        
        class ClipTool::FaceClipStrategy : public ClipTool::ClipStrategy {
        private:
            const Model::BrushFace* m_face;
        public:
            FaceClipStrategy() :
            m_face(nullptr) {}
        private:
            void doPick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const override {}
            
            void doRender(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) override {
                if (m_face != nullptr) {
                    Renderer::RenderService renderService(renderContext, renderBatch);
                    
                    const Model::BrushFace::VertexList vertices = m_face->vertices();
                    
                    Vec3f::List positions;
                    positions.reserve(vertices.size());
                    
                    for (const Model::BrushVertex* vertex : vertices)
                        positions.push_back(vertex->position());
                    
                    renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
                    renderService.renderPolygonOutline(positions);

                    renderService.setForegroundColor(pref(Preferences::ClipFaceColor));
                    renderService.renderFilledPolygon(positions);
                }
            }
            
            void doRenderFeedback(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& point) const override {}

            Vec3 doGetHelpVector() const { return Vec3::Null; }
            
            bool doComputeThirdPoint(Vec3& point) const override { return false; }

            bool doCanClip() const override { return m_face != nullptr; }
            bool doHasPoints() const override { return false; }
            bool doCanAddPoint(const Vec3& point) const override { return false; }
            void doAddPoint(const Vec3& point, const Vec3::List& helpVectors) override {}
            bool doCanRemoveLastPoint() const override { return false; }
            void doRemoveLastPoint() override {}
            
            bool doCanDragPoint(const Model::PickResult& pickResult, Vec3& initialPosition) const override { return false; }
            void doBeginDragPoint(const Model::PickResult& pickResult) override {}
            void doBeginDragLastPoint() override {}
            bool doDragPoint(const Vec3& newPosition, const Vec3::List& helpVectors) override { return false; }
            void doEndDragPoint() override {}
            void doCancelDragPoint() override {}
            
            bool doSetFace(const Model::BrushFace* face) override {
                ensure(face != nullptr, "face is null");
                m_face = face;
                return true; }
            
            void doReset() override {
                m_face = nullptr;
            }
            
            size_t doGetPoints(Vec3& point1, Vec3& point2, Vec3& point3) const override {
                if (m_face == nullptr)
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
        m_strategy(nullptr),
        m_remainingBrushRenderer(new Renderer::BrushRenderer(false)),
        m_clippedBrushRenderer(new Renderer::BrushRenderer(true)),
        m_ignoreNotifications(false),
        m_dragging(false) {}
        
        ClipTool::~ClipTool() {
            delete m_strategy;
            delete m_remainingBrushRenderer;
            delete m_clippedBrushRenderer;
            MapUtils::clearAndDelete(m_frontBrushes);
            MapUtils::clearAndDelete(m_backBrushes);
        }
        
        const Grid& ClipTool::grid() const {
            return lock(m_document)->grid();
        }

        void ClipTool::toggleSide() {
            if (canClip()) {
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
        }
        
        void ClipTool::pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            if (m_strategy != nullptr)
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
            m_clippedBrushRenderer->setEdgeColor(Color(pref(Preferences::EdgeColor), 0.5f));
            m_clippedBrushRenderer->setShowEdges(true);
            m_clippedBrushRenderer->setTint(false);
            m_clippedBrushRenderer->setTransparencyAlpha(0.5f);
            m_clippedBrushRenderer->render(renderContext, renderBatch);
        }
        
        void ClipTool::renderStrategy(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) {
            if (m_strategy != nullptr)
                m_strategy->render(renderContext, renderBatch, pickResult);
        }
        
        void ClipTool::renderFeedback(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& point) const {
            if (m_strategy != nullptr) {
                m_strategy->renderFeedback(renderContext, renderBatch, point);
            } else {
                PointClipStrategy().renderFeedback(renderContext, renderBatch, point);
            }
        }

        bool ClipTool::canClip() const {
            return m_strategy != nullptr && m_strategy->canClip();
        }

        void ClipTool::performClip() {
            if (!m_dragging && canClip()) {
                const TemporarilySetBool ignoreNotifications(m_ignoreNotifications);
                MapDocumentSPtr document = lock(m_document);
                const Transaction transaction(document, "Clip Brushes");

                // need to make a copies here so that we are not affected by the deselection
                const Model::ParentChildrenMap toAdd = clipBrushes();
                const Model::NodeList toRemove = document->selectedNodes().nodes();
                const Model::NodeList addedNodes = document->addNodes(toAdd);

                document->deselectAll();
                document->removeNodes(toRemove);
                document->select(addedNodes);

                update();
            }
        }
        
        Model::ParentChildrenMap ClipTool::clipBrushes() {
            Model::ParentChildrenMap result;
            if (!m_frontBrushes.empty()) {
                if (keepFrontBrushes()) {
                    MapUtils::merge(result, m_frontBrushes);
                    m_frontBrushes.clear();
                } else {
                    MapUtils::clearAndDelete(m_frontBrushes);
                }
            }
            
            if (!m_backBrushes.empty()) {
                if (keepBackBrushes()) {
                    MapUtils::merge(result, m_backBrushes);
                    m_backBrushes.clear();
                } else {
                    MapUtils::clearAndDelete(m_backBrushes);
                }
            }
            
            resetStrategy();
            return result;
        }
        
        Vec3 ClipTool::defaultClipPointPos() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectionBounds().center();
        }
        
        bool ClipTool::canAddPoint(const Vec3& point) const {
            return m_strategy == nullptr || m_strategy->canAddPoint(point);
        }
        
        bool ClipTool::hasPoints() const {
            return m_strategy != nullptr && m_strategy->hasPoints();
        }

        void ClipTool::addPoint(const Vec3& point, const Vec3::List& helpVectors) {
            assert(canAddPoint(point));
            if (m_strategy == nullptr)
                m_strategy = new PointClipStrategy();
            
            m_strategy->addPoint(point, helpVectors);
            update();
        }
        
        bool ClipTool::canRemoveLastPoint() const {
            return m_strategy != nullptr && m_strategy->canRemoveLastPoint();
        }

        bool ClipTool::removeLastPoint() {
            if (canRemoveLastPoint()) {
                m_strategy->removeLastPoint();
                update();
                return true;
            }
            return false;
        }

        bool ClipTool::beginDragPoint(const Model::PickResult& pickResult, Vec3& initialPosition) {
            assert(!m_dragging);
            if (m_strategy == nullptr)
                return false;
            if (!m_strategy->canDragPoint(pickResult, initialPosition))
                return false;
            m_strategy->beginDragPoint(pickResult);
            m_dragging = true;
            return true;
        }
        
        void ClipTool::beginDragLastPoint() {
            assert(!m_dragging);
            ensure(m_strategy != nullptr, "strategy is null");
            m_strategy->beginDragLastPoint();
            m_dragging = true;
        }

        bool ClipTool::dragPoint(const Vec3& newPosition, const Vec3::List& helpVectors) {
            assert(m_dragging);
            ensure(m_strategy != nullptr, "strategy is null");
            if (!m_strategy->dragPoint(newPosition, helpVectors))
                return false;
            update();
            return true;
        }
        
        void ClipTool::endDragPoint() {
            assert(m_dragging);
            ensure(m_strategy != nullptr, "strategy is null");
            m_strategy->endDragPoint();
            m_dragging = false;
            refreshViews();
        }

        void ClipTool::cancelDragPoint() {
            assert(m_dragging);
            ensure(m_strategy != nullptr, "strategy is null");
            m_strategy->cancelDragPoint();
            m_dragging = false;
            refreshViews();
        }

        void ClipTool::setFace(const Model::BrushFace* face) {
            delete m_strategy;
            m_strategy = new FaceClipStrategy();
            m_strategy->setFace(face);
            update();
        }
        
        bool ClipTool::reset() {
            if (m_strategy != nullptr) {
                resetStrategy();
                return true;
            }
            return false;
        }
        
        void ClipTool::resetStrategy() {
            delete m_strategy;
            m_strategy = nullptr;
            update();
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
            const auto& brushes = document->selectedNodes().brushes();
            const auto& worldBounds = document->worldBounds();
            
            if (canClip()) {
                Vec3 point1, point2, point3;
                const auto numPoints = m_strategy->getPoints(point1, point2, point3);
                unused(numPoints);
                ensure(numPoints == 3, "invalid number of points");
                
                auto* world = document->world();
                for (auto* brush : brushes) {
                    auto* parent = brush->parent();
                    
                    auto* frontFace = world->createFace(point1, point2, point3, document->currentTextureName());
                    auto* backFace = world->createFace(point1, point3, point2, document->currentTextureName());
                    setFaceAttributes(brush->faces(), frontFace, backFace);
                    
                    auto* frontBrush = brush->clone(worldBounds);
                    if (frontBrush->clip(worldBounds, frontFace))
                        m_frontBrushes[parent].push_back(frontBrush);
                    else
                        delete frontBrush;
                    
                    auto* backBrush = brush->clone(worldBounds);
                    if (backBrush->clip(worldBounds, backFace))
                        m_backBrushes[parent].push_back(backBrush);
                    else
                        delete backBrush;
                }
            } else {
                for (auto* brush : brushes) {
                    auto* parent = brush->parent();
                    auto* frontBrush = brush->clone(worldBounds);
                    m_frontBrushes[parent].push_back(frontBrush);
                }
            }
        }
        
        void ClipTool::setFaceAttributes(const Model::BrushFaceList& faces, Model::BrushFace* frontFace, Model::BrushFace* backFace) const {
            ensure(!faces.empty(), "no faces");
            
            auto faceIt = std::begin(faces);
            const auto faceEnd = std::end(faces);
            const auto* bestFrontFace = *faceIt++;
            const auto* bestBackFace = bestFrontFace;
            
            while (faceIt != faceEnd) {
                const auto* face = *faceIt;
                
                const auto bestFrontDiff = bestFrontFace->boundary().normal - frontFace->boundary().normal;
                const auto frontDiff = face->boundary().normal - frontFace->boundary().normal;
                if (frontDiff.squaredLength() < bestFrontDiff.squaredLength())
                    bestFrontFace = face;
                
                const auto bestBackDiff = bestBackFace->boundary().normal - backFace->boundary().normal;
                const auto backDiff = face->boundary().normal - backFace->boundary().normal;
                if (backDiff.squaredLength() < bestBackDiff.squaredLength())
                    bestBackFace = face;
                ++faceIt;
            }
            
            ensure(bestFrontFace != nullptr, "bestFrontFace is null");
            ensure(bestBackFace != nullptr, "bestBackFace is null");
            frontFace->setAttributes(bestFrontFace);
            backFace->setAttributes(bestBackFace);
        }
        
        void ClipTool::clearRenderers() {
            m_remainingBrushRenderer->clear();
            m_clippedBrushRenderer->clear();
        }
        
        void ClipTool::updateRenderers() {
            if (canClip()) {
                if (keepFrontBrushes())
                    addBrushesToRenderer(m_frontBrushes, m_remainingBrushRenderer);
                else
                    addBrushesToRenderer(m_frontBrushes, m_clippedBrushRenderer);
                
                if (keepBackBrushes())
                    addBrushesToRenderer(m_backBrushes, m_remainingBrushRenderer);
                else
                    addBrushesToRenderer(m_backBrushes, m_clippedBrushRenderer);
            } else {
                addBrushesToRenderer(m_frontBrushes, m_remainingBrushRenderer);
                addBrushesToRenderer(m_backBrushes, m_remainingBrushRenderer);
            }
        }
        
        void ClipTool::addBrushesToRenderer(const Model::ParentChildrenMap& map, Renderer::BrushRenderer* renderer) {
            Model::CollectBrushesVisitor collect;
            
            for (const auto& entry : map) {
                const Model::NodeList& brushes = entry.second;
                Model::Node::accept(std::begin(brushes), std::end(brushes), collect);
            }
            
            renderer->addBrushes(collect.brushes());
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
            resetStrategy();
            return true;
        }
        
        bool ClipTool::doDeactivate() {
            unbindObservers();

            delete m_strategy;
            m_strategy = nullptr;
            clearRenderers();
            clearBrushes();
            
            return true;
        }
        
        bool ClipTool::doRemove() {
            return removeLastPoint();
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
