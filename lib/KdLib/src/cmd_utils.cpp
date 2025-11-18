/*
 Copyright (C) 2025 Kristian Duske

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

#include "kd/cmd_utils.h"

namespace kdl
{

std::vector<std::string> cmd_parse_args(const std::string_view str)
{
  auto result = std::vector<std::string>{};

  auto quoted = false;
  auto escaped = false;

  auto s = str.begin();
  auto i = str.begin();
  while (i != str.end())
  {
    const auto c = *i;
    if (c == '\\' && !escaped)
    {
      escaped = true;
    }
    else
    {
      if (c == '"' && !escaped)
      {
        if (!quoted)
        {
          s = std::next(i);
        }
        else if (i > s)
        {
          result.emplace_back(s, i);
          s = std::next(i);
        }
        quoted = !quoted;
      }
      else if (c == ' ' && !quoted)
      {
        if (i > s)
        {
          result.emplace_back(s, i);
        }
        s = std::next(i);
      }
      escaped = false;
    }

    std::advance(i, 1);
  }

  if (i > s && !quoted)
  {
    result.emplace_back(s, i);
  }

  return result;
}

} // namespace kdl
