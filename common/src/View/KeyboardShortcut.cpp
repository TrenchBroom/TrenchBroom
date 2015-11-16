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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "KeyboardShortcut.h"

#include <wx/event.h>
#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>

namespace TrenchBroom {
    namespace View {
        bool KeyboardShortcut::MacModifierOrder::operator()(const int lhs, const int rhs) const {
            if (lhs == WXK_NONE)
                return lhs != WXK_NONE;
            if (lhs == WXK_ALT)
                return rhs != WXK_ALT;
            if (lhs == WXK_SHIFT)
                return rhs != WXK_ALT && rhs != WXK_SHIFT;
            if (lhs == WXK_CONTROL)
                return rhs == WXK_NONE;
            assert(false);
            return false;
        }

        bool KeyboardShortcut::WinModifierOrder::operator()(const int lhs, const int rhs) const {
            if (lhs == WXK_NONE)
                return lhs != WXK_NONE;
            if (lhs == WXK_CONTROL)
                return rhs != WXK_CONTROL;
            if (lhs == WXK_ALT)
                return rhs != WXK_CONTROL && rhs != WXK_ALT;
            if (lhs == WXK_SHIFT)
                return rhs == WXK_NONE;
            assert(false);
            return false;
        }

        const KeyboardShortcut KeyboardShortcut::Empty = KeyboardShortcut();
        
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
        
        bool KeyboardShortcut::isShortcutValid(const int key, const int modifier1, const int modifier2, const int modifier3) {
#ifdef __linux__
            // TAB and Escape are never allowed on GTK2:
            if (key == WXK_TAB || key == WXK_ESCAPE)
                return false;
            // cursor keys are only allowed if they have modifiers
            if (key == WXK_LEFT || key == WXK_RIGHT || key == WXK_UP || key == WXK_DOWN)
                return modifier1 != WXK_NONE || modifier2 != WXK_NONE || modifier3 != WXK_NONE;
#endif
            return true;
        }
        
        wxString KeyboardShortcut::shortcutDisplayString(const int key, int modifier1, int modifier2, int modifier3) {
            sortModifierKeys(modifier1, modifier2, modifier3);
            wxString text;
#if defined __APPLE__
            text << modifierDisplayString(modifier1) << modifierDisplayString(modifier2) << modifierDisplayString(modifier3) << keyDisplayString(key);
#else
            text << modifierDisplayString(modifier1);
            if (modifier1 != WXK_NONE && modifier2 != WXK_NONE)
                text << "+";
            text << modifierDisplayString(modifier2);
            if ((modifier1 != WXK_NONE || modifier2 != WXK_NONE) && modifier3 != WXK_NONE)
                text << "+";
            text << modifierDisplayString(modifier3);
            if ((modifier1 != WXK_NONE || modifier2 != WXK_NONE || modifier3 != WXK_NONE) && key != WXK_NONE)
                text << "+";
            text << keyMenuString(key);
#endif
            return text;
        }
        
        wxString KeyboardShortcut::keyMenuString(const int key) {
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
                case WXK_PAGEUP:
                    return "PgUp";
                case WXK_PAGEDOWN:
                    return "PgDn";
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
                default:
                    if (key >= 33 && key <= 126) {
                        wxString str;
                        str << static_cast<char>(key);
                        return str;
                    }
                    return "";
            }
        }
        
