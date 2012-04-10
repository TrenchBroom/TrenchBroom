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
#include "Camera.h"
#include "Editor.h"
#include "Picker.h"

namespace TrenchBroom {
    namespace Controller {
        bool CameraTool::scrolled(ToolEvent& event) {
            if (!cameraModiferPressed(event) && !orbitModifierPressed(event)) return false;
            
            float forward = event.scrollX * m_moveSensitivity;
            float right = 0;
            float up = 0;
            m_editor.camera().moveBy(forward, right, up);
            return true;
        }
        
        bool CameraTool::beginLeftDrag(ToolEvent& event) {
            if (!cameraModiferPressed(event) && !orbitModifierPressed(event)) return false;
            
            if (orbitModifierPressed(event)) {
                Model::Hit* hit = event.hits->first(Model::HT_ENTITY | Model::HT_FACE, true);
                if (hit != NULL) m_orbitCenter = hit->hitPoint;
                else m_orbitCenter = m_editor.camera().defaultPoint();
                m_orbit = true;
            }
            return true;
        }
        
        void CameraTool::leftDrag(ToolEvent& event) {
            if (m_orbit) {
                float hAngle = -event.deltaX * m_lookSensitivity;
                float vAngle = event.deltaY * m_lookSensitivity * (m_invert ? 1 : -1);
                m_editor.camera().orbit(m_orbitCenter, hAngle, vAngle);
            } else {
                float yawAngle = -event.deltaX * m_lookSensitivity;
                float pitchAngle = event.deltaY * m_lookSensitivity * (m_invert ? 1 : -1);
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
            float forward = 0;
            float right = event.deltaX * m_panSensitivity;
            float up = -event.deltaY * m_panSensitivity;
            m_editor.camera().moveBy(forward, right, up);
        }
        
        bool CameraTool::cameraModiferPressed(ToolEvent& event) {
            return event.modifierKeys == MK_SHIFT;
        }
        
        bool CameraTool::orbitModifierPressed(ToolEvent& event) {
            return event.modifierKeys == (MK_SHIFT | MK_CMD);
        }
    }
}