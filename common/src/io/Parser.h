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

#pragma once

#include "Exceptions.h"
#include "io/Token.h"

#include "kdl/string_utils.h"

#include <fmt/format.h>

#include <string>
#include <vector>

namespace tb::io
{
class ParserStatus;

template <typename TokenType>
class Parser
{
private:
  using Token = TokenTemplate<TokenType>;

public:
  virtual ~Parser() = default;

protected:
  void expect(const std::string& expected, const Token& token) const
  {
    if (token.data() != expected)
    {
      throw ParserException{
        token.location(),
        fmt::format("Expected string '{}', but got '{}'", expected, token.data())};
    }
  }

  void expect(const std::vector<std::string>& expected, const Token& token) const
  {
    for (const auto& str : expected)
    {
      if (token.data() == str)
      {
        return;
      }
    }
    throw ParserException{
      token.location(),
      fmt::format(
        "Expected string '{}', but got '{}'",
        kdl::str_join(expected, "', '", "', or '", "' or '"),
        token.data())};
  }
};

} // namespace tb::io
