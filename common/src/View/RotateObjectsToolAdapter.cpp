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

#include "RotateObjectsToolAdapter.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "View/RotateObjectsTool.h"
#include "View/InputState.h"

namespace TrenchBroom {
    namespace View {
        RotateObjectsToolAdapter::RotateObjectsToolAdapter(RotateObjectsTool* tool, MoveToolHelper* moveHelper) :
        ToolAdapterBase(),
        m_tool(tool),
        m_moveHelper(moveHelper),
        m_rotateHelper(new RotateToolHelper(this, *this)),
        m_helper(NULL) {}

        RotateObjectsToolAdapter::~RotateObjectsToolAdapter() {
            delete m_moveHelper;
            delete m_rotateHelper;
        }

        Tool* RotateObjectsToolAdapter::doGetTool() {
            return m_tool;
        }

        void RotateObjectsToolAdapter::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            const Model::Hit hit = doPick(inputState);
            if (hit.isMatch())
                pickResult.addHit(hit);
        }

        void RotateObjectsToolAdapter::doModifierKeyChange(const InputState& inputState) {
            if (m_helper != NULL && dragging())
                resetPlane(inputState);
        }
        
        bool RotateObjectsToolAdapter::doMouseClick(const InputState& inputState) {
            const Model::Hit& hit = inputState.pickResult().query().type(RotateObjectsHandle::HandleHit).occluded().first();
            if (hit.isMatch() && inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                const RotateObjectsHandle::HitArea area = hit.target<RotateObjectsHandle::HitArea>();
                m_tool->updateToolPageAxis(area);
                return true;
            }
            return false;
        }

        bool RotateObjectsToolAdapter::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (m_moveHelper->startPlaneDrag(inputState, plane, initialPoint)) {
                m_helper = m_moveHelper;
                return true;
            }
            
            if (m_rotateHelper->startPlaneDrag(inputState, plane, initialPoint)) {
                m_helper = m_rotateHelper;
                return true;
            }
            
