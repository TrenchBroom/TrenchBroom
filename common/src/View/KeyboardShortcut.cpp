/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include <QKeyEvent>

namespace TrenchBroom {
    namespace View {
        KeyboardShortcut::KeyboardShortcut(int qtKey)
        : m_qtKey(qtKey) {}

        QKeySequence KeyboardShortcut::keySequence() const {
            return QKeySequence(m_qtKey);
        }

        bool KeyboardShortcut::matchesKeyDown(const QKeyEvent* event) const {
            const auto ourKey = keySequence();
            if (ourKey.isEmpty()) {
                return false;
            }

            const auto theirKeyInt = event->key();
            const auto ourKeyInt = keySequence()[0];
            return ourKeyInt == theirKeyInt;
        }

        bool KeyboardShortcut::matchesKeyUp(const QKeyEvent* event) const {
           const Qt::KeyboardModifiers AllModifiers =
                   Qt::ShiftModifier
                   | Qt::ControlModifier
                   | Qt::AltModifier
                   | Qt::MetaModifier
                   | Qt::KeypadModifier
                   | Qt::GroupSwitchModifier;

            const auto ourKey = keySequence();
            if (ourKey.isEmpty()) {
                return false;
            }

            const auto theirKeyInt = event->key();
            const auto ourKeyInt = keySequence()[0];

            if (ourKeyInt == theirKeyInt) {
                return true;
            }

            // if any of the modifiers were released, it matches
            const auto releasedModifiers = event->key() & AllModifiers;
            const auto ourModifiers = ourKeyInt & AllModifiers;
            return (releasedModifiers & ourModifiers) != 0;
        }

#if 0
        bool KeyboardShortcut::MacModifierOrder::operator()(const int lhs, const int rhs) const {
            if (lhs == WXK_ALT)
                return rhs != WXK_ALT;
            if (lhs == WXK_SHIFT)
                return rhs != WXK_ALT && rhs != WXK_SHIFT;
            if (lhs == WXK_CONTROL)
                return rhs == WXK_NONE;
            return false;
        }

        bool KeyboardShortcut::WinModifierOrder::operator()(const int lhs, const int rhs) const {
            if (lhs == WXK_CONTROL)
                return rhs != WXK_CONTROL;
            if (lhs == WXK_ALT)
                return rhs != WXK_CONTROL && rhs != WXK_ALT;
            if (lhs == WXK_SHIFT)
                return rhs == WXK_NONE;
            return false;
        }

        const KeyboardShortcut KeyboardShortcut::Empty = KeyboardShortcut();

        void KeyboardShortcut::sortModifierKeys(int& key1, int& key2, int& key3) {
            ModifierSet modifierSet;
            modifierSet.insert(key1);
            modifierSet.insert(key2);
            modifierSet.insert(key3);

            key1 = key2 = key3 = WXK_NONE;
            ModifierSet::iterator it = std::begin(modifierSet);
            if (it != std::end(modifierSet)) {
                key1 = *it++;
                if (it != std::end(modifierSet)) {
                    key2 = *it++;
                    if (it != std::end(modifierSet)) {
                        key3 = *it++;
                    }
                }
            }
        }

        bool KeyboardShortcut::isShortcutValid(const int key, const int modifier1, const int modifier2, const int modifier3) {
#ifdef __WXGTK20__
            // TAB and Escape are never allowed on GTK2:
            if (key == WXK_TAB || key == WXK_ESCAPE)
                return false;
            // cursor keys are only allowed if they have modifiers
            if (key == WXK_LEFT || key == WXK_RIGHT || key == WXK_UP || key == WXK_DOWN)
                return modifier1 != WXK_NONE || modifier2 != WXK_NONE || modifier3 != WXK_NONE;
#endif
            return true;
        }

        QString KeyboardShortcut::shortcutDisplayString(const int key, int modifier1, int modifier2, int modifier3) {
            sortModifierKeys(modifier1, modifier2, modifier3);
            QString text;
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

        QString KeyboardShortcut::keyMenuString(const int key) {
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
                        QString str;
                        str << static_cast<char>(key);
                        return str;
                    }
                    return "";
            }
        }

