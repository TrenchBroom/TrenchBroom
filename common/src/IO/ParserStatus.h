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

#ifndef TrenchBroom_ParserStatus
#define TrenchBroom_ParserStatus

#include "Logger.h"
#include "StringUtils.h"

namespace TrenchBroom {
    namespace IO {
        class ParserStatus {
        private:
            Logger& m_logger;
            String m_prefix;
        protected:
            explicit ParserStatus(Logger& logger, String prefix);
        public:
            virtual ~ParserStatus();
        public:
            void progress(double progress);

            void debug(size_t line, size_t column, const String& str);
            void info(size_t line, size_t column, const String& str);
            void warn(size_t line, size_t column, const String& str);
            void error(size_t line, size_t column, const String& str);
            void errorAndThrow(size_t line, size_t column, const String& str);

            void debug(size_t line, const String& str);
            void info(size_t line, const String& str);
            void warn(size_t line, const String& str);
            void error(size_t line, const String& str);
            void errorAndThrow(size_t line, const String& str);

            void debug(const String& str);
            void info(const String& str);
            void warn(const String& str);
            void error(const String& str);
            void errorAndThrow(const String& str);
        private:
            void log(Logger::LogLevel level, size_t line, size_t column, const String& str);
            String buildMessage(size_t line, size_t column, const String& str) const;

            void log(Logger::LogLevel level, size_t line, const String& str);
            String buildMessage(size_t line, const String& str) const;

            void log(Logger::LogLevel level, const String& str);
            String buildMessage(const String& str) const;
        private:
            virtual void doProgress(double progress) = 0;
            virtual void doLog(Logger::LogLevel level, const String& str);
        };
    }
}

#endif /* defined(TrenchBroom_ParserStatus) */
