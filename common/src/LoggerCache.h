/*
 Copyright (C) 2024 Kristian Duske

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
#include <string_view>
#include <vector>

namespace TrenchBroom
{
enum class LogLevel;
}

namespace TrenchBroom::View
{

class LoggerCache
{
private:
  struct Message
  {
    LogLevel level;
    std::string str;
  };

  std::vector<Message> m_cachedMessages;

public:
  void cacheMessage(LogLevel level, std::string_view message);

  template <typename F>
  void getCachedMessages(const F& f)
  {
    for (const auto& message : m_cachedMessages)
    {
      f(message.level, message.str);
    }
    m_cachedMessages.clear();
  }
};

} // namespace TrenchBroom::View
