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

#ifndef TrenchBroom_Exceptions_h
#define TrenchBroom_Exceptions_h

#include <exception>

#include "StringUtils.h"

namespace TrenchBroom {
    class Exception : public std::exception {
    private:
        String m_msg;
    public:
        Exception() throw() {}
        Exception(const String& str) throw() {
            *this << str;
        }
        
        ~Exception() throw() {}
        
        template <typename T>
        Exception& operator<< (T value) {
            StringStream stream;
            stream << m_msg << value;
            m_msg = stream.str();
            return *this;
        }
        
        const char* what() const throw() {
            return m_msg.c_str();
        }
    };
    
    class GeometryException : public Exception {
    public:
        GeometryException() throw() {}
        GeometryException(const String& str) throw() : Exception(str) {}
        ~GeometryException() throw() {}
    };
            
    class EntityPropertyException : public Exception {
    public:
        EntityPropertyException() throw() {}
        EntityPropertyException(const String& str) throw() : Exception(str) {}
        ~EntityPropertyException() throw() {}
    };

    class ParserException : public Exception {
    public:
        ParserException() throw() {}
        ParserException(const String& str) throw() : Exception(str) {}
        ParserException(const size_t line, const size_t column, const String& str) throw() : Exception() {
            *this << line;
            *this << column;
            *this << ": " << str;
        }
        ~ParserException() throw() {}
    };
            
    class VboException : public Exception {
    public:
        VboException() throw() {}
        VboException(const String& str) throw() : Exception(str) {}
        ~VboException() throw() {}
    };
            
    class PathException : public Exception {
    public:
        PathException() throw() {}
        PathException(const String& str) throw() : Exception(str) {}
        ~PathException() throw() {}
    };
            
    class FileSystemException : public Exception {
    public:
        FileSystemException() throw() {}
        FileSystemException(const String& str) throw() : Exception(str) {}
        ~FileSystemException() throw() {}
    };
            
    class WadException : public Exception {
    public:
        WadException() throw() {}
        WadException(const String& str) throw() : Exception(str) {}
        ~WadException() throw() {}
    };
        
    class CommandProcessorException : public Exception {
    public:
        CommandProcessorException() throw() {}
        CommandProcessorException(const String& str) throw() : Exception(str) {}
        ~CommandProcessorException() throw() {}
    };

    class RenderException : public Exception {
    public:
        RenderException() throw() {}
        RenderException(const String& str) throw() : Exception(str) {}
        ~RenderException() throw() {}
    };
            
    class OctreeException : public Exception {
    public:
        OctreeException() throw() {}
        OctreeException(const String& str) throw() : Exception(str) {}
        ~OctreeException() throw() {}
    };
}

#endif
