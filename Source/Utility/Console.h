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

#ifndef __TrenchBroom__Console__
#define __TrenchBroom__Console__

#include "Utility/String.h"

#include <wx/textctrl.h>

#include <vector>

namespace TrenchBroom {
    namespace Utility {
        class Console {
        protected:
            typedef enum {
                LLDebug,
                LLInfo,
                LLWarn,
                LLError
            } LogLevel;
            
            class LogMessage {
            protected:
                LogLevel m_level;
                String m_string;
            public:
                LogMessage(const LogLevel level, const String& string) :
                m_level(level) {
                    String trimmed = Utility::trim(string);
                    StringStream buffer;
                    bool previousWasNewline = false;
                    for (unsigned int i = 0; i < trimmed.length(); i++) {
                        char c = trimmed[i];
                        if (c == '\r')
                            continue;
                        if (c == '\n') {
                            if (!previousWasNewline)
                                buffer << c;
                            previousWasNewline = true;
                        } else {
                            buffer << c;
                            previousWasNewline = false;
                        }
                    }
                    m_string = buffer.str();
                }
                
                inline const LogLevel level() const {
                    return m_level;
                }
                
                inline const String& string() const {
                    return m_string;
                }
            };
            
            typedef std::vector<LogMessage> LogMessageList;

            LogMessageList m_buffer;
            
            wxTextCtrl* m_textCtrl;
            
            void logToDebug(const LogMessage& message);
            void logToConsole(const LogMessage& message);
            void logToFile(const LogMessage& message);
        public:
            Console() : m_textCtrl(NULL) {}
            
            void setTextCtrl(wxTextCtrl* textCtrl);
            
            void log(const LogMessage& message);
            
            void debug(const String& message);
            void debug(const char* format, ...);
            void info(const String& message);
            void info(const char* format, ...);
            void warn(const String& message);
            void warn(const char* format, ...);
            void error(const String& message);
            void error(const char* format, ...);
        };
    }
}

#endif /* defined(__TrenchBroom__Console__) */
