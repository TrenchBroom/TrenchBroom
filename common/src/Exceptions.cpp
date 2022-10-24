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

#include "Exceptions.h"

#include <sstream>

namespace TrenchBroom
{
Exception::Exception() noexcept {}

Exception::Exception(std::string&& str) noexcept
  : m_msg(std::move(str))
{
}

const char* Exception::what() const noexcept
{
  return m_msg.c_str();
}

ParserException::ParserException(
  const size_t line, const size_t column, const std::string& str)
  : Exception(buildMessage(line, column, str))
{
}

ParserException::ParserException(const size_t line, const std::string& str)
  : Exception(buildMessage(line, str))
{
}

std::string ParserException::buildMessage(
  const size_t line, const size_t column, const std::string& str)
{
  auto msg = std::stringstream();
  msg << "At line " << line << ", column " << column << ":";
  if (!str.empty())
  {
    msg << " " << str;
  }
  return msg.str();
}

std::string ParserException::buildMessage(const size_t line, const std::string& str)
{
  auto msg = std::stringstream();
  msg << "At line " << line << ":";
  if (!str.empty())
  {
    msg << " " << str;
  }
  return msg.str();
}

FileSystemException::FileSystemException(const std::string& str, const PathException& e)
  : Exception(str + " (" + e.what() + ")")
{
}

FileNotFoundException::FileNotFoundException(const std::string& path)
  : Exception("File not found: '" + path + "'")
{
}

FileNotFoundException::FileNotFoundException(
  const std::string& path, const PathException& e)
  : Exception("File not found: '" + path + "' (" + e.what() + ")")
{
}
} // namespace TrenchBroom
