
/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
#include <gtest/gtest.h>
#include "TestUtils.h"

namespace TrenchBroom {
    namespace View {
        TEST(KeyboardShortcutTest, contextName) {
            ASSERT_WXSTR_EQ(wxString("Any"), KeyboardShortcut::contextName(KeyboardShortcut::SCAny));
            ASSERT_WXSTR_EQ(wxString("Vertex Tool"), KeyboardShortcut::contextName(KeyboardShortcut::SCVertexTool));
            ASSERT_WXSTR_EQ(wxString("Clip Tool"), KeyboardShortcut::contextName(KeyboardShortcut::SCClipTool));
            ASSERT_WXSTR_EQ(wxString("Rotate Tool"), KeyboardShortcut::contextName(KeyboardShortcut::SCRotateTool));
            ASSERT_WXSTR_EQ(wxString("Objects"), KeyboardShortcut::contextName(KeyboardShortcut::SCObjects));
            ASSERT_WXSTR_EQ(wxString("Textures"), KeyboardShortcut::contextName(KeyboardShortcut::SCTextures));
            ASSERT_WXSTR_EQ(wxString("Objects, Textures"), KeyboardShortcut::contextName(KeyboardShortcut::SCObjects | KeyboardShortcut::SCTextures));
        }
        
        void assertSortModifierKeys(int key1, int key2, int key3, const int exp1, const int exp2, const int exp3) {
            KeyboardShortcut::sortModifierKeys(key1, key2, key3);
            ASSERT_EQ(exp1, key1);
            ASSERT_EQ(exp2, key2);
            ASSERT_EQ(exp3, key3);
        }
        