        wxString KeyboardShortcut::keyDisplayString(const int key) {
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
                case WXK_PAGEUP:
                    return L"\u21DE";
                case WXK_PAGEDOWN:
                    return L"\u21DF";
                case WXK_INSERT:
                    return L"Ins";
                case WXK_F1:
                    return L"F1";
                case WXK_F2:
                    return L"F2";
                case WXK_F3:
                    return L"F3";
                case WXK_F4:
                    return L"F4";
                case WXK_F5:
                    return L"F5";
                case WXK_F6:
                    return L"F6";
                case WXK_F7:
                    return L"F7";
                case WXK_F8:
                    return L"F8";
                case WXK_F9:
                    return L"F9";
                case WXK_F10:
                    return L"F01";
                case WXK_F11:
                    return L"F11";
                case WXK_F12:
                    return L"F12";
                case WXK_F13:
                    return L"F13";
                case WXK_F14:
                    return L"F14";
                case WXK_F15:
                    return L"F15";
                case WXK_F16:
                    return L"F16";
                case WXK_F17:
                    return L"F17";
                case WXK_F18:
                    return L"F18";
                case WXK_F19:
                    return L"F19";
                case WXK_F20:
                    return L"F20";
                case WXK_F21:
                    return L"F21";
                case WXK_F22:
                    return L"F22";
                case WXK_F23:
                    return L"F23";
                case WXK_F24:
                    return L"F24";
                default:
                    if (key >= 33 && key <= 126) {
                        wxUniChar c(key);
                        wxString result;
                        result << c;
                        return result;
                    }
                    return L"";
            }
#else
            return keyMenuString(key);
#endif
        }
        
        wxString KeyboardShortcut::modifierMenuString(const int key) {
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
        
        wxString KeyboardShortcut::modifierDisplayString(const int key) {
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
            return modifierMenuString(key);
#endif
        }
        
        bool KeyboardShortcut::parseShortcut(const wxString& string, int& key, int& modifier1, int& modifier2, int& modifier3) {
            modifier1 = modifier2 = modifier3 = key = WXK_NONE;
            
            int keys[4];
            for (size_t i = 0; i < 4; i++)
                keys[i] = WXK_NONE;
            
#if defined __APPLE__
            size_t keyIndex = string.Length();
            for (size_t i = 0; i < string.Length(); i++) {
                if (i > 3)
                    return false;
                
                wxUniChar c = string[i];
                if (c == L'\u21E7') {
                    keys[i] = WXK_SHIFT;
                } else if (c == L'\u2325') {
                    keys[i] = WXK_ALT;
                } else if (c == L'\u2318') {
                    keys[i] = WXK_CONTROL;
                } else {
                    keyIndex = i;
                    break;
                }
            }
            
            if (keyIndex < string.Length()) {
                wxString keyString = string.SubString(keyIndex, string.size() - 1);
                keys[3] = parseKeyDisplayString(keyString);
                if (keys[3] == WXK_NONE)
                    return false;
            }
#else
            size_t index = 0;
            wxStringTokenizer tokenizer(string, L"+");
            while (tokenizer.HasMoreTokens()) {
                if (index > 3)
                    return false;
                
                wxString token = tokenizer.GetNextToken();
                if (token == L"Ctrl") {
                    keys[index] = WXK_CONTROL;
                } else if (token == L"Alt") {
                    keys[index] = WXK_ALT;
                } else if (token == L"Shift") {
                    keys[index] = WXK_SHIFT;
                } else {
                    keys[3] = parseKeyDisplayString(token);
                    if (keys[3] == WXK_NONE)
                        return false;
                }
                
                index++;
            }
#endif
            
            modifier1 = keys[0];
            modifier2 = keys[1];
            modifier3 = keys[2];
            key = keys[3];
            sortModifierKeys(modifier1, modifier2, modifier3);
            return true;
        }
        
        int KeyboardShortcut::parseKeyDisplayString(const wxString& string) {
#if defined __APPLE__
            if (string == L"\u232B")
                return WXK_BACK;
            if (string == L"\u21E5")
                return WXK_TAB;
            if (string == L"\u21A9")
                return WXK_RETURN;
            if (string == L"\u238B")
                return WXK_ESCAPE;
            if (string == L"\u2423")
                return WXK_SPACE;
            if (string == L"\u2326")
                return WXK_DELETE;
            if (string == L"\u21F2")
                return WXK_END;
            if (string == L"\u21F1")
                return WXK_HOME;
            if (string == L"\u2190")
                return WXK_LEFT;
            if (string == L"\u2191")
                return WXK_UP;
            if (string == L"\u2192")
                return WXK_RIGHT;
            if (string == L"\u2193")
                return WXK_DOWN;
            if (string == L"\u21DE")
                return WXK_PAGEUP;
            if (string == L"\u21DF")
                return WXK_PAGEDOWN;
#else
            if (string == L"Back")
                return WXK_BACK;
            if (string == L"Tab")
                return WXK_TAB;
            if (string == L"Enter")
                return WXK_RETURN;
            if (string == L"Esc")
                return WXK_ESCAPE;
            if (string == L"Space")
                return WXK_SPACE;
            if (string == L"Del")
                return WXK_DELETE;
            if (string == L"End")
                return WXK_END;
            if (string == L"Home")
                return WXK_HOME;
            if (string == L"Left")
                return WXK_LEFT;
            if (string == L"Up")
                return WXK_UP;
            if (string == L"Right")
                return WXK_RIGHT;
            if (string == L"Down")
                return WXK_DOWN;
#endif
            
            if (string == L"Ins")
                return WXK_INSERT;
            if (string == L"F1")
                return WXK_F1;
            if (string == L"F2")
                return WXK_F2;
            if (string == L"F3")
                return WXK_F3;
            if (string == L"F4")
                return WXK_F4;
            if (string == L"F5")
                return WXK_F5;
            if (string == L"F6")
                return WXK_F6;
            if (string == L"F7")
                return WXK_F7;
            if (string == L"F8")
                return WXK_F8;
            if (string == L"F9")
                return WXK_F9;
            if (string == L"F10")
                return WXK_F10;
            if (string == L"F11")
                return WXK_F11;
            if (string == L"F12")
                return WXK_F12;
            if (string == L"F13")
                return WXK_F13;
            if (string == L"F14")
                return WXK_F14;
            if (string == L"F15")
                return WXK_F15;
            if (string == L"F16")
                return WXK_F16;
            if (string == L"F17")
                return WXK_F17;
            if (string == L"F18")
                return WXK_F18;
            if (string == L"F19")
                return WXK_F19;
            if (string == L"F20")
                return WXK_F20;
            if (string == L"F21")
                return WXK_F21;
            if (string == L"F22")
                return WXK_F22;
            if (string == L"F23")
                return WXK_F23;
            if (string == L"F24")
                return WXK_F24;
            
            if (string.Length() == 1) {
                wxUniChar c = string[0];
                return static_cast<int>(c);
            }
            
            return WXK_NONE;
        }
        
        KeyboardShortcut::KeyboardShortcut(const int key, const int modifier1, const int modifier2, const int modifier3) :
        m_key(key),
        m_modifier1(modifier1),
        m_modifier2(modifier2),
        m_modifier3(modifier3) {
            sortModifierKeys(m_modifier1, m_modifier2, m_modifier3);
        }
        
        KeyboardShortcut::KeyboardShortcut(const wxString& string) {
            wxStringInputStream stringStream(string);
            wxTextInputStream stream(stringStream, ':');
            
            wxUniChar colon;
            stream >> m_key;
            stream >> m_modifier1;
            stream >> m_modifier2;
            stream >> m_modifier3;
            
            sortModifierKeys(m_modifier1, m_modifier2, m_modifier3);
        }
        
        bool KeyboardShortcut::operator<(const KeyboardShortcut& other) const {
            return compare(other) < 0;
        }
        
        bool KeyboardShortcut::operator==(const KeyboardShortcut& other) const {
            return compare(other) == 0;
        }

        int KeyboardShortcut::compare(const KeyboardShortcut& other) const {
            if (m_key < other.m_key)
                return -1;
            if (m_key > other.m_key)
                return 1;
            if (m_modifier1 < other.m_modifier1)
                return -1;
            if (m_modifier1 > other.m_modifier1)
                return 1;
            if (m_modifier2 < other.m_modifier2)
                return -1;
            if (m_modifier2 > other.m_modifier2)
                return 1;
            if (m_modifier3 < other.m_modifier3)
                return -1;
            if (m_modifier3 > other.m_modifier3)
                return 1;
            return 0;
        }

        int KeyboardShortcut::key() const {
            return m_key;
        }
        
        bool KeyboardShortcut::hasKey() const {
            return m_key != WXK_NONE;
        }

        int KeyboardShortcut::modifier1() const {
            return m_modifier1;
        }
        
        int KeyboardShortcut::modifier2() const {
            return m_modifier2;
        }
        
        int KeyboardShortcut::modifier3() const {
            return m_modifier3;
        }
        
        bool KeyboardShortcut::hasModifier() const {
            return m_modifier1 != WXK_NONE || m_modifier2 != WXK_NONE || m_modifier3 != WXK_NONE;
        }
        
        bool KeyboardShortcut::hasModifier(const size_t index) const {
            return modifier(index) != WXK_NONE;
        }
        
        int KeyboardShortcut::modifier(const size_t index) const {
            assert(index < 3);
            switch (index) {
                case 0:
                    return m_modifier1;
                case 1:
                    return m_modifier2;
                default:
                    return m_modifier3;
            }
        }

        wxAcceleratorEntry KeyboardShortcut::acceleratorEntry(const int id) const {
            return wxAcceleratorEntry(acceleratorFlags(), m_key, id);
        }

        int KeyboardShortcut::acceleratorFlags() const {
            int flags = wxACCEL_NORMAL;
            
            const int modifiers[3] = { m_modifier1, m_modifier2, m_modifier3 };
            for (size_t i = 0; i < 3; ++i) {
                switch (modifiers[i]) {
                    case WXK_SHIFT:
                        flags |= wxACCEL_SHIFT;
                        break;
                    case WXK_CONTROL:
                        flags |= wxACCEL_CTRL;
                        break;
                    case WXK_ALT:
                        flags |= wxACCEL_ALT;
                        break;
                    default:
                        break;
                }
            }
            
            return flags;
        }

        bool KeyboardShortcut::matches(const wxKeyEvent& event) const {
            const int key = event.GetKeyCode();
            const int modifier1 = event.ControlDown() ? WXK_CONTROL : 0;
            const int modifier2 = event.AltDown() ? WXK_ALT : 0;
            const int modifier3 = event.ShiftDown() ? WXK_SHIFT : 0;
            return matches(key, modifier1, modifier2, modifier3);
        }

        bool KeyboardShortcut::matches(const int key, const int modifier1, const int modifier2, const int modifier3) const {
            if (key != m_key)
                return false;
            
            int myModifierKeys[] = { m_modifier1, m_modifier2, m_modifier3 };
            int inModifierKeys[] = { modifier1, modifier2, modifier3 };
            sortModifierKeys(inModifierKeys[0], inModifierKeys[1], inModifierKeys[2]);
            
            for (size_t i = 0; i < 3; i++)
                if (inModifierKeys[i] != myModifierKeys[i])
                    return false;
            return true;
        }
        
        bool KeyboardShortcut::matchesKey(const wxKeyEvent& event) const {
            return event.GetKeyCode() == m_key;
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
                    if (!hasModifier())
                        return false;
                    if (m_modifier1 == WXK_SHIFT && m_modifier2 == WXK_NONE)
                        return false;
                    return true;
            }
        }
        
        wxString KeyboardShortcut::shortcutMenuString() const {
            wxString text = modifierMenuString();
            if (text.empty())
                return keyMenuString();
            
            text << "+" << keyMenuString();
            return text;
        }
        
        wxString KeyboardShortcut::shortcutMenuItemString(const wxString& name) const {
            if (!hasKey())
                return name;
            
            wxString result;
            result << name << "\t";
            if (hasModifier())
                result << modifierMenuString() << "+";
            result << keyMenuString();
            return result;
        }
        
        wxString KeyboardShortcut::shortcutDisplayString() const {
            return shortcutDisplayString(m_key, m_modifier1, m_modifier2, m_modifier3);
        }
        
        wxString KeyboardShortcut::keyMenuString() const {
            return keyMenuString(m_key);
        }
        
        wxString KeyboardShortcut::keyDisplayString() const {
            return keyDisplayString(m_key);
        }

        wxString KeyboardShortcut::modifierMenuString() const {
            wxString text;
            text << modifierMenuString(m_modifier1);
            if (m_modifier1 != WXK_NONE && m_modifier2 != WXK_NONE)
                text << "+";
            
            text << modifierMenuString(m_modifier2);
            if ((m_modifier1 != WXK_NONE || m_modifier2 != WXK_NONE) && m_modifier3 != WXK_NONE)
                text << "+";
            
            text << modifierMenuString(m_modifier3);
            return text;
        }
        
        wxString KeyboardShortcut::asJsonString() const {
            wxString str;
            str << "{ key:" << key() << ", modifiers: [";
            
            bool hadModifier = false;
            for (size_t i = 0; i < 3; ++i) {
                if (hasModifier(i)) {
                    if (hadModifier)
                        str << ", ";
                    str << modifier(i);
                    hadModifier = true;
                } else {
                    hadModifier = false;
                }
            }
            
            str << "] }";
            
            return str;
        }

        wxString KeyboardShortcut::asString() const {
            wxString str;
            str << m_key << ":" << m_modifier1 << ":" << m_modifier2 << ":" << m_modifier3;
            return str;
        }
    }
}
