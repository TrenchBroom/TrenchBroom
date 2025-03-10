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

#include "Interpolate.h"

#include "el/Expression.h"
#include "el/Value.h"
#include "io/ELParser.h"

#include "kdl/string_utils.h"

#include <sstream>
#include <string>

namespace tb::el
{

namespace
{

auto findExpressions(std::string_view str)
{
  auto result = std::vector<std::tuple<std::size_t, std::size_t>>{};
  size_t totalOffset = 0;
  while (auto part = kdl::str_find_next_delimited_string(str, "${", "}"))
  {
    if (!part->length)
    {
      throw ParserException{FileLocation{0, part->start}, "Unterminated expression"};
    }

    result.emplace_back(totalOffset + part->start, *part->length);
    str = str.substr(part->start + *part->length);
    totalOffset += part->start + *part->length;
  }
  return result;
}

auto parseExpressions(
  const std::string_view str,
  const std::vector<std::tuple<std::size_t, std::size_t>>& expressionsPositions)
{
  auto result = std::vector<el::ExpressionNode>{};
  result.reserve(expressionsPositions.size());
  for (const auto& [start, length] : expressionsPositions)
  {
    const auto expressionStr = str.substr(start + 2, length - 3);
    auto parser = io::ELParser{io::ELParser::Mode::Strict, expressionStr};
    result.push_back(parser.parse());
  }
  return result;
}

auto evaluateExpressions(
  const std::vector<el::ExpressionNode>& expressions, const EvaluationContext& context)
{
  auto result = std::vector<el::Value>{};
  result.reserve(expressions.size());
  for (const auto& expression : expressions)
  {
    result.push_back(expression.evaluate(context));
  }
  return result;
}

auto substituteValues(
  const std::string_view str,
  const std::vector<std::tuple<std::size_t, std::size_t>>& expressionsPositions,
  const std::vector<el::Value>& values)
{
  auto result = std::stringstream{};
  std::size_t previousEnd = 0;
  for (std::size_t i = 0; i < expressionsPositions.size(); ++i)
  {
    const auto [start, length] = expressionsPositions[i];
    result << str.substr(previousEnd, start - previousEnd);
    result << values[i].convertTo(el::ValueType::String).stringValue();
    previousEnd = start + length;
  }
  result << str.substr(previousEnd);
  return result.str();
}

} // namespace

std::string interpolate(const std::string_view str, const EvaluationContext& context)
{
  const auto expressionsPositions = findExpressions(str);
  const auto expressions = parseExpressions(str, expressionsPositions);
  const auto values = evaluateExpressions(expressions, context);
  return substituteValues(str, expressionsPositions, values);
}

} // namespace tb::el
