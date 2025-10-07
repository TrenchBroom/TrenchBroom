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

#include "LegacyModelDefinitionParser.h"

#include "el/Expression.h"
#include "el/Value.h"
#include "io/ParserException.h"
#include "io/ParserStatus.h"
#include "mdl/ModelDefinition.h"

#include "kdl/string_compare.h"

#include <fmt/format.h>

#include <algorithm>
#include <string>

namespace tb::io
{
namespace
{

auto tokenNames()
{
  using namespace MdlToken;

  return LegacyModelDefinitionTokenizer::TokenNameMap{
    {Integer, "integer"},
    {String, "quoted string"},
    {Word, "word"},
    {Comma, "','"},
    {Equality, "'='"},
    {CParenthesis, "')'"},
    {Eof, "end of file"},
  };
}

} // namespace

LegacyModelDefinitionTokenizer::LegacyModelDefinitionTokenizer(
  const std::string_view str, const size_t line, const size_t column)
  : Tokenizer{tokenNames(), str, "", 0, line, column}
{
}

const std::string LegacyModelDefinitionTokenizer::WordDelims = " \t\n\r()[]{};,=";

LegacyModelDefinitionTokenizer::Token LegacyModelDefinitionTokenizer::emitToken()
{
  while (!eof())
  {
    const auto startLine = line();
    const auto startColumn = column();
    const auto startLocation = location();
    const auto* c = curPos();
    switch (*c)
    {
    case '=':
      advance();
      return Token{MdlToken::Equality, c, c + 1, offset(c), startLine, startColumn};
    case ')':
      advance();
      return Token{MdlToken::CParenthesis, c, c + 1, offset(c), startLine, startColumn};
    case ',':
      advance();
      return Token{MdlToken::Comma, c, c + 1, offset(c), startLine, startColumn};
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      advance();
      break;
    case '"':
      advance();
      c = curPos();
      return Token{
        MdlToken::String, c, readQuotedString(), offset(c), startLine, startColumn};
    default: {
      if (const auto* e = readInteger(WordDelims))
      {
        return Token{MdlToken::Integer, c, e, offset(c), startLine, startColumn};
      }
      if (const auto* e = readUntil(WordDelims))
      {
        return Token{MdlToken::Word, c, e, offset(c), startLine, startColumn};
      }
      throw ParserException{startLocation, fmt::format("Unexpected character: {}", *c)};
    }
    }
  }
  return Token{MdlToken::Eof, nullptr, nullptr, length(), line(), column()};
}

LegacyModelDefinitionParser::LegacyModelDefinitionParser(
  const std::string_view str, const size_t line, const size_t column)
  : m_tokenizer{str, line, column}
{
}

TokenizerState LegacyModelDefinitionParser::tokenizerState() const
{
  return m_tokenizer.snapshot();
}

Result<el::ExpressionNode> LegacyModelDefinitionParser::parse(ParserStatus& status)
{
  try
  {
    return parseModelDefinition(status);
  }
  catch (const ParserException& e)
  {
    return Error{e.what()};
  }
}

el::ExpressionNode LegacyModelDefinitionParser::parseModelDefinition(ParserStatus& status)
{
  auto token =
    m_tokenizer.peekToken(MdlToken::String | MdlToken::Word | MdlToken::CParenthesis);
  const auto startLocation = token.location();

  if (token.hasType(MdlToken::CParenthesis))
  {
    return el::ExpressionNode{
      el::LiteralExpression{el::Value::Undefined}, token.location()};
  }

  auto modelExpressions = std::vector<el::ExpressionNode>{};
  do
  {
    token = m_tokenizer.peekToken(MdlToken::String | MdlToken::Word);
    if (token.hasType(MdlToken::String))
    {
      modelExpressions.push_back(parseStaticModelDefinition());
    }
    else
    {
      modelExpressions.push_back(parseDynamicModelDefinition(status));
    }

    token = m_tokenizer.peekToken(MdlToken::Comma | MdlToken::CParenthesis);
    if (token.hasType(MdlToken::Comma))
    {
      m_tokenizer.nextToken();
    }
  } while (token.hasType(MdlToken::Comma));

  // The legacy model expressions are evaluated back to front.
  std::ranges::reverse(modelExpressions);
  return el::ExpressionNode{
    el::SwitchExpression{std::move(modelExpressions)}, startLocation};
}

el::ExpressionNode LegacyModelDefinitionParser::parseStaticModelDefinition()
{
  auto token = m_tokenizer.nextToken(MdlToken::String);
  const auto startLocation = token.location();

  auto map = el::MapType{};
  map[mdl::ModelSpecificationKeys::Path] = el::Value(token.data());

  auto indices = std::vector<size_t>{};

  token = m_tokenizer.peekToken(
    MdlToken::Integer | MdlToken::Word | MdlToken::Comma | MdlToken::CParenthesis);
  if (token.hasType(MdlToken::Integer))
  {
    token = m_tokenizer.nextToken();
    indices.push_back(token.toInteger<size_t>());
    token = m_tokenizer.peekToken(
      MdlToken::Integer | MdlToken::Word | MdlToken::Comma | MdlToken::CParenthesis);
    if (token.hasType(MdlToken::Integer))
    {
      token = m_tokenizer.nextToken();
      indices.push_back(token.toInteger<size_t>());
      token =
        m_tokenizer.peekToken(MdlToken::Word | MdlToken::Comma | MdlToken::CParenthesis);
    }
  }

  if (!indices.empty())
  {
    map[mdl::ModelSpecificationKeys::Skin] = el::Value(indices[0]);
    if (indices.size() > 1)
    {
      map[mdl::ModelSpecificationKeys::Frame] = el::Value(indices[1]);
    }
  }

  auto modelExpression =
    el::ExpressionNode{el::LiteralExpression{el::Value{std::move(map)}}, startLocation};

  if (token.hasType(MdlToken::Word))
  {
    token = m_tokenizer.nextToken();

    auto attributeKey = token.data();
    const auto location = token.location();
    auto keyExpression =
      el::ExpressionNode{el::VariableExpression{std::move(attributeKey)}, location};

    m_tokenizer.nextToken(MdlToken::Equality);
    token = m_tokenizer.nextToken(MdlToken::String | MdlToken::Integer);
    if (token.hasType(MdlToken::String))
    {
      auto attributeValue = token.data();
      auto valueExpression = el::ExpressionNode{
        el::LiteralExpression{el::Value{std::move(attributeValue)}}, token.location()};
      auto premiseExpression = el::ExpressionNode{
        el::BinaryExpression{
          el::BinaryOperation::Equal,
          std::move(keyExpression),
          std::move(valueExpression)},
        location};

      return el::ExpressionNode{
        el::BinaryExpression{
          el::BinaryOperation::Case,
          std::move(premiseExpression),
          std::move(modelExpression)},
        startLocation};
    }
    const auto flagValue = token.toInteger<int>();
    auto valueExpression =
      el::ExpressionNode{el::LiteralExpression{el::Value{flagValue}}, token.location()};
    auto premiseExpression = el::ExpressionNode{
      el::BinaryExpression{
        el::BinaryOperation::Equal, std::move(keyExpression), std::move(valueExpression)},
      location};

    return el::ExpressionNode{
      el::BinaryExpression{
        el::BinaryOperation::Case,
        std::move(premiseExpression),
        std::move(modelExpression)},
      startLocation};
  }

  return modelExpression;
}

el::ExpressionNode LegacyModelDefinitionParser::parseDynamicModelDefinition(
  ParserStatus& status)
{
  auto token = m_tokenizer.peekToken();
  const auto location = token.location();

  auto map = std::map<std::string, el::ExpressionNode>{
    {mdl::ModelSpecificationKeys::Path, parseNamedValue("pathKey")},
  };

  token = m_tokenizer.peekToken(MdlToken::Word | MdlToken::CParenthesis);
  if (!token.hasType(MdlToken::CParenthesis))
  {
    do
    {
      if (kdl::ci::str_is_equal("skinKey", token.data()))
      {
        map.emplace(mdl::ModelSpecificationKeys::Skin, parseNamedValue("skinKey"));
      }
      else if (kdl::ci::str_is_equal("frameKey", token.data()))
      {
        map.emplace(mdl::ModelSpecificationKeys::Frame, parseNamedValue("frameKey"));
      }
      else
      {
        const auto msg =
          fmt::format("Expected 'skinKey' or 'frameKey', but found '{}'", token.data());
        status.error(token.location(), msg);
        throw ParserException{token.location(), msg};
      }
      token = m_tokenizer.peekToken(MdlToken::Word | MdlToken::CParenthesis);
    } while (token.hasType(MdlToken::Word));
  }

  return el::ExpressionNode{el::MapExpression{std::move(map)}, location};
}

el::ExpressionNode LegacyModelDefinitionParser::parseNamedValue(const std::string& name)
{
  auto token = Token{};
  token = m_tokenizer.nextToken(MdlToken::Word);

  const auto location = token.location();
  if (!kdl::ci::str_is_equal(name, token.data()))
  {
    throw ParserException{
      location, fmt::format("Expected '{}', but got '{}'", name, token.data())};
  }

  token = m_tokenizer.nextToken(MdlToken::Equality);
  token = m_tokenizer.nextToken(MdlToken::String);

  return el::ExpressionNode{el::VariableExpression{token.data()}, location};
}

} // namespace tb::io
