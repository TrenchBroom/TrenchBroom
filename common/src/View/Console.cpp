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

#include <QDebug>
#include <QVBoxLayout>
#include <QTextEdit>

#include <iostream>

namespace TrenchBroom {
    namespace View {
        Console::Console(QWidget* parent) :
        TabBookPage(parent) {
            m_textView = new QTextEdit();
            m_textView->setReadOnly(true);
            m_textView->setWordWrapMode(QTextOption::NoWrap);
            m_textView->setReadOnly(true);

            QFont font;
            font.setFixedPitch(true);
			m_textView->setFont(font);

            QVBoxLayout* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(m_textView);
            setLayout(sizer);
        }

        void Console::doLog(const LogLevel level, const String& message) {
            doLog(level, QString::fromStdString(message));
        }

        void Console::doLog(const LogLevel level, const QString& message) {
            if (!message.isEmpty()) {
                logToDebugOut(level, message);
                logToConsole(level, message);
                FileLogger::instance().log(level, message);
            }
        }

        void Console::logToDebugOut(const LogLevel level, const QString& message) {
            qDebug("%s", message.toStdString().c_str());
        }

        void Console::logToConsole(const LogLevel level, const QString& message) {
            // FIXME: still needed?
            //wxWindowUpdateLocker locker(m_textView);

            m_textView->append(message);
            // FIXME: color
#if 0
            const long start = m_textView->GetLastPosition();
            m_textView->AppendText(message);
            m_textView->AppendText("\n");

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
#endif
        }
    }
}
