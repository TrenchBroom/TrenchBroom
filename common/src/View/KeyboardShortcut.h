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

#ifndef TrenchBroom_KeyboardShortcut
#define TrenchBroom_KeyboardShortcut

#include <wx/accel.h>
#include <wx/defs.h>
#include <wx/string.h>

#include <set>

class wxKeyEvent;

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcut {
        private:
            class MacModifierOrder {
            public:
                bool operator()(int lhs, int rhs) const;
            };

            class WinModifierOrder {
            public:
                bool operator()(int lhs, int rhs) const;
            };

#if defined __APPLE__
            using ModifierSet = std::set<int, MacModifierOrder>;
#else
            using ModifierSet = std::set<int, WinModifierOrder>;
#endif
        public:
            static const KeyboardShortcut Empty;

            static void sortModifierKeys(int& key1, int& key2, int& key3);
            static bool isShortcutValid(int key, int modifier1 = WXK_NONE, int modifier2 = WXK_NONE, int modifier3 = WXK_NONE);

            static wxString shortcutDisplayString(int key, int modifier1, int modifier2, int modifier3);

            static wxString keyMenuString(int key);
            static wxString keyDisplayString(int key);

            static wxString modifierMenuString(int key);
            static wxString modifierDisplayString(int key);

            static bool parseShortcut(const wxString& string, int& key, int& modifier1, int& modifier2, int& modifier3);
            static int parseKeyDisplayString(const wxString& string);
        private:
            int m_key;
            int m_modifier1;
            int m_modifier2;
            int m_modifier3;
        public:
            KeyboardShortcut(int key = WXK_NONE, int modifier1 = WXK_NONE, int modifier2 = WXK_NONE, int modifier3 = WXK_NONE);
            KeyboardShortcut(const wxString& string);

            bool operator<(const KeyboardShortcut& other) const;
            bool operator==(const KeyboardShortcut& other) const;
            int compare(const KeyboardShortcut& other) const;

            int key() const;
            bool hasKey() const;

            int modifier1() const;
            int modifier2() const;
            int modifier3() const;
            bool hasModifier() const;

            bool hasModifier(size_t index) const;
            int modifier(size_t index) const;

            wxAcceleratorEntry acceleratorEntry(int id) const;
            int acceleratorFlags() const;

            bool matchesKeyDown(const wxKeyEvent& event) const;
            bool matchesKeyUp(const wxKeyEvent& event) const;
            bool matches(int key, int modifier1 = WXK_NONE, int modifier2 = WXK_NONE, int modifier3 = WXK_NONE) const;
            bool matchesKey(const wxKeyEvent& event) const;
            bool alwaysShowModifier() const;

            wxString shortcutMenuString() const;
            wxString shortcutMenuItemString(const wxString& name) const;
            wxString shortcutDisplayString() const;

            wxString keyMenuString() const;
            wxString keyDisplayString() const;

            wxString modifierMenuString() const;

            wxString asJsonString() const;
            wxString asString() const;
        };
    }
}

#endif /* defined(TrenchBroom_KeyboardShortcut) */
