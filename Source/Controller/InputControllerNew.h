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

#ifndef __TrenchBroom__InputControllerNew__
#define __TrenchBroom__InputControllerNew__

#include <iostream>
#include "Controller/Input.h"

namespace TrenchBroom {
    namespace Controller {
        class InputState {
        public:
            MouseButtonState mouseButtons;
            float mouseX;
            float mouseY;
            float mouseDX;
            float mouseDY;
            float scrollX;
            float scrollY;
            
            // edit state
            // picking hits
            // camera
            
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
        
        class InputController {
            
        };
    }
}

#endif /* defined(__TrenchBroom__InputControllerNew__) */
