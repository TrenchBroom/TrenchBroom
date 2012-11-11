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

#ifndef TrenchBroom_Input_h
#define TrenchBroom_Input_h

#include "Model/Picker.h"
#include "Renderer/Camera.h"

#include <wx/utils.h>

namespace TrenchBroom {
    namespace Controller {
        typedef unsigned int ModifierKeyState;

        namespace ModifierKeys {
            static const ModifierKeyState MKNone      = 0;
            static const ModifierKeyState MKShift     = 1 << 0;
            static const ModifierKeyState MKCtrlCmd   = 1 << 1; // Cmd on Mac, Ctrl on other systems
            static const ModifierKeyState MKAlt       = 1 << 2;
        }

        typedef unsigned int MouseButtonState;

        namespace MouseButtons {
            static const MouseButtonState MBNone      = 0;
            static const MouseButtonState MBLeft      = 1 << 0;
            static const MouseButtonState MBRight     = 1 << 1;
            static const MouseButtonState MBMiddle    = 1 << 2;
        }

        class InputState {
        private:
            MouseButtonState m_mouseButtons;
            int m_mouseX;
            int m_mouseY;
            int m_mouseDX;
            int m_mouseDY;
            float m_scrollX;
            float m_scrollY;
            
            const Renderer::Camera& m_camera;
            bool m_valid;
            Ray m_pickRay;
            Model::Picker& m_picker;
            Model::PickResult* m_pickResult;
            
            // edit state
            // picking hits
            // camera
            
        public:
            InputState(const Renderer::Camera& camera, Model::Picker& picker) :
            m_mouseButtons(MouseButtons::MBNone),
            m_mouseX(0),
            m_mouseY(0),
            m_mouseDX(0),
            m_mouseDY(0),
            m_scrollX(0.0f),
            m_scrollY(0.0f),
            m_camera(camera),
            m_valid(false),
            m_picker(picker),
            m_pickResult(NULL) {
                wxMouseState mouseState = wxGetMouseState();
                // make sure the mouse deltas are 0:
                m_mouseX = mouseState.GetX();
                m_mouseY = mouseState.GetY();
                mouseMove(mouseState.GetX(), mouseState.GetY());
            }
            
            ~InputState() {
                if (m_pickResult != NULL) {
                    delete m_pickResult;
                    m_pickResult = NULL;
                }
            }
            
            inline ModifierKeyState modifierKeys() const {
                wxMouseState mouseState = wxGetMouseState();
                
                ModifierKeyState state = ModifierKeys::MKNone;
                if (mouseState.CmdDown())
                    state |= ModifierKeys::MKCtrlCmd;
                if (mouseState.ShiftDown())
                    state |= ModifierKeys::MKShift;
                if (mouseState.AltDown())
                    state |= ModifierKeys::MKAlt;
                return state;
            }
            
            inline MouseButtonState mouseButtons() const {
                return m_mouseButtons;
            }
            
            inline void mouseDown(MouseButtonState button) {
                m_mouseButtons |= button;
            }
            
            inline void mouseUp(MouseButtonState button) {
                m_mouseButtons &= ~button;
            }
            
            inline int x() const {
                return m_mouseX;
            }
            
            inline int y() const {
                return m_mouseY;
            }
            
            inline int deltaX() const {
                return m_mouseDX;
            }
            
            inline int deltaY() const {
                return m_mouseDY;
            }
            
            inline void mouseMove(int x, int y) {
                m_mouseDX = x - m_mouseX;
                m_mouseX = x;
                m_mouseDY = y - m_mouseY;
                m_mouseY = y;
            }
            
            inline float scroll() const {
                if (m_scrollY != 0.0f)
                    return m_scrollY;
                return m_scrollX;
            }
            
            inline float scrollX() const {
                return m_scrollX;
            }
            
            inline float scrollY() const {
                return m_scrollY;
            }
            
            inline void scroll(float x, float y) {
                m_scrollX = x;
                m_scrollY = y;
            }
            
            inline const Renderer::Camera& camera() const {
                return m_camera;
            }
            
            inline const Ray& pickRay() {
                validate();
                return m_pickRay;
            }
            
            inline void invalidate() {
                m_valid = false;
            }
            
            inline void validate() {
                if (m_valid)
                    return;
                m_valid = true;
                m_pickRay = m_camera.pickRay(static_cast<float>(m_mouseX), static_cast<float>(m_mouseY));
                if (m_pickResult != NULL)
                    delete m_pickResult;
                m_pickResult = m_picker.pick(pickRay());
            }
        
            inline Model::PickResult& pickResult() {
                validate();
                return *m_pickResult;
            }
        
        };
    }
}

#endif