        QString KeyboardShortcut::keyDisplayString(const int key) {
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
                        QString result;
                        result << c;
                        return result;
                    }
                    return L"";
            }
#else
            return keyMenuString(key);
#endif
        }

        QString KeyboardShortcut::modifierMenuString(const int key) {
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

        QString KeyboardShortcut::modifierDisplayString(const int key) {
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

        bool KeyboardShortcut::parseShortcut(const QString& string, int& key, int& modifier1, int& modifier2, int& modifier3) {
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
                QString keyString = string.SubString(keyIndex, string.size() - 1);
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

                QString token = tokenizer.GetNextToken();
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

        int KeyboardShortcut::parseKeyDisplayString(const QString& string) {
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
			if (string == L"PgUp")
				return WXK_PAGEUP;
			if (string == L"PgDn")
				return WXK_PAGEDOWN;
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

        KeyboardShortcut::KeyboardShortcut(const wxString& string) :
        m_key(WXK_NONE),
        m_modifier1(WXK_NONE),
        m_modifier2(WXK_NONE),
        m_modifier3(WXK_NONE) {
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

        bool KeyboardShortcut::matchesKeyDown(const QKeyEvent* event) const {
            const int key = event->key();
            const int modifier1 = (event->modifiers() & Qt::ControlModifier) ? WXK_CONTROL : WXK_NONE;
            const int modifier2 = (event->modifiers() & Qt::AltModifier)  ? WXK_ALT : WXK_NONE;
            const int modifier3 = (event->modifiers() & Qt::ShiftModifier)  ? WXK_SHIFT : WXK_NONE;
            return matches(key, modifier1, modifier2, modifier3);
        }

        bool KeyboardShortcut::matchesKeyUp(const QKeyEvent* event) const {
            // FIXME: will need some wx<->Qt translation
            const int key = event->key();

            if (key == m_key)
                return true;

            // FIXME: needs to be updated
            int myModifierKeys[] = { m_modifier1, m_modifier2, m_modifier3 };
            for (int i = 0; i < 3; ++i) {
                if (key == myModifierKeys[i]) {
                    return true;
                }
            }

            return false;
        }

        bool KeyboardShortcut::matchesKeyDown(const wxKeyEvent &event) const {
            const int key = event.GetKeyCode();
            const int modifier1 = event.ControlDown() ? WXK_CONTROL : WXK_NONE;
            const int modifier2 = event.AltDown() ? WXK_ALT : WXK_NONE;
            const int modifier3 = event.ShiftDown() ? WXK_SHIFT : WXK_NONE;
            return matches(key, modifier1, modifier2, modifier3);
        }

        bool KeyboardShortcut::matchesKeyUp(const wxKeyEvent& event) const {
            const int key = event.GetKeyCode();

            if (key == m_key)
                return true;

            int myModifierKeys[] = { m_modifier1, m_modifier2, m_modifier3 };
            for (int i = 0; i < 3; ++i) {
                if (key == myModifierKeys[i]) {
                    return true;
                }
            }

            return false;
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

        QString KeyboardShortcut::shortcutMenuString() const {
            QString text = modifierMenuString();
            if (text.empty())
                return keyMenuString();

            text << "+" << keyMenuString();
            return text;
        }

        QString KeyboardShortcut::shortcutMenuItemString(const QString& name) const {
            if (!hasKey())
                return name;

            QString result;
            result << name << "\t";
            if (hasModifier())
                result << modifierMenuString() << "+";
            result << keyMenuString();
            return result;
        }

        QString KeyboardShortcut::shortcutDisplayString() const {
            return shortcutDisplayString(m_key, m_modifier1, m_modifier2, m_modifier3);
        }

        QString KeyboardShortcut::keyMenuString() const {
            return keyMenuString(m_key);
        }

        QString KeyboardShortcut::keyDisplayString() const {
            return keyDisplayString(m_key);
        }

        QString KeyboardShortcut::modifierMenuString() const {
            QString text;
            text << modifierMenuString(m_modifier1);
            if (m_modifier1 != WXK_NONE && m_modifier2 != WXK_NONE)
                text << "+";

            text << modifierMenuString(m_modifier2);
            if ((m_modifier1 != WXK_NONE || m_modifier2 != WXK_NONE) && m_modifier3 != WXK_NONE)
                text << "+";

            text << modifierMenuString(m_modifier3);
            return text;
        }

        QString KeyboardShortcut::asJsonString() const {
            QString str;
            str << "{ key:" << key() << ", modifiers: [";

            std::vector<int> modifiers;
            modifiers.reserve(3);

            for (size_t i = 0; i < 3; ++i) {
                if (hasModifier(i)) {
                    modifiers.push_back(modifier(i));
                }
            }

            VectorUtils::sort(modifiers);

            str << StringUtils::join(
                VectorUtils::map(modifiers, [](auto m) { return std::to_string(m); }), ", ");
            str << "] }";

            return str;
        }

        QString KeyboardShortcut::asString() const {
            QString str;
            str << m_key << ":" << m_modifier1 << ":" << m_modifier2 << ":" << m_modifier3;
            return str;
        }
#endif

        int wxModifierToQt(const int wxMod) {
            const auto wxMOD_ALT         = 0x0001;
            const auto wxMOD_CONTROL     = 0x0002; // Command key on macOS
            const auto wxMOD_SHIFT       = 0x0004;
            const auto wxMOD_META        = 0x0008;
            const auto wxMOD_RAW_CONTROL = 0x0010; // Control key on macOS

            int result = 0;

            if (wxMod & wxMOD_ALT) {
                result |= Qt::AltModifier;
            }
            if (wxMod & wxMOD_CONTROL) {
                result |= Qt::ControlModifier;
            }
            if (wxMod & wxMOD_SHIFT) {
                result |= Qt::ShiftModifier;
            }
            // Both wxMOD_META and wxMOD_RAW_CONTROL map to Qt::MetaModifier, because wx uses wxMOD_RAW_CONTROL
            // for the Control key on macOS, and Qt uses Qt::MetaModifier
            if (wxMod & (wxMOD_META | wxMOD_RAW_CONTROL)) {
                result |= Qt::MetaModifier;
            }

            return result;
        }

        int wxKeyToQt(const int wxKey) {
            const auto WXK_BACK = 8;
            const auto WXK_TAB = 9;
            const auto WXK_RETURN = 13;
            const auto WXK_ESCAPE = 27;
            const auto WXK_SPACE = 32;
            const auto WXK_DELETE = 127;
            const auto WXK_START = 300;
            const auto WXK_LBUTTON = 301;
            const auto WXK_RBUTTON = 302;
            const auto WXK_CANCEL = 303;
            const auto WXK_MBUTTON = 304;
            const auto WXK_CLEAR = 305;
            const auto WXK_SHIFT = 306;
            const auto WXK_ALT = 307;
            const auto WXK_CONTROL = 308;
            const auto WXK_MENU = 309;
            const auto WXK_PAUSE = 310;
            const auto WXK_CAPITAL = 311;
            const auto WXK_END = 312;
            const auto WXK_HOME = 313;
            const auto WXK_LEFT = 314;
            const auto WXK_UP = 315;
            const auto WXK_RIGHT = 316;
            const auto WXK_DOWN = 317;
            const auto WXK_SELECT = 318;
            const auto WXK_PRINT = 319;
            const auto WXK_EXECUTE = 320;
            const auto WXK_SNAPSHOT = 321;
            const auto WXK_INSERT = 322;
            const auto WXK_HELP = 323;
            const auto WXK_NUMPAD0 = 324;
            const auto WXK_NUMPAD1 = 325;
            const auto WXK_NUMPAD2 = 326;
            const auto WXK_NUMPAD3 = 327;
            const auto WXK_NUMPAD4 = 328;
            const auto WXK_NUMPAD5 = 329;
            const auto WXK_NUMPAD6 = 330;
            const auto WXK_NUMPAD7 = 331;
            const auto WXK_NUMPAD8 = 332;
            const auto WXK_NUMPAD9 = 333;
            const auto WXK_MULTIPLY = 334;
            const auto WXK_ADD = 335;
            const auto WXK_SEPARATOR = 336;
            const auto WXK_SUBTRACT = 337;
            const auto WXK_DECIMAL = 338;
            const auto WXK_DIVIDE = 339;
            const auto WXK_F1 = 340;
            const auto WXK_F2 = 341;
            const auto WXK_F3 = 342;
            const auto WXK_F4 = 343;
            const auto WXK_F5 = 344;
            const auto WXK_F6 = 345;
            const auto WXK_F7 = 346;
            const auto WXK_F8 = 347;
            const auto WXK_F9 = 348;
            const auto WXK_F10 = 349;
            const auto WXK_F11 = 350;
            const auto WXK_F12 = 351;
            const auto WXK_F13 = 352;
            const auto WXK_F14 = 353;
            const auto WXK_F15 = 354;
            const auto WXK_F16 = 355;
            const auto WXK_F17 = 356;
            const auto WXK_F18 = 357;
            const auto WXK_F19 = 358;
            const auto WXK_F20 = 359;
            const auto WXK_F21 = 360;
            const auto WXK_F22 = 361;
            const auto WXK_F23 = 362;
            const auto WXK_F24 = 363;
            const auto WXK_NUMLOCK = 364;
            const auto WXK_SCROLL = 365;
            const auto WXK_PAGEUP = 366;
            const auto WXK_PAGEDOWN = 367;
            const auto WXK_NUMPAD_SPACE = 368;
            const auto WXK_NUMPAD_TAB = 369;
            const auto WXK_NUMPAD_ENTER = 370;
            const auto WXK_NUMPAD_F1 = 371;
            const auto WXK_NUMPAD_F2 = 372;
            const auto WXK_NUMPAD_F3 = 373;
            const auto WXK_NUMPAD_F4 = 374;
            const auto WXK_NUMPAD_HOME = 375;
            const auto WXK_NUMPAD_LEFT = 376;
            const auto WXK_NUMPAD_UP = 377;
            const auto WXK_NUMPAD_RIGHT = 378;
            const auto WXK_NUMPAD_DOWN = 379;
            const auto WXK_NUMPAD_PAGEUP = 380;
            const auto WXK_NUMPAD_PAGEDOWN = 381;
            const auto WXK_NUMPAD_END = 382;
            const auto WXK_NUMPAD_BEGIN = 383;
            const auto WXK_NUMPAD_INSERT = 384;
            const auto WXK_NUMPAD_DELETE = 385;
            const auto WXK_NUMPAD_EQUAL = 386;
            const auto WXK_NUMPAD_MULTIPLY = 387;
            const auto WXK_NUMPAD_ADD = 388;
            const auto WXK_NUMPAD_SEPARATOR = 389;
            const auto WXK_NUMPAD_SUBTRACT = 390;
            const auto WXK_NUMPAD_DECIMAL = 391;
            const auto WXK_NUMPAD_DIVIDE = 392;
            const auto WXK_WINDOWS_LEFT = 393;
            const auto WXK_WINDOWS_RIGHT = 394;
            const auto WXK_WINDOWS_MENU = 395;
            const auto WXK_BROWSER_BACK = 417;
            const auto WXK_BROWSER_FORWARD = 418;
            const auto WXK_BROWSER_REFRESH = 419;
            const auto WXK_BROWSER_STOP = 420;
            const auto WXK_BROWSER_SEARCH = 421;
            const auto WXK_BROWSER_FAVORITES = 422;
            const auto WXK_BROWSER_HOME = 423;
            const auto WXK_VOLUME_MUTE = 424;
            const auto WXK_VOLUME_DOWN = 425;
            const auto WXK_VOLUME_UP = 426;
            const auto WXK_MEDIA_NEXT_TRACK = 427;
            const auto WXK_MEDIA_PREV_TRACK = 428;
            const auto WXK_MEDIA_STOP = 429;
            const auto WXK_MEDIA_PLAY_PAUSE = 430;
            const auto WXK_LAUNCH_MAIL = 431;

            // special cases
            switch (wxKey) {
                case WXK_BACK: return Qt::Key_Backspace;
                case WXK_TAB: return Qt::Key_Tab;
                case WXK_RETURN: return Qt::Key_Return;
                case WXK_ESCAPE: return Qt::Key_Escape;
                case WXK_SPACE: return Qt::Key_Space;
                case WXK_DELETE: return Qt::Key_Delete;
                case WXK_START: return 0;
                case WXK_LBUTTON: return 0;
                case WXK_RBUTTON: return 0;
                case WXK_CANCEL: return Qt::Key_Cancel;
                case WXK_MBUTTON: return 0;
                case WXK_CLEAR: return Qt::Key_Clear;
                case WXK_SHIFT: return Qt::Key_Shift;
                case WXK_ALT: return Qt::Key_Alt;
                case WXK_CONTROL: return Qt::Key_Control;
                case WXK_MENU: return Qt::Key_Menu;
                case WXK_PAUSE: return Qt::Key_Pause;
                case WXK_CAPITAL: return 0;
                case WXK_END: return Qt::Key_End;
                case WXK_HOME: return Qt::Key_Home;
                case WXK_LEFT: return Qt::Key_Left;
                case WXK_UP: return Qt::Key_Up;
                case WXK_RIGHT: return Qt::Key_Right;
                case WXK_DOWN: return Qt::Key_Down;
                case WXK_SELECT: return Qt::Key_Select;
                case WXK_PRINT: return Qt::Key_Print;
                case WXK_EXECUTE: return Qt::Key_Execute;
                case WXK_SNAPSHOT: return 0;
                case WXK_INSERT: return Qt::Key_Insert;
                case WXK_HELP: return Qt::Key_Help;
                case WXK_NUMPAD0: return Qt::KeypadModifier | Qt::Key_0;
                case WXK_NUMPAD1: return Qt::KeypadModifier | Qt::Key_1;
                case WXK_NUMPAD2: return Qt::KeypadModifier | Qt::Key_2;
                case WXK_NUMPAD3: return Qt::KeypadModifier | Qt::Key_3;
                case WXK_NUMPAD4: return Qt::KeypadModifier | Qt::Key_4;
                case WXK_NUMPAD5: return Qt::KeypadModifier | Qt::Key_5;
                case WXK_NUMPAD6: return Qt::KeypadModifier | Qt::Key_6;
                case WXK_NUMPAD7: return Qt::KeypadModifier | Qt::Key_7;
                case WXK_NUMPAD8: return Qt::KeypadModifier | Qt::Key_8;
                case WXK_NUMPAD9: return Qt::KeypadModifier | Qt::Key_9;
                case WXK_MULTIPLY: return Qt::Key_multiply;
                case WXK_ADD: return Qt::Key_Plus;
                case WXK_SEPARATOR: return 0;
                case WXK_SUBTRACT: return Qt::Key_Minus;
                case WXK_DECIMAL: return Qt::Key_Period;
                case WXK_DIVIDE: return Qt::Key_division;
                case WXK_F1: return Qt::Key_F1;
                case WXK_F2: return Qt::Key_F2;
                case WXK_F3: return Qt::Key_F3;
                case WXK_F4: return Qt::Key_F4;
                case WXK_F5: return Qt::Key_F5;
                case WXK_F6: return Qt::Key_F6;
                case WXK_F7: return Qt::Key_F7;
                case WXK_F8: return Qt::Key_F8;
                case WXK_F9: return Qt::Key_F9;
                case WXK_F10: return Qt::Key_F10;
                case WXK_F11: return Qt::Key_F11;
                case WXK_F12: return Qt::Key_F12;
                case WXK_F13: return Qt::Key_F13;
                case WXK_F14: return Qt::Key_F14;
                case WXK_F15: return Qt::Key_F15;
                case WXK_F16: return Qt::Key_F16;
                case WXK_F17: return Qt::Key_F17;
                case WXK_F18: return Qt::Key_F18;
                case WXK_F19: return Qt::Key_F19;
                case WXK_F20: return Qt::Key_F20;
                case WXK_F21: return Qt::Key_F21;
                case WXK_F22: return Qt::Key_F22;
                case WXK_F23: return Qt::Key_F23;
                case WXK_F24: return Qt::Key_F24;
                case WXK_NUMLOCK: return Qt::Key_NumLock;
                case WXK_SCROLL: return Qt::Key_ScrollLock;
                case WXK_PAGEUP: return Qt::Key_PageUp;
                case WXK_PAGEDOWN: return Qt::Key_PageDown;
                case WXK_NUMPAD_SPACE: return Qt::KeypadModifier | Qt::Key_Space;
                case WXK_NUMPAD_TAB: return Qt::KeypadModifier | Qt::Key_Tab;
                case WXK_NUMPAD_ENTER: return Qt::KeypadModifier | Qt::Key_Enter;
                case WXK_NUMPAD_F1: return Qt::KeypadModifier | Qt::Key_F1;
                case WXK_NUMPAD_F2: return Qt::KeypadModifier | Qt::Key_F2;
                case WXK_NUMPAD_F3: return Qt::KeypadModifier | Qt::Key_F3;
                case WXK_NUMPAD_F4: return Qt::KeypadModifier | Qt::Key_F4;
                case WXK_NUMPAD_HOME: return Qt::KeypadModifier | Qt::Key_Home;
                case WXK_NUMPAD_LEFT: return Qt::KeypadModifier | Qt::Key_Left;
                case WXK_NUMPAD_UP: return Qt::KeypadModifier | Qt::Key_Up;
                case WXK_NUMPAD_RIGHT: return Qt::KeypadModifier | Qt::Key_Right;
                case WXK_NUMPAD_DOWN: return Qt::KeypadModifier | Qt::Key_Down;
                case WXK_NUMPAD_PAGEUP: return Qt::KeypadModifier | Qt::Key_PageUp;
                case WXK_NUMPAD_PAGEDOWN: return Qt::KeypadModifier | Qt::Key_PageDown;
                case WXK_NUMPAD_END: return Qt::KeypadModifier | Qt::Key_End;
                case WXK_NUMPAD_BEGIN: return 0;
                case WXK_NUMPAD_INSERT: return Qt::KeypadModifier | Qt::Key_Insert;
                case WXK_NUMPAD_DELETE: return Qt::KeypadModifier | Qt::Key_Delete;
                case WXK_NUMPAD_EQUAL: return Qt::KeypadModifier | Qt::Key_Equal;
                case WXK_NUMPAD_MULTIPLY: return Qt::KeypadModifier | Qt::Key_multiply;
                case WXK_NUMPAD_ADD: return Qt::KeypadModifier | Qt::Key_Plus;
                case WXK_NUMPAD_SEPARATOR: return 0;
                case WXK_NUMPAD_SUBTRACT: return Qt::KeypadModifier | Qt::Key_Minus;
                case WXK_NUMPAD_DECIMAL: return Qt::KeypadModifier | Qt::Key_Period;
                case WXK_NUMPAD_DIVIDE: return Qt::KeypadModifier | Qt::Key_division;
                case WXK_WINDOWS_LEFT: return Qt::Key_Meta;
                case WXK_WINDOWS_RIGHT: return Qt::Key_Meta;
                case WXK_WINDOWS_MENU: return Qt::Key_ApplicationRight;
                case WXK_BROWSER_BACK: return Qt::Key_Back;
                case WXK_BROWSER_FORWARD: return Qt::Key_Forward;
                case WXK_BROWSER_REFRESH: return Qt::Key_Refresh;
                case WXK_BROWSER_STOP: return Qt::Key_Stop;
                case WXK_BROWSER_SEARCH: return Qt::Key_Search;
                case WXK_BROWSER_FAVORITES: return Qt::Key_Favorites;
                case WXK_BROWSER_HOME: return Qt::Key_HomePage;
                case WXK_VOLUME_MUTE: return Qt::Key_VolumeMute;
                case WXK_VOLUME_DOWN: return Qt::Key_VolumeDown;
                case WXK_VOLUME_UP: return Qt::Key_VolumeUp;
                case WXK_MEDIA_NEXT_TRACK: return Qt::Key_MediaNext;
                case WXK_MEDIA_PREV_TRACK: return Qt::Key_MediaPrevious;
                case WXK_MEDIA_STOP: return Qt::Key_MediaStop;
                case WXK_MEDIA_PLAY_PAUSE: return Qt::Key_MediaPlay;
                case WXK_LAUNCH_MAIL: return Qt::Key_LaunchMail;
                default: break;
            }

            // Map lowercase letters to uppercase
            if (wxKey >= 'a' && wxKey <= 'z') {
                return 'A' + (wxKey - 'a');
            }

            // Pass through ASCII
            if (wxKey >= 0 && wxKey <= 127) {
                return wxKey;
            }

            return 0;
        }
    }
}
