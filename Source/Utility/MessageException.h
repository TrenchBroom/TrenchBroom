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

#ifndef TrenchBroom_MessageException_h
#define TrenchBroom_MessageException_h

#include "Utility/String.h"
#include <exception>

namespace TrenchBroom {
    namespace Utility {
        class MessageException : public std::exception {
        protected:
            String m_msg;
            
            MessageException() throw() {}
        public:
            MessageException(const char* format, ...) throw() {
                va_list(arguments);
                va_start(arguments, format);
                formatString(format, arguments, m_msg);
                va_end(arguments);
            }

            MessageException(const String& msg) throw() : m_msg(msg) {}
            
            MessageException(const StringStream& str) throw() : m_msg(str.str()) {}
            
            virtual ~MessageException() throw() {}
            
            virtual const char* what() const throw() {
                return m_msg.c_str();
            }        
        };
    }
}

#endif
