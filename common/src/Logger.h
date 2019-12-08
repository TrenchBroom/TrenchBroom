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

#ifndef TrenchBroom_Logger
#define TrenchBroom_Logger

#include "StringType.h"

#include <sstream>

class QString;

namespace TrenchBroom {
    class Logger {
    public:
        typedef enum {
            LogLevel_Debug,
            LogLevel_Info,
            LogLevel_Warn,
            LogLevel_Error
        } LogLevel;

        class stream {
        private:
            Logger* m_logger;
            LogLevel m_logLevel;
            std::stringstream m_buf;
        public:
            stream(Logger* logger, LogLevel logLevel);
            ~stream();
        public:
            template <typename T>
            stream& operator<<(T&& arg) {
                m_buf << std::forward<T>(arg);
                return *this;
            }
        };
    public:
        virtual ~Logger();

        stream debug();
        void debug(const char* message);
        void debug(const String& message);
        void debug(const QString& message);

        stream info();
        void info(const char* message);
        void info(const String& message);
        void info(const QString& message);

        stream warn();
        void warn(const char* message);
        void warn(const String& message);
        void warn(const QString& message);

        stream error();
        void error(const char* message);
        void error(const String& message);
        void error(const QString& message);

        void log(LogLevel level, const String& message);
        void log(LogLevel level, const QString& message);
    private:
        virtual void doLog(LogLevel level, const String& message) = 0;
        virtual void doLog(LogLevel level, const QString& message) = 0;
    };



    class NullLogger : public Logger {
    private:
        void doLog(LogLevel level, const String& message) override;
        void doLog(LogLevel level, const QString& message) override;
    };
}

#endif /* defined(TrenchBroom_Logger) */
