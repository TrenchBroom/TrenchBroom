/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "UVCameraTool.h"

#include "VecMath.h"
#include "View/InputState.h"
#include "Renderer/OrthographicCamera.h"

namespace TrenchBroom {
    namespace View {
        UVCameraTool::UVCameraTool(Renderer::OrthographicCamera& camera) :
        ToolControllerBase(),
        Tool(true),
        m_camera(camera) {}
        
        Tool* UVCameraTool::doGetTool() {
            return this;
        }
        
        void UVCameraTool::doMouseScroll(const InputState& inputState) {
            const Vec3f oldWorldPos = m_camera.unproject(static_cast<float>(inputState.mouseX()),
                                                         static_cast<float>(inputState.mouseY()),
                                                         0.0f);
            
            if (inputState.scrollY() > 0)
                m_camera.zoom(1.1f);
            else
                m_camera.zoom(1.0f / 1.1f);
            
            const Vec3f newWorldPos = m_camera.unproject(static_cast<float>(inputState.mouseX()),
                                                         static_cast<float>(inputState.mouseY()),
                                                         0.0f);
            
            const Vec3f delta = oldWorldPos - newWorldPos;
            m_camera.moveBy(delta);
        }
        
        bool UVCameraTool::doStartMouseDrag(const InputState& inputState) {
            return inputState.mouseButtonsPressed(MouseButtons::MBRight) || inputState.mouseButtonsPressed(MouseButtons::MBMiddle);
        }
        
        bool UVCameraTool::doMouseDrag(const InputState& inputState) {
            const int oldX = inputState.mouseX() - inputState.mouseDX();
            const int oldY = inputState.mouseY() - inputState.mouseDY();
            
            const Vec3f oldWorldPos = m_camera.unproject(static_cast<float>(oldX),
                                                         static_cast<float>(oldY),
                                                         0.0f);
            const Vec3f newWorldPos = m_camera.unproject(static_cast<float>(inputState.mouseX()),
                                                         static_cast<float>(inputState.mouseY()),
                                                         0.0f);
            const Vec3f delta = oldWorldPos - newWorldPos;
            m_camera.moveBy(delta);
            return true;
        }
        
        void UVCameraTool::doEndMouseDrag(const InputState& inputState) {}
        
        void UVCameraTool::doCancelMouseDrag() {}
        
        bool UVCameraTool::doCancel() {
            return false;
        }
    }
}
