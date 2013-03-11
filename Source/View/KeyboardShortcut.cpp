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

#include "KeyboardShortcut.h"

namespace TrenchBroom {
    namespace View {
        wxString KeyboardShortcut::contextName(int context) {
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
        
        void KeyboardShortcut::sortModifierKeys(int& key1, int& key2, int& key3) {
            ModifierSet modifierSet;
            modifierSet.insert(key1);
            modifierSet.insert(key2);
            modifierSet.insert(key3);
            
            key1 = key2 = key3 = WXK_NONE;
            ModifierSet::iterator it = modifierSet.begin();
            if (it != modifierSet.end()) {
                key1 = *it++;
                if (it != modifierSet.end()) {
                    key2 = *it++;
                    if (it != modifierSet.end()) {
                        key3 = *it++;
                    }
                }
            }
        }
        
        wxString KeyboardShortcut::modifierKeyMenuText(int key) {
            switch (key) {
                case WXK_SHIFT:
                    return "Shift";
                case WXK_ALT:
                    return "Alt";
                case WXK_CONTROL:
                    return "Ctrl";
                default:
                    return "";
            }
        }
        
        wxString KeyboardShortcut::modifierKeyDisplayText(int key) {
#if defined __APPLE__
            switch (key) {
                case WXK_SHIFT:
                    return L"\u21E7";
                case WXK_ALT:
                    return L"\u2325";
                case WXK_CONTROL:
                    return L"\u2318";
                default:
                    return "";
            }
#else
            return modifierKeyMenuText(key);
#endif
        }
        
        wxString KeyboardShortcut::keyMenuText(int key) {
            switch (key) {
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
                    if (key >= 33 && key <= 126) {
                        wxString str;
                        str << static_cast<char>(key);
                        return str;
                    }
                    return "";
                    break;
            }
        }
        
        wxString KeyboardShortcut::keyDisplayText(int key) {
#if defined __APPLE__
            switch (key) {
                case WXK_BACK:
                    return L"\u232B";
                case WXK_TAB:
                    return L"\u21E5";
                case WXK_RETURN:
                    return L"\u21A9";
                case WXK_ESCAPE:
                    return L"\u238B";
                case WXK_SPACE:
                    return L"\u2423";
                case WXK_DELETE:
                    return L"\u2326";
                case WXK_END:
                    return L"\u21F2";
                case WXK_HOME:
                    return L"\u21F1";
                case WXK_LEFT:
                    return L"\u2190";
                case WXK_UP:
                    return L"\u2191";
                case WXK_RIGHT:
                    return L"\u2192";
                case WXK_DOWN:
                    return L"\u2193";
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
                    return L"\u21DE";
                case WXK_PAGEDOWN:
                    return L"\u21DF";
                default:
                    if (key >= 33 && key <= 126) {
                        wxString str;
                        str << static_cast<char>(key);
                        return str;
                    }
                    return "";
                    break;
            }
#else
            return keyMenuText(key);
#endif
        }

        wxString KeyboardShortcut::shortcutDisplayText(int modifierKey1, int modifierKey2, int modifierKey3, int key) {
            wxString text;
#if defined __APPLE__
            text << modifierKeyDisplayText(modifierKey1) << modifierKeyDisplayText(modifierKey2) << modifierKeyDisplayText(modifierKey3) << keyDisplayText(key);
#else
            text << modifierKeyDisplayText(modifierKey1);
            if (modifierKey1 != WXK_NONE && modifierKey2 != WXK_NONE)
                text << "+";
            text << modifierKeyDisplayText(modifierKey2);
            if ((modifierKey1 != WXK_NONE || modifierKey2 != WXK_NONE) && modifierKey3 != WXK_NONE)
                text << "+";
            text << modifierKeyDisplayText(modifierKey3);
            if ((modifierKey1 != WXK_NONE || modifierKey2 != WXK_NONE || modifierKey3 != WXK_NONE) && key != WXK_NONE)
                text << "+";
            text << keyMenuText(key);
#endif
            return text;
        }

        KeyboardShortcut::KeyboardShortcut(int commandId, int context, const String& text) :
        m_commandId(commandId),
        m_modifierKey1(WXK_NONE),
        m_modifierKey2(WXK_NONE),
        m_modifierKey3(WXK_NONE),
        m_key(WXK_NONE),
        m_context(context),
        m_text(text) {}
        
        KeyboardShortcut::KeyboardShortcut(int commandId, int key, int context, const String& text) :
        m_commandId(commandId),
        m_modifierKey1(WXK_NONE),
        m_modifierKey2(WXK_NONE),
        m_modifierKey3(WXK_NONE),
        m_key(key),
        m_context(context),
        m_text(text) {}
        
        KeyboardShortcut::KeyboardShortcut(int commandId, int modifierKey1, int key, int context, const String& text) :
        m_commandId(commandId),
        m_modifierKey1(modifierKey1),
        m_modifierKey2(WXK_NONE),
        m_modifierKey3(WXK_NONE),
        m_key(key),
        m_context(context),
        m_text(text) {}
        
        KeyboardShortcut::KeyboardShortcut(int commandId, int modifierKey1, int modifierKey2, int key, int context, const String& text) :
        m_commandId(commandId),
        m_modifierKey1(modifierKey1),
        m_modifierKey2(modifierKey2),
        m_modifierKey3(WXK_NONE),
        m_key(key),
        m_context(context),
        m_text(text) {
            sortModifierKeys(m_modifierKey1, m_modifierKey2, m_modifierKey3);
        }
        
        KeyboardShortcut::KeyboardShortcut(int commandId, int modifierKey1, int modifierKey2, int modifierKey3, int key, int context, const String& text) :
        m_commandId(commandId),
        m_modifierKey1(modifierKey1),
        m_modifierKey2(modifierKey2),
        m_modifierKey3(modifierKey3),
        m_key(key),
        m_context(context),
        m_text(text) {
            sortModifierKeys(m_modifierKey1, m_modifierKey2, m_modifierKey3);
        }
        
        KeyboardShortcut::KeyboardShortcut(const String& string) {
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
            
            sortModifierKeys(m_modifierKey1, m_modifierKey2, m_modifierKey3);
        }
        
        bool KeyboardShortcut::alwaysShowModifier() const {
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
        
        wxString KeyboardShortcut::modifierKeyMenuText() const {
            wxString text;
            text << modifierKeyMenuText(m_modifierKey1);
            if (m_modifierKey1 != WXK_NONE && m_modifierKey2 != WXK_NONE)
                text << "+";
            
            text << modifierKeyMenuText(m_modifierKey2);
            if ((m_modifierKey1 != WXK_NONE || m_modifierKey2 != WXK_NONE) && m_modifierKey3 != WXK_NONE)
                text << "+";
            
            text << modifierKeyMenuText(m_modifierKey3);
            return text;
        }
        
        wxString KeyboardShortcut::keyMenuText() const {
            return keyMenuText(m_key);
        }
        
        wxString KeyboardShortcut::shortcutMenuText() const {
            wxString text = modifierKeyMenuText();
            if (text.empty())
                return keyMenuText();
            
            text << "+" << keyMenuText();
            return text;
        }
        
        wxString KeyboardShortcut::menuText() const {
            if (m_key == WXK_NONE)
                return m_text;
            
            wxString text;
            text << m_text << "\t";
            if (hasModifier())
                text << modifierKeyMenuText() << "+";
            text << keyMenuText();
            return text;
        }
        
        wxString KeyboardShortcut::keyDisplayText() const {
            return keyDisplayText(m_key);
        }
        
        wxString KeyboardShortcut::shortcutDisplayText() const {
            return shortcutDisplayText(m_modifierKey1, m_modifierKey2, m_modifierKey3, m_key);
        }
        
        String KeyboardShortcut::asString() const {
            StringStream str;
            str << m_commandId << ":" << m_modifierKey1 << ":" << m_modifierKey2 << ":" << m_modifierKey3 << ":" << m_key << ":" << m_context << ":" << m_text;
            return str.str();
        }
    }
}
