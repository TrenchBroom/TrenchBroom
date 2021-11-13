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

#pragma once

#include <exception>
#include <string>

namespace TrenchBroom {
class Exception : public std::exception {
protected:
  std::string m_msg;

public:
  Exception() noexcept;
  explicit Exception(std::string&& str) noexcept;

  const char* what() const noexcept override;
};

class BrushFaceReferenceException : public Exception {
public:
  using Exception::Exception;
};

class EntityAttributeException : public Exception {
public:
  using Exception::Exception;
};

class ParserException : public Exception {
public:
  using Exception::Exception;
  ParserException(size_t line, size_t column, const std::string& str = "");
  explicit ParserException(size_t line, const std::string& str = "");

private:
  static std::string buildMessage(size_t line, size_t column, const std::string& str);
  static std::string buildMessage(size_t line, const std::string& str);
};

class VboException : public Exception {
public:
  using Exception::Exception;
};

class PathException : public Exception {
public:
  using Exception::Exception;
};

class FileSystemException : public Exception {
public:
  using Exception::Exception;
  FileSystemException(const std::string& str, const PathException& e);
};

class FileNotFoundException : public Exception {
public:
  using Exception::Exception;
  explicit FileNotFoundException(const std::string& path);
  FileNotFoundException(const std::string& path, const PathException& e);
};

class AssetException : public Exception {
public:
  using Exception::Exception;
};

class CommandProcessorException : public Exception {
public:
  using Exception::Exception;
};

class RenderException : public Exception {
public:
  using Exception::Exception;
};

class NodeTreeException : public Exception {
public:
  using Exception::Exception;
};

class GameException : public Exception {
public:
  using Exception::Exception;
};

class FileFormatException : public Exception {
public:
  using Exception::Exception;
};
} // namespace TrenchBroom
