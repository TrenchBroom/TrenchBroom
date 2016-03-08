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

#include "ParserStatus.h"
#include "Exceptions.h"

namespace TrenchBroom {
    namespace IO {
        ParserStatus::ParserStatus(Logger* logger) :
        m_logger(logger) {}

        ParserStatus::~ParserStatus() {}

        void ParserStatus::progress(const double progress) {
            assert(progress >= 0.0 && progress <= 1.0);
            doProgress(progress);
        }

        void ParserStatus::debug(const size_t line, const size_t column, const String& str) {
            log(Logger::LogLevel_Debug, line, column, str);
        }
        
        void ParserStatus::info(const size_t line, const size_t column, const String& str) {
            log(Logger::LogLevel_Debug, line, column, str);
        }
        
        void ParserStatus::warn(const size_t line, const size_t column, const String& str) {
            log(Logger::LogLevel_Debug, line, column, str);
        }
        
        void ParserStatus::error(const size_t line, const size_t column, const String& str) {
            log(Logger::LogLevel_Debug, line, column, str);
        }

        void ParserStatus::errorAndThrow(size_t line, size_t column, const String& str) {
            error(line, column, str);
            throw ParserException(buildMessage(line, column, str));
        }

        void ParserStatus::log(const Logger::LogLevel level, const size_t line, const size_t column, const String& str) {
            if (m_logger != NULL)
                m_logger->log(level, buildMessage(line, column, str));
        }

        String ParserStatus::buildMessage(const size_t line, const size_t column, const String& str) const {
            StringStream msg;
            msg << str << " (line " << line << ", column " << column << ")";
            return msg.str();
        }
    }
}
