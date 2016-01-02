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
        
        ClipTool::PointSnapper::~PointSnapper() {}
        
        bool ClipTool::PointSnapper::snap(const Vec3& point, Vec3& result) const {
            return doSnap(point, result);
        }
        
        ClipTool::ClipStrategy::~ClipStrategy() {}
        
        void ClipTool::ClipStrategy::pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            doPick(pickRay, camera, pickResult);
        }
        
        void ClipTool::ClipStrategy::render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) {
            doRender(renderContext, renderBatch, pickResult);
        }
        
        void ClipTool::ClipStrategy::renderFeedback(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& point, const PointSnapper& snapper) const {
            doRenderFeedback(renderContext, renderBatch, point, snapper);
        }

        bool ClipTool::ClipStrategy::computeThirdPoint(Vec3& point) const {
            return doComputeThirdPoint(point);
        }

        bool ClipTool::ClipStrategy::canClip() const {
            return doCanClip();
        }
        
        bool ClipTool::ClipStrategy::canAddPoint(const Vec3& point, const PointSnapper& snapper) const {
            return doCanAddPoint(point, snapper);
        }
        
        void ClipTool::ClipStrategy::addPoint(const Vec3& point, const PointSnapper& snapper, const Vec3::List& helpVectors) {
            assert(canAddPoint(point, snapper));
            return doAddPoint(point, snapper, helpVectors);
        }
        
        bool ClipTool::ClipStrategy::removeLastPoint() {
            return doRemoveLastPoint();
        }
        
        bool ClipTool::ClipStrategy::beginDragPoint(const Model::PickResult& pickResult, Vec3& initialPosition) {
            return doBeginDragPoint(pickResult, initialPosition);
        }
        
        bool ClipTool::ClipStrategy::dragPoint(const Vec3& newPosition, const PointSnapper& snapper, const Vec3::List& helpVectors, Vec3& snappedPosition) {
            return doDragPoint(newPosition, snapper, helpVectors, snappedPosition);
        }
        
        void ClipTool::ClipStrategy::endDragPoint() {
            doEndDragPoint();
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
        public:
            PointClipStrategy() :
            m_numPoints(0),
            m_dragIndex(4) {}
        private:
            void doPick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
                for (size_t i = 0; i < m_numPoints; ++i) {
                    const Vec3& point = m_points[i].point;
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
            
            void doRenderFeedback(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& point, const PointSnapper& snapper) const {
                Vec3 snapped;
                snapper.snap(point, snapped);
                
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
                renderService.renderPointHandle(snapped);
            }

            bool doComputeThirdPoint(Vec3& point) const {
                assert(m_numPoints == 2);
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
            
            bool doCanClip() const {
                if (m_numPoints < 2)
                    return false;
                if (m_numPoints == 2) {
                    Vec3 point3;
                    if (!computeThirdPoint(point3))
                        return false;
                }
                return true;
            }
            
            bool doCanAddPoint(const Vec3& point, const PointSnapper& snapper) const {
                if (m_numPoints == 3)
                    return false;
                
                Vec3 snapped;
                if (!snapper.snap(point, snapped))
                    return false;
                if (m_numPoints == 2 && linearlyDependent(m_points[0].point, m_points[1].point, snapped))
                    return false;
                return true;
            }
            
            void doAddPoint(const Vec3& point, const PointSnapper& snapper, const Vec3::List& helpVectors) {
                Vec3 snapped;
                assertResult(snapper.snap(point, snapped));

                m_points[m_numPoints] = ClipPoint(snapped, helpVectors);
                ++m_numPoints;
            }
            
            bool doRemoveLastPoint() {
                if (m_numPoints == 0)
                    return false;
                --m_numPoints;
                return true;
            }
            
            bool doBeginDragPoint(const Model::PickResult& pickResult, Vec3& initialPosition) {
                const Model::Hit& hit = pickResult.query().type(PointHit).occluded().first();
                if (!hit.isMatch())
                    return false;
                m_dragIndex = hit.target<size_t>();
                initialPosition = m_points[m_dragIndex].point;
                return true;
            }
            
            bool doDragPoint(const Vec3& newPosition, const PointSnapper& snapper, const Vec3::List& helpVectors, Vec3& snappedPosition) {
                assert(m_dragIndex < m_numPoints);
                
                if (!snapper.snap(newPosition, snappedPosition))
                    return false;
                
                if (m_numPoints == 2 && linearlyDependent(m_points[0].point, m_points[1].point, snappedPosition))
                    return false;

                if (helpVectors.empty())
                    m_points[m_dragIndex] = ClipPoint(snappedPosition, m_points[m_dragIndex].helpVectors);
                else
                    m_points[m_dragIndex] = ClipPoint(snappedPosition, helpVectors);
                return true;
            }
            
            void doEndDragPoint() {
                m_dragIndex = 4;
            }

            bool doSetFace(const Model::BrushFace* face) {
                return false;
            }
            
            void doReset() {
                m_numPoints = 0;
            }
            
            size_t doGetPoints(Vec3& point1, Vec3& point2, Vec3& point3) const {
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
                        assert(false);
                        return 0;
                }
            }
        private:
            void renderPoints(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
                
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
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
                renderService.renderPointHandleHighlight(m_points[index].point);
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
                    
                    const Model::BrushFace::VertexList vertices = m_face->vertices();
                    
                    Vec3f::List positions;
                    positions.reserve(vertices.size());
                    
                    Model::BrushFace::VertexList::const_iterator it, end;
                    for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                        const Model::BrushVertex* vertex = *it;
                        positions.push_back(vertex->position());
                    }
                    
                    renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
                    renderService.renderPolygonOutline(positions);

                    renderService.setForegroundColor(pref(Preferences::ClipFaceColor));
                    renderService.renderFilledPolygon(positions);
                }
            }
            
            void doRenderFeedback(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& point, const PointSnapper& snapper) const {}

            Vec3 doGetHelpVector() const { return Vec3::Null; }
            
            bool doComputeThirdPoint(Vec3& point) const { return false; }

            bool doCanClip() const { return m_face != NULL; }
            bool doCanAddPoint(const Vec3& point, const PointSnapper& snapper) const { return false; }
            void doAddPoint(const Vec3& point, const PointSnapper& snapper, const Vec3::List& helpVectors) {}
            bool doRemoveLastPoint() { return false; }
            bool doBeginDragPoint(const Model::PickResult& pickResult, Vec3& initialPosition ) { return false; }
            bool doDragPoint(const Vec3& newPosition, const PointSnapper& snapper, const Vec3::List& helpVectors, Vec3& snappedPosition) { return false; }
            void doEndDragPoint() {}
            
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
            m_clippedBrushRenderer->setEdgeColor(Color(pref(Preferences::EdgeColor), 0.5f));
            m_clippedBrushRenderer->setShowEdges(true);
            m_clippedBrushRenderer->setTint(false);
            m_clippedBrushRenderer->setTransparencyAlpha(0.5f);
            m_clippedBrushRenderer->render(renderContext, renderBatch);
        }
        
        void ClipTool::renderStrategy(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) {
            if (m_strategy != NULL)
                m_strategy->render(renderContext, renderBatch, pickResult);
        }
        
        void ClipTool::renderFeedback(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& point, const PointSnapper& snapper) const {
            if (m_strategy != NULL) {
                m_strategy->renderFeedback(renderContext, renderBatch, point, snapper);
            } else {
                PointClipStrategy().renderFeedback(renderContext, renderBatch, point, snapper);
            }
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
        
        void ClipTool::addPoint(const Vec3& point, const PointSnapper& snapper, const Vec3::List& helpVectors) {
            assert(canAddPoint(point, snapper));
            if (m_strategy == NULL)
                m_strategy = new PointClipStrategy();
            
            m_strategy->addPoint(point, snapper, helpVectors);
            update();
        }
        
        bool ClipTool::removeLastPoint() {
            if (m_strategy != NULL && m_strategy->removeLastPoint()) {
                update();
                return true;
            }
            return false;
        }

        bool ClipTool::beginDragPoint(const Model::PickResult& pickResult, Vec3& initialPosition) {
            if (m_strategy == NULL)
                return false;
            return m_strategy->beginDragPoint(pickResult, initialPosition);
        }
        
        bool ClipTool::dragPoint(const Vec3& newPosition, const PointSnapper& snapper, const Vec3::List& helpVectors, Vec3& snappedPosition) {
            assert(m_strategy != NULL);
            if (!m_strategy->dragPoint(newPosition, snapper, helpVectors, snappedPosition))
                return false;
            update();
            return true;
        }
        
        void ClipTool::endDragPoint() {
            assert(m_strategy != NULL);
            m_strategy->endDragPoint();
            refreshViews();
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
                unused(numPoints);
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
        
        void ClipTool::addBrushesToRenderer(const Model::ParentChildrenMap& map, Renderer::BrushRenderer* renderer) {
            Model::CollectBrushesVisitor collect;
            
            Model::ParentChildrenMap::const_iterator it, end;
            for (it = map.begin(), end = map.end(); it != end; ++it) {
                const Model::NodeList& brushes = it->second;
                Model::Node::accept(brushes.begin(), brushes.end(), collect);
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

        String ClipTool::doGetIconName() const {
            return "ClipTool.png";
        }
    }
}
