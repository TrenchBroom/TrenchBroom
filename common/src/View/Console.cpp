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

#include "Console.h"

#include "FileLogger.h"
#include "View/ViewConstants.h"

#include <wx/log.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/wupdlock.h>

#include <iostream>

namespace TrenchBroom {
    namespace View {
        Console::Console(wxWindow* parent) :
        TabBookPage(parent),
        m_textView(new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxTE_RICH2)) {
			m_textView->SetFont(Fonts::fixedWidthFont());

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_textView, 1, wxEXPAND);
            SetSizer(sizer);
        }

        void Console::doLog(const LogLevel level, const String& message) {
            doLog(level, wxString(message));
        }

        void Console::doLog(const LogLevel level, const wxString& message) {
            if (!message.empty()) {
                logToDebugOut(level, message);
                logToConsole(level, message);
                FileLogger::instance().log(level, message);
            }
        }

        void Console::logToDebugOut(const LogLevel level, const wxString& message) {
            wxLogDebug(message);
        }

        void Console::logToConsole(const LogLevel level, const wxString& message) {
            wxWindowUpdateLocker locker(m_textView);

            const long start = m_textView->GetLastPosition();
            m_textView->AppendText(message);
            m_textView->AppendText("\n");
#ifndef __APPLE__
			m_textView->ScrollLines(5);
#endif
            const long end = m_textView->GetLastPosition();

            switch (level) {
                case LogLevel_Debug:
                    m_textView->SetStyle(start, end, wxTextAttr(Colors::disabledText(), m_textView->GetBackgroundColour()));
                    break;
                case LogLevel_Info:
                    // m_textView->SetStyle(start, end, wxTextAttr(*wxBLACK, m_textView->GetBackgroundColour()));
                    break;
                case LogLevel_Warn:
                    m_textView->SetStyle(start, end, wxTextAttr(Colors::defaultText(), m_textView->GetBackgroundColour()));
                    break;
                case LogLevel_Error:
                    m_textView->SetStyle(start, end, wxTextAttr(wxColor(250, 30, 60), m_textView->GetBackgroundColour()));
                    break;
            }
        }
    }
}
