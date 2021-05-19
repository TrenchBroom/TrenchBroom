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
#include "Model/HitFilter.h"
#include "Model/ModelUtils.h"
#include "Model/PatchNode.h"
#include "Model/PickResult.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "Renderer/PerspectiveCamera.h"

#include <vecmath/forward.h>
#include <vecmath/intersection.h>
#include <vecmath/plane.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace View {
        CameraTool3D::CameraTool3D(std::weak_ptr<MapDocument> document, Renderer::PerspectiveCamera& camera) :
        ToolControllerBase{},
        Tool{true},
        m_document{document},
        m_camera{camera},
        m_orbit{false} {}

        void CameraTool3D::fly(const int dx, const int dy, const bool forward, const bool backward, const bool left, const bool right, const unsigned int time) {
            constexpr float speed = 256.0f / 1000.0f; // 64 units per second
            const float dist  = speed * static_cast<float>(time);

            auto delta = vm::vec3f{};
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

            const float hAngle = static_cast<float>(dx) * lookSpeedH();
            const float vAngle = static_cast<float>(dy) * lookSpeedV();
            m_camera.rotate(hAngle, vAngle);
        }

        Tool* CameraTool3D::doGetTool() {
            return this;
        }

        const Tool* CameraTool3D::doGetTool() const {
            return this;
        }

        void CameraTool3D::doMouseScroll(const InputState& inputState) {
            const float factor = pref(Preferences::CameraMouseWheelInvert) ? -1.0f : 1.0f;
            const bool zoom = inputState.modifierKeysPressed(ModifierKeys::MKShift);
            const float scrollDist = inputState.scrollY();

            if (m_orbit) {
                const auto orbitPlane = vm::plane3f{m_orbitCenter, m_camera.direction()};
                const float maxDistance = vm::max(vm::intersect_ray_plane(m_camera.viewRay(), orbitPlane) - 32.0f, 0.0f);
                const float distance = vm::min(factor * scrollDist * moveSpeed(false), maxDistance);
                m_camera.moveBy(distance * m_camera.direction());
            } else if (adjustFlySpeed(inputState)) {
                const float speed = pref(Preferences::CameraFlyMoveSpeed);
                // adjust speed by 5% of the current speed per scroll line
                const float deltaSpeed = factor * speed * 0.05f * scrollDist;
                const float newSpeed = vm::clamp(speed + deltaSpeed, Preferences::MinCameraFlyMoveSpeed, Preferences::MaxCameraFlyMoveSpeed);

                // prefs are only changed when releasing RMB
                auto& prefs = PreferenceManager::instance();
                prefs.set(Preferences::CameraFlyMoveSpeed, newSpeed);
            } else if (move(inputState)) {
                if (zoom) {
                    const float zoomFactor = 1.0f + scrollDist / 50.0f * factor;
                    m_camera.zoom(zoomFactor);
                } else {
                    const auto moveDirection = pref(Preferences::CameraMoveInCursorDir) ? vm::vec3f{inputState.pickRay().direction} : m_camera.direction();
                    const float distance = scrollDist * moveSpeed(false);
                    m_camera.moveBy(factor * distance * moveDirection);
                }
            }
        }

        void CameraTool3D::doMouseUp(const InputState& inputState) {
            if (inputState.mouseButtonsPressed(MouseButtons::MBRight)) {
                auto& prefs = PreferenceManager::instance();
                if (!prefs.saveInstantly()) {
                    prefs.saveChanges();
                }
            }
        }

        bool CameraTool3D::doStartMouseDrag(const InputState& inputState) {
            using namespace Model::HitFilters;

            if (orbit(inputState)) {
                const auto& hit = inputState.pickResult().first(type(Model::nodeHitType()) && minDistance(3.0));
                if (hit.isMatch()) {
                    m_orbitCenter = vm::vec3f{hit.hitPoint()};
                } else {
                    m_orbitCenter = vm::vec3f{Renderer::Camera::defaultPoint(inputState.pickRay())};
                }
                m_orbit = true;
                return true;
            }
            
            if (look(inputState)) {
                return true;
            }
            
            if (pan(inputState)) {
                return true;
            }

            return false;
        }

        bool CameraTool3D::doMouseDrag(const InputState& inputState) {
            if (m_orbit) {
                const float hAngle = static_cast<float>(inputState.mouseDX()) * lookSpeedH();
                const float vAngle = static_cast<float>(inputState.mouseDY()) * lookSpeedV();
                m_camera.orbit(m_orbitCenter, hAngle, vAngle);
                return true;
            }
            
            if (look(inputState)) {
                const float hAngle = static_cast<float>(inputState.mouseDX()) * lookSpeedH();
                const float vAngle = static_cast<float>(inputState.mouseDY()) * lookSpeedV();
                m_camera.rotate(hAngle, vAngle);
                return true;
            }
            
            if (pan(inputState)) {
                const bool altMove = pref(Preferences::CameraEnableAltMove);
                auto delta = vm::vec3f{};
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
            return (inputState.mouseButtonsPressed(MouseButtons::MBNone) &&
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

        bool CameraTool3D::adjustFlySpeed(const InputState& inputState) const {
            return (inputState.mouseButtonsPressed(MouseButtons::MBRight) &&
                    inputState.checkModifierKeys(MK_No, MK_No, MK_No));
        }

        float CameraTool3D::lookSpeedH() const {
            float speed = pref(Preferences::CameraLookSpeed) / -50.0f;
            if (pref(Preferences::CameraLookInvertH)) {
                speed *= -1.0f;
            }
            speed *= vm::min(1.0f, m_camera.zoomedFov() / m_camera.fov());
            return speed;
        }

        float CameraTool3D::lookSpeedV() const {
            float speed = pref(Preferences::CameraLookSpeed) / -50.0f;
            if (pref(Preferences::CameraLookInvertV)) {
                speed *= -1.0f;
            }
            speed *= vm::min(1.0f, m_camera.zoomedFov() / m_camera.fov());
            return speed;
        }

        float CameraTool3D::panSpeedH() const {
            float speed = pref(Preferences::CameraPanSpeed);
            if (pref(Preferences::CameraPanInvertH)) {
                speed *= -1.0f;
            }
            speed *= vm::min(1.0f, m_camera.zoomedFov() / m_camera.fov());
            return speed;
        }

        float CameraTool3D::panSpeedV() const {
            float speed = pref(Preferences::CameraPanSpeed);
            if (pref(Preferences::CameraPanInvertV)) {
                speed *= -1.0f;
            }
            speed *= vm::min(1.0f, m_camera.zoomedFov() / m_camera.fov());
            return speed;
        }

        float CameraTool3D::moveSpeed(const bool altMode) const {
            float speed = pref(Preferences::CameraMoveSpeed) * 20.0f;
            if (altMode && pref(Preferences::CameraAltMoveInvert)) {
                speed *= -1.0f;
            }
            speed *= vm::min(1.0f, m_camera.zoomedFov() / m_camera.fov());
            return speed;
        }

        bool CameraTool3D::doCancel() {
            return false;
        }
    }
}
