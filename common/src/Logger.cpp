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

#include "Logger.h"

#include "StringUtils.h"

#include <QString>

#include <cstdarg>

namespace TrenchBroom {
    Logger::stream::stream(Logger* logger, const LogLevel logLevel) :
    m_logger(logger),
    m_logLevel(logLevel) {}

    Logger::stream::~stream() {
        m_logger->log(m_logLevel, m_buf.str());
    }

    Logger::~Logger() {}

    Logger::stream Logger::debug() {
        return Logger::stream(this, LogLevel_Debug);
    }

    void Logger::debug([[maybe_unused]] const char* format, ...) {
#ifndef NDEBUG
        va_list arguments;
        va_start(arguments, format);
        const String message = StringUtils::formatStringV(format, arguments);
        va_end(arguments);
        debug(message);
#endif
    }

    void Logger::debug([[maybe_unused]] const String& message) {
#ifndef NDEBUG
        log(LogLevel_Debug, message);
#endif
    }

    void Logger::debug([[maybe_unused]] const QString& message) {
#ifndef NDEBUG
        log(LogLevel_Debug, message);
#endif
    }

    Logger::stream Logger::info() {
        return stream(this, LogLevel_Info);
    }

    void Logger::info(const char* format, ...) {
        va_list arguments;
        va_start(arguments, format);
        const String message = StringUtils::formatStringV(format, arguments);
        va_end(arguments);
        info(message);
    }

    void Logger::info(const String& message) {
        log(LogLevel_Info, message);
    }

    void Logger::info(const QString& message) {
        log(LogLevel_Info, message);
    }

    Logger::stream Logger::warn() {
        return stream(this, LogLevel_Warn);
    }

    void Logger::warn(const char* format, ...) {
        va_list arguments;
        va_start(arguments, format);
        const String message = StringUtils::formatStringV(format, arguments);
        va_end(arguments);
        warn(message);
    }

    void Logger::warn(const String& message) {
        log(LogLevel_Warn, message);
    }

    void Logger::warn(const QString& message) {
        log(LogLevel_Warn, message);
    }

    Logger::stream Logger::error() {
        return stream(this, LogLevel_Error);
    }

    void Logger::error(const char* format, ...) {
        va_list arguments;
        va_start(arguments, format);
        const String message = StringUtils::formatStringV(format, arguments);
        va_end(arguments);
        error(message);
    }

    void Logger::error(const String& message) {
        log(LogLevel_Error, message);
    }

    void Logger::error(const QString& message) {
        log(LogLevel_Error, message);
    }

    void Logger::log(const LogLevel level, const String& message) {
#ifdef NDEBUG
        if (level != LogLevel_Debug)
#endif
        doLog(level, message);
    }

    void Logger::log(const LogLevel level, const QString& message) {
#ifdef NDEBUG
        if (level != LogLevel_Debug)
#endif
        doLog(level, message);
    }

    void NullLogger::doLog(Logger::LogLevel /* level */, const String& /* message */) {}
    void NullLogger::doLog(Logger::LogLevel /* level */, const QString& /* message */) {}
}
