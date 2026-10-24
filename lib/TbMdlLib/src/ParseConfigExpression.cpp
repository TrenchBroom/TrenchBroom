/*
 Copyright (C) 2026 Kristian Duske

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

#include "mdl/ParseConfigExpression.h"

#include "el/EvaluationContext.h"
#include "el/ParseExpression.h"
#include "el/Value.h"

#include "kd/result.h"

namespace tb::mdl
{
namespace
{

bool hasVersion(const el::ExpressionNode& expression, const el::IntegerType version)
{
  return el::withEvaluationContext([&](auto& context) {
           return expression.evaluate(context)
                    .atOrDefault("version")
                    .convertTo(el::ValueType::Number)
                    .integerValue()
                  == version;
         })
         | kdl::value_or(false);
}

} // namespace

Result<el::ExpressionNode> parseConfigExpression(
  const std::string_view str, const el::IntegerType legacyVersion)
{
  return el::parseExpression(el::ParseMode::Strict, str)
         | kdl::and_then([&](auto expression) -> Result<el::ExpressionNode> {
             if (hasVersion(expression, legacyVersion))
             {
               return el::parseExpression(
                 el::ParseMode::Strict, str, el::StringEscaping::Legacy);
             }
             return expression;
           });
}

} // namespace tb::mdl
