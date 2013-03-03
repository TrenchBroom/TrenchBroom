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

#include "IO/FileManager.h"

#if defined __APPLE__
#include "NSLog.h"
#endif

#include <cstdarg>
#include <fstream>
#include <wx/datetime.h>
#include <wx/wx.h>

namespace TrenchBroom {
    namespace Utility {
        void Console::logToDebug(const LogMessage& message) {
            // wxLogDebug(message.string().c_str());
        }
        
        void Console::logToConsole(const LogMessage& message) {
            long start = m_textCtrl->GetLastPosition();
            m_textCtrl->AppendText(message.string());
            m_textCtrl->AppendText("\n");
            long end = m_textCtrl->GetLastPosition();
            switch (message.level()) {
                case LLDebug:
                    m_textCtrl->SetStyle(start, end, wxTextAttr(*wxLIGHT_GREY, *wxBLACK)); // SetDefaultStyle doesn't work on OS X / Cocoa
                    break;
                case LLInfo:
                    m_textCtrl->SetStyle(start, end, wxTextAttr(*wxWHITE, *wxBLACK)); // SetDefaultStyle doesn't work on OS X / Cocoa
                    break;
                case LLWarn:
                    m_textCtrl->SetStyle(start, end, wxTextAttr(*wxYELLOW, *wxBLACK)); // SetDefaultStyle doesn't work on OS X / Cocoa
                    break;
                case LLError:
                    m_textCtrl->SetStyle(start, end, wxTextAttr(*wxRED, *wxBLACK)); // SetDefaultStyle doesn't work on OS X / Cocoa
                    break;
            }
        }

        void Console::logToFile(const LogMessage& message) {
#if defined _WIN32
            IO::FileManager fileManager;
            const String logDirectory = fileManager.logDirectory();
            const String logFilePath = fileManager.appendPath(logDirectory, "TrenchBroom.log");
            std::fstream logStream(logFilePath, std::ios::out | std::ios::app);
            if (logStream.is_open()) {
                wxDateTime now = wxDateTime::Now();
                logStream << wxGetProcessId() << " " << now.FormatISOCombined(' ') << ": " << message.string() << std::endl;
            }
#elif defined __APPLE__
            NSLogWrapper(message.string());
#elif defined __linux__
#endif
        }

        void Console::setTextCtrl(wxTextCtrl* textCtrl) {
            m_textCtrl = textCtrl;
            if (m_textCtrl != NULL) {
                for (unsigned int i = 0; i < m_buffer.size(); i++) {
                    const LogMessage& message = m_buffer[i];
                    logToConsole(message);
                }
                m_buffer.clear();
            }
        }
        
        void Console::log(const LogMessage& message) {
            if (message.string().empty())
                return;

            logToDebug(message);
            logToFile(message);
            if (m_textCtrl != NULL)
                logToConsole(message);
            else
                m_buffer.push_back(message);
        }
        
        void Console::debug(const String& message) {
            log(LogMessage(LLDebug, message));
        }
        
        void Console::debug(const char* format, ...) {
            String message;
            va_list(arguments);
            va_start(arguments, format);
            formatString(format, arguments, message);
            va_end(arguments);
            debug(message);
        }

        void Console::info(const String& message) {
            log(LogMessage(LLInfo, message));
        }
        
        void Console::info(const char* format, ...) {
            String message;
            va_list(arguments);
            va_start(arguments, format);
            formatString(format, arguments, message);
            va_end(arguments);
            info(message);
        }

        void Console::warn(const String& message) {
            log(LogMessage(LLWarn, message));
        }
        
        void Console::warn(const char* format, ...) {
            String message;
            va_list(arguments);
            va_start(arguments, format);
            formatString(format, arguments, message);
            va_end(arguments);
            warn(message);
        }

        void Console::error(const String& message) {
            log(LogMessage(LLError, message));
        }

        void Console::error(const char* format, ...) {
            String message;
            va_list(arguments);
            va_start(arguments, format);
            formatString(format, arguments, message);
            va_end(arguments);
            error(message);
        }
    }
}
