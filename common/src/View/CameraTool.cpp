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

#include "CameraTool.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/HitAdapter.h"
#include "Model/ModelHitFilters.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "Renderer/Camera.h"
#include "Renderer/OrthographicCamera.h"

#include <iostream>

namespace TrenchBroom {
    namespace View {
        CameraTool::CameraTool(MapDocumentWPtr document) :
        ToolImpl(document),
        m_camera(NULL),
        m_orbit(false) {}
        
        void CameraTool::setCamera(Renderer::Camera* camera) {
            m_camera = camera;
        }

        void CameraTool::fly(int dx, int dy, const bool forward, const bool backward, const bool left, const bool right, const unsigned int time) {
            assert(m_camera != NULL);
            
            static const float speed = 256.0f / 1000.0f; // 64 units per second
            const float dist  = speed * time;
            
            Vec3 delta;
            if (forward)
                delta += m_camera->direction() * dist;
            if (backward)
                delta -= m_camera->direction() * dist;
            if (left)
                delta -= m_camera->right() * dist;
            if (right)
                delta += m_camera->right() * dist;
            m_camera->moveBy(delta);
            
            const float hAngle = static_cast<float>(dx) * lookSpeedH();
            const float vAngle = static_cast<float>(dy) * lookSpeedV();
            m_camera->rotate(hAngle, vAngle);
        }

        void CameraTool::doScroll(const InputState& inputState) {
            assert(m_camera != NULL);

            if (m_orbit) {
                const Plane3f orbitPlane(m_orbitCenter, m_camera->direction());
                const float maxDistance = std::max(orbitPlane.intersectWithRay(m_camera->viewRay()) - 32.0f, 0.0f);
                const float distance = std::min(inputState.scrollY() * moveSpeed(false), maxDistance);
                m_camera->moveBy(distance * m_camera->direction());
            } else if (move(inputState)) {
                PreferenceManager& prefs = PreferenceManager::instance();
                const Vec3f moveDirection = prefs.get(Preferences::CameraMoveInCursorDir) ? Vec3f(inputState.pickRay().direction) : m_camera->direction();
                const float distance = inputState.scrollY() * moveSpeed(false);
                m_camera->moveBy(distance * moveDirection);
            } else if (zoom(inputState)) {
                const float speed = 1.0f;
                if (inputState.scrollY() != 0.0f) {
                    const Vec2f mousePos(static_cast<float>(inputState.mouseX()), static_cast<float>(inputState.mouseY()));
                    const Vec3f oldWorldPos = m_camera->unproject(mousePos.x(), mousePos.y(), 0.0f);
                    
                    const Vec2f factors = Vec2f::One * (1.0f + inputState.scrollY() / 50.0f * speed);
                    Renderer::OrthographicCamera* orthoCam = static_cast<Renderer::OrthographicCamera*>(m_camera);
                    orthoCam->zoom(factors);

                    const Vec3f newWorldPos = m_camera->unproject(mousePos.x(), mousePos.y(), 0.0f);
                    const Vec3f delta = newWorldPos - oldWorldPos;
                    m_camera->moveBy(-delta);
                }
            }
        }

        bool CameraTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_camera != NULL);
            
