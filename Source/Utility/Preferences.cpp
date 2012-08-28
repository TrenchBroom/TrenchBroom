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

#include "Preferences.h"

namespace TrenchBroom {
    namespace Preferences {
        bool PreferenceManager::parseMouseState(const String& str, Controller::MouseState& mouseState) const {
            Controller::ModifierKeyState modifierKeys = Controller::ModifierKeys::None;
            Controller::MouseButtonState mouseButtons = Controller::MouseButtons::None;
            
            StringList parts = Utility::split(str, '+');
            bool modifiers = true;
            unsigned int i = 0;
            while (i < parts.size()) {
                String part = Utility::trim(parts[i]);
                if (modifiers) {
                    if (part == "Shift") {
                        modifierKeys |= Controller::ModifierKeys::Shift;
                        i++;
                    } else if (part == "Ctrl") {
                        modifierKeys |= Controller::ModifierKeys::Ctrl;
                        i++;
                    } else if (part == "Alt") {
                        modifierKeys |= Controller::ModifierKeys::Alt;
                        i++;
                    } else if (part == "Cmd") {
                        modifierKeys |= Controller::ModifierKeys::Cmd;
                        i++;
                    } else {
                        modifiers = false;
                    }
                } else {
                    if (part == "MBLeft") {
                        mouseButtons |= Controller::MouseButtons::Left;
                        i++;
                    } else if (part == "MBRight") {
                        mouseButtons |= Controller::MouseButtons::Right;
                        i++;
                    } else if (part == "MBMiddle") {
                        mouseButtons |= Controller::MouseButtons::Middle;
                        i++;
                    } else {
                        return false;
                    }
                }
            }
            
            mouseState.setModifierKeys(modifierKeys);
            mouseState.setMouseButtons(mouseButtons);
            return true;
        }
    }
}