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

#include "ClipToolController.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ClipToolController::MoveClipPointBase::MoveClipPointBase(ClipTool* tool) :
        m_tool(tool) {
            assert(m_tool != NULL);
        }

        ClipToolController::MoveClipPointBase::~MoveClipPointBase() {}

        Tool* ClipToolController::MoveClipPointBase::doGetTool() {
            return m_tool;
        }
        
        RestrictedDragPolicy::DragInfo ClipToolController::MoveClipPointBase::doStartDrag(const InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return DragInfo();
            
            Vec3 initialPoint;
            if (!m_tool->beginDragPoint(inputState.pickResult(), initialPoint))
                return DragInfo();
            
            DragRestricter* restricter = doCreateDragRestricter(inputState, initialPoint);
            DragSnapper* snapper = doCreateDragSnapper(inputState);
            return DragInfo(restricter, snapper, initialPoint);
        }
        
        RestrictedDragPolicy::DragResult ClipToolController::MoveClipPointBase::doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
            if (m_tool->dragPoint(curPoint, doGetHelpVectors(inputState)))
                return DR_Continue;
            return DR_Deny;
        }
        
        void ClipToolController::MoveClipPointBase::doEndDrag(const InputState& inputState) {
            m_tool->endDragPoint();
        }
        
        void ClipToolController::MoveClipPointBase::doCancelDrag() {
            m_tool->cancelDragPoint();
        }
        
        bool ClipToolController::MoveClipPointBase::doCancel() {
            return false;
        }

        ClipToolController::ClipToolController(ClipTool* tool) :
        m_tool(tool) {}
        
        ClipToolController::~ClipToolController() {}

        Tool* ClipToolController::doGetTool() {
            return m_tool;
        }
        
        void ClipToolController::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            m_tool->pick(inputState.pickRay(), inputState.camera(), pickResult);
        }
        
        bool ClipToolController::doMouseClick(const InputState& inputState) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                !inputState.modifierKeysPressed(ModifierKeys::MKNone))
                return false;
            return doAddClipPoint(inputState);
        }
        
        void ClipToolController::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            renderContext.setHideSelection();
            renderContext.setForceHideSelectionGuide();
        }
        
        void ClipToolController::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch, inputState.pickResult());
            doRenderFeedback(inputState, renderContext, renderBatch);
        }
        
        bool ClipToolController::doCancel() {
            return m_tool->removeLastPoint() || m_tool->reset();
        }

        class ClipToolController2D::MoveClipPointPart : public ClipToolController::MoveClipPointBase {
        public:
            MoveClipPointPart(ClipTool* tool) :
            MoveClipPointBase(tool) {}
        private:
            DragRestricter* doCreateDragRestricter(const InputState& inputState, const Vec3& initialPoint) const {
                return new PlaneDragRestricter(Plane3(initialPoint, inputState.camera().direction().firstAxis()));
            }
            
            DragSnapper* doCreateDragSnapper(const InputState& inputState) const {
                return new AbsoluteDragSnapper(m_tool->grid());
            }
            
            Vec3::List doGetHelpVectors(const InputState& inputState) const {
                return Vec3::List(1, inputState.camera().direction());
            }
        };
        
        ClipToolController2D::ClipToolController2D(ClipTool* tool) :
        ClipToolController(tool) {
            addController(new MoveClipPointPart(tool));
        }
        
        bool ClipToolController2D::doAddClipPoint(const InputState& inputState) {
            const Renderer::Camera& camera = inputState.camera();
            const Vec3 viewDir = camera.direction().firstAxis();
            
            const Ray3& pickRay = inputState.pickRay();
            const Vec3 defaultPos = m_tool->defaultClipPointPos();
            const FloatType distance = pickRay.intersectWithPlane(viewDir, defaultPos);
            if (Math::isnan(distance))
                return false;
            
            Vec3 position = pickRay.pointAtDistance(distance);
            const Grid& grid = m_tool->grid();
            position = grid.snap(position);
            
            if (!m_tool->canAddPoint(position))
                return false;
            
            m_tool->addPoint(position, Vec3::List(1, viewDir));
            return true;
        }

        void ClipToolController2D::doRenderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (dragging())
                return;
            
            const Renderer::Camera& camera = inputState.camera();
            const Vec3 viewDir = camera.direction().firstAxis();
            
            const Ray3& pickRay = inputState.pickRay();
            const Vec3 defaultPos = m_tool->defaultClipPointPos();
            const FloatType distance = pickRay.intersectWithPlane(viewDir, defaultPos);
            if (Math::isnan(distance))
                return;
            
            Vec3 position = pickRay.pointAtDistance(distance);
            const Grid& grid = m_tool->grid();
            position = grid.snap(position);
            
            if (!m_tool->canAddPoint(position))
                return;
            
            m_tool->renderFeedback(renderContext, renderBatch, position);
        }
        
        Vec3::List ClipToolController3D::selectHelpVectors(Model::BrushFace* face, const Vec3& hitPoint) {
            assert(face != NULL);
            
            Vec3::List result;
            const Model::BrushFaceList incidentFaces = selectIncidentFaces(face, hitPoint);
            Model::BrushFaceList::const_iterator it, end;
            for (it = incidentFaces.begin(), end = incidentFaces.end(); it != end; ++it) {
                const Model::BrushFace* incidentFace = *it;
                const Vec3& normal = incidentFace->boundary().normal;
                result.push_back(normal.firstAxis());
            }
            
            return result;
        }
        
        Model::BrushFaceList ClipToolController3D::selectIncidentFaces(Model::BrushFace* face, const Vec3& hitPoint) {
            const Model::BrushFace::VertexList vertices = face->vertices();
            Model::BrushFace::VertexList::const_iterator vIt, vEnd;
            for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                const Model::BrushVertex* vertex = *vIt;
                if (vertex->position().equals(hitPoint)) {
                    const Model::Brush* brush = face->brush();
                    return brush->incidentFaces(vertex);
                }
            }
            
            const Model::BrushFace::EdgeList edges = face->edges();
            Model::BrushFace::EdgeList::const_iterator eIt, eEnd;
            for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd; ++eIt) {
                Model::BrushEdge* edge = *eIt;
                if (edge->contains(hitPoint)) {
                    Model::BrushFaceList result;
                    result.push_back(edge->firstFace()->payload());
                    result.push_back(edge->secondFace()->payload());
                    return result;
                }
            }
            
            return Model::BrushFaceList(1, face);
        }

        class ClipToolController3D::MoveClipPointPart : public MoveClipPointBase {
        public:
            MoveClipPointPart(ClipTool* tool) :
            MoveClipPointBase(tool) {}
        private:
            DragRestricter* doCreateDragRestricter(const InputState& inputState, const Vec3& initialPoint) const {
                SurfaceDragRestricter* restricter = new SurfaceDragRestricter();
                restricter->setPickable(true);
                restricter->setType(Model::Brush::BrushHit);
                restricter->setOccluded(Model::Hit::AnyType);
                return restricter;
            }
            
            class ClipPointSnapper : public SurfaceDragSnapper {
            public:
                ClipPointSnapper(const Grid& grid) :
                SurfaceDragSnapper(grid) {}
            private:
                Plane3 doGetPlane(const InputState& inputState, const Model::Hit& hit) const {
                    assert(hit.type() == Model::Brush::BrushHit);
                    const Model::BrushFace* face = Model::hitToFace(hit);
                    return face->boundary();
                }
            };
            
            DragSnapper* doCreateDragSnapper(const InputState& inputState) const {
                SurfaceDragSnapper* snapper = new ClipPointSnapper(m_tool->grid());
                snapper->setPickable(true);
                snapper->setType(Model::Brush::BrushHit);
                snapper->setOccluded(Model::Hit::AnyType);
                return snapper;
            }
            
            Vec3::List doGetHelpVectors(const InputState& inputState) const {
                const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
                assert(hit.isMatch());
                
                Model::BrushFace* face = Model::hitToFace(hit);
                return selectHelpVectors(face, hit.hitPoint());
            }
        };
        
        ClipToolController3D::ClipToolController3D(ClipTool* tool) :
        ClipToolController(tool) {
            addController(new MoveClipPointPart(tool));
        }

        bool ClipToolController3D::doMouseDoubleClick(const InputState& inputState) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                !inputState.modifierKeysPressed(ModifierKeys::MKNone))
                return false;

            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return false;
            
            const Model::BrushFace* face = Model::hitToFace(hit);
            m_tool->setFace(face);
            return true;
        }
        
        bool ClipToolController3D::doAddClipPoint(const InputState& inputState) {
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return false;
            
            Model::BrushFace* face = hit.target<Model::BrushFace*>();
            const Grid& grid = m_tool->grid();
            const Vec3 position = grid.snap(hit.hitPoint(), face->boundary());
            
            if (!m_tool->canAddPoint(position))
                return false;
            
            const Vec3::List helpVectors = selectHelpVectors(face, position);
            m_tool->addPoint(position, helpVectors);
            return true;
        }

        void ClipToolController3D::doRenderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (dragging())
                return;
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return;
            
            Model::BrushFace* face = hit.target<Model::BrushFace*>();
            const Grid& grid = m_tool->grid();
            const Vec3 position = grid.snap(hit.hitPoint(), face->boundary());
            
            if (!m_tool->canAddPoint(position))
                return;
            
            m_tool->renderFeedback(renderContext, renderBatch, position);
        }
    }
}
