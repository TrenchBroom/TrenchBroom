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

#include <wx/tokenzr.h>

namespace TrenchBroom {
    namespace View {
        const KeyboardShortcut KeyboardShortcut::Empty(wxID_ANY, SCAny, "");

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

        bool KeyboardShortcut::isShortcutValid(const int key, const int modifierKey1, const int modifierKey2, const int modifierKey3) {
#ifdef __linux__
            // TAB and Escape are never allowed on GTK2:
            if (key == WXK_TAB || key == WXK_ESCAPE)
                return false;
            // cursor keys are only allowed if they have modifiers
            if (key == WXK_LEFT || key == WXK_RIGHT || key == WXK_UP || key == WXK_DOWN)
                return modifierKey1 != WXK_NONE || modifierKey2 != WXK_NONE || modifierKey3 != WXK_NONE;
#endif
            return true;
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
                    break;
            }
#else
            return keyMenuText(key);
#endif
        }

        int KeyboardShortcut::parseKeyDisplayText(const wxString string) {
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

        bool KeyboardShortcut::parseShortcut(const wxString& string, int& modifierKey1, int& modifierKey2, int& modifierKey3, int& key) {
            modifierKey1 = modifierKey2 = modifierKey3 = key = WXK_NONE;

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
                keys[3] = parseKeyDisplayText(keyString);
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
                    keys[3] = parseKeyDisplayText(token);
                    if (keys[3] == WXK_NONE)
                        return false;
                }

                index++;
            }
#endif

            modifierKey1 = keys[0];
            modifierKey2 = keys[1];
            modifierKey3 = keys[2];
            key = keys[3];
            return true;
        }

        KeyboardShortcut::KeyboardShortcut(const int commandId, const int context, const String& text) :
        m_commandId(commandId),
        m_modifierKey1(WXK_NONE),
        m_modifierKey2(WXK_NONE),
        m_modifierKey3(WXK_NONE),
        m_key(WXK_NONE),
        m_context(context),
        m_text(text) {}

        KeyboardShortcut::KeyboardShortcut(const int commandId, const int key, const int context, const String& text) :
        m_commandId(commandId),
        m_modifierKey1(WXK_NONE),
        m_modifierKey2(WXK_NONE),
        m_modifierKey3(WXK_NONE),
        m_key(key),
        m_context(context),
        m_text(text) {}

        KeyboardShortcut::KeyboardShortcut(const int commandId, const int modifierKey1, const int key, const int context, const String& text) :
        m_commandId(commandId),
        m_modifierKey1(modifierKey1),
        m_modifierKey2(WXK_NONE),
        m_modifierKey3(WXK_NONE),
        m_key(key),
        m_context(context),
        m_text(text) {}

        KeyboardShortcut::KeyboardShortcut(const int commandId, const int modifierKey1, const int modifierKey2, const int key, const int context, const String& text) :
        m_commandId(commandId),
        m_modifierKey1(modifierKey1),
        m_modifierKey2(modifierKey2),
        m_modifierKey3(WXK_NONE),
        m_key(key),
        m_context(context),
        m_text(text) {
            sortModifierKeys(m_modifierKey1, m_modifierKey2, m_modifierKey3);
        }

        KeyboardShortcut::KeyboardShortcut(const int commandId, const int modifierKey1, const int modifierKey2, const int modifierKey3, const int key, const int context, const String& text) :
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

        bool KeyboardShortcut::matches(const int key, const int modifierKey1, const int modifierKey2, const int modifierKey3) const {
            if (key != m_key)
                return false;

            int inModifierKeys[] = { modifierKey1, modifierKey2, modifierKey3 };
            sortModifierKeys(inModifierKeys[0], inModifierKeys[1], inModifierKeys[2]);

            int myModifierKeys[] = { m_modifierKey1, m_modifierKey2, m_modifierKey3 };
            sortModifierKeys(myModifierKeys[0], myModifierKeys[1], myModifierKeys[2]);

            for (size_t i = 0; i < 3; i++)
                if (inModifierKeys[i] != myModifierKeys[i])
                    return false;
            return true;
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
                    if (m_modifierKey1 == WXK_SHIFT && m_modifierKey2 == WXK_NONE)
                        return false;
                    return true;
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
