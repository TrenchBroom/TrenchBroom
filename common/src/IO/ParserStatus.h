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

#include <string>

namespace TrenchBroom
{
struct FileLocation;
class Logger;
enum class LogLevel;
} // namespace TrenchBroom

namespace TrenchBroom::IO
{
class ParserStatus
{
private:
  Logger& m_logger;
  std::string m_prefix;

protected:
  ParserStatus(Logger& logger, std::string prefix);

public:
  virtual ~ParserStatus();

public:
  void progress(double progress);

  void debug(const FileLocation& location, const std::string& str);
  void info(const FileLocation& location, const std::string& str);
  void warn(const FileLocation& location, const std::string& str);
  void error(const FileLocation& location, const std::string& str);
  [[noreturn]] void errorAndThrow(const FileLocation& location, const std::string& str);

  void debug(const std::string& str);
  void info(const std::string& str);
  void warn(const std::string& str);
  void error(const std::string& str);
  [[noreturn]] void errorAndThrow(const std::string& str);

private:
  void log(LogLevel level, const FileLocation& location, const std::string& str);
  std::string buildMessage(const FileLocation& location, const std::string& str) const;

  void log(LogLevel level, const std::string& str);
  std::string buildMessage(const std::string& str) const;

private:
  virtual void doProgress(double progress) = 0;
  virtual void doLog(LogLevel level, const std::string& str);
};

} // namespace TrenchBroom::IO
