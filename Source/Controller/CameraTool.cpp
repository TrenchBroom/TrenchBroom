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
#include "Controller/Input.h"
#include "Utility/Preferences.h"
#include "Renderer/Camera.h"

#include <wx/event.h>

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
            return prefs.getFloat(Preferences::CameraMoveSpeed);
        }
        
        bool CameraTool::handleScrolled(InputEvent& event) {
            if (event.modifierKeys != ModifierKeys::None)
                return false;
            
            CameraMoveEvent cameraEvent;
            cameraEvent.setForward(event.scrollY * moveSpeed() / 10.0f);
            cameraEvent.setRight(event.scrollX * moveSpeed() / 10.0f);
            postEvent(cameraEvent);
            return true;
        }
        
        bool CameraTool::handleBeginDrag(InputEvent& event) {
            if(event.mouseButtons == MouseButtons::Right) {
                if (event.modifierKeys == ModifierKeys::Shift) {
                    /*
                    Model::Hit* hit = event.pickResults->first(Model::TB_HT_ENTITY | Model::TB_HT_FACE, true);
                    if (hit != NULL) m_orbitCenter = hit->hitPoint;
                    else 
                    m_orbitCenter = m_camera.defaultPoint();
                    m_orbit = true;
                     */
                    return true;
                } else if (event.modifierKeys == ModifierKeys::None) {
                    return true;
                }
            } else if (event.mouseButtons == (MouseButtons::Left | MouseButtons::Right) && event.modifierKeys == ModifierKeys::None) {
                return true;
            }
            
            return false;
        }
        
        bool CameraTool::handleDrag(InputEvent& event) {
            if (event.mouseButtons == MouseButtons::Right) {
                if (m_orbit) {
                    CameraOrbitEvent cameraEvent;
                    cameraEvent.setHAngle(event.deltaX * lookSpeed(false));
                    cameraEvent.setVAngle(event.deltaY * lookSpeed(true));
                    cameraEvent.setCenter(m_orbitCenter);
                    postEvent(cameraEvent);
                } else {
                    CameraLookEvent cameraEvent;
                    cameraEvent.setHAngle(event.deltaX * lookSpeed(false));
                    cameraEvent.setVAngle(event.deltaY * lookSpeed(true));
                    postEvent(cameraEvent);
                }
                
                return true;
            } else if (event.mouseButtons == (MouseButtons::Left | MouseButtons::Right) && event.modifierKeys == ModifierKeys::None) {
                CameraMoveEvent cameraEvent;
                cameraEvent.setRight(event.deltaX * panSpeed(false));
                cameraEvent.setUp(event.deltaY * panSpeed(true));
                postEvent(cameraEvent);
                return true;
            }
            
            return false;
        }
        
        void CameraTool::handleEndDrag(InputEvent& event) {
            m_orbit = false;
        }
    }
}