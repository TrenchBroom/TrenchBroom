/*
 Copyright (C) 2020 Kristian Duske

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

#include "Logger.h"

#include <unordered_map>
#include <vector>

namespace TrenchBroom {
class TestLogger : public Logger {
private:
  std::unordered_map<LogLevel, std::size_t> m_messages;

public:
  std::size_t countMessages() const;
  std::size_t countMessages(LogLevel level) const;

private:
  void doLog(LogLevel level, const std::string& message) override;
  void doLog(LogLevel level, const QString& message) override;
};
} // namespace TrenchBroom
