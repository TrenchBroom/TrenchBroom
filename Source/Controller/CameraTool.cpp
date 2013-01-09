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
        
        float CameraTool::moveSpeed() {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            return prefs.getFloat(Preferences::CameraMoveSpeed) * 20.0f;
        }
        
        void CameraTool::handleScroll(InputState& inputState) {
            if (inputState.modifierKeys() != ModifierKeys::MKNone &&
                inputState.modifierKeys() != ModifierKeys::MKAlt)
                return;
            if (m_orbit) {
                const Renderer::Camera& camera = inputState.camera();
                Plane orbitPlane(camera.direction(), m_orbitCenter);
                float maxForward = orbitPlane.intersectWithRay(Ray(camera.position(), camera.direction())) - 32.0f;

                float forward = inputState.scrollY() * moveSpeed();
                if (maxForward < 0.0f)
                    maxForward = 0.0f;
                if (forward > maxForward)
                    forward = maxForward;
                
                CameraMoveEvent cameraEvent;
                cameraEvent.setForward(forward);
                postEvent(cameraEvent);
            } else {
                CameraMoveEvent cameraEvent;
                cameraEvent.setForward(inputState.scrollY() * moveSpeed());
                cameraEvent.setRight(inputState.scrollX() * moveSpeed());
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
                CameraMoveEvent cameraEvent;
                cameraEvent.setRight(inputState.deltaX() * panSpeed(false));
                cameraEvent.setUp(inputState.deltaY() * panSpeed(true));
                postEvent(cameraEvent);
            }
            return true;
        }
        
        void CameraTool::handleEndDrag(InputState& inputState) {
            m_orbit = false;
        }
    }
}
