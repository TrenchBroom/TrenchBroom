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

#ifndef TrenchBroom_MapExceptions_h
#define TrenchBroom_MapExceptions_h

#include <cstdio>
#include <cstdarg>
#include <exception>
#include <string>

namespace TrenchBroom {
    namespace Model {
        class GeometryException : public std::exception {
        protected:
            std::string m_msg;
        public:
			GeometryException(const char* format, ...) throw() {
                static char buffer[1024];
                va_list arglist;
                va_start(arglist, format);
                vsprintf(buffer, format, arglist);
                va_end(arglist);
                
                m_msg = buffer;
            }
            
            ~GeometryException() throw() {}
            
			virtual const char* what() const throw() {
			    return m_msg.c_str();
			}
        };
    }
}

#endif
