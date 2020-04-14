
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

#include "View/KeyboardShortcut.h"
#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "TestUtils.h"

namespace TrenchBroom {
    namespace View {
        /*
        void assertSortModifierKeys(int key1, int key2, int key3, const int exp1, const int exp2, const int exp3);
        void assertSortModifierKeys(int key1, int key2, int key3, const int exp1, const int exp2, const int exp3) {
            KeyboardShortcut::sortModifierKeys(key1, key2, key3);
            ASSERT_EQ(exp1, key1);
            ASSERT_EQ(exp2, key2);
            ASSERT_EQ(exp3, key3);
        }

        TEST_CASE("KeyboardShortcutTest.sortModifierKeys", "[KeyboardShortcutTest]") {
            assertSortModifierKeys(WXK_ALT, WXK_NONE, WXK_NONE,
                                   WXK_ALT, WXK_NONE, WXK_NONE);
            assertSortModifierKeys(WXK_SHIFT, WXK_NONE, WXK_NONE,
                                   WXK_SHIFT, WXK_NONE, WXK_NONE);
            assertSortModifierKeys(WXK_CONTROL, WXK_NONE, WXK_NONE,
                                   WXK_CONTROL, WXK_NONE, WXK_NONE);

            assertSortModifierKeys(WXK_NONE, WXK_ALT, WXK_NONE,
                                   WXK_ALT, WXK_NONE, WXK_NONE);
            assertSortModifierKeys(WXK_NONE, WXK_SHIFT, WXK_NONE,
                                   WXK_SHIFT, WXK_NONE, WXK_NONE);
            assertSortModifierKeys(WXK_NONE, WXK_CONTROL, WXK_NONE,
                                   WXK_CONTROL, WXK_NONE, WXK_NONE);

            assertSortModifierKeys(WXK_NONE, WXK_NONE, WXK_ALT,
                                   WXK_ALT, WXK_NONE, WXK_NONE);
            assertSortModifierKeys(WXK_NONE, WXK_NONE, WXK_SHIFT,
                                   WXK_SHIFT, WXK_NONE, WXK_NONE);
            assertSortModifierKeys(WXK_NONE, WXK_NONE, WXK_CONTROL,
                                   WXK_CONTROL, WXK_NONE, WXK_NONE);

#ifdef __APPLE__
            assertSortModifierKeys(WXK_ALT, WXK_SHIFT, WXK_CONTROL,
                                   WXK_ALT, WXK_SHIFT, WXK_CONTROL);
            assertSortModifierKeys(WXK_ALT, WXK_CONTROL, WXK_SHIFT,
                                   WXK_ALT, WXK_SHIFT, WXK_CONTROL);
            assertSortModifierKeys(WXK_CONTROL, WXK_ALT, WXK_SHIFT,
                                   WXK_ALT, WXK_SHIFT, WXK_CONTROL);
            assertSortModifierKeys(WXK_SHIFT, WXK_ALT, WXK_CONTROL,
                                   WXK_ALT, WXK_SHIFT, WXK_CONTROL);
            assertSortModifierKeys(WXK_SHIFT, WXK_CONTROL, WXK_ALT,
                                   WXK_ALT, WXK_SHIFT, WXK_CONTROL);
#else
            assertSortModifierKeys(WXK_ALT, WXK_SHIFT, WXK_CONTROL,
                                   WXK_CONTROL, WXK_ALT, WXK_SHIFT);
            assertSortModifierKeys(WXK_ALT, WXK_CONTROL, WXK_SHIFT,
                                   WXK_CONTROL, WXK_ALT, WXK_SHIFT);
            assertSortModifierKeys(WXK_CONTROL, WXK_ALT, WXK_SHIFT,
                                   WXK_CONTROL, WXK_ALT, WXK_SHIFT);
            assertSortModifierKeys(WXK_SHIFT, WXK_ALT, WXK_CONTROL,
                                   WXK_CONTROL, WXK_ALT, WXK_SHIFT);
            assertSortModifierKeys(WXK_SHIFT, WXK_CONTROL, WXK_ALT,
                                   WXK_CONTROL, WXK_ALT, WXK_SHIFT);
#endif
        }

#ifdef __WXGTK20__
        TEST_CASE("KeyboardShortcutTest.isShortcutValid", "[KeyboardShortcutTest]") {
            ASSERT_FALSE(KeyboardShortcut::isShortcutValid(WXK_TAB));
            ASSERT_FALSE(KeyboardShortcut::isShortcutValid(WXK_TAB, WXK_CONTROL));
            ASSERT_FALSE(KeyboardShortcut::isShortcutValid(WXK_ESCAPE));
            ASSERT_FALSE(KeyboardShortcut::isShortcutValid(WXK_ESCAPE, WXK_SHIFT));

            ASSERT_FALSE(KeyboardShortcut::isShortcutValid(WXK_LEFT));
            ASSERT_TRUE(KeyboardShortcut::isShortcutValid(WXK_LEFT, WXK_SHIFT));
            ASSERT_FALSE(KeyboardShortcut::isShortcutValid(WXK_RIGHT));
            ASSERT_TRUE(KeyboardShortcut::isShortcutValid(WXK_RIGHT, WXK_CONTROL));
            ASSERT_FALSE(KeyboardShortcut::isShortcutValid(WXK_UP));
            ASSERT_TRUE(KeyboardShortcut::isShortcutValid(WXK_UP, WXK_ALT));
            ASSERT_FALSE(KeyboardShortcut::isShortcutValid(WXK_DOWN));
            ASSERT_TRUE(KeyboardShortcut::isShortcutValid(WXK_DOWN, WXK_CONTROL, WXK_ALT));
        }
#endif

        TEST_CASE("KeyboardShortcutTest.staticModifierKeyMenuText", "[KeyboardShortcutTest]") {
            ASSERT_WXSTR_EQ(QString("Ctrl"), KeyboardShortcut::modifierMenuString(WXK_CONTROL));
            ASSERT_WXSTR_EQ(QString("Alt"), KeyboardShortcut::modifierMenuString(WXK_ALT));
            ASSERT_WXSTR_EQ(QString("Shift"), KeyboardShortcut::modifierMenuString(WXK_SHIFT));
            ASSERT_WXSTR_EQ(QString(""), KeyboardShortcut::modifierMenuString(WXK_TAB));
        }

        TEST_CASE("KeyboardShortcutTest.modifierDisplayString", "[KeyboardShortcutTest]") {
#ifdef __APPLE__
            ASSERT_WXSTR_EQ(QString(L"\u2318"), KeyboardShortcut::modifierDisplayString(WXK_CONTROL));
            ASSERT_WXSTR_EQ(QString(L"\u2325"), KeyboardShortcut::modifierDisplayString(WXK_ALT));
            ASSERT_WXSTR_EQ(QString(L"\u21E7"), KeyboardShortcut::modifierDisplayString(WXK_SHIFT));
            ASSERT_WXSTR_EQ(QString(""), KeyboardShortcut::modifierDisplayString(WXK_TAB));
#else
            ASSERT_WXSTR_EQ(QString("Ctrl"), KeyboardShortcut::modifierDisplayString(WXK_CONTROL));
            ASSERT_WXSTR_EQ(QString("Alt"), KeyboardShortcut::modifierDisplayString(WXK_ALT));
            ASSERT_WXSTR_EQ(QString("Shift"), KeyboardShortcut::modifierDisplayString(WXK_SHIFT));
            ASSERT_WXSTR_EQ(QString(""), KeyboardShortcut::modifierDisplayString(WXK_TAB));
#endif
        }

        TEST_CASE("KeyboardShortcutTest.staticShortcutDisplayText", "[KeyboardShortcutTest]") {
#ifdef __APPLE__
            ASSERT_WXSTR_EQ(QString("C"), KeyboardShortcut::shortcutDisplayString('C', WXK_NONE, WXK_NONE, WXK_NONE));
            ASSERT_WXSTR_EQ(QString(L"\u238B"), KeyboardShortcut::shortcutDisplayString(WXK_ESCAPE, WXK_NONE, WXK_NONE, WXK_NONE));
            ASSERT_WXSTR_EQ(QString("F11"), KeyboardShortcut::shortcutDisplayString(WXK_F11, WXK_NONE, WXK_NONE, WXK_NONE));
            ASSERT_WXSTR_EQ(QString(L"\u2318D"), KeyboardShortcut::shortcutDisplayString('D', WXK_CONTROL, WXK_NONE, WXK_NONE));
            ASSERT_WXSTR_EQ(QString(L"\u2318D"), KeyboardShortcut::shortcutDisplayString('D', WXK_NONE, WXK_NONE, WXK_CONTROL));
            ASSERT_WXSTR_EQ(QString(L"\u2325\u2318S"), KeyboardShortcut::shortcutDisplayString('S', WXK_CONTROL, WXK_NONE, WXK_ALT));
#else
            ASSERT_WXSTR_EQ(QString("C"), KeyboardShortcut::shortcutDisplayString('C', WXK_NONE, WXK_NONE, WXK_NONE));
            ASSERT_WXSTR_EQ(QString("Esc"), KeyboardShortcut::shortcutDisplayString(WXK_ESCAPE, WXK_NONE, WXK_NONE, WXK_NONE));
            ASSERT_WXSTR_EQ(QString("F11"), KeyboardShortcut::shortcutDisplayString(WXK_F11, WXK_NONE, WXK_NONE, WXK_NONE));
            ASSERT_WXSTR_EQ(QString("Ctrl+D"), KeyboardShortcut::shortcutDisplayString('D', WXK_CONTROL, WXK_NONE, WXK_NONE));
            ASSERT_WXSTR_EQ(QString("Ctrl+D"), KeyboardShortcut::shortcutDisplayString('D', WXK_NONE, WXK_NONE, WXK_CONTROL));
            ASSERT_WXSTR_EQ(QString("Ctrl+Alt+S"), KeyboardShortcut::shortcutDisplayString('S', WXK_ALT, WXK_CONTROL, WXK_NONE));
#endif
        }

        TEST_CASE("KeyboardShortcutTest.parseShortcut", "[KeyboardShortcutTest]") {
            int m1, m2, m3, k;

            ASSERT_TRUE(KeyboardShortcut::parseShortcut("", k, m1, m2, m3));
            ASSERT_EQ(WXK_NONE, m1);
            ASSERT_EQ(WXK_NONE, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ(WXK_NONE, k);

            ASSERT_FALSE(KeyboardShortcut::parseShortcut("asdf", k, m1, m2, m3));
            ASSERT_FALSE(KeyboardShortcut::parseShortcut(" D", k, m1, m2, m3));
            ASSERT_FALSE(KeyboardShortcut::parseShortcut("D ", k, m1, m2, m3));

            ASSERT_TRUE(KeyboardShortcut::parseShortcut("D", k, m1, m2, m3));
            ASSERT_EQ(WXK_NONE, m1);
            ASSERT_EQ(WXK_NONE, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ('D', k);

#ifdef __APPLE__
            ASSERT_FALSE(KeyboardShortcut::parseShortcut("\u2318+D", k, m1, m2, m3));

            ASSERT_TRUE(KeyboardShortcut::parseShortcut("\u2318", k, m1, m2, m3));
            ASSERT_EQ(WXK_CONTROL, m1);
            ASSERT_EQ(WXK_NONE, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ(WXK_NONE, k);

            ASSERT_TRUE(KeyboardShortcut::parseShortcut("\u2318D", k, m1, m2, m3));
            ASSERT_EQ(WXK_CONTROL, m1);
            ASSERT_EQ(WXK_NONE, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ('D', k);

            ASSERT_TRUE(KeyboardShortcut::parseShortcut("\u2318\u2325D", k, m1, m2, m3));
            ASSERT_EQ(WXK_ALT, m1);
            ASSERT_EQ(WXK_CONTROL, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ('D', k);

            ASSERT_TRUE(KeyboardShortcut::parseShortcut("\u2318\u2325\u21E7\u21E5", k, m1, m2, m3));
            ASSERT_EQ(WXK_ALT, m1);
            ASSERT_EQ(WXK_SHIFT, m2);
            ASSERT_EQ(WXK_CONTROL, m3);
            ASSERT_EQ(WXK_TAB, k);
#else
            ASSERT_FALSE(KeyboardShortcut::parseShortcut("Ctrl D", k, m1, m2, m3));

            ASSERT_TRUE(KeyboardShortcut::parseShortcut("Ctrl", k, m1, m2, m3));
            ASSERT_EQ(WXK_CONTROL, m1);
            ASSERT_EQ(WXK_NONE, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ(WXK_NONE, k);

            ASSERT_TRUE(KeyboardShortcut::parseShortcut("Ctrl+D", k, m1, m2, m3));
            ASSERT_EQ(WXK_CONTROL, m1);
            ASSERT_EQ(WXK_NONE, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ('D', k);

            ASSERT_TRUE(KeyboardShortcut::parseShortcut("Alt+Ctrl+D", k, m1, m2, m3));
            ASSERT_EQ(WXK_CONTROL, m1);
            ASSERT_EQ(WXK_ALT, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ('D', k);

            ASSERT_TRUE(KeyboardShortcut::parseShortcut("Alt+Ctrl+Shift+Tab", k, m1, m2, m3));
            ASSERT_EQ(WXK_CONTROL, m1);
            ASSERT_EQ(WXK_ALT, m2);
            ASSERT_EQ(WXK_SHIFT, m3);
            ASSERT_EQ(WXK_TAB, k);

			ASSERT_TRUE(KeyboardShortcut::parseShortcut("Shift+PgUp", k, m1, m2, m3));
			ASSERT_EQ(WXK_SHIFT, m1);
			ASSERT_EQ(WXK_NONE, m2);
			ASSERT_EQ(WXK_NONE, m3);
			ASSERT_EQ(WXK_PAGEUP, k);

			ASSERT_TRUE(KeyboardShortcut::parseShortcut("Shift+PgDn", k, m1, m2, m3));
			ASSERT_EQ(WXK_SHIFT, m1);
			ASSERT_EQ(WXK_NONE, m2);
			ASSERT_EQ(WXK_NONE, m3);
			ASSERT_EQ(WXK_PAGEDOWN, k);
#endif
        }

        TEST_CASE("KeyboardShortcutTest.constructWithString", "[KeyboardShortcutTest]") {
            StringStream test;
            test << static_cast<int>('D') << ":" << WXK_CONTROL << ":" << WXK_ALT << ":" << WXK_NONE;

            const KeyboardShortcut shortcut(test.str());
#ifdef __APPLE__
            ASSERT_EQ(WXK_ALT, shortcut.modifier1());
            ASSERT_EQ(WXK_CONTROL, shortcut.modifier2());
#else
            ASSERT_EQ(WXK_CONTROL, shortcut.modifier1());
            ASSERT_EQ(WXK_ALT, shortcut.modifier2());
#endif
            ASSERT_EQ(WXK_NONE, shortcut.modifier3());
            ASSERT_EQ('D', shortcut.key());
        }

        TEST_CASE("KeyboardShortcutTest.matches", "[KeyboardShortcutTest]") {
            const KeyboardShortcut shortcut('D', WXK_CONTROL, WXK_ALT);

            ASSERT_FALSE(shortcut.matches('S', WXK_CONTROL, WXK_ALT));
            ASSERT_FALSE(shortcut.matches('D', WXK_CONTROL, WXK_SHIFT));
            ASSERT_FALSE(shortcut.matches('D', WXK_SHIFT));
            ASSERT_FALSE(shortcut.matches('D', WXK_NONE));
            ASSERT_TRUE(shortcut.matches('D', WXK_CONTROL, WXK_ALT));
            ASSERT_TRUE(shortcut.matches('D', WXK_ALT, WXK_CONTROL));
            ASSERT_TRUE(shortcut.matches('D', WXK_ALT, WXK_NONE, WXK_CONTROL));
            ASSERT_TRUE(shortcut.matches('D', WXK_NONE, WXK_ALT, WXK_CONTROL));
        }

        TEST_CASE("KeyboardShortcutTest.modifierKeyMenuText", "[KeyboardShortcutTest]") {
            const KeyboardShortcut shortcut('D', WXK_ALT, WXK_CONTROL);
#ifdef __APPLE__
            ASSERT_WXSTR_EQ(QString("Alt+Ctrl"), shortcut.modifierMenuString());
#else
            ASSERT_WXSTR_EQ(QString("Ctrl+Alt"), shortcut.modifierMenuString());
#endif
        }
         */
    }
}
