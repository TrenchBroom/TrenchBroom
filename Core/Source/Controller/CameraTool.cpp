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
#include <cstdio>
#include "Controller/Camera.h"
#include "Controller/Editor.h"
#include "Model/Map/Picker.h"
#include "Model/Preferences.h"

namespace TrenchBroom {
    namespace Controller {
        float CameraTool::lookSpeed(bool vertical) {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            float speed = prefs.cameraLookSpeed() / -50.0f;
            if (vertical && prefs.cameraLookInvertY())
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool::panSpeed(bool vertical) {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            float speed = prefs.cameraPanSpeed() * 1.0f;
            if (vertical)
                speed *= prefs.cameraPanInvertY() ? -1.0f : 1.0f;
            else
                speed *= prefs.cameraPanInvertX() ? 1.0f : -1.0f;
            return speed;
        }
        
        float CameraTool::moveSpeed() {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            return prefs.cameraMoveSpeed() * 12.0f;
        }

        bool CameraTool::handleScrolled(InputEvent& event) {
            if (!cameraModiferPressed(event) && !orbitModifierPressed(event))
                return false;
            
            float forward = event.scrollX * moveSpeed();
            float right = 0;
            float up = 0;
            editor().camera().moveBy(forward, right, up);
            return true;
        }
        
        bool CameraTool::handleBeginDrag(InputEvent& event) {
            if (event.mouseButton == TB_MB_LEFT) {
                if (!cameraModiferPressed(event) && !orbitModifierPressed(event))
                    return false;
                
                if (orbitModifierPressed(event)) {
                    Model::Hit* hit = event.hits->first(Model::TB_HT_ENTITY | Model::TB_HT_FACE, true);
                    if (hit != NULL) m_orbitCenter = hit->hitPoint;
                    else m_orbitCenter = editor().camera().defaultPoint();
                    m_orbit = true;
                }
                return true;
            } else if (event.mouseButton == TB_MB_RIGHT) {
                return cameraModiferPressed(event) || orbitModifierPressed(event);
            }
            
            return false;
        }
        
        bool CameraTool::handleDrag(InputEvent& event) {
            if (event.mouseButton == TB_MB_LEFT) {
                if (m_orbit) {
                    float hAngle = event.deltaX * lookSpeed(false);
                    float vAngle = event.deltaY * lookSpeed(true);
                    editor().camera().orbit(m_orbitCenter, hAngle, vAngle);
                } else {
                    float yawAngle = event.deltaX * lookSpeed(false);
                    float pitchAngle = event.deltaY * lookSpeed(true);
                    editor().camera().rotate(yawAngle, pitchAngle);
                }
                
                return true;
            } else if (event.mouseButton == TB_MB_RIGHT) {
                float forward = 0;
                float right = event.deltaX * panSpeed(false);
                float up = event.deltaY * panSpeed(true);
                editor().camera().moveBy(forward, right, up);
                return true;
            }
            
            return false;
        }
        
        void CameraTool::handleEndDrag(InputEvent& event) {
            m_orbit = false;
        }

        bool CameraTool::cameraModiferPressed(InputEvent& event) {
            return event.modifierKeys == Model::Preferences::sharedPreferences->cameraKey();
        }
        
        bool CameraTool::orbitModifierPressed(InputEvent& event) {
            return event.modifierKeys == Model::Preferences::sharedPreferences->cameraOrbitKey();
        }
    }
}