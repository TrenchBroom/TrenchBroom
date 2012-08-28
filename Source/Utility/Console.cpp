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

#include "Console.h"

#include <wx/wx.h>

namespace TrenchBroom {
    namespace Utility {
        void Console::formatMessage(const char* format, va_list arguments, String& result) {
            static char buffer[4096];
            
#if defined _MSC_VER
            vsprintf_s(buffer, format, arguments);
#else
            vsprintf(buffer, format, arguments);
#endif

            result = buffer;
        }
        
        Console::Console(wxTextCtrl* textCtrl) : m_textCtrl(textCtrl) {}
        
        Console::~Console() {}
        
        void Console::log(const String& message, bool setDefaultColor) {
            if (setDefaultColor) {
                long start = m_textCtrl->GetLastPosition();
                m_textCtrl->AppendText(message);
                long end = m_textCtrl->GetLastPosition();
                m_textCtrl->SetStyle(start, end, wxTextAttr(*wxLIGHT_GREY, *wxBLACK)); // SetDefaultStyle doesn't work on OS X / Cocoa
            } else {
                m_textCtrl->AppendText(message);
            }
            wxYieldIfNeeded();
        }
        
        void Console::info(const String& message) {
            log(message + "\n");
        }
        
        void Console::info(const char* format, ...) {
            String message;
            va_list(arguments);
            va_start(arguments, format);
            formatMessage(format, arguments, message);
            va_end(arguments);
            info(message);
        }

        void Console::warn(const String& message) {
            long start = m_textCtrl->GetLastPosition();
            log(message + "\n", false);
            long end = m_textCtrl->GetLastPosition();
            m_textCtrl->SetStyle(start, end, wxTextAttr(*wxYELLOW, *wxBLACK)); // SetDefaultStyle doesn't work on OS X / Cocoa
        }
        
        void Console::warn(const char* format, ...) {
            String message;
            va_list(arguments);
            va_start(arguments, format);
            formatMessage(format, arguments, message);
            va_end(arguments);
            warn(message);
        }

        void Console::error(const String& message) {
            long start = m_textCtrl->GetLastPosition();
            log(message + "\n", false);
            long end = m_textCtrl->GetLastPosition();
            m_textCtrl->SetStyle(start, end, wxTextAttr(*wxRED, *wxBLACK)); // SetDefaultStyle doesn't work on OS X / Cocoa
        }

        void Console::error(const char* format, ...) {
            String message;
            va_list(arguments);
            va_start(arguments, format);
            formatMessage(format, arguments, message);
            va_end(arguments);
            error(message);
        }
    }
}