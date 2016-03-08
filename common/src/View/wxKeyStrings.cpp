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

#include "wxKeyStrings.h"

namespace TrenchBroom {
    namespace View {
        wxKeyStrings::wxKeyStrings() :
        m_strings(WXK_WINDOWS_MENU + 10),
        m_separator("+") {
            m_strings[WXK_BACK]             = "Backspace";
            m_strings[WXK_TAB]              = "Tab";
            m_strings[WXK_RETURN]           = "Return";
            m_strings[WXK_ESCAPE]           = "Escape";
            m_strings[WXK_SPACE]            = "Space";
            m_strings[WXK_DELETE]           = "Delete";
            m_strings[WXK_CANCEL]           = "Cancel";
            m_strings[WXK_CLEAR]            = "Clear";
            m_strings[WXK_SHIFT]            = "Shift";
            m_strings[WXK_ALT]              = "Alt";
            m_strings[WXK_CONTROL]          = "Ctrl";
            m_strings[WXK_MENU]             = "Menu";
            m_strings[WXK_PAUSE]            = "Pause";
            m_strings[WXK_CAPITAL]          = "Capital";
            m_strings[WXK_END]              = "End";
            m_strings[WXK_HOME]             = "Home";
            m_strings[WXK_LEFT]             = "&#x2190;";
            m_strings[WXK_UP]               = "&#x2191;";
            m_strings[WXK_RIGHT]            = "&#x2192;";
            m_strings[WXK_DOWN]             = "&#x2193;";
            m_strings[WXK_SELECT]           = "Select";
            m_strings[WXK_PRINT]            = "Print";
            m_strings[WXK_EXECUTE]          = "Execute";
            m_strings[WXK_SNAPSHOT]         = "Snapshot";
            m_strings[WXK_INSERT]           = "Insert";
            m_strings[WXK_HELP]             = "Help";
            m_strings[WXK_NUMPAD0]          = "Numpad 0";
            m_strings[WXK_NUMPAD1]          = "Numpad 1";
            m_strings[WXK_NUMPAD2]          = "Numpad 2";
            m_strings[WXK_NUMPAD3]          = "Numpad 3";
            m_strings[WXK_NUMPAD4]          = "Numpad 4";
            m_strings[WXK_NUMPAD5]          = "Numpad 5";
            m_strings[WXK_NUMPAD6]          = "Numpad 6";
            m_strings[WXK_NUMPAD7]          = "Numpad 7";
            m_strings[WXK_NUMPAD8]          = "Numpad 8";
            m_strings[WXK_NUMPAD9]          = "Numpad 9";
            m_strings[WXK_MULTIPLY]         = "*";
            m_strings[WXK_ADD]              = "+";
            m_strings[WXK_SUBTRACT]         = "-";
            m_strings[WXK_DECIMAL]          = ".";
            m_strings[WXK_DIVIDE]           = "/";
            m_strings[WXK_F1]               = "F1";
            m_strings[WXK_F2]               = "F2";
            m_strings[WXK_F3]               = "F3";
            m_strings[WXK_F4]               = "F4";
            m_strings[WXK_F5]               = "F5";
            m_strings[WXK_F6]               = "F6";
            m_strings[WXK_F7]               = "F7";
            m_strings[WXK_F8]               = "F8";
            m_strings[WXK_F9]               = "F9";
            m_strings[WXK_F10]              = "F10";
            m_strings[WXK_F11]              = "F11";
            m_strings[WXK_F12]              = "F12";
            m_strings[WXK_F13]              = "F13";
            m_strings[WXK_F14]              = "F14";
            m_strings[WXK_F15]              = "F15";
            m_strings[WXK_F16]              = "F16";
            m_strings[WXK_F17]              = "F17";
            m_strings[WXK_F18]              = "F18";
            m_strings[WXK_F19]              = "F19";
            m_strings[WXK_F20]              = "F20";
            m_strings[WXK_F21]              = "F21";
            m_strings[WXK_F22]              = "F22";
            m_strings[WXK_F23]              = "F23";
            m_strings[WXK_F24]              = "F24";
            m_strings[WXK_NUMLOCK]          = "Numlock";
            m_strings[WXK_SCROLL]           = "Scroll";
            m_strings[WXK_PAGEUP]           = "Page Up";
            m_strings[WXK_PAGEDOWN]         = "Page Down";
            m_strings[WXK_NUMPAD_SPACE]     = "Numpad Space";
            m_strings[WXK_NUMPAD_TAB]       = "Numpad Tab";
            m_strings[WXK_NUMPAD_ENTER]     = "Numpad Enter";
            m_strings[WXK_NUMPAD_F1]        = "Numpad F1";
            m_strings[WXK_NUMPAD_F2]        = "Numpad F2";
            m_strings[WXK_NUMPAD_F3]        = "Numpad F3";
            m_strings[WXK_NUMPAD_F4]        = "Numpad F4";
            m_strings[WXK_NUMPAD_HOME]      = "Numpad Home";
            m_strings[WXK_NUMPAD_LEFT]      = "Numpad &#x2190;";
            m_strings[WXK_NUMPAD_UP]        = "Numpad &#x2191;";
            m_strings[WXK_NUMPAD_RIGHT]     = "Numpad &#x2192;";
            m_strings[WXK_NUMPAD_DOWN]      = "Numpad &#x2193;";
            m_strings[WXK_NUMPAD_PAGEUP]    = "Numpad Page Up";
            m_strings[WXK_NUMPAD_PAGEDOWN]  = "Numpad Page Down";
            m_strings[WXK_NUMPAD_END]       = "Numpad End";
            m_strings[WXK_NUMPAD_BEGIN]     = "Numpad Home";
            m_strings[WXK_NUMPAD_INSERT]    = "Numpad Insert";
            m_strings[WXK_NUMPAD_DELETE]    = "Numpad Delete";
            m_strings[WXK_NUMPAD_EQUAL]     = "Numpad =";
            m_strings[WXK_NUMPAD_MULTIPLY]  = "Numpad *";
            m_strings[WXK_NUMPAD_ADD]       = "Numpad +";
            m_strings[WXK_NUMPAD_SEPARATOR] = "Numpad Separator";
            m_strings[WXK_NUMPAD_SUBTRACT]  = "Numpad -";
            m_strings[WXK_NUMPAD_DECIMAL]   = "Numpad .";
            m_strings[WXK_NUMPAD_DIVIDE]    = "Numpad /";
            m_strings[WXK_WINDOWS_LEFT]     = "Left Windows Key";
            m_strings[WXK_WINDOWS_RIGHT]    = "Right Windows Key";
            m_strings[WXK_WINDOWS_MENU]     = "Windows Menu Key";
            m_strings[WXK_RAW_CONTROL]      = "Ctrl";
            m_strings[WXK_COMMAND]          = "Cmd";
            
            for (char c = '!'; c <= '~'; ++c)
                m_strings[static_cast<wxKeyCode>(c)] = c;
            
            m_strings[static_cast<wxKeyCode>('"')] = "&quot;";
            m_strings[static_cast<wxKeyCode>('<')] = "&gt;";
            m_strings[static_cast<wxKeyCode>('>')] = "&lt;";
        }
        
