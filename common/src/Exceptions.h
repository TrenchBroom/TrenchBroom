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
        Exception() noexcept : m_trace(TrenchBroomStackWalker::getStackTrace()) {}
        Exception(const std::string& str) noexcept : m_msg(str), m_trace(TrenchBroomStackWalker::getStackTrace()) {}

        const char* what() const noexcept {
            return m_msg.c_str();
        }
        
        const std::string stackTrace() {
            return m_trace;
        }
    };
    
    template <class C>
    class ExceptionStream : public Exception {
    public:
        ExceptionStream() noexcept {}
        ExceptionStream(const std::string& str) noexcept : Exception(str) {}
        
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
        GeometryException() noexcept {}
        GeometryException(const std::string& str) noexcept : ExceptionStream(str) {}
    };
            
    class EntityAttributeException : public ExceptionStream<EntityAttributeException> {
    public:
        EntityAttributeException() noexcept {}
        EntityAttributeException(const std::string& str) noexcept : ExceptionStream(str) {}
    };

    class ParserException : public ExceptionStream<ParserException> {
    public:
        ParserException() noexcept {}
        ParserException(const std::string& str) noexcept : ExceptionStream(str) {}
        ParserException(const size_t line, const size_t column, const std::string& str = "") noexcept : ExceptionStream() {
            if (!str.empty())
                *this << str << " ";
            *this << "[line " << line << ", column " << column << "]";
        }
    };
            
    class VboException : public ExceptionStream<VboException> {
    public:
        VboException() noexcept {}
        VboException(const std::string& str) noexcept : ExceptionStream(str) {}
    };
            
    class PathException : public ExceptionStream<PathException> {
    public:
        PathException() noexcept {}
        PathException(const std::string& str) noexcept : ExceptionStream(str) {}
    };
            
    class FileSystemException : public ExceptionStream<FileSystemException> {
    public:
        FileSystemException() noexcept {}
        FileSystemException(const std::string& str) noexcept : ExceptionStream(str) {}
        FileSystemException(const std::string& str, const PathException& e) noexcept : ExceptionStream(str + " (" + e.what() + ")") {}
    };
            
    class FileNotFoundException : public ExceptionStream<FileNotFoundException> {
    public:
        FileNotFoundException() noexcept {}
        FileNotFoundException(const std::string& str) noexcept : ExceptionStream(str) {}
        FileNotFoundException(const std::string& str, const PathException& e) noexcept : ExceptionStream(str + " (" + e.what() + ")") {}
    };
    
    class AssetException : public ExceptionStream<AssetException> {
    public:
        AssetException() noexcept {}
        AssetException(const std::string& str) noexcept : ExceptionStream(str) {}
    };
        
    class CommandProcessorException : public ExceptionStream<CommandProcessorException> {
    public:
        CommandProcessorException() noexcept {}
        CommandProcessorException(const std::string& str) noexcept : ExceptionStream(str) {}
    };

    class RenderException : public ExceptionStream<RenderException> {
    public:
        RenderException() noexcept {}
        RenderException(const std::string& str) noexcept : ExceptionStream(str) {}
    };
            
    class OctreeException : public ExceptionStream<OctreeException> {
    public:
        OctreeException() noexcept {}
        OctreeException(const std::string& str) noexcept : ExceptionStream(str) {}
    };
            
    class GameException : public ExceptionStream<GameException> {
    public:
        GameException() noexcept {}
        GameException(const std::string& str) noexcept : ExceptionStream(str) {}
    };
            
    class ResourceNotFoundException : public ExceptionStream<ResourceNotFoundException> {
    public:
        ResourceNotFoundException() noexcept {}
        ResourceNotFoundException(const std::string& str) noexcept : ExceptionStream(str) {}
    };
            
    class FileFormatException : public ExceptionStream<FileFormatException> {
    public:
        FileFormatException() noexcept {}
        FileFormatException(const std::string& str) noexcept : ExceptionStream(str) {}
    };

    class ConditionFailedException : public ExceptionStream<ConditionFailedException> {
    public:
        ConditionFailedException() noexcept {}
        ConditionFailedException(const std::string& str) noexcept : ExceptionStream(str) {}
        ConditionFailedException(const char *file, const int line, const char *condition, const std::string& message) noexcept : ExceptionStream() {
            *this << file << ":" << line << ": Condition '" << condition << "' failed: " << message;
        }
    };
}

#endif
