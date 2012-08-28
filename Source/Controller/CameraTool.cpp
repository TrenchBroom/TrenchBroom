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

#include "Controller/Input.h"
#include "Utility/Preferences.h"
#include "Renderer/Camera.h"

namespace TrenchBroom {
    namespace Controller {
        float CameraTool::lookSpeed(bool vertical) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float speed = prefs.getFloat(Preferences::CameraLookSpeed);
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
            return prefs.getFloat(Preferences::CameraMoveSpeed) * 12.0f;
        }
        
        bool CameraTool::handleScrolled(InputEvent& event) {
            if (!cameraModifierPressed(event) && !orbitModifierPressed(event))
                return false;
            
            float forward = event.scrollX * moveSpeed();
            float right = 0;
            float up = 0;
            m_camera.moveBy(forward, right, up);
            return true;
        }
        
        bool CameraTool::handleBeginDrag(InputEvent& event) {
            if (event.mouseButtons == MouseButtons::Left) {
                if (!cameraModifierPressed(event) && !orbitModifierPressed(event))
                    return false;
                
                /*
                if (orbitModifierPressed(event)) {
                    Model::Hit* hit = event.pickResults->first(Model::TB_HT_ENTITY | Model::TB_HT_FACE, true);
                    if (hit != NULL) m_orbitCenter = hit->hitPoint;
                    else m_orbitCenter = m_camera.defaultPoint();
                    m_orbit = true;
                }
                 */
                return true;
            } else if (event.mouseButtons == MouseButtons::Right) {
                return cameraModifierPressed(event) || orbitModifierPressed(event);
            }
            
            return false;
        }
        
        bool CameraTool::handleDrag(InputEvent& event) {
            if (event.mouseButtons == MouseButtons::Left) {
                if (m_orbit) {
                    float hAngle = event.deltaX * lookSpeed(false);
                    float vAngle = event.deltaY * lookSpeed(true);
                    m_camera.orbit(m_orbitCenter, hAngle, vAngle);
                } else {
                    float yawAngle = event.deltaX * lookSpeed(false);
                    float pitchAngle = event.deltaY * lookSpeed(true);
                    m_camera.rotate(yawAngle, pitchAngle);
                }
                
                return true;
            } else if (event.mouseButtons == MouseButtons::Right) {
                float forward = 0;
                float right = event.deltaX * panSpeed(false);
                float up = event.deltaY * panSpeed(true);
                m_camera.moveBy(forward, right, up);
                return true;
            }
            
            return false;
        }
        
        void CameraTool::handleEndDrag(InputEvent& event) {
            m_orbit = false;
        }
        
        bool CameraTool::cameraModifierPressed(InputEvent& event) {
            return false;
        }
        
        bool CameraTool::orbitModifierPressed(InputEvent& event) {
            return false;
        }
    }
}