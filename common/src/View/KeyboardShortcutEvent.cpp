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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "KeyboardShortcutEvent.h"

DEFINE_EVENT_TYPE(EVT_KEYBOARD_SHORTCUT_EVENT)

namespace TrenchBroom {
    namespace View {
        IMPLEMENT_DYNAMIC_CLASS(KeyboardShortcutEvent, wxEvent)
        KeyboardShortcutEvent::KeyboardShortcutEvent() :
        m_key(WXK_NONE),
        m_modifier1(WXK_NONE),
        m_modifier2(WXK_NONE),
        m_modifier3(WXK_NONE) {}
        
        KeyboardShortcutEvent::KeyboardShortcutEvent(const int key, const int modifier1, const int modifier2, const int modifier3) :
        m_key(key),
        m_modifier1(modifier1),
        m_modifier2(modifier2),
        m_modifier3(modifier3) {}
        
        int KeyboardShortcutEvent::key() const {
            return m_key;
        }
        
        int KeyboardShortcutEvent::modifier1() const {
            return m_modifier1;
        }
        
        int KeyboardShortcutEvent::modifier2() const {
            return m_modifier2;
        }
        
        int KeyboardShortcutEvent::modifier3() const {
            return m_modifier3;
        }

        wxEvent* KeyboardShortcutEvent::Clone() const {
            return new KeyboardShortcutEvent(*this);
        }
    }
}
