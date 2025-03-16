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

#include "ParserStatus.h"

#include "FileLocation.h"
#include "Logger.h"
#include "io/ParserException.h"

#include <cassert>
#include <sstream>
#include <string>

namespace tb::io
{
ParserStatus::ParserStatus(Logger& logger, std::string prefix)
  : m_logger{logger}
  , m_prefix{std::move(prefix)}
{
}

ParserStatus::~ParserStatus() {}

void ParserStatus::progress(const double progress)
{
  assert(progress >= 0.0 && progress <= 1.0);
  doProgress(progress);
}

void ParserStatus::debug(const FileLocation& location, const std::string& str)
{
  log(LogLevel::Debug, location, str);
}

void ParserStatus::info(const FileLocation& location, const std::string& str)
{
  log(LogLevel::Info, location, str);
}

void ParserStatus::warn(const FileLocation& location, const std::string& str)
{
  log(LogLevel::Warn, location, str);
}

void ParserStatus::error(const FileLocation& location, const std::string& str)
{
  log(LogLevel::Error, location, str);
}

void ParserStatus::errorAndThrow(const FileLocation& location, const std::string& str)
{
  error(location, str);
  throw ParserException(buildMessage(location, str));
}

void ParserStatus::debug(const std::string& str)
{
  log(LogLevel::Debug, str);
}

void ParserStatus::info(const std::string& str)
{
  log(LogLevel::Info, str);
}

void ParserStatus::warn(const std::string& str)
{
  log(LogLevel::Warn, str);
}

void ParserStatus::error(const std::string& str)
{
  log(LogLevel::Error, str);
}

void ParserStatus::errorAndThrow(const std::string& str)
{
  error(str);
  throw ParserException(buildMessage(str));
}

void ParserStatus::log(
  const LogLevel level, const FileLocation& location, const std::string& str)
{
  doLog(level, buildMessage(location, str));
}

std::string ParserStatus::buildMessage(
  const FileLocation& location, const std::string& str) const
{
  auto msg = std::stringstream{};
  if (!m_prefix.empty())
  {
    msg << m_prefix << ": ";
  }
  msg << str << " (at " << location << ")";
  return msg.str();
}

void ParserStatus::log(const LogLevel level, const std::string& str)
{
  doLog(level, buildMessage(str));
}

std::string ParserStatus::buildMessage(const std::string& str) const
{
  auto msg = std::stringstream{};
  if (!m_prefix.empty())
  {
    msg << m_prefix << ": ";
  }
  msg << str << " (unknown location)";
  return msg.str();
}

void ParserStatus::doLog(const LogLevel level, const std::string& str)
{
  m_logger.log(level, str);
}

} // namespace tb::io
