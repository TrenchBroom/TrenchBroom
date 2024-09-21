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
#include "IO/Token.h"

#include "kdl/range_utils.h"
#include "kdl/string_utils.h"

#include <fmt/format.h>

#include <map>
#include <ranges>
#include <string>
#include <vector>

namespace TrenchBroom::IO
{
class ParserStatus;

template <typename TokenType>
class Parser
{
protected:
  using TokenNameMap = std::map<TokenType, std::string>;

private:
  using Token = TokenTemplate<TokenType>;
  mutable TokenNameMap m_tokenNames;

public:
  virtual ~Parser() = default;

protected:
  bool check(const TokenType typeMask, const Token& token) const
  {
    return token.hasType(typeMask);
  }

  const Token& expect(const TokenType typeMask, const Token& token) const
  {
    if (!check(typeMask, token))
    {
      throw ParserException{token.location(), expectString(tokenName(typeMask), token)};
    }
    return token;
  }

  const Token& expect(
    ParserStatus& status, const TokenType typeMask, const Token& token) const
  {
    if (!check(typeMask, token))
    {
      expect(status, tokenName(typeMask), token);
    }
    return token;
  }

  void expect(
    ParserStatus& /* status */, const std::string& typeName, const Token& token) const
  {
    const auto msg = expectString(typeName, token);
    throw ParserException{token.location(), msg};
  }

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

private:
  std::string expectString(const std::string& expected, const Token& token) const
  {
    return fmt::format(
      "Expected {}, but got {} (raw data: '{}')",
      expected,
      tokenName(token.type()),
      token.data());
  }

protected:
  std::string tokenName(const TokenType typeMask) const
  {
    if (m_tokenNames.empty())
    {
      m_tokenNames = tokenNames();
    }

    const auto filterByType = std::views::filter(
      [&typeMask](const auto& pair) { return (typeMask & pair.first) != 0; });

    const auto names = m_tokenNames | filterByType | std::views::values
                       | kdl::to<std::vector<std::string>>();
    return names.empty()       ? "unknown token type"
           : names.size() == 1 ? names[0]
                               : kdl::str_join(names, ", ", ", or ", " or ");
  }

private:
  virtual TokenNameMap tokenNames() const = 0;
};

} // namespace TrenchBroom::IO
