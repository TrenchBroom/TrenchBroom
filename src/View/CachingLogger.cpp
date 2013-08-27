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

#include "CachingLogger.h"

namespace TrenchBroom {
    namespace View {
        CachingLogger::Message::Message(const Logger::LogLevel i_level, const String& i_str) :
        level(i_level),
        str(i_str) {}

        CachingLogger::CachingLogger() :
        m_logger(NULL) {}
        
        void CachingLogger::setParentLogger(Logger* logger) {
            m_logger = logger;
            if (m_logger != NULL) {
                MessageList::const_iterator it, end;
                for (it = m_cachedMessages.begin(), end = m_cachedMessages.end(); it != end; ++it) {
                    const Message& message = *it;
                    log(message.level, message.str);
                }
                m_cachedMessages.clear();
            }
        }
        
        void CachingLogger::log(const LogLevel level, const String& message) {
            if (m_logger == NULL)
                m_cachedMessages.push_back(Message(level, message));
            else
                m_logger->log(level, message);
        }
    }
}
