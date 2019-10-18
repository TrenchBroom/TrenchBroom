/*
 Copyright (C) 2010-2017 Kristian Duske

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

class Exception : public std::exception {
protected:
    std::string m_msg;
public:
    Exception() noexcept;
    explicit Exception(const std::string& str) noexcept;

    const char* what() const noexcept override;
};

template <class C>
class ExceptionStream : public Exception {
public:
    ExceptionStream() noexcept :
    Exception() {}

    explicit ExceptionStream(const std::string& str) noexcept :
    Exception(str) {}

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
    using ExceptionStream::ExceptionStream;
};

class EntityAttributeException : public ExceptionStream<EntityAttributeException> {
public:
    using ExceptionStream::ExceptionStream;
};

class ParserException : public ExceptionStream<ParserException> {
public:
    using ExceptionStream::ExceptionStream;
    ParserException(const size_t line, const size_t column, const std::string& str = "") noexcept :
    ExceptionStream() {
        *this << "At line " << line << ", column " << column << ":";
        if (!str.empty()) {
            *this << " " << str;
        }
    }

    explicit ParserException(const size_t line, const std::string& str = "") noexcept : ExceptionStream() {
        *this << "At line " << line << ":";
        if (!str.empty()) {
            *this << " " << str;
        }
    }
};

class VboException : public ExceptionStream<VboException> {
public:
    using ExceptionStream::ExceptionStream;
};

class PathException : public ExceptionStream<PathException> {
public:
    using ExceptionStream::ExceptionStream;
};

class FileSystemException : public ExceptionStream<FileSystemException> {
public:
    using ExceptionStream::ExceptionStream;
    FileSystemException(const std::string& str, const PathException& e) noexcept : ExceptionStream(str + " (" + e.what() + ")") {}
};

class FileNotFoundException : public ExceptionStream<FileNotFoundException> {
public:
    using ExceptionStream::ExceptionStream;
    FileNotFoundException(const std::string& str, const PathException& e) noexcept : ExceptionStream(str + " (" + e.what() + ")") {}
};

class AssetException : public ExceptionStream<AssetException> {
public:
    using ExceptionStream::ExceptionStream;
};

class CommandProcessorException : public ExceptionStream<CommandProcessorException> {
public:
    using ExceptionStream::ExceptionStream;
};

class RenderException : public ExceptionStream<RenderException> {
public:
    using ExceptionStream::ExceptionStream;
};

class NodeTreeException : public ExceptionStream<NodeTreeException> {
public:
    using ExceptionStream::ExceptionStream;
};

class GameException : public ExceptionStream<GameException> {
public:
    using ExceptionStream::ExceptionStream;
};

class ResourceNotFoundException : public ExceptionStream<ResourceNotFoundException> {
public:
    using ExceptionStream::ExceptionStream;
};

class FileFormatException : public ExceptionStream<FileFormatException> {
public:
    using ExceptionStream::ExceptionStream;
};

#endif
