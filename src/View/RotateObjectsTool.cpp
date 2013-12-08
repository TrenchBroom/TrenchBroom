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

#include "RotateObjectsTool.h"

#include "Macros.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilters.h"
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
        const Model::Hit::HitType RotateObjectsTool::HandleHit = Model::Hit::freeHitType();

        RotateObjectsTool::RotateObjectsTool(BaseTool* next, MapDocumentPtr document, ControllerPtr controller, MovementRestriction& movementRestriction, Renderer::TextureFont& font) :
        Tool(next, document, controller),
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
        
        void RotateObjectsTool::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            updateHandleAxes(inputState);
            
            const RotateObjectsHandle::Hit hit = m_handle.pick(inputState.pickRay());
            if (hit.matches())
                pickResult.addHit(Model::Hit(HandleHit, hit.distance(), hit.point(), hit.area()));
        }
        
        bool RotateObjectsTool::doMouseDown(const InputState& inputState) {
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), HandleHit, true);
            return first.matches && inputState.mouseButtonsPressed(MouseButtons::MBLeft);
        }
        
        bool RotateObjectsTool::doMouseUp(const InputState& inputState) {
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), HandleHit, true);
            return first.matches && inputState.mouseButtonsPressed(MouseButtons::MBLeft);
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
        
        void RotateObjectsTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            updateHandleAxes(inputState);
            
            m_handle.renderHandle(renderContext, highlightHandleArea(inputState));
            
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), HandleHit, true);
            if (m_helper != NULL)
                m_helper->render(inputState, dragging(), renderContext);
            else if (first.matches && first.hit.target<RotateObjectsHandle::HitArea>() == RotateObjectsHandle::HACenter)
                m_moveHelper.render(inputState, dragging(), renderContext);
        }

        RotateObjectsHandle::HitArea RotateObjectsTool::highlightHandleArea(const InputState& inputState) const {
            if (dragging() && m_helper == &m_moveHelper)
                return RotateObjectsHandle::HACenter;
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), HandleHit, true);
            if (!first.matches)
                return RotateObjectsHandle::HANone;
            return first.hit.target<RotateObjectsHandle::HitArea>();
        }

        void RotateObjectsTool::resetHandlePosition() {
            const Model::ObjectList& objects = document()->selectedObjects();
            assert(!objects.empty());
            const BBox3 bounds = Model::Object::bounds(objects);
            const Vec3 position = document()->grid().snap(bounds.center());
            m_handle.setPosition(position);
        }

        void RotateObjectsTool::updateHandleAxes(const InputState& inputState) {
            m_handle.updateAxes(inputState.camera().position());
        }

        bool RotateObjectsTool::doHandleMove(const InputState& inputState) const {
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), HandleHit, true);
            if (!first.matches)
                return false;
            return first.hit.target<RotateObjectsHandle::HitArea>() == RotateObjectsHandle::HACenter;
        }
        
        Vec3 RotateObjectsTool::doGetMoveOrigin(const InputState& inputState) const {
            return m_handle.getPointHandlePosition(RotateObjectsHandle::HACenter);
        }
        
        bool RotateObjectsTool::doStartMove(const InputState& inputState) {
            return true;
        }
        
        Vec3 RotateObjectsTool::doSnapDelta(const InputState& inputState, const Vec3& delta) const {
            return document()->grid().snap(delta);
        }
        
        MoveResult RotateObjectsTool::doMove(const Vec3& delta) {
            const Vec3 position = m_handle.getPointHandlePosition(RotateObjectsHandle::HACenter);
            m_handle.setPosition(position + delta);
            return Continue;
        }
        
        void RotateObjectsTool::doEndMove(const InputState& inputState) {}
        void RotateObjectsTool::doCancelMove(const InputState& inputState) {}

        bool RotateObjectsTool::doHandleRotate(const InputState& inputState) const {
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), HandleHit, true);
            if (!first.matches)
                return false;
            return first.hit.target<RotateObjectsHandle::HitArea>() != RotateObjectsHandle::HACenter;
        }
        
        RotateInfo RotateObjectsTool::doGetRotateInfo(const InputState& inputState) const {
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), HandleHit, true);
            assert(first.matches);
            const RotateObjectsHandle::HitArea area = first.hit.target<RotateObjectsHandle::HitArea>();
            assert(area != RotateObjectsHandle::HANone &&
                   area != RotateObjectsHandle::HACenter);
            
            RotateInfo info;
            info.center = m_handle.getPointHandlePosition(RotateObjectsHandle::HACenter);
            info.axis = m_handle.getRotationAxis(area);
            info.origin = m_handle.getPointHandlePosition(area);
            info.plane = Plane3(info.origin, info.axis);
            return info;
        }
        
        bool RotateObjectsTool::doStartRotate(const InputState& inputState) {
            controller()->beginUndoableGroup(String("Rotate ") + String(document()->selectedObjects().size() == 1 ? "object" : "objects"));

            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), HandleHit, true);
            assert(first.matches);
            const RotateObjectsHandle::HitArea area = first.hit.target<RotateObjectsHandle::HitArea>();
            _unused(area);
            assert(area != RotateObjectsHandle::HANone &&
                   area != RotateObjectsHandle::HACenter);

            return true;
        }
        
        FloatType RotateObjectsTool::doGetAngle(const InputState& inputState, const Vec3& handlePoint, const Vec3& curPoint, const Vec3& axis) const {
            const Vec3 handlePos = m_handle.getPointHandlePosition(RotateObjectsHandle::HACenter);
            const Vec3 refVector = (handlePoint - handlePos).normalized();
            const Vec3 curVector = (curPoint - handlePos).normalized();
            return document()->grid().snapAngle(angleBetween(curVector, refVector, axis));
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
