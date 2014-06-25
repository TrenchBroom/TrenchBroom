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

#include "RotateObjectsTool.h"

#include "Macros.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/HitAdapter.h"
#include "Model/ModelTypes.h"
#include "Model/Object.h"
#include "Renderer/Camera.h"
#include "Renderer/Circle.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Transformation.h"
#include "View/ControllerFacade.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Hit::HitType RotateObjectsTool::HandleHit = Hit::freeHitType();

        RotateObjectsTool::RotateObjectsTool(MapDocumentWPtr document, ControllerWPtr controller, const Renderer::Camera& camera, MovementRestriction& movementRestriction, Renderer::TextureFont& font) :
        ToolImpl(document, controller),
        m_camera(camera),
        m_helper(NULL),
        m_moveHelper(movementRestriction, *this),
        m_rotateHelper(*this, font) {}

        bool RotateObjectsTool::initiallyActive() const {
            return false;
        }
        
        bool RotateObjectsTool::doActivate(const InputState& inputState) {
            if (!document()->hasSelectedObjects())
                return false;
            resetHandlePosition();
            updateHandleAxes(inputState);
            return true;
        }
        
        bool RotateObjectsTool::doDeactivate(const InputState& inputState) {
            return true;
        }
        
        void RotateObjectsTool::doPick(const InputState& inputState, Hits& hits) {
            if (document()->selectedObjects().empty())
                return;

            updateHandleAxes(inputState);
            
            const RotateObjectsHandle::Hit hit = m_handle.pick(inputState.pickRay());
            if (hit.matches())
                hits.addHit(Hit(HandleHit, hit.distance(), hit.point(), hit.area()));
        }
        
        bool RotateObjectsTool::doMouseDown(const InputState& inputState) {
            const Hit& hit = inputState.hits().findFirst(HandleHit, true);
            return hit.isMatch() && inputState.mouseButtonsPressed(MouseButtons::MBLeft);
        }
        
        bool RotateObjectsTool::doMouseUp(const InputState& inputState) {
            const Hit& hit = inputState.hits().findFirst(HandleHit, true);
            return hit.isMatch() && inputState.mouseButtonsPressed(MouseButtons::MBLeft);
        }

        void RotateObjectsTool::doMouseMove(const InputState& inputState) {
            updateHandleAxes(inputState);
        }
        
        bool RotateObjectsTool::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            updateHandleAxes(inputState);

            if (m_moveHelper.startPlaneDrag(inputState, plane, initialPoint)) {
                m_helper = &m_moveHelper;
                return true;
            }
            
            if (m_rotateHelper.startPlaneDrag(inputState, plane, initialPoint)) {
                m_helper = &m_rotateHelper;
                return true;
            }
            
            return false;
        }
        
        bool RotateObjectsTool::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            assert(m_helper != NULL);
            updateHandleAxes(inputState);
            return m_helper->planeDrag(inputState, lastPoint, curPoint, refPoint);
        }
        
        void RotateObjectsTool::doEndPlaneDrag(const InputState& inputState) {
            assert(m_helper != NULL);
            m_helper->endPlaneDrag(inputState);
            updateHandleAxes(inputState);
            m_helper = NULL;
        }
        
        void RotateObjectsTool::doCancelPlaneDrag(const InputState& inputState) {
            assert(m_helper != NULL);
            m_helper->cancelPlaneDrag(inputState);
            updateHandleAxes(inputState);
            m_helper = NULL;
        }
        
        void RotateObjectsTool::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            const Hit& hit = inputState.hits().findFirst(HandleHit, true);
            if (dragging() || hit.isMatch())
                renderContext.setForceShowSelectionGuide();
        }

        void RotateObjectsTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            if (document()->selectedObjects().empty())
                return;
            
            updateHandleAxes(inputState);
            
            m_handle.renderHandle(renderContext, highlightHandleArea(inputState));
            
            const Hit& hit = inputState.hits().findFirst(HandleHit, true);
            if (m_helper != NULL)
                m_helper->render(inputState, dragging(), renderContext);
            else if (hit.isMatch() && hit.target<RotateObjectsHandle::HitArea>() == RotateObjectsHandle::HitArea_Center)
                m_moveHelper.render(inputState, dragging(), renderContext);
        }

        RotateObjectsHandle::HitArea RotateObjectsTool::highlightHandleArea(const InputState& inputState) const {
            if (dragging() && m_helper == &m_moveHelper)
                return RotateObjectsHandle::HitArea_Center;

            const Hit& hit = inputState.hits().findFirst(HandleHit, true);
            if (!hit.isMatch())
                return RotateObjectsHandle::HitArea_None;
            return hit.target<RotateObjectsHandle::HitArea>();
        }

        void RotateObjectsTool::resetHandlePosition() {
            const Model::ObjectList& objects = document()->selectedObjects();
            assert(!objects.empty());
            const BBox3 bounds = Model::Object::bounds(objects);
            const Vec3 position = document()->grid().snap(bounds.center());
            m_handle.setPosition(position);
        }

        void RotateObjectsTool::updateHandleAxes(const InputState& inputState) {
            m_handle.updateAxes(m_camera.position());
        }

        bool RotateObjectsTool::doHandleMove(const InputState& inputState) const {
            const Hit& hit = inputState.hits().findFirst(HandleHit, true);
            if (!hit.isMatch())
                return false;
            return hit.target<RotateObjectsHandle::HitArea>() == RotateObjectsHandle::HitArea_Center;
        }
        
        Vec3 RotateObjectsTool::doGetMoveOrigin(const InputState& inputState) const {
            return m_handle.getPointHandlePosition(RotateObjectsHandle::HitArea_Center);
        }
        
        bool RotateObjectsTool::doStartMove(const InputState& inputState) {
            return true;
        }
        
        Vec3 RotateObjectsTool::doSnapDelta(const InputState& inputState, const Vec3& delta) const {
            return document()->grid().snap(delta);
        }
        
        MoveResult RotateObjectsTool::doMove(const Vec3& delta) {
            const Vec3 position = m_handle.getPointHandlePosition(RotateObjectsHandle::HitArea_Center);
            m_handle.setPosition(position + delta);
            return MoveResult_Continue;
        }
        
        void RotateObjectsTool::doEndMove(const InputState& inputState) {}
        void RotateObjectsTool::doCancelMove(const InputState& inputState) {}

        bool RotateObjectsTool::doHandleRotate(const InputState& inputState) const {
            const Hit& hit = inputState.hits().findFirst(HandleHit, true);
            if (!hit.isMatch())
                return false;
            return hit.target<RotateObjectsHandle::HitArea>() != RotateObjectsHandle::HitArea_Center;
        }
        
        RotateInfo RotateObjectsTool::doGetRotateInfo(const InputState& inputState) const {
            const Hit& hit = inputState.hits().findFirst(HandleHit, true);
            assert(hit.isMatch());
            const RotateObjectsHandle::HitArea area = hit.target<RotateObjectsHandle::HitArea>();
            assert(area != RotateObjectsHandle::HitArea_None &&
                   area != RotateObjectsHandle::HitArea_Center);
            
            RotateInfo info;
            info.center = m_handle.getPointHandlePosition(RotateObjectsHandle::HitArea_Center);
            info.axis = m_handle.getRotationAxis(area);
            info.origin = m_handle.getPointHandlePosition(area);
            info.plane = Plane3(info.origin, info.axis);
            return info;
        }
        
        bool RotateObjectsTool::doStartRotate(const InputState& inputState) {
            controller()->beginUndoableGroup(String("Rotate ") + String(document()->selectedObjects().size() == 1 ? "object" : "objects"));

            const Hit& hit = inputState.hits().findFirst(HandleHit, true);
            _UNUSED(hit);
            assert(hit.isMatch());
            
            assert(hit.target<RotateObjectsHandle::HitArea>() != RotateObjectsHandle::HitArea_None &&
                   hit.target<RotateObjectsHandle::HitArea>() != RotateObjectsHandle::HitArea_Center);

            return true;
        }
        
        FloatType RotateObjectsTool::doGetAngle(const InputState& inputState, const Vec3& handlePoint, const Vec3& curPoint, const Vec3& axis) const {
            const Vec3 handlePos = m_handle.getPointHandlePosition(RotateObjectsHandle::HitArea_Center);
            const Vec3 refVector = (handlePoint - handlePos).normalized();
            const Vec3 curVector = (curPoint - handlePos).normalized();
            const FloatType angle = document()->grid().snapAngle(angleBetween(curVector, refVector, axis));
            return Math::remainder(angle, Math::C::twoPi());
        }
        
        bool RotateObjectsTool::doRotate(const Vec3& center, const Vec3& axis, const FloatType angle) {
            controller()->rollbackGroup();
            controller()->rotateObjects(document()->selectedObjects(), center, axis, angle, document()->textureLock());
            return true;
        }
        
        void RotateObjectsTool::doEndRotate(const InputState& inputState) {
            controller()->closeGroup();
        }
        
        void RotateObjectsTool::doCancelRotate(const InputState& inputState) {
            controller()->rollbackGroup();
            controller()->closeGroup();
        }
    }
}
