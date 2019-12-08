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

#include "ParserStatus.h"
#include "Exceptions.h"

#include <cassert>
#include <sstream>

namespace TrenchBroom {
    namespace IO {
        ParserStatus::ParserStatus(Logger& logger, String prefix) :
        m_logger(logger),
        m_prefix(std::move(prefix)) {}

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

        void ParserStatus::errorAndThrow(const size_t line, const size_t column, const String& str) {
            error(line, column, str);
            throw ParserException(buildMessage(line, column, str));
        }

        void ParserStatus::debug(const size_t line, const String& str) {
            log(Logger::LogLevel_Debug, line, str);
        }

        void ParserStatus::info(const size_t line, const String& str) {
            log(Logger::LogLevel_Info, line, str);
        }

        void ParserStatus::warn(const size_t line, const String& str) {
            log(Logger::LogLevel_Warn, line, str);
        }

        void ParserStatus::error(const size_t line, const String& str) {
            log(Logger::LogLevel_Error, line, str);
        }

        void ParserStatus::errorAndThrow(size_t line, const String& str) {
            error(line, str);
            throw ParserException(buildMessage(line, str));
        }

        void ParserStatus::debug(const String& str) {
            log(Logger::LogLevel_Debug, str);
        }

        void ParserStatus::info(const String& str) {
            log(Logger::LogLevel_Info, str);
        }

        void ParserStatus::warn(const String& str) {
            log(Logger::LogLevel_Warn, str);
        }

        void ParserStatus::error(const String& str) {
            log(Logger::LogLevel_Error, str);
        }

        void ParserStatus::errorAndThrow(const String& str) {
            error(str);
            throw ParserException(buildMessage(str));
        }

        void ParserStatus::log(const Logger::LogLevel level, const size_t line, const size_t column, const String& str) {
            doLog(level, buildMessage(line, column, str));
        }

        String ParserStatus::buildMessage(const size_t line, const size_t column, const String& str) const {
            std::stringstream msg;
            if (!m_prefix.empty()) {
                msg << m_prefix << ": ";
            }
            msg << str << " (line " << line << ", column " << column << ")";
            return msg.str();
        }

        void ParserStatus::log(const Logger::LogLevel level, const size_t line, const String& str) {
            doLog(level, buildMessage(line, str));
        }

        String ParserStatus::buildMessage(const size_t line, const String& str) const {
            std::stringstream msg;
            if (!m_prefix.empty()) {
                msg << m_prefix << ": ";
            }
            msg << str << " (line " << line << ")";
            return msg.str();
        }

        void ParserStatus::log(Logger::LogLevel level, const String& str) {
            doLog(level, buildMessage(str));
        }

        String ParserStatus::buildMessage(const String& str) const {
            std::stringstream msg;
            if (!m_prefix.empty()) {
                msg << m_prefix << ": ";
            }
            msg << str << " (unknown position)";
            return msg.str();
        }

        void ParserStatus::doLog(const Logger::LogLevel level, const String& str) {
            m_logger.log(level, str);
        }
    }
}
