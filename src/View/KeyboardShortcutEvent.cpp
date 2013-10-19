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

#include "KeyboardShortcutEvent.h"

DEFINE_EVENT_TYPE(EVT_KEYBOARD_SHORTCUT_EVENT)

namespace TrenchBroom {
    namespace View {
        IMPLEMENT_DYNAMIC_CLASS(KeyboardShortcutEvent, wxEvent)
        KeyboardShortcutEvent::KeyboardShortcutEvent() :
        m_modifierKey1(WXK_NONE),
        m_modifierKey2(WXK_NONE),
        m_modifierKey3(WXK_NONE),
        m_key(WXK_NONE) {}
        
        KeyboardShortcutEvent::KeyboardShortcutEvent(int modifierKey1, int modifierKey2, int modifierKey3, int key) :
        m_modifierKey1(modifierKey1),
        m_modifierKey2(modifierKey2),
        m_modifierKey3(modifierKey3),
        m_key(key) {}
        
        wxEvent* KeyboardShortcutEvent::Clone() const {
            return new KeyboardShortcutEvent(*this);
        }
    }
}
