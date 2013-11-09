/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CameraTool.h"

#include "Controller/CameraEvent.h"
#include "Model/BrushTypes.h"
#include "Model/Entity.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Controller {
        float CameraTool::lookSpeed(bool vertical) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float speed = prefs.getFloat(Preferences::CameraLookSpeed) / -50.0f;
            if (vertical) {
                if (prefs.getBool(Preferences::CameraLookInvertY))
                    speed *= -1.0f;
            } else if (prefs.getBool(Preferences::CameraLookInvertX)) {
                speed *= -1.0f;
            }
            return speed;
        }
        
        float CameraTool::panSpeed(bool vertical) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float speed = prefs.getFloat(Preferences::CameraPanSpeed);
            if (vertical)
                speed *= prefs.getBool(Preferences::CameraPanInvertY) ? -1.0f : 1.0f;
            else
                speed *= prefs.getBool(Preferences::CameraPanInvertX) ? 1.0f : -1.0f;
            return speed;
        }
        
        float CameraTool::moveSpeed(bool altMode) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float speed = prefs.getFloat(Preferences::CameraMoveSpeed) * 20.0f;
            if (altMode && prefs.getBool(Preferences::CameraAltModeInvertAxis))
                speed *= -1.0f;
            return speed;
        }
        
        void CameraTool::handleScroll(InputState& inputState) {
            if (inputState.modifierKeys() != ModifierKeys::MKNone &&
                inputState.modifierKeys() != ModifierKeys::MKAlt)
                return;
            if (m_orbit) {
                const Renderer::Camera& camera = inputState.camera();
                Planef orbitPlane(camera.direction(), m_orbitCenter);
                float maxForward = orbitPlane.intersectWithRay(Rayf(camera.position(), camera.direction())) - 32.0f;

                float forward = inputState.scrollY() * moveSpeed(false);
                if (maxForward < 0.0f)
                    maxForward = 0.0f;
                if (forward > maxForward)
                    forward = maxForward;
                
                CameraMoveEvent cameraEvent;
                cameraEvent.setForward(forward);
                postEvent(cameraEvent);
            } else {
                Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
                const Renderer::Camera& camera = inputState.camera();
                const Vec3f moveDirection = prefs.getBool(Preferences::CameraMoveInCursorDir) ? inputState.pickRay().direction : camera.direction();
                
                const float distance = inputState.scrollY() * moveSpeed(false);
                const Vec3f moveVector = distance * moveDirection;
                
                CameraMoveEvent cameraEvent;
                cameraEvent.setForward(moveVector.dot(camera.direction()));
                cameraEvent.setRight(moveVector.dot(camera.right()));
                cameraEvent.setUp(moveVector.dot(camera.up()));
                postEvent(cameraEvent);
            }
        }

        bool CameraTool::handleStartDrag(InputState& inputState) {
            if (inputState.mouseButtons() == MouseButtons::MBRight) {
                if (inputState.modifierKeys() == ModifierKeys::MKAlt) {
                    Model::Hit* hit = inputState.pickResult().first(Model::HitType::ObjectHit, true, m_filter);
                    if (hit != NULL)
                        m_orbitCenter = hit->hitPoint();
                    else
                        m_orbitCenter = inputState.camera().defaultPoint();
                    m_orbit = true;
                    return true;
                } else if (inputState.modifierKeys() == ModifierKeys::MKNone) {
                    return true;
                }
            } else if (inputState.mouseButtons() == MouseButtons::MBMiddle &&
                       (inputState.modifierKeys() == ModifierKeys::MKNone || inputState.modifierKeys() == ModifierKeys::MKAlt)) {
                return true;
            }
            
            return false;
        }
        
        bool CameraTool::handleDrag(InputState& inputState) {
            if (inputState.mouseButtons() == MouseButtons::MBRight) {
                if (m_orbit) {
                    CameraOrbitEvent cameraEvent;
                    cameraEvent.setHAngle(inputState.deltaX() * lookSpeed(false));
                    cameraEvent.setVAngle(inputState.deltaY() * lookSpeed(true));
                    cameraEvent.setCenter(m_orbitCenter);
                    postEvent(cameraEvent);
                } else {
                    CameraLookEvent cameraEvent;
                    cameraEvent.setHAngle(inputState.deltaX() * lookSpeed(false));
                    cameraEvent.setVAngle(inputState.deltaY() * lookSpeed(true));
                    postEvent(cameraEvent);
                }
            } else if (inputState.mouseButtons() == MouseButtons::MBMiddle) {
                Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
                bool enableAltMove = prefs.getBool(Preferences::CameraEnableAltMove);

                CameraMoveEvent cameraEvent;
                if (enableAltMove && inputState.modifierKeys() == ModifierKeys::MKAlt) {
                    cameraEvent.setRight(inputState.deltaX() * panSpeed(false));
                    cameraEvent.setForward(inputState.deltaY() * -moveSpeed(true));
                } else {
                    cameraEvent.setRight(inputState.deltaX() * panSpeed(false));
                    cameraEvent.setUp(inputState.deltaY() * panSpeed(true));
                }
                postEvent(cameraEvent);
            }
            return true;
        }
        
        void CameraTool::handleEndDrag(InputState& inputState) {
            m_orbit = false;
        }
    }
}
