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

#include "CameraTool3D.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "Renderer/PerspectiveCamera.h"

#include <iostream>

namespace TrenchBroom {
    namespace View {
        CameraTool3D::CameraTool3D(MapDocumentWPtr document, Renderer::PerspectiveCamera& camera) :
        ToolAdapterBase(),
        Tool(true),
        m_document(document),
        m_camera(camera),
        m_orbit(false) {}
        
        void CameraTool3D::fly(int dx, int dy, const bool forward, const bool backward, const bool left, const bool right, const unsigned int time) {
            
            static const float speed = 256.0f / 1000.0f; // 64 units per second
            const float dist  = speed * time;
            
            Vec3 delta;
            if (forward)
                delta += m_camera.direction() * dist;
            if (backward)
                delta -= m_camera.direction() * dist;
            if (left)
                delta -= m_camera.right() * dist;
            if (right)
                delta += m_camera.right() * dist;
            m_camera.moveBy(delta);
            
            const float hAngle = static_cast<float>(dx) * lookSpeedH();
            const float vAngle = static_cast<float>(dy) * lookSpeedV();
            m_camera.rotate(hAngle, vAngle);
        }
        
        Tool* CameraTool3D::doGetTool() {
            return this;
        }
        
        void CameraTool3D::doMouseScroll(const InputState& inputState) {
            const float factor = pref(Preferences::CameraMouseWheelInvert) ? -1.0f : 1.0f;;
            if (m_orbit) {
                const Plane3f orbitPlane(m_orbitCenter, m_camera.direction());
                const float maxDistance = std::max(orbitPlane.intersectWithRay(m_camera.viewRay()) - 32.0f, 0.0f);
                const float distance = std::min(factor * inputState.scrollY() * moveSpeed(false), maxDistance);
                m_camera.moveBy(distance * m_camera.direction());
            } else if (move(inputState)) {
                const Vec3f moveDirection = pref(Preferences::CameraMoveInCursorDir) ? Vec3f(inputState.pickRay().direction) : m_camera.direction();
                const float distance = inputState.scrollY() * moveSpeed(false);
                m_camera.moveBy(factor * distance * moveDirection);
            }
        }
        
        bool CameraTool3D::doStartMouseDrag(const InputState& inputState) {
            if (orbit(inputState)) {
                const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit | Model::Entity::EntityHit | Model::Group::GroupHit).occluded().minDistance(3.0).first();
                if (hit.isMatch())
                    m_orbitCenter = hit.hitPoint();
                else
                    m_orbitCenter = inputState.camera().defaultPoint(inputState.pickRay());
                m_orbit = true;
                return true;
            } else if (look(inputState)) {
                return true;
            } else if (pan(inputState)) {
                return true;
            }
            return false;
        }
        
        bool CameraTool3D::doMouseDrag(const InputState& inputState) {
            if (m_orbit) {
                const float hAngle = inputState.mouseDX() * lookSpeedH();
                const float vAngle = inputState.mouseDY() * lookSpeedV();
                m_camera.orbit(m_orbitCenter, hAngle, vAngle);
                return true;
            } else if (look(inputState)) {
                const float hAngle = inputState.mouseDX() * lookSpeedH();
                const float vAngle = inputState.mouseDY() * lookSpeedV();
                m_camera.rotate(hAngle, vAngle);
                return true;
            } else if (pan(inputState)) {
                const bool altMove = pref(Preferences::CameraEnableAltMove);
                Vec3f delta;
                if (altMove && inputState.modifierKeysPressed(ModifierKeys::MKAlt)) {
                    delta += inputState.mouseDX() * panSpeedH() * m_camera.right();
                    delta += inputState.mouseDY() * -moveSpeed(altMove) * m_camera.direction();
                } else {
                    delta += inputState.mouseDX() * panSpeedH() * m_camera.right();
                    delta += inputState.mouseDY() * panSpeedV() * m_camera.up();
                }
                m_camera.moveBy(delta);
                return true;
            }
            return false;
        }
        
        void CameraTool3D::doEndMouseDrag(const InputState& inputState) {
            m_orbit = false;
        }
        
        void CameraTool3D::doCancelMouseDrag() {
            m_orbit = false;
        }
        
        bool CameraTool3D::move(const InputState& inputState) const {
            return ((inputState.mouseButtonsPressed(MouseButtons::MBNone) ||
                     inputState.mouseButtonsPressed(MouseButtons::MBRight)) &&
                    inputState.checkModifierKeys(MK_No, MK_No, MK_No));
        }
        
        bool CameraTool3D::look(const InputState& inputState) const {
            return (inputState.mouseButtonsPressed(MouseButtons::MBRight) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKNone));
        }
        
        bool CameraTool3D::pan(const InputState& inputState) const {
            return (inputState.mouseButtonsPressed(MouseButtons::MBMiddle) &&
                    (inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                     inputState.modifierKeysPressed(ModifierKeys::MKAlt)));
        }
        
        bool CameraTool3D::orbit(const InputState& inputState) const {
            return (inputState.mouseButtonsPressed(MouseButtons::MBRight) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKAlt));
        }
        
        float CameraTool3D::lookSpeedH() const {
            float speed = pref(Preferences::CameraLookSpeed) / -50.0f;
            if (pref(Preferences::CameraLookInvertH))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool3D::lookSpeedV() const {
            float speed = pref(Preferences::CameraLookSpeed) / -50.0f;
            if (pref(Preferences::CameraLookInvertV))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool3D::panSpeedH() const {
            float speed = pref(Preferences::CameraPanSpeed);
            if (pref(Preferences::CameraPanInvertH))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool3D::panSpeedV() const {
            float speed = pref(Preferences::CameraPanSpeed);
            if (pref(Preferences::CameraPanInvertV))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool3D::moveSpeed(const bool altMode) const {
            float speed = pref(Preferences::CameraMoveSpeed) * 20.0f;
            // if (slow)
                // speed /= 10.0f;
            if (altMode && pref(Preferences::CameraAltMoveInvert))
                speed *= -1.0f;
            return speed;
        }
        
        bool CameraTool3D::doCancel() {
            return false;
        }
    }
}
