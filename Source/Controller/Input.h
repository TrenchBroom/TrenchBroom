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

#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Controller {
        namespace ModifierKeys {
            static const unsigned int None  = 0;
            static const unsigned int Shift = 1 << 0;
            static const unsigned int Ctrl  = 1 << 1;
            static const unsigned int Alt   = 1 << 2;
            static const unsigned int Cmd   = 1 << 3;
        }
        
        typedef unsigned int ModifierKeyState;
        
        namespace MouseButtons {
            static const unsigned int None      = 0;
            static const unsigned int Left      = 1 << 0;
            static const unsigned int Right     = 1 << 1;
            static const unsigned int Middle    = 1 << 2;
        }
        
        typedef unsigned int MouseButtonState;

        class InputEvent {
        public:
            ModifierKeyState modifierKeys;
            MouseButtonState mouseButtons;
            float mouseX;
            float mouseY;
            float deltaX;
            float deltaY;
            float scrollX;
            float scrollY;
            Ray ray;
            InputEvent() : modifierKeys(ModifierKeys::None), mouseButtons(MouseButtons::None) {}
        };

        class MouseState {
        private:
            ModifierKeyState m_modifierKeys;
            MouseButtonState m_mouseButtons;
        public:
            MouseState() :
            m_modifierKeys(ModifierKeys::None),
            m_mouseButtons(MouseButtons::None) {}
            
            MouseState(ModifierKeyState modifierKeys, MouseButtonState mouseButtons) :
            m_modifierKeys(modifierKeys),
            m_mouseButtons(mouseButtons) {}
            
            inline ModifierKeyState modifierKeys() const {
                return m_modifierKeys;
            }
            
            inline void setModifierKeys(ModifierKeyState modifierKeys) {
                m_modifierKeys = modifierKeys;
            }
            
            inline MouseButtonState mouseButtons() const {
                return m_mouseButtons;
            }
            
            inline void setMouseButtons(MouseButtonState mouseButtons) {
                m_mouseButtons = mouseButtons;
            }
            
            bool matches(InputEvent& event) {
                return m_modifierKeys == event.modifierKeys && m_mouseButtons == event.mouseButtons;
            }
        };
    }
}

#endif
