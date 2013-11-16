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

#ifndef __TrenchBroom__CachingLogger__
#define __TrenchBroom__CachingLogger__

#include "StringUtils.h"
#include "Logger.h"

#include <vector>

namespace TrenchBroom {
    namespace View {
        class CachingLogger : public Logger {
        private:
            struct Message {
            public:
                LogLevel level;
                String str;
                
                Message(const LogLevel i_level, const String& i_str);
            };
            
            typedef std::vector<Message> MessageList;
            
            MessageList m_cachedMessages;
            Logger* m_logger;
        public:
            CachingLogger();
            
            void setParentLogger(Logger* logger);
            void log(const LogLevel level, const String& message);
        };
    }
}

#endif /* defined(__TrenchBroom__CachingLogger__) */
