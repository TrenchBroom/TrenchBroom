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
#include <set>

#include <wx/defs.h>
#include <wx/string.h>

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcut {
        private:
            class MacModifierOrder {
            public:
                inline bool operator()(int lhs, int rhs) const {
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
            };

            class WinModifierOrder {
            public:
                inline bool operator()(int lhs, int rhs) const {
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
            };
            
#if defined __APPLE__
            typedef std::set<int, MacModifierOrder> ModifierSet;
#else
            typedef std::set<int, WinModifierOrder> ModifierSet;
#endif
        public:
            typedef enum {
                SCVertexTool    = 1 << 1,
                SCClipTool      = 1 << 2,
                SCRotateTool    = 1 << 3,
                SCObjects       = 1 << 4,
                SCTextures      = 1 << 5,
                SCAny           = SCVertexTool | SCClipTool | SCRotateTool | SCObjects | SCTextures
            } ShortcutContext;

            static wxString contextName(int context);
            static void sortModifierKeys(int& key1, int& key2, int& key3);
            static wxString modifierKeyMenuText(int key);
            static wxString modifierKeyDisplayText(int key);
            static wxString keyMenuText(int key);
            static wxString keyDisplayText(int key);
            static wxString shortcutDisplayText(int modifierKey1, int modifierKey2, int modifierKey3, int key);
        private:
            int m_commandId;
            int m_modifierKey1;
            int m_modifierKey2;
            int m_modifierKey3;
            int m_key;
            int m_context;
            String m_text;
        public:
            KeyboardShortcut(int commandId, int context, const String& text);
            KeyboardShortcut(int commandId, int key, int context, const String& text);
            KeyboardShortcut(int commandId, int modifierKey1, int key, int context, const String& text);
            KeyboardShortcut(int commandId, int modifierKey1, int modifierKey2, int key, int context, const String& text);
            KeyboardShortcut(int commandId, int modifierKey1, int modifierKey2, int modifierKey3, int key, int context, const String& text);
            KeyboardShortcut(const String& string);

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

            inline bool hasModifier() const {
                return m_modifierKey1 != WXK_NONE || m_modifierKey2 != WXK_NONE || m_modifierKey3 != WXK_NONE;
            }
            
            bool alwaysShowModifier() const;
            wxString modifierKeyMenuText() const;
            wxString keyMenuText() const;
            wxString shortcutMenuText() const;
            wxString menuText() const;
            wxString keyDisplayText() const;
            wxString shortcutDisplayText() const;
            String asString() const;
        };
    }
}

#endif /* defined(__TrenchBroom__KeyboardShortcut__) */