        String wxKeyStrings::operator[](const size_t index) const {
            assert(index < m_strings.size());
            return m_strings[index];
        }

        void wxKeyStrings::appendJS(const String& platform, StringStream& result) const {
            for (size_t i = 0; i < m_strings.size(); ++i) {
                if (!m_strings[i].empty()) {
                    result << "keys[\"" << platform << "\"][" << i << "] = \"" << (m_strings[i] == "\\" ? "\\\\" : m_strings[i]) << "\";" << std::endl;
                }
            }
            
            result << "keys[\"" << platform << "\"][\"+\"] = \"" << m_separator << "\";" << std::endl;
        }

        wxKeyStringsMac::wxKeyStringsMac() {
            m_separator = "";
            m_strings[WXK_BACK]             = "&#x232B;";
            m_strings[WXK_TAB]              = "&#x21E5;";
            m_strings[WXK_RETURN]           = "&#x21A9;";
            m_strings[WXK_ESCAPE]           = "&#x238B;";
            // m_strings[WXK_SPACE]            = "&#x2423;"; The space string illegible.
            m_strings[WXK_DELETE]           = "&#x232B;";
            m_strings[WXK_SHIFT]            = "&#x21E7;";
            m_strings[WXK_ALT]              = "&#x2325;";
            m_strings[WXK_CONTROL]          = "&#x2303;";
            m_strings[WXK_END]              = "&#x21F2;";
            m_strings[WXK_HOME]             = "&#x21F1;";
            m_strings[WXK_INSERT]           = "Insert";
            m_strings[WXK_HELP]             = "Help";
            m_strings[WXK_PAGEUP]           = "&#x21DE;";
            m_strings[WXK_PAGEDOWN]         = "&#x21DF;";
            m_strings[WXK_NUMPAD_SPACE]     = "Numpad &#x2423;";
            m_strings[WXK_NUMPAD_TAB]       = "Numpad &#x21E5;";
            m_strings[WXK_NUMPAD_ENTER]     = "Numpad &#x21A9;";
            m_strings[WXK_NUMPAD_HOME]      = "Numpad &#x21F1;";
            m_strings[WXK_NUMPAD_LEFT]      = "Numpad &#x2190;";
            m_strings[WXK_NUMPAD_UP]        = "Numpad &#x2191;";
            m_strings[WXK_NUMPAD_RIGHT]     = "Numpad &#x2192;";
            m_strings[WXK_NUMPAD_DOWN]      = "Numpad &#x2193;";
            m_strings[WXK_NUMPAD_PAGEUP]    = "Numpad &#x21DE;";
            m_strings[WXK_NUMPAD_PAGEDOWN]  = "Numpad &#x21DF;";
            m_strings[WXK_NUMPAD_END]       = "Numpad &#x21F2;";
            m_strings[WXK_NUMPAD_BEGIN]     = "Numpad &#x21F1;";
            m_strings[WXK_NUMPAD_INSERT]    = "Numpad Insert";
            m_strings[WXK_NUMPAD_DELETE]    = "Numpad &#x232B;";
            m_strings[WXK_RAW_CONTROL]      = "&#x2303;";
            m_strings[WXK_COMMAND]          = "&#x2318;";
        }
    }
}
