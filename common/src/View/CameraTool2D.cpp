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

#include "CameraTool2D.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "View/InputState.h"
#include "Renderer/OrthographicCamera.h"

namespace TrenchBroom {
    namespace View {
        CameraTool2D::CameraTool2D(Renderer::OrthographicCamera& camera) :
        ToolAdapterBase(),
        Tool(true),
        m_camera(camera) {}
        
        Tool* CameraTool2D::doGetTool() {
            return this;
        }
        
        void CameraTool2D::doMouseScroll(const InputState& inputState) {
            if (zoom(inputState)) {
                const float speed = pref(Preferences::CameraMouseWheelInvert) ? -1.0f : 1.0f;
                if (inputState.scrollY() != 0.0f) {
                    const Vec2f mousePos(static_cast<float>(inputState.mouseX()), static_cast<float>(inputState.mouseY()));
                    const Vec3f oldWorldPos = m_camera.unproject(mousePos.x(), mousePos.y(), 0.0f);
                    
                    const float factor = 1.0f + inputState.scrollY() / 50.0f * speed;
                    m_camera.zoom(factor);
                    
                    const Vec3f newWorldPos = m_camera.unproject(mousePos.x(), mousePos.y(), 0.0f);
                    const Vec3f delta = newWorldPos - oldWorldPos;
                    m_camera.moveBy(-delta);
                }
            }
        }
        
        bool CameraTool2D::doStartMouseDrag(const InputState& inputState) {
            if (pan(inputState)) {
                m_lastMousePos = Vec2f(static_cast<float>(inputState.mouseX()),
                                       static_cast<float>(inputState.mouseY()));
                return true;
            }
            return false;
        }
        
        bool CameraTool2D::doMouseDrag(const InputState& inputState) {
            if (pan(inputState)) {
                const Vec2f currentMousePos(static_cast<float>(inputState.mouseX()), static_cast<float>(inputState.mouseY()));
                const Vec3f lastWorldPos = m_camera.unproject(m_lastMousePos.x(), m_lastMousePos.y(), 0.0f);
                const Vec3f currentWorldPos = m_camera.unproject(currentMousePos.x(), currentMousePos.y(), 0.0f);
                const Vec3f delta = currentWorldPos - lastWorldPos;
                m_camera.moveBy(-delta);
                m_lastMousePos = currentMousePos;
                return true;
            }
            return false;
        }
        
        void CameraTool2D::doEndMouseDrag(const InputState& inputState) {}
        void CameraTool2D::doCancelMouseDrag() {}
        
        bool CameraTool2D::doCancel() {
            return false;
        }
        
        bool CameraTool2D::zoom(const InputState& inputState) const {
            return (inputState.mouseButtonsPressed(MouseButtons::MBNone) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKNone));
        }
        
        bool CameraTool2D::look(const InputState& inputState) const {
            return (inputState.mouseButtonsPressed(MouseButtons::MBRight) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKNone));
        }
        
        bool CameraTool2D::pan(const InputState& inputState) const {
            return inputState.mouseButtonsPressed(MouseButtons::MBRight) || inputState.mouseButtonsPressed(MouseButtons::MBMiddle);
        }
    }
}