            if (orbit(inputState)) {
                const Hit& hit = Model::firstHit(inputState.hits(), Model::Brush::BrushHit | Model::Entity::EntityHit, document()->editorContext(), true);
                if (hit.isMatch()) {
                    m_orbit = true;
                    m_orbitCenter = hit.hitPoint();
                }
                return true;
            } else if (look(inputState)) {
                return true;
            } else if (pan3D(inputState)) {
                return true;
            } else if (pan2D(inputState)) {
                m_lastMousePos = Vec2f(static_cast<float>(inputState.mouseX()),
                                       static_cast<float>(inputState.mouseY()));
                return true;
            }
            return false;
        }
        
        bool CameraTool::doMouseDrag(const InputState& inputState) {
            assert(m_camera != NULL);
            
            if (m_orbit) {
                const float hAngle = inputState.mouseDX() * lookSpeedH();
                const float vAngle = inputState.mouseDY() * lookSpeedV();
                m_camera->orbit(m_orbitCenter, hAngle, vAngle);
                return true;
            } else if (look(inputState)) {
                const float hAngle = inputState.mouseDX() * lookSpeedH();
                const float vAngle = inputState.mouseDY() * lookSpeedV();
                m_camera->rotate(hAngle, vAngle);
                return true;
            } else if (pan3D(inputState)) {
                PreferenceManager& prefs = PreferenceManager::instance();
                const bool altMove = prefs.get(Preferences::CameraEnableAltMove);
                Vec3f delta;
                if (altMove && inputState.modifierKeysPressed(ModifierKeys::MKAlt)) {
                    delta += inputState.mouseDX() * panSpeedH() * m_camera->right();
                    delta += inputState.mouseDY() * -moveSpeed(altMove) * m_camera->direction();
                } else {
                    delta += inputState.mouseDX() * panSpeedH() * m_camera->right();
                    delta += inputState.mouseDY() * panSpeedV() * m_camera->up();
                }
                m_camera->moveBy(delta);
                return true;
            } else if (pan2D(inputState)) {
                const Vec2f currentMousePos(static_cast<float>(inputState.mouseX()), static_cast<float>(inputState.mouseY()));
                const Vec3f lastWorldPos = m_camera->unproject(m_lastMousePos.x(), m_lastMousePos.y(), 0.0f);
                const Vec3f currentWorldPos = m_camera->unproject(currentMousePos.x(), currentMousePos.y(), 0.0f);
                const Vec3f delta = currentWorldPos - lastWorldPos;
                m_camera->moveBy(-delta);
                m_lastMousePos = currentMousePos;
                return true;
            }
            return false;
        }
        
        void CameraTool::doEndMouseDrag(const InputState& inputState) {
            m_orbit = false;
        }
        
        void CameraTool::doCancelMouseDrag() {
            m_orbit = false;
        }

        bool CameraTool::move(const InputState& inputState) const {
            return move2D(inputState) || move3D(inputState);
        }

        bool CameraTool::move2D(const InputState& inputState) const {
            return false;
        }
        
        bool CameraTool::move3D(const InputState& inputState) const {
            return (inputState.inputSource() == IS_MapView3D &&
                    (inputState.mouseButtonsPressed(MouseButtons::MBNone) ||
                     inputState.mouseButtonsPressed(MouseButtons::MBRight)) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKNone));
        }

        bool CameraTool::zoom(const InputState& inputState) const {
            return (inputState.inputSource() != IS_MapView3D &&
                    inputState.mouseButtonsPressed(MouseButtons::MBNone) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKNone));
        }

        bool CameraTool::look(const InputState& inputState) const {
            return look2D(inputState) || look3D(inputState);
        }
        
        bool CameraTool::look2D(const InputState& inputState) const {
            return false;
        }
        
        bool CameraTool::look3D(const InputState& inputState) const {
            return (inputState.inputSource() == IS_MapView3D &&
                    inputState.mouseButtonsPressed(MouseButtons::MBRight) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKNone));
        }

        bool CameraTool::pan(const InputState& inputState) const {
            return pan2D(inputState) || pan3D(inputState);
        }
        
        bool CameraTool::pan2D(const InputState& inputState) const {
            return (inputState.inputSource() != IS_MapView3D &&
                    inputState.mouseButtonsPressed(MouseButtons::MBRight));
        }
        
        bool CameraTool::pan3D(const InputState& inputState) const {
            return (inputState.inputSource() == IS_MapView3D &&
                    inputState.mouseButtonsPressed(MouseButtons::MBMiddle) &&
                    (inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                     inputState.modifierKeysPressed(ModifierKeys::MKAlt)));
        }

        bool CameraTool::orbit(const InputState& inputState) const {
            return (inputState.inputSource() == IS_MapView3D &&
                    inputState.mouseButtonsPressed(MouseButtons::MBRight) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKAlt));
        }

        float CameraTool::lookSpeedH() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            float speed = prefs.get(Preferences::CameraLookSpeed) / -50.0f;
            if (prefs.get(Preferences::CameraLookInvertH))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool::lookSpeedV() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            float speed = prefs.get(Preferences::CameraLookSpeed) / -50.0f;
            if (prefs.get(Preferences::CameraLookInvertV))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool::panSpeedH() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            float speed = prefs.get(Preferences::CameraPanSpeed);
            if (prefs.get(Preferences::CameraPanInvertH))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool::panSpeedV() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            float speed = prefs.get(Preferences::CameraPanSpeed);
            if (prefs.get(Preferences::CameraPanInvertV))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool::moveSpeed(const bool altMode) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            float speed = prefs.get(Preferences::CameraMoveSpeed) * 20.0f;
            if (altMode && prefs.get(Preferences::CameraAltMoveInvert))
                speed *= -1.0f;
            return speed;
        }
    }
}
