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

#include "Console.h"

#include <wx/log.h>
#include <wx/panel.h>
#include <wx/simplebook.h>

#include <iostream>

namespace TrenchBroom {
    namespace View {
        Console::Console(wxWindow* parent, wxSimplebook* extraBook) :
        wxTextCtrl(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxTE_RICH2) {
            SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY, *wxBLACK));
            SetBackgroundColour(*wxBLACK);
            extraBook->AddPage(new wxPanel(extraBook), "");
            
            
        }

        void Console::doLog(const LogLevel level, const String& message) {
            if (!message.empty()) {
                logToDebugOut(level, message);
                logToConsole(level, message);
                logNotifier(level, message);
            }
        }

        void Console::logToDebugOut(const LogLevel level, const String& message) {
            char* buffer = new char[message.size() + 1];
            message.copy(buffer, message.size());
            buffer[message.size()] = 0;
            wxLogDebug(buffer);
            delete [] buffer;
        }

        void Console::logToConsole(const LogLevel level, const String& message) {
            const long start = GetLastPosition();
            AppendText(message);
            AppendText("\n");
            const long end = GetLastPosition();
            
            switch (level) {
                case LogLevel_Debug:
                    SetStyle(start, end, wxTextAttr(*wxLIGHT_GREY, *wxBLACK)); 
                    break;
                case LogLevel_Info:
                    SetStyle(start, end, wxTextAttr(*wxWHITE, *wxBLACK));
                    break;
                case LogLevel_Warn:
                    SetStyle(start, end, wxTextAttr(*wxYELLOW, *wxBLACK));
                    break;
                case LogLevel_Error:
                    SetStyle(start, end, wxTextAttr(*wxRED, *wxBLACK));
                    break;
            }
        }
    }
}
