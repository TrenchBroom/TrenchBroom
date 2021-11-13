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

#include "TestParserStatus.h"

#include <string>

namespace TrenchBroom {
namespace IO {
NullLogger TestParserStatus::_logger;

TestParserStatus::TestParserStatus()
  : ParserStatus(_logger, "") {}

size_t TestParserStatus::countStatus(const LogLevel level) const {
  const auto it = m_messages.find(level);
  if (it == std::end(m_messages))
    return 0u;
  return it->second.size();
}

const std::vector<std::string>& TestParserStatus::messages(const LogLevel level) const {
  static const auto Empty = std::vector<std::string>();

  const auto it = m_messages.find(level);
  if (it == std::end(m_messages))
    return Empty;
  return it->second;
}

void TestParserStatus::doProgress(const double) {}

void TestParserStatus::doLog(const LogLevel level, const std::string& str) {
  m_messages[level].push_back(str);
}
} // namespace IO
} // namespace TrenchBroom
