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

#pragma once

#include "ParserException.h"
#include "ParserStatus.h"
#include "el/ELParser.h"
#include "io/LegacyModelDefinitionParser.h"
#include "mdl/ModelDefinition.h"

#include <fmt/format.h>

namespace tb::io
{
template <typename TokenType>
Result<el::ExpressionNode> ensureNextToken(
  Tokenizer<TokenType>& tokenizer,
  const TokenType expectedTokenType,
  el::ExpressionNode expression)
{
  try
  {
    tokenizer.nextToken(expectedTokenType);
    return expression;
  }
  catch (const ParserException& e)
  {
    return Error{e.what()};
  }
}

template <typename TokenType>
auto parseElModelExpression(
  Tokenizer<TokenType>& tokenizer, const FileLocation& location, const TokenType cToken)
{
  const auto line = location.line;
  const auto column = *location.column;

  auto parser =
    el::ELParser{el::ELParser::Mode::Lenient, tokenizer.remainder(), line, column};
  return parser.parse() | kdl::and_then([&](auto expression) {
           // advance our tokenizer by the amount that the `parser` parsed
           tokenizer.adoptState(parser.tokenizerState());
           return ensureNextToken(tokenizer, cToken, std::move(expression));
         });
}

template <typename TokenType>
auto parseLegacyModelExpression(
  Tokenizer<TokenType>& tokenizer,
  ParserStatus& status,
  const FileLocation& location,
  const TokenType cToken)
{
  const auto line = location.line;
  const auto column = *location.column;

  auto parser = LegacyModelDefinitionParser{tokenizer.remainder(), line, column};
  return parser.parse(status) | kdl::and_then([&](auto expression) {
           // advance our tokenizer by the amount that `parser` parsed
           tokenizer.adoptState(parser.tokenizerState());
           return ensureNextToken(tokenizer, cToken, std::move(expression));
         })
         | kdl::transform([&](auto expression) {
             status.warn(
               location,
               fmt::format(
                 "Legacy model expressions are deprecated, replace with '{}'",
                 expression.asString()));

             return expression;
           });
}

Result<el::ExpressionNode> optimizeModelExpression(const el::ExpressionNode& expression);

template <typename TokenType>
auto parseModelDefinition(
  Tokenizer<TokenType>& tokenizer, ParserStatus& status, const TokenType cToken)
{
  const auto snapshot = tokenizer.snapshot();
  const auto location = tokenizer.location();

  return parseElModelExpression(tokenizer, location, cToken)
         | kdl::or_else([&](auto elParseError) {
             // parsing as EL model failed, restore and try as legacy model
             tokenizer.restore(snapshot);
             return parseLegacyModelExpression(tokenizer, status, location, cToken)
                    | kdl::or_else([&](const auto&) {
                        // parsing as legacy model also failed, restore and return the
                        // original parse error
                        tokenizer.restore(snapshot);
                        return Result<el::ExpressionNode>{std::move(elParseError)};
                      });
           })
         | kdl::and_then(optimizeModelExpression) | kdl::transform([](auto expression) {
             return mdl::ModelDefinition{std::move(expression)};
           });
}

} // namespace tb::io
