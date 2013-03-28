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

#ifndef TrenchBroom_IOException_h
#define TrenchBroom_IOException_h

#include "Utility/MessageException.h"
#include "Utility/String.h"

#include <iostream>

namespace TrenchBroom {
    namespace IO {
        class IOException : public Utility::MessageException {
        public:
            IOException(const char* format, ...) throw() : MessageException() {
                va_list(arguments);
                va_start(arguments, format);
                Utility::formatString(format, arguments, m_msg);
                va_end(arguments);
            }
            
            IOException(const String& msg) throw() : MessageException(msg) {}
            
            IOException(const StringStream& str) throw() : MessageException(str) {}
            
            static IOException openError(const String& path = "") {
                return IOException("Unable to open file %s", path.c_str());
            }
            
            static IOException badStream(const std::istream& stream) {
                return IOException("Error reading file");
            }

            static IOException unexpectedEof() {
                return IOException("Reached end of file");
            }
        };
    }
}

#endif
