/*
 Copyright (C) 2010 Kristian Duske

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
namespace
{

std::string buildMessage(
  const std::optional<FileLocation>& location, const std::string& str)
{
  auto msg = std::stringstream();

  msg << "At ";
  if (location)
  {
    msg << *location;
  }
  else
  {
    msg << "unknown location";
  }

  msg << ":";
  if (!str.empty())
  {
    msg << " " << str;
  }
  return msg.str();
}

} // namespace

Exception::Exception() noexcept = default;

Exception::Exception(std::string str) noexcept
  : m_msg{std::move(str)}
{
}

const char* Exception::what() const noexcept
{
  return m_msg.c_str();
}

ParserException::ParserException(
  const std::optional<FileLocation>& location, const std::string& str)
  : Exception{buildMessage(location, str)}
{
}

} // namespace TrenchBroom