            return false;
        }
        
        bool RotateObjectsToolAdapter::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            assert(m_helper != NULL);
            return m_helper->planeDrag(inputState, lastPoint, curPoint, refPoint);
        }
        
        void RotateObjectsToolAdapter::doEndPlaneDrag(const InputState& inputState) {
            assert(m_helper != NULL);
            m_helper->endPlaneDrag(inputState);
            m_helper = NULL;
        }
        
        void RotateObjectsToolAdapter::doCancelPlaneDrag() {
            assert(m_helper != NULL);
            m_helper->cancelPlaneDrag();
            m_helper = NULL;
        }
        
        void RotateObjectsToolAdapter::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            assert(m_helper != NULL);
            m_helper->resetPlane(inputState, plane, initialPoint);
        }

        void RotateObjectsToolAdapter::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            const Model::Hit& hit = inputState.pickResult().query().type(RotateObjectsHandle::HandleHit).occluded().first();
            if (dragging() || hit.isMatch())
                renderContext.setForceShowSelectionGuide();
        }
        
        void RotateObjectsToolAdapter::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            const RotateObjectsHandle::HitArea highlight = highlightHandleArea(inputState);
            doRenderHandle(renderContext, renderBatch, highlight);
            
            if (m_helper != NULL)
                m_helper->render(inputState, renderContext, renderBatch);
            else if (highlight == RotateObjectsHandle::HitArea_Center)
                m_moveHelper->render(inputState, renderContext, renderBatch);
        }
        
        RotateObjectsHandle::HitArea RotateObjectsToolAdapter::highlightHandleArea(const InputState& inputState) const {
            if (dragging() && m_helper == m_moveHelper)
                return RotateObjectsHandle::HitArea_Center;
            
            const Model::Hit& hit = inputState.pickResult().query().type(RotateObjectsHandle::HandleHit).occluded().first();
            if (!hit.isMatch())
                return RotateObjectsHandle::HitArea_None;
            return hit.target<RotateObjectsHandle::HitArea>();
        }
        
        bool RotateObjectsToolAdapter::doCancel() {
            return false;
        }

        bool RotateObjectsToolAdapter::doHandleMove(const InputState& inputState) const {
            const Model::Hit& hit = inputState.pickResult().query().type(RotateObjectsHandle::HandleHit).occluded().first();
            return hit.isMatch() && hit.target<RotateObjectsHandle::HitArea>() == RotateObjectsHandle::HitArea_Center;
        }
        
        Vec3 RotateObjectsToolAdapter::doGetMoveOrigin(const InputState& inputState) const {
            return m_tool->rotationCenter();
        }
        
        bool RotateObjectsToolAdapter::doStartMove(const InputState& inputState) {
            return true;
        }
        
        Vec3 RotateObjectsToolAdapter::doSnapDelta(const InputState& inputState, const Vec3& delta) const {
            return m_tool->snapRotationCenterMoveDelta(delta);
        }
        
        MoveResult RotateObjectsToolAdapter::doMove(const InputState& inputState, const Vec3& delta) {
            m_tool->setRotationCenter(m_tool->rotationCenter() + delta);
            return MoveResult_Continue;
        }
        
        void RotateObjectsToolAdapter::doEndMove(const InputState& inputState) {}
        void RotateObjectsToolAdapter::doCancelMove() {}

        bool RotateObjectsToolAdapter::doHandleRotate(const InputState& inputState) const {
            const Model::Hit& hit = inputState.pickResult().query().type(RotateObjectsHandle::HandleHit).occluded().first();
            return hit.isMatch() && hit.target<RotateObjectsHandle::HitArea>() != RotateObjectsHandle::HitArea_Center;
        }
        
        RotateInfo RotateObjectsToolAdapter::doGetRotateInfo(const InputState& inputState) const {
            const Model::Hit& hit = inputState.pickResult().query().type(RotateObjectsHandle::HandleHit).occluded().first();
            assert(hit.isMatch());
            const RotateObjectsHandle::HitArea area = hit.target<RotateObjectsHandle::HitArea>();
            assert(area != RotateObjectsHandle::HitArea_None &&
                   area != RotateObjectsHandle::HitArea_Center);
            
            RotateInfo info;
            info.center = m_tool->rotationCenter();
            info.axis   = doGetRotationAxis(inputState, area);
            info.origin = doGetRotationAxisHandle(inputState, area);
            info.plane  = Plane3(info.origin, info.axis);
            return info;
        }
        
        bool RotateObjectsToolAdapter::doStartRotate(const InputState& inputState) {
            m_tool->beginRotation();
            
            const Model::Hit& hit = inputState.pickResult().query().type(RotateObjectsHandle::HandleHit).occluded().first();
            unused(hit);
            assert(hit.isMatch());
            assert(hit.target<RotateObjectsHandle::HitArea>() != RotateObjectsHandle::HitArea_None &&
                   hit.target<RotateObjectsHandle::HitArea>() != RotateObjectsHandle::HitArea_Center);
            
            return true;
        }
        
        FloatType RotateObjectsToolAdapter::doGetAngle(const InputState& inputState, const Vec3& handlePoint, const Vec3& curPoint, const Vec3& axis) const {
            const Vec3 handlePos = m_tool->rotationCenter();
            const Vec3 refVector = (handlePoint - handlePos).normalized();
            const Vec3 curVector = (curPoint - handlePos).normalized();
            const FloatType angle = m_tool->snapRotationAngle(angleBetween(curVector, refVector, axis));
            return angle - Math::roundDownToMultiple(angle, Math::C::twoPi());
        }
        
        bool RotateObjectsToolAdapter::doRotate(const Vec3& center, const Vec3& axis, const FloatType angle) {
            m_tool->applyRotation(center, axis, angle);
            return true;
        }
        
        void RotateObjectsToolAdapter::doEndRotate(const InputState& inputState) {
            m_tool->commitRotation();
        }
        
        void RotateObjectsToolAdapter::doCancelRotate() {
            m_tool->cancelRotation();
        }

        RotateObjectsToolAdapter2D::RotateObjectsToolAdapter2D(RotateObjectsTool* tool) :
        RotateObjectsToolAdapter(tool, new MoveToolHelper2D(this, this)) {}

        Model::Hit RotateObjectsToolAdapter2D::doPick(const InputState& inputState) {
            return m_tool->pick2D(inputState.pickRay(), inputState.camera());
        }

        Vec3 RotateObjectsToolAdapter2D::doGetRotationAxis(const InputState& inputState, const RotateObjectsHandle::HitArea area) const {
            return m_tool->rotationAxis(area);
        }

        Vec3 RotateObjectsToolAdapter2D::doGetRotationAxisHandle(const InputState& inputState, RotateObjectsHandle::HitArea area) const {
            return m_tool->rotationAxisHandle(area, Vec3(inputState.camera().position()));
        }

        void RotateObjectsToolAdapter2D::doRenderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const RotateObjectsHandle::HitArea highlight) {
            m_tool->renderHandle2D(renderContext, renderBatch, highlight);
        }
        
        RotateObjectsToolAdapter3D::RotateObjectsToolAdapter3D(RotateObjectsTool* tool, MovementRestriction& movementRestriction) :
        RotateObjectsToolAdapter(tool, new MoveToolHelper3D(this, this, movementRestriction)) {}
        
        Model::Hit RotateObjectsToolAdapter3D::doPick(const InputState& inputState) {
            return m_tool->pick3D(inputState.pickRay(), inputState.camera());
        }
        
        Vec3 RotateObjectsToolAdapter3D::doGetRotationAxis(const InputState& inputState, const RotateObjectsHandle::HitArea area) const {
            return m_tool->rotationAxis(area);
        }
        
        Vec3 RotateObjectsToolAdapter3D::doGetRotationAxisHandle(const InputState& inputState, RotateObjectsHandle::HitArea area) const {
            return m_tool->rotationAxisHandle(area, Vec3(inputState.camera().position()));
        }
        
        void RotateObjectsToolAdapter3D::doRenderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const RotateObjectsHandle::HitArea highlight) {
            m_tool->renderHandle3D(renderContext, renderBatch, highlight);
        }
    }
}
