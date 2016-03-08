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

#ifndef TrenchBroom_Exceptions_h
#define TrenchBroom_Exceptions_h

#include <exception>

#include "StringUtils.h"

namespace TrenchBroom {
    
    class Exception : public std::exception {
    protected:
        String m_msg;
    public:
        Exception() throw() {}
        Exception(const String& str) throw() : m_msg(str) {}
        virtual ~Exception() throw() {}

        const char* what() const throw() {
            return m_msg.c_str();
        }
    };
    
    template <class C>
    class ExceptionStream : public Exception {
    public:
        ExceptionStream() throw() {}
        ExceptionStream(const String& str) throw() : Exception(str) {}
        virtual ~ExceptionStream() throw() {}
        
        template <typename T>
        C& operator<< (T value) {
            StringStream stream;
            stream << m_msg << value;
            m_msg = stream.str();
            return static_cast<C&>(*this);
        }
    };
    
    class GeometryException : public ExceptionStream<GeometryException> {
    public:
        GeometryException() throw() {}
        GeometryException(const String& str) throw() : ExceptionStream(str) {}
        ~GeometryException() throw() {}
    };
            
    class EntityAttributeException : public ExceptionStream<EntityAttributeException> {
    public:
        EntityAttributeException() throw() {}
        EntityAttributeException(const String& str) throw() : ExceptionStream(str) {}
        ~EntityAttributeException() throw() {}
    };

    class ParserException : public ExceptionStream<ParserException> {
    public:
        ParserException() throw() {}
        ParserException(const String& str) throw() : ExceptionStream(str) {}
        ParserException(const size_t line, const size_t column, const String& str = "") throw() : ExceptionStream() {
            *this << str << " (line " << line << ", column " << column << ")";
        }
        ~ParserException() throw() {}
    };
            
    class VboException : public ExceptionStream<VboException> {
    public:
        VboException() throw() {}
        VboException(const String& str) throw() : ExceptionStream(str) {}
        ~VboException() throw() {}
    };
            
    class PathException : public ExceptionStream<PathException> {
    public:
        PathException() throw() {}
        PathException(const String& str) throw() : ExceptionStream(str) {}
        ~PathException() throw() {}
    };
            
    class FileSystemException : public ExceptionStream<FileSystemException> {
    public:
        FileSystemException() throw() {}
        FileSystemException(const String& str) throw() : ExceptionStream(str) {}
        FileSystemException(const String& str, const PathException& e) throw() : ExceptionStream(str + " (" + e.what() + ")") {}
        ~FileSystemException() throw() {}
    };
            
    class AssetException : public ExceptionStream<AssetException> {
    public:
        AssetException() throw() {}
        AssetException(const String& str) throw() : ExceptionStream(str) {}
        ~AssetException() throw() {}
    };
        
    class CommandProcessorException : public ExceptionStream<CommandProcessorException> {
    public:
        CommandProcessorException() throw() {}
        CommandProcessorException(const String& str) throw() : ExceptionStream(str) {}
        ~CommandProcessorException() throw() {}
    };

    class RenderException : public ExceptionStream<RenderException> {
    public:
        RenderException() throw() {}
        RenderException(const String& str) throw() : ExceptionStream(str) {}
        ~RenderException() throw() {}
    };
            
    class OctreeException : public ExceptionStream<OctreeException> {
    public:
        OctreeException() throw() {}
        OctreeException(const String& str) throw() : ExceptionStream(str) {}
        ~OctreeException() throw() {}
    };
            
    class GameException : public ExceptionStream<GameException> {
    public:
        GameException() throw() {}
        GameException(const String& str) throw() : ExceptionStream(str) {}
        ~GameException() throw() {}
    };
            
    class ResourceNotFoundException : public ExceptionStream<ResourceNotFoundException> {
    public:
        ResourceNotFoundException() throw() {}
        ResourceNotFoundException(const String& str) throw() : ExceptionStream(str) {}
        ~ResourceNotFoundException() throw() {}
    };
            
    class FileFormatException : public ExceptionStream<FileFormatException> {
    public:
        FileFormatException() throw() {}
        FileFormatException(const String& str) throw() : ExceptionStream(str) {}
        ~FileFormatException() throw() {}
    };
}

#endif