        TEST(KeyboardShortcutTest, sortModifierKeys) {
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
        
#ifdef __linux__
        TEST(KeyboardShortcutTest, isShortcutValid) {
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
        
        TEST(KeyboardShortcutTest, staticModifierKeyMenuText) {
            ASSERT_WXSTR_EQ(wxString("Ctrl"), KeyboardShortcut::modifierKeyMenuText(WXK_CONTROL));
            ASSERT_WXSTR_EQ(wxString("Alt"), KeyboardShortcut::modifierKeyMenuText(WXK_ALT));
            ASSERT_WXSTR_EQ(wxString("Shift"), KeyboardShortcut::modifierKeyMenuText(WXK_SHIFT));
            ASSERT_WXSTR_EQ(wxString(""), KeyboardShortcut::modifierKeyMenuText(WXK_TAB));
        }
        
        TEST(KeyboardShortcutTest, modifierKeyDisplayText) {
#ifdef __APPLE__
            ASSERT_WXSTR_EQ(wxString(L"\u2318"), KeyboardShortcut::modifierKeyDisplayText(WXK_CONTROL));
            ASSERT_WXSTR_EQ(wxString(L"\u2325"), KeyboardShortcut::modifierKeyDisplayText(WXK_ALT));
            ASSERT_WXSTR_EQ(wxString(L"\u21E7"), KeyboardShortcut::modifierKeyDisplayText(WXK_SHIFT));
            ASSERT_WXSTR_EQ(wxString(""), KeyboardShortcut::modifierKeyDisplayText(WXK_TAB));
#else
            ASSERT_WXSTR_EQ(wxString("Ctrl"), KeyboardShortcut::modifierKeyDisplayText(WXK_CONTROL));
            ASSERT_WXSTR_EQ(wxString("Alt"), KeyboardShortcut::modifierKeyDisplayText(WXK_ALT));
            ASSERT_WXSTR_EQ(wxString("Shift"), KeyboardShortcut::modifierKeyDisplayText(WXK_SHIFT));
            ASSERT_WXSTR_EQ(wxString(""), KeyboardShortcut::modifierKeyDisplayText(WXK_TAB));
#endif
        }
        
        TEST(KeyboardShortcutTest, staticShortcutDisplayText) {
#ifdef __APPLE__
            ASSERT_WXSTR_EQ(wxString("C"), KeyboardShortcut::shortcutDisplayText(WXK_NONE, WXK_NONE, WXK_NONE, 'C'));
            ASSERT_WXSTR_EQ(wxString(L"\u238B"), KeyboardShortcut::shortcutDisplayText(WXK_NONE, WXK_NONE, WXK_NONE, WXK_ESCAPE));
            ASSERT_WXSTR_EQ(wxString("F11"), KeyboardShortcut::shortcutDisplayText(WXK_NONE, WXK_NONE, WXK_NONE, WXK_F11));
            ASSERT_WXSTR_EQ(wxString(L"\u2318D"), KeyboardShortcut::shortcutDisplayText(WXK_CONTROL, WXK_NONE, WXK_NONE, 'D'));
            ASSERT_WXSTR_EQ(wxString(L"\u2318D"), KeyboardShortcut::shortcutDisplayText(WXK_NONE, WXK_NONE, WXK_CONTROL, 'D'));
            ASSERT_WXSTR_EQ(wxString(L"\u2325\u2318S"), KeyboardShortcut::shortcutDisplayText(WXK_CONTROL, WXK_NONE, WXK_ALT, 'S'));
#else
            ASSERT_WXSTR_EQ(wxString("C"), KeyboardShortcut::shortcutDisplayText(WXK_NONE, WXK_NONE, WXK_NONE, 'C'));
            ASSERT_WXSTR_EQ(wxString("Esc"), KeyboardShortcut::shortcutDisplayText(WXK_NONE, WXK_NONE, WXK_NONE, WXK_ESCAPE));
            ASSERT_WXSTR_EQ(wxString("F11"), KeyboardShortcut::shortcutDisplayText(WXK_NONE, WXK_NONE, WXK_NONE, WXK_F11));
            ASSERT_WXSTR_EQ(wxString("Ctrl+D"), KeyboardShortcut::shortcutDisplayText(WXK_CONTROL, WXK_NONE, WXK_NONE, 'D'));
            ASSERT_WXSTR_EQ(wxString("Ctrl+D"), KeyboardShortcut::shortcutDisplayText(WXK_NONE, WXK_NONE, WXK_CONTROL, 'D'));
            ASSERT_WXSTR_EQ(wxString("Ctrl+Alt+S"), KeyboardShortcut::shortcutDisplayText(WXK_ALT, WXK_CONTROL, WXK_NONE, 'S'));
#endif
        }
        
        TEST(KeyboardShortcutTest, parseShortcut) {
            int m1, m2, m3, k;
            
            ASSERT_TRUE(KeyboardShortcut::parseShortcut("", m1, m2, m3, k));
            ASSERT_EQ(WXK_NONE, m1);
            ASSERT_EQ(WXK_NONE, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ(WXK_NONE, k);
            
            ASSERT_FALSE(KeyboardShortcut::parseShortcut("asdf", m1, m2, m3, k));
            ASSERT_FALSE(KeyboardShortcut::parseShortcut(" D", m1, m2, m3, k));
            ASSERT_FALSE(KeyboardShortcut::parseShortcut("D ", m1, m2, m3, k));

            ASSERT_TRUE(KeyboardShortcut::parseShortcut("D", m1, m2, m3, k));
            ASSERT_EQ(WXK_NONE, m1);
            ASSERT_EQ(WXK_NONE, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ('D', k);
            
#ifdef __APPLE__
            ASSERT_FALSE(KeyboardShortcut::parseShortcut("\u2318+D", m1, m2, m3, k));

            ASSERT_TRUE(KeyboardShortcut::parseShortcut("\u2318", m1, m2, m3, k));
            ASSERT_EQ(WXK_CONTROL, m1);
            ASSERT_EQ(WXK_NONE, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ(WXK_NONE, k);

            ASSERT_TRUE(KeyboardShortcut::parseShortcut("\u2318D", m1, m2, m3, k));
            ASSERT_EQ(WXK_CONTROL, m1);
            ASSERT_EQ(WXK_NONE, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ('D', k);

            ASSERT_TRUE(KeyboardShortcut::parseShortcut("\u2318\u2325D", m1, m2, m3, k));
            ASSERT_EQ(WXK_ALT, m1);
            ASSERT_EQ(WXK_CONTROL, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ('D', k);
            
            ASSERT_TRUE(KeyboardShortcut::parseShortcut("\u2318\u2325\u21E7\u21E5", m1, m2, m3, k));
            ASSERT_EQ(WXK_ALT, m1);
            ASSERT_EQ(WXK_SHIFT, m2);
            ASSERT_EQ(WXK_CONTROL, m3);
            ASSERT_EQ(WXK_TAB, k);
#else
            ASSERT_FALSE(KeyboardShortcut::parseShortcut("Ctrl D", m1, m2, m3, k));
            
            ASSERT_TRUE(KeyboardShortcut::parseShortcut("Ctrl", m1, m2, m3, k));
            ASSERT_EQ(WXK_CONTROL, m1);
            ASSERT_EQ(WXK_NONE, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ(WXK_NONE, k);
            
            ASSERT_TRUE(KeyboardShortcut::parseShortcut("Ctrl+D", m1, m2, m3, k));
            ASSERT_EQ(WXK_CONTROL, m1);
            ASSERT_EQ(WXK_NONE, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ('D', k);
            
            ASSERT_TRUE(KeyboardShortcut::parseShortcut("Alt+Ctrl+D", m1, m2, m3, k));
            ASSERT_EQ(WXK_CONTROL, m1);
            ASSERT_EQ(WXK_ALT, m2);
            ASSERT_EQ(WXK_NONE, m3);
            ASSERT_EQ('D', k);
            
            ASSERT_TRUE(KeyboardShortcut::parseShortcut("Alt+Ctrl+Shift+Tab", m1, m2, m3, k));
            ASSERT_EQ(WXK_CONTROL, m1);
            ASSERT_EQ(WXK_ALT, m2);
            ASSERT_EQ(WXK_SHIFT, m3);
            ASSERT_EQ(WXK_TAB, k);
#endif
        }
        
        TEST(KeyboardShortcutTest, constructWithString) {
            StringStream test;
            test << "7:" << WXK_CONTROL << ":" << WXK_ALT << ":" << WXK_NONE << ":" << static_cast<int>('D') << ":" << KeyboardShortcut::SCObjects << ":Duplicate";
            
            const KeyboardShortcut shortcut(test.str());
            ASSERT_EQ(7, shortcut.commandId());
#ifdef __APPLE__
            ASSERT_EQ(WXK_ALT, shortcut.modifierKey1());
            ASSERT_EQ(WXK_CONTROL, shortcut.modifierKey2());
#else
            ASSERT_EQ(WXK_CONTROL, shortcut.modifierKey1());
            ASSERT_EQ(WXK_ALT, shortcut.modifierKey2());
#endif
            ASSERT_EQ(WXK_NONE, shortcut.modifierKey3());
            ASSERT_EQ('D', shortcut.key());
            ASSERT_EQ(KeyboardShortcut::SCObjects, shortcut.context());
            ASSERT_EQ(String("Duplicate"), shortcut.text());
        }
        
        TEST(KeyboardShortcutTest, matches) {
            const KeyboardShortcut shortcut(0, WXK_CONTROL, WXK_ALT, 'D', KeyboardShortcut::SCObjects, "Test");
            
            ASSERT_FALSE(shortcut.matches('S', WXK_CONTROL, WXK_ALT));
            ASSERT_FALSE(shortcut.matches('D', WXK_CONTROL, WXK_SHIFT));
            ASSERT_FALSE(shortcut.matches('D', WXK_SHIFT));
            ASSERT_FALSE(shortcut.matches('D', WXK_NONE));
            ASSERT_TRUE(shortcut.matches('D', WXK_CONTROL, WXK_ALT));
            ASSERT_TRUE(shortcut.matches('D', WXK_ALT, WXK_CONTROL));
            ASSERT_TRUE(shortcut.matches('D', WXK_ALT, WXK_NONE, WXK_CONTROL));
            ASSERT_TRUE(shortcut.matches('D', WXK_NONE, WXK_ALT, WXK_CONTROL));
        }
        
        TEST(KeyboardShortcutTest, modifierKeyMenuText) {
            const KeyboardShortcut shortcut(0, WXK_ALT, WXK_CONTROL, 'D', KeyboardShortcut::SCObjects, "Test");
#ifdef __APPLE__
            ASSERT_WXSTR_EQ(wxString("Alt+Ctrl"), shortcut.modifierKeyMenuText());
#else
            ASSERT_WXSTR_EQ(wxString("Ctrl+Alt"), shortcut.modifierKeyMenuText());
#endif
        }
    }
}
