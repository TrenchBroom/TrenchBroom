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

#ifndef __TrenchBroom__KeyboardShortcut__
#define __TrenchBroom__KeyboardShortcut__

#include "StringUtils.h"

#include <cassert>
#include <set>

#include <wx/defs.h>
#include <wx/string.h>

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcut {
        private:
            class MacModifierOrder {
            public:
                bool operator()(const int lhs, const int rhs) const;
            };
            
            class WinModifierOrder {
            public:
                bool operator()(const int lhs, const int rhs) const;
            };
            
#if defined __APPLE__
            typedef std::set<int, MacModifierOrder> ModifierSet;
#else
            typedef std::set<int, WinModifierOrder> ModifierSet;
#endif
        public:
            static const KeyboardShortcut Empty;
            typedef enum {
                SCVertexTool    = 1 << 1,
                SCClipTool      = 1 << 2,
                SCRotateTool    = 1 << 3,
                SCObjects       = 1 << 4,
                SCTextures      = 1 << 5,
                SCAny           = SCVertexTool | SCClipTool | SCRotateTool | SCObjects | SCTextures
            } ShortcutContext;
            
            static wxString contextName(const int context);
            static void sortModifierKeys(int& key1, int& key2, int& key3);
            static bool isShortcutValid(const int key, const int modifierKey1 = WXK_NONE, const int modifierKey2 = WXK_NONE, const int modifierKey3 = WXK_NONE);
            static wxString modifierKeyMenuText(const int key);
            static wxString modifierKeyDisplayText(const int key);
            static wxString keyMenuText(const int key);
            static wxString keyDisplayText(const int key);
            static int parseKeyDisplayText(const wxString string);
            static wxString shortcutDisplayText(int modifierKey1, int modifierKey2, int modifierKey3, int key);
            static bool parseShortcut(const wxString& string, int& modifierKey1, int& modifierKey2, int& modifierKey3, int& key);
        private:
            int m_commandId;
            int m_modifierKey1;
            int m_modifierKey2;
            int m_modifierKey3;
            int m_key;
            int m_context;
            String m_text;
        public:
            KeyboardShortcut(const int commandId, const int context, const String& text);
            KeyboardShortcut(const int commandId, const int key, const int context, const String& text);
            KeyboardShortcut(const int commandId, const int modifierKey1, const int key, const int context, const String& text);
            KeyboardShortcut(const int commandId, const int modifierKey1, const int modifierKey2, const int key, const int context, const String& text);
            KeyboardShortcut(const int commandId, const int modifierKey1, const int modifierKey2, const int modifierKey3, const int key, const int context, const String& text);
            KeyboardShortcut(const String& string);
            
            int commandId() const;
            int modifierKey1() const;
            int modifierKey2() const;
            int modifierKey3() const;
            int key() const;
            int context() const;
            const String& text() const;
            bool hasModifier() const;
            
            bool matches(const int key, const int modifierKey1 = WXK_NONE, const int modifierKey2 = WXK_NONE, const int modifierKey3 = WXK_NONE) const;
            bool alwaysShowModifier() const;
            wxString modifierKeyMenuText() const;
            wxString keyMenuText() const;
            wxString shortcutMenuText() const;
            wxString menuText(const String& additionalText = "") const;
            wxString keyDisplayText() const;
            wxString shortcutDisplayText() const;
            String asString() const;
        };
    }
}

#endif /* defined(__TrenchBroom__KeyboardShortcut__) */
