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

#include <sstream>
#include <string_view>

namespace TrenchBroom
{
enum class LogLevel
{
  Debug,
  Info,
  Warn,
  Error
};

class Logger
{
public:
  class stream
  {
  private:
    Logger& m_logger;
    LogLevel m_logLevel;
    std::stringstream m_buf;

  public:
    stream(Logger& logger, LogLevel logLevel);
    ~stream();

  public:
    template <typename T>
    stream& operator<<(T&& arg)
    {
      m_buf << std::forward<T>(arg);
      return *this;
    }
  };

public:
  virtual ~Logger();

  stream debug();
  void debug(std::string_view message);

  stream info();
  void info(std::string_view message);

  stream warn();
  void warn(std::string_view message);

  stream error();
  void error(std::string_view message);

  void log(LogLevel level, std::string_view message);

private:
  virtual void doLog(LogLevel level, std::string_view message) = 0;
};

class NullLogger : public Logger
{
private:
  void doLog(LogLevel level, std::string_view message) override;
};
} // namespace TrenchBroom
