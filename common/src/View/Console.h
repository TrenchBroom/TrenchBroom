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

#ifndef TrenchBroom_Console
#define TrenchBroom_Console

#include "Logger.h"
#include "View/TabBook.h"

#include <string>

class QTextEdit;
class QString;
class QWidget;

namespace TrenchBroom {
    namespace View {
        class Console : public TabBookPage, public Logger {
        private:
            QTextEdit* m_textView;
        public:
            explicit Console(QWidget* parent = nullptr);
        private:
            void doLog(LogLevel level, const std::string& message) override;
            void doLog(LogLevel level, const QString& message) override;
            void logToDebugOut(LogLevel level, const QString& message);
            void logToConsole(LogLevel level, const QString& message);
        };
    }
}

#endif /* defined(TrenchBroom_Console) */
