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

#ifndef TrenchBroom_CachingLogger
#define TrenchBroom_CachingLogger

#include "StringUtils.h"
#include "Logger.h"

#include <vector>

#include <wx/string.h>

namespace TrenchBroom {
    namespace View {
        class CachingLogger : public Logger {
        private:
            struct Message {
            public:
                LogLevel level;
                wxString str;
                
                Message(const LogLevel i_level, const wxString& i_str);
            };
            
            typedef std::vector<Message> MessageList;
            
            MessageList m_cachedMessages;
            Logger* m_logger;
        public:
            CachingLogger();
            
            void setParentLogger(Logger* logger);
        private:
            void doLog(LogLevel level, const String& message);
            void doLog(LogLevel level, const wxString& message);
        };
    }
}

#endif /* defined(TrenchBroom_CachingLogger) */
