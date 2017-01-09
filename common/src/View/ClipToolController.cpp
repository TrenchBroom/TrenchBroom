/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
        ClipToolController::Callback::Callback(ClipTool* tool) :
        m_tool(tool) {
            ensure(m_tool != NULL, "tool is null");
        }
        
        ClipToolController::Callback::~Callback() {}
        
        ClipTool* ClipToolController::Callback::tool() const {
            return m_tool;
        }

        bool ClipToolController::Callback::addClipPoint(const InputState& inputState, Vec3& position) {
            if (!doGetNewClipPointPosition(inputState, position))
                return false;
            if (!m_tool->canAddPoint(position))
                return false;
            m_tool->addPoint(position, getHelpVectors(inputState));
            return true;
        }

        bool ClipToolController::Callback::setClipFace(const TrenchBroom::View::InputState &inputState) {
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return false;
            
            const Model::BrushFace* face = Model::hitToFace(hit);
            m_tool->setFace(face);
            return true;
        }
        
        void ClipToolController::Callback::renderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (inputState.anyToolDragging())
                return;
            
            Vec3 position;
            if (!doGetNewClipPointPosition(inputState, position))
                return;
            
            if (!m_tool->canAddPoint(position))
                return;
            
            m_tool->renderFeedback(renderContext, renderBatch, position);
        }

        ClipToolController::PartBase::PartBase(Callback* callback) :
        m_callback(callback) {
            ensure(m_callback != NULL, "callback is null");
        }
        
        ClipToolController::PartBase::~PartBase() {
            delete m_callback;
        }

        ClipToolController::AddClipPointPart::AddClipPointPart(Callback* callback) :
        PartBase(callback) {}

        Tool* ClipToolController::AddClipPointPart::doGetTool() {
            return m_callback->tool();
        }
        
        bool ClipToolController::AddClipPointPart::doMouseClick(const InputState& inputState) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                !inputState.modifierKeysPressed(ModifierKeys::MKNone))
                return false;
            Vec3 temp;
            return m_callback->addClipPoint(inputState, temp);
        }
        
        bool ClipToolController::AddClipPointPart::doMouseDoubleClick(const InputState& inputState) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                !inputState.modifierKeysPressed(ModifierKeys::MKNone))
                return false;
            return m_callback->setClipFace(inputState);
        }
        
        RestrictedDragPolicy::DragInfo ClipToolController::AddClipPointPart::doStartDrag(const InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return DragInfo();
            
            Vec3 initialPoint;
            if (!m_callback->addClipPoint(inputState, initialPoint))
                return DragInfo();
            
            m_secondPointSet = false;
            DragRestricter* restricter = m_callback->createDragRestricter(inputState, initialPoint);
            DragSnapper* snapper = m_callback->createDragSnapper(inputState);
            return DragInfo(restricter, snapper, initialPoint);
        }
        
        RestrictedDragPolicy::DragResult ClipToolController::AddClipPointPart::doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
            
            if (!m_secondPointSet) {
                Vec3 position;
                if (m_callback->addClipPoint(inputState, position)) {
                    m_callback->tool()->beginDragLastPoint();
                    m_secondPointSet = true;
                    return DR_Continue;
                }
            } else {
                if (m_callback->tool()->dragPoint(curPoint, m_callback->getHelpVectors(inputState)))
                    return DR_Continue;
            }
            return DR_Deny;
        }
        
        void ClipToolController::AddClipPointPart::doEndDrag(const InputState& inputState) {
            if (m_secondPointSet)
                m_callback->tool()->endDragPoint();
        }
        
        void ClipToolController::AddClipPointPart::doCancelDrag() {
            if (m_secondPointSet) {
                m_callback->tool()->cancelDragPoint();
                m_callback->tool()->removeLastPoint();
            }
            m_callback->tool()->removeLastPoint();
        }
        
        void ClipToolController::AddClipPointPart::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_callback->renderFeedback(inputState, renderContext, renderBatch);
        }

        bool ClipToolController::AddClipPointPart::doCancel() {
            return false;
        }

        ClipToolController::MoveClipPointPart::MoveClipPointPart(Callback* callback) :
        PartBase(callback) {}

        Tool* ClipToolController::MoveClipPointPart::doGetTool() {
            return m_callback->tool();
        }
        
        RestrictedDragPolicy::DragInfo ClipToolController::MoveClipPointPart::doStartDrag(const InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return DragInfo();
            
            Vec3 initialPoint;
            if (!m_callback->tool()->beginDragPoint(inputState.pickResult(), initialPoint))
                return DragInfo();
            
            DragRestricter* restricter = m_callback->createDragRestricter(inputState, initialPoint);
            DragSnapper* snapper = m_callback->createDragSnapper(inputState);
            return DragInfo(restricter, snapper, initialPoint);
        }
        
        RestrictedDragPolicy::DragResult ClipToolController::MoveClipPointPart::doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
            if (m_callback->tool()->dragPoint(curPoint, m_callback->getHelpVectors(inputState)))
                return DR_Continue;
            return DR_Deny;
        }
        
        void ClipToolController::MoveClipPointPart::doEndDrag(const InputState& inputState) {
            m_callback->tool()->endDragPoint();
        }
        
        void ClipToolController::MoveClipPointPart::doCancelDrag() {
            m_callback->tool()->cancelDragPoint();
        }
        
        bool ClipToolController::MoveClipPointPart::doCancel() {
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
        
        void ClipToolController::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            renderContext.setHideSelection();
            renderContext.setForceHideSelectionGuide();
        }
        
        void ClipToolController::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch, inputState.pickResult());
            ToolControllerGroup::doRender(inputState, renderContext, renderBatch);
        }
        
        bool ClipToolController::doCancel() {
            if (m_tool->removeLastPoint()) {
                if (!m_tool->hasPoints())
                    m_tool->reset();
                return true;
            }
            return false;
        }

        class ClipToolController2D::Callback2D : public Callback {
        public:
            Callback2D(ClipTool* tool) :
            Callback(tool) {}
            
            DragRestricter* createDragRestricter(const InputState& inputState, const Vec3& initialPoint) const {
                return new PlaneDragRestricter(Plane3(initialPoint, inputState.camera().direction().firstAxis()));
            }
            
            DragSnapper* createDragSnapper(const InputState& inputState) const {
                return new AbsoluteDragSnapper(m_tool->grid());
            }
            
            Vec3::Array getHelpVectors(const InputState& inputState) const {
                return Vec3::Array(1, inputState.camera().direction());
            }

            bool doGetNewClipPointPosition(const InputState& inputState, Vec3& position) const {
                const Renderer::Camera& camera = inputState.camera();
                const Vec3 viewDir = camera.direction().firstAxis();
                
                const Ray3& pickRay = inputState.pickRay();
                const Vec3 defaultPos = m_tool->defaultClipPointPos();
                const FloatType distance = pickRay.intersectWithPlane(viewDir, defaultPos);
                if (Math::isnan(distance))
                    return false;
                
                position = pickRay.pointAtDistance(distance);
                const Grid& grid = m_tool->grid();
                position = grid.snap(position);
                return true;
            }
        };
        
        ClipToolController2D::ClipToolController2D(ClipTool* tool) :
        ClipToolController(tool) {
            addController(new AddClipPointPart(new Callback2D(tool)));
            addController(new MoveClipPointPart(new Callback2D(tool)));
        }
        
        Vec3::Array ClipToolController3D::selectHelpVectors(Model::BrushFace* face, const Vec3& hitPoint) {
            ensure(face != NULL, "face is null");
            
            Vec3::Array result;
            for (const Model::BrushFace* incidentFace : selectIncidentFaces(face, hitPoint)) {
                const Vec3& normal = incidentFace->boundary().normal;
                result.push_back(normal.firstAxis());
            }
            
            return result;
        }
        
        Model::BrushFaceArray ClipToolController3D::selectIncidentFaces(Model::BrushFace* face, const Vec3& hitPoint) {
            for (const Model::BrushVertex* vertex : face->vertices()) {
                if (vertex->position().equals(hitPoint)) {
                    const Model::Brush* brush = face->brush();
                    return brush->incidentFaces(vertex);
                }
            }
            
            for (const Model::BrushEdge* edge : face->edges()) {
                if (edge->contains(hitPoint)) {
                    Model::BrushFaceArray result;
                    result.push_back(edge->firstFace()->payload());
                    result.push_back(edge->secondFace()->payload());
                    return result;
                }
            }
            
            return Model::BrushFaceArray(1, face);
        }

        class ClipToolController3D::Callback3D : public Callback {
        public:
            Callback3D(ClipTool* tool) :
            Callback(tool) {}
            
            DragRestricter* createDragRestricter(const InputState& inputState, const Vec3& initialPoint) const {
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
                    ensure(hit.type() == Model::Brush::BrushHit, "invalid hit type");
                    const Model::BrushFace* face = Model::hitToFace(hit);
                    return face->boundary();
                }
            };
            
            DragSnapper* createDragSnapper(const InputState& inputState) const {
                SurfaceDragSnapper* snapper = new ClipPointSnapper(m_tool->grid());
                snapper->setPickable(true);
                snapper->setType(Model::Brush::BrushHit);
                snapper->setOccluded(Model::Hit::AnyType);
                return snapper;
            }
            
            Vec3::Array getHelpVectors(const InputState& inputState) const {
                const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
                ensure(hit.isMatch(), "hit is not a match");
                
                Model::BrushFace* face = Model::hitToFace(hit);
                return selectHelpVectors(face, hit.hitPoint());
            }

            bool doGetNewClipPointPosition(const InputState& inputState, Vec3& position) const {
                const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
                if (!hit.isMatch())
                    return false;
                
                Model::BrushFace* face = hit.target<Model::BrushFace*>();
                const Grid& grid = m_tool->grid();
                position = grid.snap(hit.hitPoint(), face->boundary());
                return true;
            }
        };
        
        ClipToolController3D::ClipToolController3D(ClipTool* tool) :
        ClipToolController(tool) {
            addController(new AddClipPointPart(new Callback3D(tool)));
            addController(new MoveClipPointPart(new Callback3D(tool)));
        }
    }
}
