/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
    
    void Logger::debug(const String& message) {
        log(LLDebug, message);
    }
    
    void Logger::debug(const char* format, ...) {
        va_list(arguments);
        va_start(arguments, format);
        const String message = StringUtils::formatString(format, arguments);
        va_end(arguments);
        debug(message);
    }
    
    void Logger::info(const String& message) {
        log(LLInfo, message);
    }
    
    void Logger::info(const char* format, ...) {
        va_list(arguments);
        va_start(arguments, format);
        const String message = StringUtils::formatString(format, arguments);
        va_end(arguments);
        info(message);
    }
    
    void Logger::warn(const String& message) {
        log(LLWarn, message);
    }
    
    void Logger::warn(const char* format, ...) {
        va_list(arguments);
        va_start(arguments, format);
        const String message = StringUtils::formatString(format, arguments);
        va_end(arguments);
        warn(message);
    }
    
    void Logger::error(const String& message) {
        log(LLError, message);
    }
    
    void Logger::error(const char* format, ...) {
        va_list(arguments);
        va_start(arguments, format);
        const String message = StringUtils::formatString(format, arguments);
        va_end(arguments);
        error(message);
    }
}
