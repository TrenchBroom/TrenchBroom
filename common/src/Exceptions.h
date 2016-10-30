/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
#include <string>
#include <sstream>

#include "TrenchBroomStackWalker.h"

namespace TrenchBroom {
    
    class Exception : public std::exception {
    protected:
        std::string m_msg;
        std::string m_trace;
    public:
        Exception() throw() : m_trace(TrenchBroomStackWalker::getStackTrace()) {}
        Exception(const std::string& str) throw() : m_msg(str), m_trace(TrenchBroomStackWalker::getStackTrace()) {}

        const char* what() const throw() {
            return m_msg.c_str();
        }
        
        const std::string stackTrace() {
            return m_trace;
        }
    };
    
    template <class C>
    class ExceptionStream : public Exception {
    public:
        ExceptionStream() throw() {}
        ExceptionStream(const std::string& str) throw() : Exception(str) {}
        
        template <typename T>
        C& operator<< (T value) {
            std::stringstream stream;
            stream << m_msg << value;
            m_msg = stream.str();
            return static_cast<C&>(*this);
        }
    };
    
    class GeometryException : public ExceptionStream<GeometryException> {
    public:
        GeometryException() throw() {}
        GeometryException(const std::string& str) throw() : ExceptionStream(str) {}
    };
            
    class EntityAttributeException : public ExceptionStream<EntityAttributeException> {
    public:
        EntityAttributeException() throw() {}
        EntityAttributeException(const std::string& str) throw() : ExceptionStream(str) {}
    };

    class ParserException : public ExceptionStream<ParserException> {
    public:
        ParserException() throw() {}
        ParserException(const std::string& str) throw() : ExceptionStream(str) {}
        ParserException(const size_t line, const size_t column, const std::string& str = "") throw() : ExceptionStream() {
            if (!str.empty())
                *this << str << " ";
            *this << "[line " << line << ", column " << column << "]";
        }
    };
            
    class VboException : public ExceptionStream<VboException> {
    public:
        VboException() throw() {}
        VboException(const std::string& str) throw() : ExceptionStream(str) {}
    };
            
    class PathException : public ExceptionStream<PathException> {
    public:
        PathException() throw() {}
        PathException(const std::string& str) throw() : ExceptionStream(str) {}
    };
            
    class FileSystemException : public ExceptionStream<FileSystemException> {
    public:
        FileSystemException() throw() {}
        FileSystemException(const std::string& str) throw() : ExceptionStream(str) {}
        FileSystemException(const std::string& str, const PathException& e) throw() : ExceptionStream(str + " (" + e.what() + ")") {}
    };
            
    class FileNotFoundException : public ExceptionStream<FileNotFoundException> {
    public:
        FileNotFoundException() throw() {}
        FileNotFoundException(const std::string& str) throw() : ExceptionStream(str) {}
        FileNotFoundException(const std::string& str, const PathException& e) throw() : ExceptionStream(str + " (" + e.what() + ")") {}
    };
    
    class AssetException : public ExceptionStream<AssetException> {
    public:
        AssetException() throw() {}
        AssetException(const std::string& str) throw() : ExceptionStream(str) {}
    };
        
    class CommandProcessorException : public ExceptionStream<CommandProcessorException> {
    public:
        CommandProcessorException() throw() {}
        CommandProcessorException(const std::string& str) throw() : ExceptionStream(str) {}
    };

    class RenderException : public ExceptionStream<RenderException> {
    public:
        RenderException() throw() {}
        RenderException(const std::string& str) throw() : ExceptionStream(str) {}
    };
            
    class OctreeException : public ExceptionStream<OctreeException> {
    public:
        OctreeException() throw() {}
        OctreeException(const std::string& str) throw() : ExceptionStream(str) {}
    };
            
    class GameException : public ExceptionStream<GameException> {
    public:
        GameException() throw() {}
        GameException(const std::string& str) throw() : ExceptionStream(str) {}
    };
            
    class ResourceNotFoundException : public ExceptionStream<ResourceNotFoundException> {
    public:
        ResourceNotFoundException() throw() {}
        ResourceNotFoundException(const std::string& str) throw() : ExceptionStream(str) {}
    };
            
    class FileFormatException : public ExceptionStream<FileFormatException> {
    public:
        FileFormatException() throw() {}
        FileFormatException(const std::string& str) throw() : ExceptionStream(str) {}
    };

    class ConditionFailedException : public ExceptionStream<ConditionFailedException> {
    public:
        ConditionFailedException() throw() {}
        ConditionFailedException(const std::string& str) throw() : ExceptionStream(str) {}
        ConditionFailedException(const char *file, const int line, const char *condition, const std::string& message) throw() : ExceptionStream() {
            *this << file << ":" << line << ": Condition '" << condition << "' failed: " << message;
        }
    };
}

#endif
