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

#ifndef TrenchBroom_Logger
#define TrenchBroom_Logger

#include "StringUtils.h"

class wxString;

namespace TrenchBroom {
    class Logger {
    public:
        typedef enum {
            LogLevel_Debug,
            LogLevel_Info,
            LogLevel_Warn,
            LogLevel_Error
        } LogLevel;
    public:
        virtual ~Logger();
        
        void debug(const char* format, ...);
        void debug(const String& message);
        void debug(const wxString& message);
        
        void info(const char* format, ...);
        void info(const String& message);
        void info(const wxString& message);
        
        void warn(const char* format, ...);
        void warn(const String& message);
        void warn(const wxString& message);
        
        void error(const char* format, ...);
        void error(const String& message);
        void error(const wxString& message);
        
        void log(LogLevel level, const String& message);
        void log(LogLevel level, const wxString& message);
    private:
        virtual void doLog(LogLevel level, const String& message) = 0;
        virtual void doLog(LogLevel level, const wxString& message) = 0;
    };
}

#endif /* defined(TrenchBroom_Logger) */
