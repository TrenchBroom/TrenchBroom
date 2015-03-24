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

#include "Clipper.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/World.h"
#include "Renderer/BrushRenderer.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        Clipper::PointSnapper::~PointSnapper() {}
        
        bool Clipper::PointSnapper::snap(const Vec3& point, Vec3& result) const {
            return doSnap(point, result);
        }

        Clipper::PointStrategy::~PointStrategy() {}
        
        bool Clipper::PointStrategy::computeThirdPoint(const Vec3& point1, const Vec3& point2, Vec3& point3) const {
            return doComputeThirdPoint(point1, point2, point3);
        }

        Clipper::PointStrategyFactory::~PointStrategyFactory() {}
        Clipper::PointStrategy* Clipper::PointStrategyFactory::createStrategy() const {
            return doCreateStrategy();
        }

        Clipper::PointStrategy* Clipper::DefaultPointStrategyFactory::doCreateStrategy() const {
            return NULL;
        }

        class Clipper::ClipStrategy {
        public:
            virtual ~ClipStrategy() {}
            
            bool canClip() const {
                return doCanClip();
            }
            
            bool canAddPoint(const Vec3& point, const PointSnapper& snapper) const {
                return doCanAddPoint(point, snapper);
            }
            
            void addPoint(const Vec3& point, const PointSnapper& snapper, const PointStrategyFactory& factory) {
                assert(canAddPoint(point, snapper));
                return doAddPoint(point, snapper, factory);
            }
            
            void removeLastPoint() {
                doRemoveLastPoint();
            }
            
            bool beginDragPoint(const Vec3& position) {
                return doBeginDragPoint(position);
            }
            
            Vec3 getDraggedPoint() const {
                return doGetDraggedPoint();
            }
            
            bool dragPoint(const Vec3& newPosition, const PointSnapper& snapper) {
                return doDragPoint(newPosition, snapper);
            }
            
            bool setFAce(const Model::BrushFace* face) {
                return doSetFace(face);
            }
            
            void reset() {
                doReset();
            }
            
            size_t getPoints(Vec3& point1, Vec3& point2, Vec3& point3) const {
                return doGetPoints(point1, point2, point3);
            }
        private:
            virtual bool doCanClip() const = 0;
            virtual bool doCanAddPoint(const Vec3& point, const PointSnapper& snapper) const = 0;
            virtual void doAddPoint(const Vec3& point, const PointSnapper& snapper, const PointStrategyFactory& factory) = 0;
            virtual void doRemoveLastPoint() = 0;
            virtual bool doBeginDragPoint(const Vec3& position) = 0;
            virtual Vec3 doGetDraggedPoint() const = 0;
            virtual bool doDragPoint(const Vec3& newPosition, const PointSnapper& snapper) = 0;
            virtual bool doSetFace(const Model::BrushFace* face) = 0;
            virtual void doReset() = 0;
            virtual size_t doGetPoints(Vec3& point1, Vec3& point2, Vec3& point3) const = 0;
        };
        
        class Clipper::PointClipStrategy : public Clipper::ClipStrategy {
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
            
            bool doBeginDragPoint(const Vec3& position) {
                for (size_t i = 0; i < m_numPoints; ++i) {
                    if (m_points[i] == position) {
                        m_dragIndex = i;
                        return true;
                    }
                }
                return false;
            }
            
            Vec3 doGetDraggedPoint() const {
                assert(m_dragIndex < m_numPoints);
                return m_points[m_dragIndex];
            }
            
            bool doDragPoint(const Vec3& newPosition, const PointSnapper& snapper) {
                assert(m_dragIndex < m_numPoints);

                Vec3 snapped;
                if (!snapper.snap(newPosition, snapped))
                    return false;
                
                if (m_numPoints == 2 && linearlyDependent(m_points[0], m_points[1], snapped))
                    return false;
                
                m_points[m_dragIndex] = snapped;
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
        };
        
        class Clipper::FaceClipStrategy : public Clipper::ClipStrategy {
        private:
            const Model::BrushFace* m_face;
        public:
            FaceClipStrategy() :
            m_face(NULL) {}
        private:
            bool doCanClip() const { return m_face != NULL; }
            bool doCanAddPoint(const Vec3& point, const PointSnapper& snapper) const { return false; }
            void doAddPoint(const Vec3& point, const PointSnapper& snapper, const PointStrategyFactory& factory) {}
            void doRemoveLastPoint() {}
            bool doBeginDragPoint(const Vec3& position) { return false; }
            Vec3 doGetDraggedPoint() const { return Vec3::Null; }
            bool doDragPoint(const Vec3& newPosition, const PointSnapper& snapper) { return false; }
            
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
        
        Clipper::Clipper(MapDocumentWPtr document) :
        m_document(document),
        m_clipSide(ClipSide_Front),
        m_strategy(NULL),
        m_remainingBrushRenderer(new Renderer::BrushRenderer(false)),
        m_clippedBrushRenderer(new Renderer::BrushRenderer(true)) {}

        Clipper::~Clipper() {
            reset();
        }
        
        void Clipper::toggleSide() {
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
        
        void Clipper::resetSide() {
            m_clipSide = ClipSide_Front;
            update();
        }

        bool Clipper::canClip() const {
            return m_strategy != NULL && m_strategy->canClip();
        }
        
        Model::ParentChildrenMap Clipper::clip() {
            assert(canClip());
            
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
        
        bool Clipper::canAddPoint(const Vec3& point, const PointSnapper& snapper) const {
            return m_strategy == NULL || m_strategy->canAddPoint(point, snapper);
        }
        
        void Clipper::addPoint(const Vec3& point, const PointSnapper& snapper, const PointStrategyFactory& factory) {
            assert(canAddPoint(point, snapper));
            if (m_strategy == NULL)
                m_strategy = new PointClipStrategy();
            
            m_strategy->addPoint(point, snapper, factory);
            update();
        }

        void Clipper::removeLastPoint() {
            if (m_strategy != NULL) {
                m_strategy->removeLastPoint();
                update();
            }
        }
        
        bool Clipper::beginDragPoint(const Vec3& position) {
            if (m_strategy == NULL)
                return false;
            return m_strategy->beginDragPoint(position);
        }
        
        Vec3 Clipper::draggedPoint() const {
            assert(m_strategy != NULL);
            return m_strategy->getDraggedPoint();
        }
        
        bool Clipper::dragPoint(const Vec3& newPosition, const PointSnapper& snapper) {
            assert(m_strategy != NULL);
            return m_strategy->dragPoint(newPosition, snapper);
        }

        void Clipper::reset() {
            resetStrategy();
            resetSide();
        }

        void Clipper::resetStrategy() {
            delete m_strategy;
            m_strategy = NULL;
        }

        void Clipper::update() {
            clearRenderers();
            clearBrushes();

            if (m_strategy != NULL) {
                Vec3 point1, point2, point3;
                if (m_strategy->getPoints(point1, point2, point3) == 3) {
                    clipBrushes(point1, point2, point3);
                    updateRenderers();
                }
            }
        }

        void Clipper::clearBrushes() {
            MapUtils::clearAndDelete(m_frontBrushes);
            MapUtils::clearAndDelete(m_backBrushes);
        }
        
        void Clipper::clipBrushes(const Vec3& point1, const Vec3& point2, const Vec3& point3) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushList& brushes = document->selectedNodes().brushes();
            const BBox3& worldBounds = document->worldBounds();
            
            if (canClip()) {
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
        
        void Clipper::setFaceAttributes(const Model::BrushFaceList& faces, Model::BrushFace* frontFace, Model::BrushFace* backFace) const {
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
        
        void Clipper::clearRenderers() {
            m_remainingBrushRenderer->clear();
            m_clippedBrushRenderer->clear();
        }
        
        void Clipper::updateRenderers() {
            if (keepFrontBrushes())
                addBrushesToRenderer(m_frontBrushes, m_remainingBrushRenderer);
            else
                addBrushesToRenderer(m_frontBrushes, m_clippedBrushRenderer);
            
            if (keepBackBrushes())
                addBrushesToRenderer(m_backBrushes, m_remainingBrushRenderer);
            else
                addBrushesToRenderer(m_backBrushes, m_clippedBrushRenderer);
        }
        
        
        class Clipper::AddBrushesToRendererVisitor : public Model::NodeVisitor {
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
        
        void Clipper::addBrushesToRenderer(const Model::ParentChildrenMap& map, Renderer::BrushRenderer* renderer) {
            AddBrushesToRendererVisitor visitor(renderer);
            
            Model::ParentChildrenMap::const_iterator it, end;
            for (it = map.begin(), end = map.end(); it != end; ++it) {
                const Model::NodeList& brushes = it->second;
                Model::Node::accept(brushes.begin(), brushes.end(), visitor);
            }
        }

        bool Clipper::keepFrontBrushes() const {
            return m_clipSide != ClipSide_Back;
        }
        
        bool Clipper::keepBackBrushes() const {
            return m_clipSide != ClipSide_Front;
        }
    }
}
