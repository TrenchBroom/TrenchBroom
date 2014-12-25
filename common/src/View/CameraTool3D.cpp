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
#include "Model/HitAdapter.h"
#include "Model/ModelHitFilters.h"
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
            if (m_orbit) {
                const Plane3f orbitPlane(m_orbitCenter, m_camera.direction());
                const float maxDistance = std::max(orbitPlane.intersectWithRay(m_camera.viewRay()) - 32.0f, 0.0f);
                const float distance = std::min(inputState.scrollY() * moveSpeed(false), maxDistance);
                m_camera.moveBy(distance * m_camera.direction());
            } else if (move(inputState)) {
                PreferenceManager& prefs = PreferenceManager::instance();
                const Vec3f moveDirection = prefs.get(Preferences::CameraMoveInCursorDir) ? Vec3f(inputState.pickRay().direction) : m_camera.direction();
                const float distance = inputState.scrollY() * moveSpeed(false);
                m_camera.moveBy(distance * moveDirection);
            }
        }
        
        bool CameraTool3D::doStartMouseDrag(const InputState& inputState) {
            if (orbit(inputState)) {
                MapDocumentSPtr document = lock(m_document);
                const Hit& hit = Model::firstHit(inputState.hits(), Model::Brush::BrushHit | Model::Entity::EntityHit, document->editorContext(), true);
                if (hit.isMatch()) {
                    m_orbit = true;
                    m_orbitCenter = hit.hitPoint();
                }
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
                PreferenceManager& prefs = PreferenceManager::instance();
                const bool altMove = prefs.get(Preferences::CameraEnableAltMove);
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
                    inputState.modifierKeysPressed(ModifierKeys::MKNone));
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
            PreferenceManager& prefs = PreferenceManager::instance();
            float speed = prefs.get(Preferences::CameraLookSpeed) / -50.0f;
            if (prefs.get(Preferences::CameraLookInvertH))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool3D::lookSpeedV() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            float speed = prefs.get(Preferences::CameraLookSpeed) / -50.0f;
            if (prefs.get(Preferences::CameraLookInvertV))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool3D::panSpeedH() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            float speed = prefs.get(Preferences::CameraPanSpeed);
            if (prefs.get(Preferences::CameraPanInvertH))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool3D::panSpeedV() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            float speed = prefs.get(Preferences::CameraPanSpeed);
            if (prefs.get(Preferences::CameraPanInvertV))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool3D::moveSpeed(const bool altMode) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            float speed = prefs.get(Preferences::CameraMoveSpeed) * 20.0f;
            if (altMode && prefs.get(Preferences::CameraAltMoveInvert))
                speed *= -1.0f;
            return speed;
        }
        
        bool CameraTool3D::doCancel() {
            return false;
        }
    }
}
