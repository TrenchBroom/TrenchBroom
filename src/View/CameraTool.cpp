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

#include "Preferences.h"
#include "PreferenceManager.h"
#include "View/InputState.h"
#include "Renderer/Camera.h"

namespace TrenchBroom {
    namespace View {
        CameraTool::CameraTool(BaseTool* next, Renderer::Camera& camera) :
        Tool(next),
        m_camera(camera),
        m_orbit(false) {}
        
        bool CameraTool::doStartMouseDrag(const InputState& inputState) {
            if (orbit(inputState)) {
            } else if (look(inputState)) {
                return true;
            } else if (pan(inputState)) {
                return true;
            }
            return false;
        }
        
        bool CameraTool::doMouseDrag(const InputState& inputState) {
            if (m_orbit) {
            } else if (look(inputState)) {
                const float hAngle = inputState.mouseDX() * lookSpeedH();
                const float vAngle = inputState.mouseDY() * lookSpeedV();
                m_camera.rotate(hAngle, vAngle);
                return true;
            } else if (pan(inputState)) {
                PreferenceManager prefs = PreferenceManager::instance();
                const bool altMove = prefs.getBool(Preferences::CameraEnableAltMove);
                Vec3f delta;
                if (altMove && inputState.modifierKeysPressed(ModifierKeys::MKAlt)) {
                    delta += inputState.mouseDX() * panSpeedH() * m_camera.right();
                    delta += inputState.mouseDY() * -moveSpeed() * m_camera.direction();
                } else {
                    delta += inputState.mouseDX() * panSpeedH() * m_camera.right();
                    delta += inputState.mouseDY() * panSpeedV() * m_camera.up();
                }
                m_camera.moveBy(delta);
                return true;
            }
            return false;
        }
        
        void CameraTool::doEndMouseDrag(const InputState& inputState) {
            m_orbit = false;
        }
        
        void CameraTool::doCancelMouseDrag(const InputState& inputState) {
            m_orbit = false;
        }

        bool CameraTool::look(const InputState& inputState) const {
            return (inputState.mouseButtonsPressed(MouseButtons::MBRight) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKNone));
        }
        
        bool CameraTool::pan(const InputState& inputState) const {
            return (inputState.mouseButtonsPressed(MouseButtons::MBMiddle) &&
                    (inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                     inputState.modifierKeysPressed(ModifierKeys::MKAlt)));
        }
        
        bool CameraTool::orbit(const InputState& inputState) const {
            return (inputState.mouseButtonsPressed(MouseButtons::MBRight) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKAlt));
        }

        float CameraTool::lookSpeedH() const {
            PreferenceManager prefs = PreferenceManager::instance();
            float speed = prefs.getFloat(Preferences::CameraLookSpeed) / -50.0f;
            if (prefs.getBool(Preferences::CameraLookInvertH))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool::lookSpeedV() const {
            PreferenceManager prefs = PreferenceManager::instance();
            float speed = prefs.getFloat(Preferences::CameraLookSpeed) / -50.0f;
            if (prefs.getBool(Preferences::CameraLookInvertV))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool::panSpeedH() const {
            PreferenceManager prefs = PreferenceManager::instance();
            float speed = prefs.getFloat(Preferences::CameraPanSpeed);
            if (prefs.getBool(Preferences::CameraPanInvertH))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool::panSpeedV() const {
            PreferenceManager prefs = PreferenceManager::instance();
            float speed = prefs.getFloat(Preferences::CameraPanSpeed);
            if (prefs.getBool(Preferences::CameraPanInvertV))
                speed *= -1.0f;
            return speed;
        }
        
        float CameraTool::moveSpeed() const {
            PreferenceManager prefs = PreferenceManager::instance();
            return prefs.getFloat(Preferences::CameraMoveSpeed) * 20.0f;
        }
    }
}
