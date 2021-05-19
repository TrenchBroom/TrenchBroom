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

#include "CameraTool2D.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "View/InputState.h"
#include "Renderer/OrthographicCamera.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace View {
        CameraTool2D::CameraTool2D(Renderer::OrthographicCamera& camera) :
        ToolControllerBase{},
        Tool{true},
        m_camera{camera} {}

        Tool* CameraTool2D::doGetTool() {
            return this;
        }

        const Tool* CameraTool2D::doGetTool() const {
            return this;
        }

        void CameraTool2D::doMouseScroll(const InputState& inputState) {
            if (zoom(inputState)) {
                if (inputState.scrollY() != 0.0f) {
                    const float speed = pref(Preferences::CameraMouseWheelInvert) ? -1.0f : 1.0f;
                    const float factor = 1.0f + inputState.scrollY() / 50.0f * speed;
                    const auto mousePos = vm::vec2f{inputState.mouseX(), inputState.mouseY()};

                    if (factor > 0.0f) {
                        zoom(inputState, mousePos, factor);
                    }
                }
            }
        }

        bool CameraTool2D::doStartMouseDrag(const InputState& inputState) {
            if (pan(inputState)) {
                m_lastMousePos = vm::vec2f{inputState.mouseX(), inputState.mouseY()};
                return true;
            } else if (dragZoom(inputState)) {
                m_lastMousePos = vm::vec2f{inputState.mouseX(), inputState.mouseY()};
                return true;
            }
            return false;
        }

        bool CameraTool2D::doMouseDrag(const InputState& inputState) {
            if (pan(inputState)) {
                const auto currentMousePos = vm::vec2f{inputState.mouseX(), inputState.mouseY()};
                const auto lastWorldPos = m_camera.unproject(m_lastMousePos.x(), m_lastMousePos.y(), 0.0f);
                const auto currentWorldPos = m_camera.unproject(currentMousePos.x(), currentMousePos.y(), 0.0f);
                const auto delta = currentWorldPos - lastWorldPos;
                m_camera.moveBy(-delta);
                m_lastMousePos = currentMousePos;
                return true;
            } else if (dragZoom(inputState)) {
                const auto speed = pref(Preferences::CameraAltMoveInvert) ? 1.0f : -1.0f;
                const auto factor = 1.0f + static_cast<float>(inputState.mouseDY()) / 100.0f * speed;
                zoom(inputState, m_lastMousePos, factor);
                return true;
            }
            return false;
        }

        void CameraTool2D::doEndMouseDrag(const InputState&) {}

        void CameraTool2D::doCancelMouseDrag() {}

        bool CameraTool2D::zoom(const InputState& inputState) const {
            return (inputState.mouseButtonsPressed(MouseButtons::MBNone) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKNone));
        }

        bool CameraTool2D::look(const InputState& inputState) const {
            return (inputState.mouseButtonsPressed(MouseButtons::MBRight) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKNone));
        }

        bool CameraTool2D::pan(const InputState& inputState) const {
            return (inputState.mouseButtonsPressed(MouseButtons::MBRight) ||
                    (inputState.mouseButtonsPressed(MouseButtons::MBMiddle) &&
                     !pref(Preferences::CameraEnableAltMove)));
        }

        bool CameraTool2D::dragZoom(const InputState& inputState) const {
            return (pref(Preferences::CameraEnableAltMove) &&
                    inputState.mouseButtonsPressed(MouseButtons::MBMiddle) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKAlt));
        }

        void CameraTool2D::zoom(const InputState&, const vm::vec2f& mousePos, const float factor) {
            const auto oldWorldPos = m_camera.unproject(mousePos.x(), mousePos.y(), 0.0f);

            m_camera.zoom(factor);

            const auto newWorldPos = m_camera.unproject(mousePos.x(), mousePos.y(), 0.0f);
            const auto delta = newWorldPos - oldWorldPos;
            m_camera.moveBy(-delta);
        }

        bool CameraTool2D::doCancel() {
            return false;
        }
    }
}
