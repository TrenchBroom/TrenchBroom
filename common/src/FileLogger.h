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

#ifndef FileLogger_h
#define FileLogger_h

#include "Macros.h"
#include "Logger.h"

#include <string>

class QString;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    class FileLogger : public Logger {
    private:
        FILE* m_file;
    public:
        explicit FileLogger(const IO::Path& filePath);
        ~FileLogger() override;

        static FileLogger& instance();
    private:
        void doLog(LogLevel level, const std::string& message) override;
        void doLog(LogLevel level, const QString& message) override;

        deleteCopyAndMove(FileLogger)
    };
}

#endif /* FileLogger_h */
