/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Console.h"
#include <cstdio>
#include <cstdarg>

namespace TrenchBroom {
    void log(LogLevel level, const char* format, ...) {
        static char buffer[1024];
        static std::string message;
        
        /*
        if (level < logLevel)
            return;
        */
         
        va_list arglist;
        va_start(arglist, format);
#if defined _MSC_VER
        vsprintf_s(buffer, format, arglist);
#else
        vsprintf(buffer, format, arglist);
#endif
        va_end(arglist);
        
        message = buffer;
        switch (level) {
			case TB_LL_DEBUG:
				log("Debug: " + message);
				break;
            case TB_LL_INFO:
                log("Info: " + message);
                break;
            case TB_LL_WARN:
                log("Warning: " + message);
                break;
            case TB_LL_ERR:
                log("Error: " + message);
                break;
            default:
                log("Unknown: " + message);
                break;
        }
    }
}
