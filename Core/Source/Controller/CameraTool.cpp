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
        bool CameraTool::scrolled(ToolEvent& event) {
            if (!cameraModiferPressed(event) && !orbitModifierPressed(event)) return false;
            
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            float forward = event.scrollX * prefs.cameraMoveSpeed();
            float right = 0;
            float up = 0;
            m_editor.camera().moveBy(forward, right, up);
            return true;
        }
        
        bool CameraTool::beginLeftDrag(ToolEvent& event) {
            if (!cameraModiferPressed(event) && !orbitModifierPressed(event)) return false;
            
            if (orbitModifierPressed(event)) {
                Model::Hit* hit = event.hits->first(Model::TB_HT_ENTITY | Model::TB_HT_FACE, true);
                if (hit != NULL) m_orbitCenter = hit->hitPoint;
                else m_orbitCenter = m_editor.camera().defaultPoint();
                m_orbit = true;
            }
            return true;
        }
        
        void CameraTool::leftDrag(ToolEvent& event) {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            if (m_orbit) {
                float hAngle = -event.deltaX * prefs.cameraLookSpeed();
                float vAngle = event.deltaY * prefs.cameraLookSpeed() * (prefs.cameraLookInvertY() ? -1.0f : 1.0f);
                m_editor.camera().orbit(m_orbitCenter, hAngle, vAngle);
            } else {
                float yawAngle = -event.deltaX * prefs.cameraLookSpeed();
                float pitchAngle = event.deltaY * prefs.cameraLookSpeed() * (prefs.cameraLookInvertY() ? -1.0f : 1.0f);
                m_editor.camera().rotate(yawAngle, pitchAngle);
            }
        }
        
        void CameraTool::endLeftDrag(ToolEvent& event) {
            m_orbit = false;
        }
        
        bool CameraTool::beginRightDrag(ToolEvent& event) {
            return cameraModiferPressed(event) || orbitModifierPressed(event);
        }
        
        void CameraTool::rightDrag(ToolEvent& event) {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            float forward = 0;
            float right = event.deltaX * prefs.cameraPanSpeed() * (prefs.cameraPanInvertX() ? -1.0f : 1.0f);
            float up = event.deltaY * prefs.cameraPanSpeed() * (prefs.cameraPanInvertY() ? -1.0f : 1.0f);
            m_editor.camera().moveBy(forward, right, up);
        }
        
        bool CameraTool::cameraModiferPressed(ToolEvent& event) {
            return event.modifierKeys == Model::Preferences::sharedPreferences->cameraKey();
        }
        
        bool CameraTool::orbitModifierPressed(ToolEvent& event) {
            return event.modifierKeys == Model::Preferences::sharedPreferences->cameraOrbitKey();
        }
    }
}