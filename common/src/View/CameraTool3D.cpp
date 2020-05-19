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

#include "CameraTool3D.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/Hit.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "Renderer/PerspectiveCamera.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/plane.h>
#include <vecmath/intersection.h>

#include <algorithm>

namespace TrenchBroom {
    namespace View {
        CameraTool3D::CameraTool3D(std::weak_ptr<MapDocument> document, Renderer::PerspectiveCamera& camera) :
        ToolControllerBase(),
        Tool(true),
        m_document(document),
        m_camera(camera),
        m_orbit(false) {}

        void CameraTool3D::fly(int dx, int dy, const bool forward, const bool backward, const bool left, const bool right, const unsigned int time) {
            static const auto speed = 256.0f / 1000.0f; // 64 units per second
            const auto dist  = speed * static_cast<float>(time);

            vm::vec3f delta;
            if (forward) {
                delta = delta + m_camera.direction() * dist;
            }
            if (backward) {
                delta = delta - m_camera.direction() * dist;
            }
            if (left) {
                delta = delta - m_camera.right() * dist;
            }
            if (right) {
                delta = delta + m_camera.right() * dist;
            }
            m_camera.moveBy(delta);

            const auto hAngle = static_cast<float>(dx) * lookSpeedH();
            const auto vAngle = static_cast<float>(dy) * lookSpeedV();
            m_camera.rotate(hAngle, vAngle);
        }

        Tool* CameraTool3D::doGetTool() {
            return this;
        }

        const Tool* CameraTool3D::doGetTool() const {
            return this;
        }

        void CameraTool3D::doMouseScroll(const InputState& inputState) {
            const auto factor = pref(Preferences::CameraMouseWheelInvert) ? -1.0f : 1.0f;
            const auto zoom = inputState.modifierKeysPressed(ModifierKeys::MKShift);
            const float scrollDist = inputState.scrollY();

            if (m_orbit) {
                const auto orbitPlane = vm::plane3f(m_orbitCenter, m_camera.direction());
                const auto maxDistance = std::max(vm::intersect_ray_plane(m_camera.viewRay(), orbitPlane) - 32.0f, 0.0f);
                const auto distance = std::min(factor * scrollDist * moveSpeed(false), maxDistance);
                m_camera.moveBy(distance * m_camera.direction());
            } else if (move(inputState)) {
                if (zoom) {
                    const auto zoomFactor = 1.0f + scrollDist / 50.0f * factor;
                    m_camera.zoom(zoomFactor);
                } else {
                    const auto moveDirection = pref(Preferences::CameraMoveInCursorDir) ? vm::vec3f(inputState.pickRay().direction) : m_camera.direction();
                    const auto distance = scrollDist * moveSpeed(false);
                    m_camera.moveBy(factor * distance * moveDirection);
                }
            }
        }

        bool CameraTool3D::doStartMouseDrag(const InputState& inputState) {
            if (orbit(inputState)) {
                const auto& hit = inputState.pickResult().query().pickable().type(Model::BrushNode::BrushHitType | Model::EntityNode::EntityHitType).occluded().minDistance(3.0).first();
                if (hit.isMatch()) {
                    m_orbitCenter = vm::vec3f(hit.hitPoint());
                } else {
                    m_orbitCenter = vm::vec3f(Renderer::Camera::defaultPoint(inputState.pickRay()));
                }
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
                const auto hAngle = static_cast<float>(inputState.mouseDX()) * lookSpeedH();
                const auto vAngle = static_cast<float>(inputState.mouseDY()) * lookSpeedV();
                m_camera.orbit(m_orbitCenter, hAngle, vAngle);
                return true;
            } else if (look(inputState)) {
                const auto hAngle = static_cast<float>(inputState.mouseDX()) * lookSpeedH();
                const auto vAngle = static_cast<float>(inputState.mouseDY()) * lookSpeedV();
                m_camera.rotate(hAngle, vAngle);
                return true;
            } else if (pan(inputState)) {
                const auto altMove = pref(Preferences::CameraEnableAltMove);
                vm::vec3f delta;
                if (altMove && inputState.modifierKeysPressed(ModifierKeys::MKAlt)) {
                    delta = delta + static_cast<float>(inputState.mouseDX()) * panSpeedH() * m_camera.right();
                    delta = delta + static_cast<float>(inputState.mouseDY()) * -moveSpeed(altMove) * m_camera.direction();
                } else {
                    delta = delta + static_cast<float>(inputState.mouseDX()) * panSpeedH() * m_camera.right();
                    delta = delta + static_cast<float>(inputState.mouseDY()) * panSpeedV() * m_camera.up();
                }
                m_camera.moveBy(delta);
                return true;
            }
            return false;
        }

        void CameraTool3D::doEndMouseDrag(const InputState&) {
            m_orbit = false;
        }

        void CameraTool3D::doCancelMouseDrag() {
            m_orbit = false;
        }

        bool CameraTool3D::move(const InputState& inputState) const {
            return ((inputState.mouseButtonsPressed(MouseButtons::MBNone) ||
                     inputState.mouseButtonsPressed(MouseButtons::MBRight)) &&
                    inputState.checkModifierKeys(MK_No, MK_No, MK_DontCare));
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
            if (pref(Preferences::CameraLookInvertH)) {
                speed *= -1.0f;
            }
            speed *= std::min(1.0f, m_camera.zoomedFov() / m_camera.fov());
            return speed;
        }

        float CameraTool3D::lookSpeedV() const {
            float speed = pref(Preferences::CameraLookSpeed) / -50.0f;
            if (pref(Preferences::CameraLookInvertV)) {
                speed *= -1.0f;
            }
            speed *= std::min(1.0f, m_camera.zoomedFov() / m_camera.fov());
            return speed;
        }

        float CameraTool3D::panSpeedH() const {
            float speed = pref(Preferences::CameraPanSpeed);
            if (pref(Preferences::CameraPanInvertH)) {
                speed *= -1.0f;
            }
            speed *= std::min(1.0f, m_camera.zoomedFov() / m_camera.fov());
            return speed;
        }

        float CameraTool3D::panSpeedV() const {
            float speed = pref(Preferences::CameraPanSpeed);
            if (pref(Preferences::CameraPanInvertV)) {
                speed *= -1.0f;
            }
            speed *= std::min(1.0f, m_camera.zoomedFov() / m_camera.fov());
            return speed;
        }

        float CameraTool3D::moveSpeed(const bool altMode) const {
            float speed = pref(Preferences::CameraMoveSpeed) * 20.0f;
            if (altMode && pref(Preferences::CameraAltMoveInvert)) {
                speed *= -1.0f;
            }
            speed *= std::min(1.0f, m_camera.zoomedFov() / m_camera.fov());
            return speed;
        }

        bool CameraTool3D::doCancel() {
            return false;
        }
    }
}
