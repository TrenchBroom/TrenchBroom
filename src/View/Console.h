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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__Console__
#define __TrenchBroom__Console__

#include "StringUtils.h"

#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        class Console : public wxTextCtrl {
        private:
            typedef enum {
                LLDebug,
                LLInfo,
                LLWarn,
                LLError
            } LogLevel;
        public:
            Console(wxWindow* parent);

            void debug(const String& message);
            void debug(const char* format, ...);
            void info(const String& message);
            void info(const char* format, ...);
            void warn(const String& message);
            void warn(const char* format, ...);
            void error(const String& message);
            void error(const char* format, ...);
        private:
            void log(const LogLevel level, const String& message);
            void logToConsole(const LogLevel level, const String& message);
        };
    }
}

#endif /* defined(__TrenchBroom__Console__) */
