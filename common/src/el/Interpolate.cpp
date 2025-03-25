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

#include "kdl/result_fold.h"
#include "kdl/string_utils.h"

#include <ranges>
#include <sstream>
#include <string>

namespace tb::el
{

namespace
{

Result<std::vector<std::tuple<std::size_t, std::size_t>>> findExpressions(
  std::string_view str)
{
  auto result = std::vector<std::tuple<std::size_t, std::size_t>>{};
  size_t totalOffset = 0;
  while (auto part = kdl::str_find_next_delimited_string(str, "${", "}"))
  {
    if (!part->length)
    {
      return Error{fmt::format("At position {}: Unterminated expression", part->start)};
    }

    result.emplace_back(totalOffset + part->start, *part->length);
    str = str.substr(part->start + *part->length);
    totalOffset += part->start + *part->length;
  }
  return result;
}

auto parseExpressions(
  const std::string_view str,
  const std::vector<std::tuple<std::size_t, std::size_t>>& expressionPositions)
{
  return expressionPositions | std::views::transform([&](const auto& expressionPosition) {
           const auto [start, length] = expressionPosition;
           const auto expressionStr = str.substr(start + 2, length - 3);
           auto parser = io::ELParser{io::ELParser::Mode::Strict, expressionStr};
           return parser.parse();
         })
         | kdl::fold;
}

auto evaluateExpressions(
  const std::vector<el::ExpressionNode>& expressions, EvaluationContext& context)
{
  return expressions
         | std::views::transform([&](const auto& expression) -> Result<el::Value> {
             try
             {
               return expression.evaluate(context);
             }
             catch (const Exception& e)
             {
               return Error{e.what()};
             }
           })
         | kdl::fold;
}

Result<std::string> substituteValues(
  const std::string_view str,
  const std::vector<std::tuple<std::size_t, std::size_t>>& expressionsPositions,
  const std::vector<el::Value>& values)
{
  try
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
  catch (const Exception& e)
  {
    return Error{e.what()};
  }
}

} // namespace

Result<std::string> interpolate(const std::string_view str, EvaluationContext& context)
{
  return findExpressions(str) | kdl::and_then([&](const auto& expressionPositions) {
           return parseExpressions(str, expressionPositions)
                  | kdl::and_then([&](const auto& expressions) {
                      return evaluateExpressions(expressions, context);
                    })
                  | kdl::and_then([&](const auto& values) {
                      return substituteValues(str, expressionPositions, values);
                    });
         });
}

} // namespace tb::el
