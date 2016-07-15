/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include <cstdarg>

namespace TrenchBroom {
    Logger::~Logger() {}
    
    void Logger::debug(const char* format, ...) {
#ifndef NDEBUG
        va_list(arguments);
        va_start(arguments, format);
        const String message = StringUtils::formatStringV(format, arguments);
        va_end(arguments);
        debug(message);
#endif
    }
    
    void Logger::debug(const String& message) {
#ifndef NDEBUG
        log(LogLevel_Debug, message);
#endif
    }
    
    void Logger::debug(const wxString& message) {
#ifndef NDEBUG
        log(LogLevel_Debug, message);
#endif
    }
    
    void Logger::info(const char* format, ...) {
        va_list(arguments);
        va_start(arguments, format);
        const String message = StringUtils::formatStringV(format, arguments);
        va_end(arguments);
        info(message);
    }
    
    void Logger::info(const String& message) {
        log(LogLevel_Info, message);
    }
    
    void Logger::info(const wxString& message) {
        log(LogLevel_Info, message);
    }
    
    void Logger::warn(const char* format, ...) {
        va_list(arguments);
        va_start(arguments, format);
        const String message = StringUtils::formatStringV(format, arguments);
        va_end(arguments);
        warn(message);
    }
    
    void Logger::warn(const String& message) {
        log(LogLevel_Warn, message);
    }
    
    void Logger::warn(const wxString& message) {
        log(LogLevel_Warn, message);
    }
    
    void Logger::error(const char* format, ...) {
        va_list(arguments);
        va_start(arguments, format);
        const String message = StringUtils::formatStringV(format, arguments);
        va_end(arguments);
        error(message);
    }
    
    void Logger::error(const String& message) {
        log(LogLevel_Error, message);
    }
    
    void Logger::error(const wxString& message) {
        log(LogLevel_Error, message);
    }

    void Logger::log(const LogLevel level, const String& message) {
        doLog(level, message);
    }
    
    void Logger::log(const LogLevel level, const wxString& message) {
        doLog(level, message);
    }
}
