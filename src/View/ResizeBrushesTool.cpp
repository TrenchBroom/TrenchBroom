/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "ResizeBrushesTool.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushEdge.h"
#include "Model/BrushEdgesIterator.h"
#include "Model/BrushFace.h"
#include "Model/BrushFacesIterator.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/FaceEdgesIterator.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilters.h"
#include "Model/ModelUtils.h"
#include "Renderer/Camera.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType ResizeBrushesTool::ResizeHit = Model::Hit::freeHitType();

        ResizeBrushesTool::ResizeBrushesTool(BaseTool* next, MapDocumentPtr document, ControllerFacade& controller) :
        Tool(next, document, controller) {}

        void ResizeBrushesTool::doPick(const InputState& inputState, Model::PickResult& pickResult) const {
            if (!applies(inputState))
                return;
            
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Brush::BrushHit, document()->filter(), true);
            if (first.matches)
                pickResult.addHit(Model::Hit(ResizeHit, first.hit.distance(), first.hit.hitPoint(), Model::hitAsFace(first.hit)));
            else
                pickNearFaceHit(inputState, pickResult);
        }

        void ResizeBrushesTool::doModifierKeyChange(const InputState& inputState) {
            if (applies(inputState) && !dragging())
                updateDragFaces(inputState);
        }

        void ResizeBrushesTool::doMouseMove(const InputState& inputState) {
            if (applies(inputState) && !dragging())
                updateDragFaces(inputState);
        }

        bool ResizeBrushesTool::doStartMouseDrag(const InputState& inputState) {
            if (!applies(inputState))
                return false;

            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), ResizeHit, true);
            if (!first.matches)
                return false;

            m_dragOrigin = first.hit.hitPoint();
            m_totalDelta = Vec3::Null;
            updateDragFaces(inputState);
            
            controller().beginUndoableGroup(document()->selectedBrushes().size() == 1 ? "Resize Brush" : "Resize Brushes");
            return true;
        }
        
        bool ResizeBrushesTool::doMouseDrag(const InputState& inputState) {
            assert(!m_dragFaces.empty());
            
            const Plane3 dragPlane = orthogonalDragPlane(m_dragOrigin, Vec3(inputState.camera().direction()));
            
            Model::BrushFace& dragFace = *m_dragFaces.front();
            const Vec3& faceNormal3D = dragFace.boundary().normal;
            const Vec3 faceNormal2D = dragPlane.project(faceNormal3D);
            const FloatType rayPointDist = dragPlane.intersectWithRay(inputState.pickRay());
            const Vec3 rayPoint = inputState.pickRay().pointAtDistance(rayPointDist);
            const Vec3 dragVector2D = rayPoint - m_dragOrigin;
            const FloatType dragDist = dragVector2D.dot(faceNormal2D);
            const FloatType dragDist2 = dragDist * dragDist;
            
            const View::Grid& grid = document()->grid();
            const Vec3 relativeFaceDelta = grid.snap(dragDist) * faceNormal3D;
            const Vec3 absoluteFaceDelta = grid.moveDelta(dragFace, faceNormal3D * dragDist);
            
            // select the delta that is closest to the actual delta indicated by the mouse cursor
            const Vec3f faceDelta = (std::abs(relativeFaceDelta.squaredLength() - dragDist2) <
                                     std::abs(absoluteFaceDelta.squaredLength() - dragDist2) ?
                                     relativeFaceDelta :
                                     absoluteFaceDelta);
            
            if (faceDelta.null())
                return true;
            
            if (controller().resizeBrushes(m_dragFaces, faceDelta, document()->textureLock())) {
                m_totalDelta += faceDelta;
                m_dragOrigin += faceDelta;
            }
            return true;
        }
        
        void ResizeBrushesTool::doEndMouseDrag(const InputState& inputState) {
            if (m_totalDelta.null())
                controller().rollbackGroup();
            controller().closeGroup();
            m_dragFaces.clear();
        }
        
        void ResizeBrushesTool::doCancelMouseDrag(const InputState& inputState) {
            controller().rollbackGroup();
            controller().closeGroup();
            m_dragFaces.clear();
        }
        
        void ResizeBrushesTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            if (!applies(inputState) || m_dragFaces.empty())
                return;

            PreferenceManager& prefs = PreferenceManager::instance();
            Renderer::EdgeRenderer edgeRenderer = buildEdgeRenderer(m_dragFaces);
            
            glDisable(GL_DEPTH_TEST);
            edgeRenderer.setColor(prefs.getColor(Preferences::ResizeHandleColor));
            edgeRenderer.render(renderContext);
            glEnable(GL_DEPTH_TEST);
        }

        bool ResizeBrushesTool::applies(const InputState& inputState) const {
            return inputState.modifierKeysPressed(ModifierKeys::MKShift) && document()->hasSelectedBrushes();
        }
        
        struct FindClosestFaceHit {
        private:
            const Ray3& m_pickRay;
            double m_closestDistance;
            Model::BrushFace* m_closestFace;
            double m_hitDistance;
        public:
            FindClosestFaceHit(const Ray3& pickRay) :
            m_pickRay(pickRay),
            m_closestDistance(std::numeric_limits<FloatType>::max()),
            m_closestFace(NULL) {}
            
            void operator()(Model::BrushEdge* edge) {
                const double leftDot = edge->leftFace()->boundary().normal.dot(m_pickRay.direction);
                const double rightDot = edge->rightFace()->boundary().normal.dot(m_pickRay.direction);
                
                if ((leftDot > 0.0) != (rightDot > 0.0)) {
                    const Ray3::LineDistance result = m_pickRay.distanceToSegment(edge->start()->position(), edge->end()->position());
                    if (!Math::isnan(result.distance) && result.distance < m_closestDistance) {
                        m_closestDistance = result.distance;
                        m_hitDistance = result.rayDistance;
                        m_closestFace = leftDot > rightDot ? edge->leftFace() : edge->rightFace();
                    }
                }
            }
            
            bool success() const {
                return m_closestFace != NULL;
            }
            
            Model::Hit hit() const {
                assert(success());
                return Model::Hit(ResizeBrushesTool::ResizeHit, m_hitDistance, m_pickRay.pointAtDistance(m_hitDistance), m_closestFace);
            }
        };
        
        void ResizeBrushesTool::pickNearFaceHit(const InputState& inputState, Model::PickResult& pickResult) const {
            const Model::BrushList& brushes = document()->selectedBrushes();
            
            Model::BrushEdgesIterator::OuterIterator begin = Model::BrushEdgesIterator::begin(brushes);
            Model::BrushEdgesIterator::OuterIterator end = Model::BrushEdgesIterator::end(brushes);
            
            FindClosestFaceHit findHit(inputState.pickRay());
            Model::each(begin, end, findHit, Model::MatchAll());
            
            if (findHit.success())
                pickResult.addHit(findHit.hit());
        }

        void ResizeBrushesTool::updateDragFaces(const InputState& inputState) {
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), ResizeHit, true);
            if (!first.matches)
                m_dragFaces.clear();
            else
                m_dragFaces = collectDragFaces(*first.hit.target<Model::BrushFace*>());
        }

        struct CollectDragFaces {
        private:
            const Model::BrushFace& m_dragFace;
            Model::BrushFaceList& m_faces;
        public:
            
            CollectDragFaces(const Model::BrushFace& dragFace, Model::BrushFaceList& faces) :
            m_dragFace(dragFace),
            m_faces(faces) {}
            
            void operator()(Model::BrushFace* face) {
                if (face != &m_dragFace && face->boundary().equals(m_dragFace.boundary()))
                    m_faces.push_back(face);
            }
        };
        
        Model::BrushFaceList ResizeBrushesTool::collectDragFaces(Model::BrushFace& dragFace) const {
            Model::BrushFaceList result;
            result.push_back(&dragFace);
            
            const Model::BrushList& brushes = document()->selectedBrushes();
            assert(!brushes.empty());
            
            Model::BrushFacesIterator::OuterIterator begin = Model::BrushFacesIterator::begin(brushes);
            Model::BrushFacesIterator::OuterIterator end = Model::BrushFacesIterator::end(brushes);
            
            CollectDragFaces collect(dragFace, result);
            Model::each(begin, end, collect, Model::MatchAll());
            
            return result;
        }

        struct CollectVertices {
        public:
            typedef Renderer::VertexSpecs::P3::Vertex Vertex;
        private:
            Vertex::List& m_vertices;
        public:
            CollectVertices(Vertex::List& vertices) :
            m_vertices(vertices) {}
            
            void operator()(const Model::BrushEdge* edge) {
                m_vertices.push_back(Vertex(edge->start()->position()));
                m_vertices.push_back(Vertex(edge->end()->position()));
            }
        };
        
        Renderer::EdgeRenderer ResizeBrushesTool::buildEdgeRenderer(const Model::BrushFaceList& faces) const {
            Model::FaceEdgesIterator::OuterIterator begin = Model::FaceEdgesIterator::begin(faces);
            Model::FaceEdgesIterator::OuterIterator end = Model::FaceEdgesIterator::end(faces);
            
            CollectVertices::Vertex::List vertices;
            CollectVertices collect(vertices);
            Model::each(begin, end, collect, Model::MatchAll());
            
            return Renderer::EdgeRenderer(vertices);
        }
    }
}
