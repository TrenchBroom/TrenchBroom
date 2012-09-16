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

#include <wx/utils.h>

#include "Model/Picker.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }

    namespace Renderer {
        class Camera;
    }

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

        class InputEvent {
        public:
            MouseButtonState mouseButtons;
            float mouseX;
            float mouseY;
            float deltaX;
            float deltaY;
            float scrollX;
            float scrollY;
            Ray ray;
            Model::PickResult* pickResult;
            Renderer::Camera* camera;

            InputEvent() :
            mouseButtons(MouseButtons::MBNone),
            pickResult(NULL),
            camera(NULL) {}

			~InputEvent() {
				if (pickResult != NULL) {
					delete pickResult;
					pickResult = NULL;
				}
			}

            inline const ModifierKeyState modifierKeys() const {
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

            inline float scroll() const {
                if (scrollY != 0.0f)
                    return scrollY;
                return scrollX;
            }
        };

        class MouseState {
        private:
            ModifierKeyState m_modifierKeys;
            MouseButtonState m_mouseButtons;
        public:
            MouseState() :
            m_modifierKeys(ModifierKeys::MKNone),
            m_mouseButtons(MouseButtons::MBNone) {}

            MouseState(ModifierKeyState modifierKeys, MouseButtonState mouseButtons) :
            m_modifierKeys(modifierKeys),
            m_mouseButtons(mouseButtons) {}

            inline const ModifierKeyState modifierKeys() const {
                return m_modifierKeys;
            }

            inline void setModifierKeys(ModifierKeyState modifierKeys) {
                m_modifierKeys = modifierKeys;
            }

            inline const MouseButtonState mouseButtons() const {
                return m_mouseButtons;
            }

            inline void setMouseButtons(MouseButtonState mouseButtons) {
                m_mouseButtons = mouseButtons;
            }

            bool matches(InputEvent& event) const {
                return m_modifierKeys == event.modifierKeys() && m_mouseButtons == event.mouseButtons;
            }
        };
    }
}

#endif
