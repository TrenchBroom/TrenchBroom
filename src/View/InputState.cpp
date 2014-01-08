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

#include "InputState.h"

#include <wx/utils.h>

namespace TrenchBroom {
    namespace View {
        InputState::InputState(const Renderer::Camera& camera) :
        m_modifierKeys(ModifierKeys::MKNone),
        m_mouseButtons(MouseButtons::MBNone),
        m_mouseX(0),
        m_mouseY(0),
        m_mouseDX(0),
        m_mouseDY(0),
        m_scrollX(0.0f),
        m_scrollY(0.0f),
        m_camera(camera) {
            const wxMouseState mouseState = wxGetMouseState();
            m_mouseX = mouseState.GetX();
            m_mouseY = mouseState.GetY();
        }

        InputState::InputState(const Renderer::Camera& camera, const int mouseX, const int mouseY) :
        m_modifierKeys(ModifierKeys::MKNone),
        m_mouseButtons(MouseButtons::MBNone),
        m_mouseX(mouseX),
        m_mouseY(mouseY),
        m_mouseDX(0),
        m_mouseDY(0),
        m_scrollX(0.0f),
        m_scrollY(0.0f),
        m_camera(camera) {}
        
        InputState::~InputState() {}

        ModifierKeyState InputState::modifierKeys() const {
            return m_modifierKeys;
        }
        
        bool InputState::modifierKeysDown(const ModifierKeyState keys) const {
            return (modifierKeys() & keys) != 0;
        }
        
        bool InputState::modifierKeysPressed(const ModifierKeyState keys) const {
            return modifierKeys() == keys;
        }
        
        MouseButtonState InputState::mouseButtons() const {
            return m_mouseButtons;
        }
        
        bool InputState::mouseButtonsDown(const MouseButtonState buttons) const {
            return (mouseButtons() & buttons) != 0;
        }
        
        bool InputState::mouseButtonsPressed(const MouseButtonState buttons) const {
            return mouseButtons() == buttons;
        }

        int InputState::mouseX() const {
            return m_mouseX;
        }
        
        int InputState::mouseY() const {
            return m_mouseY;
        }
        
        int InputState::mouseDX() const {
            return m_mouseDX;
        }
        
        int InputState::mouseDY() const {
            return m_mouseDY;
        }
        
        float InputState::scrollX() const {
            return m_scrollX;
        }
        
        float InputState::scrollY() const {
            return m_scrollY;
        }

        void InputState::setModifierKeys(const ModifierKeyState keys) {
            m_modifierKeys = keys;
        }
        
        void InputState::clearModifierKeys() {
            m_modifierKeys = ModifierKeys::MKNone;
        }

        void InputState::mouseDown(const MouseButtonState button) {
            m_mouseButtons |= button;
        }
        
        void InputState::mouseUp(const MouseButtonState button) {
            m_mouseButtons &= ~button;
        }
        
        void InputState::clearMouseButtons() {
            m_mouseButtons = MouseButtons::MBNone;
        }

        void InputState::mouseMove(const int mouseX, const int mouseY) {
            m_mouseDX = mouseX - m_mouseX;
            m_mouseX = mouseX;
            m_mouseDY = mouseY - m_mouseY;
            m_mouseY = mouseY;
        }
        
        void InputState::scroll(const float scrollX, const float scrollY) {
            m_scrollX = scrollX;
            m_scrollY = scrollY;
        }

        const Ray3& InputState::pickRay() const {
            return m_pickRay;
        }
        
        void InputState::setPickRay(const Ray3& pickRay) {
            m_pickRay = pickRay;
        }

        Model::PickResult& InputState::pickResult() const {
            return m_pickResult;
        }

        void InputState::setPickResult(Model::PickResult& pickResult) {
            m_pickResult = pickResult;
        }

        const Renderer::Camera& InputState::camera() const {
            return m_camera;
        }
    }
}
