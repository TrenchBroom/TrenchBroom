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

#include <string>

#include <QString>

namespace TrenchBroom {
    Logger::stream::stream(Logger* logger, const LogLevel logLevel) :
    m_logger(logger),
    m_logLevel(logLevel) {}

    Logger::stream::~stream() {
        m_logger->log(m_logLevel, m_buf.str());
    }

    Logger::~Logger() {}

    Logger::stream Logger::debug() {
        return Logger::stream(this, LogLevel::Debug);
    }

    void Logger::debug([[maybe_unused]] const char* message) {
#ifndef NDEBUG
        debug(QString(message));
#endif
    }

    void Logger::debug([[maybe_unused]] const std::string& message) {
#ifndef NDEBUG
        log(LogLevel::Debug, message);
#endif
    }

    void Logger::debug([[maybe_unused]] const QString& message) {
#ifndef NDEBUG
        log(LogLevel::Debug, message);
#endif
    }

    Logger::stream Logger::info() {
        return stream(this, LogLevel::Info);
    }

    void Logger::info(const char* message) {
        info(QString(message));
    }

    void Logger::info(const std::string& message) {
        log(LogLevel::Info, message);
    }

    void Logger::info(const QString& message) {
        log(LogLevel::Info, message);
    }

    Logger::stream Logger::warn() {
        return stream(this, LogLevel::Warn);
    }

    void Logger::warn(const char* message) {
        warn(QString(message));
    }

    void Logger::warn(const std::string& message) {
        log(LogLevel::Warn, message);
    }

    void Logger::warn(const QString& message) {
        log(LogLevel::Warn, message);
    }

    Logger::stream Logger::error() {
        return stream(this, LogLevel::Error);
    }

    void Logger::error(const char* message) {
        error(QString(message));
    }

    void Logger::error(const std::string& message) {
        log(LogLevel::Error, message);
    }

    void Logger::error(const QString& message) {
        log(LogLevel::Error, message);
    }

    void Logger::log(const LogLevel level, const std::string& message) {
#ifdef NDEBUG
        if (level != LogLevel::Debug)
#endif
        doLog(level, message);
    }

    void Logger::log(const LogLevel level, const QString& message) {
#ifdef NDEBUG
        if (level != LogLevel::Debug)
#endif
        doLog(level, message);
    }

    void NullLogger::doLog(const LogLevel /* level */, const std::string& /* message */) {}
    void NullLogger::doLog(const LogLevel /* level */, const QString& /* message */) {}
}
