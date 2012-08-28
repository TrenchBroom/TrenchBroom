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

#ifndef __TrenchBroom__InputController__
#define __TrenchBroom__InputController__

#include "Controller/Input.h"
#include "Controller/Tool.h"

class wxEvtHandler;

namespace TrenchBroom {
    namespace Controller {
        class InputController {
        protected:
            InputEvent m_currentEvent;
            MouseButtonState m_dragButtons;
            
            ToolList m_receivers;
            Tool* m_dragReceiver;
            Tool* m_mouseUpReceiver;
            int m_modalReceiverIndex;
            
            void updateHits();
            void updateMousePos(float x, float y);
        public:
            InputController(wxEvtHandler& eventHandler);
            ~InputController();
            
            void modifierKeyDown(ModifierKeyState modifierKey);
            void modifierKeyUp(ModifierKeyState modifierKey);
            bool mouseDown(MouseButtonState mouseButton, float x, float y);
            bool mouseUp(MouseButtonState mouseButton, float x, float y);
            void mouseMoved(float x, float y);
            void scrolled(float dx, float dy);
        };
    }
}

#endif /* defined(__TrenchBroom__InputController__) */
