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

#ifndef __TrenchBroom__KeyboardShortcut__
#define __TrenchBroom__KeyboardShortcut__

#include "Utility/String.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcut {
        public:
            typedef enum {
                SCVertexTool    = 1 << 1,
                SCClipTool      = 1 << 2,
                SCRotateTool    = 1 << 3,
                SCObjects       = 1 << 4,
                SCTextures      = 1 << 5,
                SCAny           = SCVertexTool | SCClipTool | SCRotateTool | SCObjects | SCTextures
            } ShortcutContext;
        
            inline static String contextName(int context) {
                if (context == SCAny)
                    return "Any";

                StringList contexts;
                if (context & SCVertexTool)
                    contexts.push_back("Vertex Tool");
                if (context & SCClipTool)
                    contexts.push_back("Clip Tool");
                if (context & SCRotateTool)
                    contexts.push_back("Rotate Tool");
                if (context & SCObjects)
                    contexts.push_back("Objects");
                if (context & SCTextures)
                    contexts.push_back("Textures");
                return Utility::join(contexts, ", ");
            }
        private:
            int m_commandId;
            int m_modifierKey1;
            int m_modifierKey2;
            int m_modifierKey3;
            int m_key;
            int m_context;
            String m_text;
        public:
            KeyboardShortcut(int commandId, int context, const String& text) :
            m_commandId(commandId),
            m_modifierKey1(WXK_NONE),
            m_modifierKey2(WXK_NONE),
            m_modifierKey3(WXK_NONE),
            m_key(WXK_NONE),
            m_context(context),
            m_text(text) {}

            KeyboardShortcut(int commandId, int key, int context, const String& text) :
            m_commandId(commandId),
            m_modifierKey1(WXK_NONE),
            m_modifierKey2(WXK_NONE),
            m_modifierKey3(WXK_NONE),
            m_key(key),
            m_context(context),
            m_text(text) {}

            KeyboardShortcut(int commandId, int modifierKey1, int key, int context, const String& text) :
            m_commandId(commandId),
            m_modifierKey1(modifierKey1),
            m_modifierKey2(WXK_NONE),
            m_modifierKey3(WXK_NONE),
            m_key(key),
            m_context(context),
            m_text(text) {}
            
            KeyboardShortcut(int commandId, int modifierKey1, int modifierKey2, int key, int context, const String& text) :
            m_commandId(commandId),
            m_modifierKey1(modifierKey1),
            m_modifierKey2(modifierKey2),
            m_modifierKey3(WXK_NONE),
            m_key(key),
            m_context(context),
            m_text(text) {}
            
            KeyboardShortcut(int commandId, int modifierKey1, int modifierKey2, int modifierKey3, int key, int context, const String& text) :
            m_commandId(commandId),
            m_modifierKey1(modifierKey1),
            m_modifierKey2(modifierKey2),
            m_modifierKey3(modifierKey3),
            m_key(key),
            m_context(context),
            m_text(text) {}
            KeyboardShortcut(const String& string) {
                StringStream stream(string);
                
                char colon;
                stream >> m_commandId;
                stream >> colon;
                assert(colon == ':');
                stream >> m_modifierKey1;
                stream >> colon;
                assert(colon == ':');
                stream >> m_modifierKey2;
                stream >> colon;
                assert(colon == ':');
                stream >> m_modifierKey3;
                stream >> colon;
                assert(colon == ':');
                stream >> m_key;
                stream >> colon;
                assert(colon == ':');
                stream >> m_context;
                stream >> colon;
                assert(colon == ':');
                m_text = stream.str().substr(static_cast<size_t>(stream.tellg()));
            }
            
            inline int commandId() const {
                return m_commandId;
            }
            
            inline int modifierKey1() const {
                return m_modifierKey1;
            }
            
            inline int modifierKey2() const {
                return m_modifierKey2;
            }
            
            inline int modifierKey3() const {
                return m_modifierKey3;
            }
            
            inline int key() const {
                return m_key;
            }
            
            inline int context() const {
                return m_context;
            }
            
            inline const String& text() const {
                return m_text;
            }
            
            inline bool alwaysShowModifier() const {
                switch (m_key) {
                    case WXK_BACK:
                    case WXK_TAB:
                    case WXK_RETURN:
                    case WXK_ESCAPE:
                    case WXK_SPACE:
                    case WXK_DELETE:
                    case WXK_END:
                    case WXK_HOME:
                    case WXK_LEFT:
                    case WXK_UP:
                    case WXK_RIGHT:
                    case WXK_DOWN:
                    case WXK_INSERT:
                    case WXK_PAGEUP:
                    case WXK_PAGEDOWN:
                        return false;
                    case WXK_F1:
                    case WXK_F2:
                    case WXK_F3:
                    case WXK_F4:
                    case WXK_F5:
                    case WXK_F6:
                    case WXK_F7:
                    case WXK_F8:
                    case WXK_F9:
                    case WXK_F10:
                    case WXK_F11:
                    case WXK_F12:
                    case WXK_F13:
                    case WXK_F14:
                    case WXK_F15:
                    case WXK_F16:
                    case WXK_F17:
                    case WXK_F18:
                    case WXK_F19:
                    case WXK_F20:
                    case WXK_F21:
                    case WXK_F22:
                    case WXK_F23:
                    case WXK_F24:
                        return true;
                    default:
                        return hasModifier();
                }
            }
            
            inline bool hasModifier() const {
                return m_modifierKey1 != WXK_NONE || m_modifierKey2 != WXK_NONE || m_modifierKey3 != WXK_NONE;
            }
            
            inline String modifierKeyMenuText() const {
                StringStream stream;
                switch (m_modifierKey1) {
                    case WXK_SHIFT:
                        stream << "Shift";
                        break;
                    case WXK_ALT:
                        stream << "Alt";
                        break;
                    case WXK_CONTROL:
                        stream << "Ctrl";
                        break;
                    default:
                        break;
                }
                
                if (m_modifierKey1 != WXK_NONE && m_modifierKey2 != WXK_NONE)
                    stream << "+";
                
                switch (m_modifierKey2) {
                    case WXK_SHIFT:
                        stream << "Shift";
                        break;
                    case WXK_ALT:
                        stream << "Alt";
                        break;
                    case WXK_CONTROL:
                        stream << "Ctrl";
                        break;
                    default:
                        break;
                }
                
                if ((m_modifierKey1 != WXK_NONE || m_modifierKey2 != WXK_NONE) && m_modifierKey3 != WXK_NONE)
                    stream << "+";
                
                switch (m_modifierKey3) {
                    case WXK_SHIFT:
                        stream << "Shift";
                        break;
                    case WXK_ALT:
                        stream << "Alt";
                        break;
                    case WXK_CONTROL:
                        stream << "Ctrl";
                        break;
                    default:
                        break;
                }
                
                return stream.str();
            }
            
            inline String keyMenuText() const {
                switch (m_key) {
                    case WXK_BACK:
                        return "Back";
                    case WXK_TAB:
                        return "Tab";
                    case WXK_RETURN:
                        return "Enter";
                    case WXK_ESCAPE:
                        return "Esc";
                    case WXK_SPACE:
                        return "Space";
                    case WXK_DELETE:
                        return "Del";
                    case WXK_END:
                        return "End";
                    case WXK_HOME:
                        return "Home";
                    case WXK_LEFT:
                        return "Left";
                    case WXK_UP:
                        return "Up";
                    case WXK_RIGHT:
                        return "Right";
                    case WXK_DOWN:
                        return "Down";
                    case WXK_INSERT:
                        return "Ins";
                    case WXK_F1:
                        return "F1";
                    case WXK_F2:
                        return "F2";
                    case WXK_F3:
                        return "F3";
                    case WXK_F4:
                        return "F4";
                    case WXK_F5:
                        return "F5";
                    case WXK_F6:
                        return "F6";
                    case WXK_F7:
                        return "F7";
                    case WXK_F8:
                        return "F8";
                    case WXK_F9:
                        return "F9";
                    case WXK_F10:
                        return "F01";
                    case WXK_F11:
                        return "F11";
                    case WXK_F12:
                        return "F12";
                    case WXK_F13:
                        return "F13";
                    case WXK_F14:
                        return "F14";
                    case WXK_F15:
                        return "F15";
                    case WXK_F16:
                        return "F16";
                    case WXK_F17:
                        return "F17";
                    case WXK_F18:
                        return "F18";
                    case WXK_F19:
                        return "F19";
                    case WXK_F20:
                        return "F20";
                    case WXK_F21:
                        return "F21";
                    case WXK_F22:
                        return "F22";
                    case WXK_F23:
                        return "F23";
                    case WXK_F24:
                        return "F24";
                    case WXK_PAGEUP:
                        return "PgUp";
                    case WXK_PAGEDOWN:
                        return "PgDn";
                    default:
                        if (m_key >= 33 && m_key <= 126) {
                            StringStream str;
                            str << static_cast<char>(m_key);
                            return str.str();
                        }
                        return "";
                        break;
                }
            }
            
            inline String shortcutMenuText() const {
                const String modifierKeyText = modifierKeyMenuText();
                if (modifierKeyText.empty())
                    return keyMenuText();
                
                StringStream text;
                text << modifierKeyText << "+" << keyMenuText();
                return text.str();
            }
            
            inline String menuText() const {
                if (m_key == WXK_NONE)
                    return m_text;
                
                StringStream text;
                text << m_text << "\t";
                if (hasModifier())
                    text << modifierKeyMenuText() << "+";
                text << keyMenuText();
                return text.str();
            }
            
            inline String asString() const {
                StringStream str;
                str << m_commandId << ":" << m_modifierKey1 << ":" << m_modifierKey2 << ":" << m_modifierKey3 << ":" << m_key << ":" << m_context << ":" << m_text;
                return str.str();
            }
        };
    }
}

#endif /* defined(__TrenchBroom__KeyboardShortcut__) */
