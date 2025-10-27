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

#include "DefParser.h"

#include "FileLocation.h"
#include "io/EntityDefinitionClassInfo.h"
#include "io/LegacyModelDefinitionParser.h"
#include "io/ParseModelDefinition.h"
#include "io/ParserException.h"
#include "io/ParserStatus.h"
#include "mdl/EntityProperties.h"
#include "mdl/ModelDefinition.h"
#include "mdl/PropertyDefinition.h"

#include "kdl/string_format.h"

#include <fmt/format.h>

#include <string>
#include <vector>

namespace tb::io
{
namespace
{
auto tokenNames()
{
  using namespace DefToken;

  return Tokenizer<Type>::TokenNameMap{
    {Integer, "integer"},
    {Decimal, "decimal"},
    {QuotedString, "quoted string"},
    {OParenthesis, "'('"},
    {CParenthesis, "')'"},
    {OBrace, "'{'"},
    {CBrace, "'}'"},
    {Word, "word"},
    {ODefinition, "'/*'"},
    {CDefinition, "'*/'"},
    {Semicolon, "';'"},
    {Newline, "newline"},
    {Comma, "','"},
    {Equality, "'='"},
    {Minus, "'-'"},
    {Eof, "end of file"},
  };
}

} // namespace

DefTokenizer::DefTokenizer(const std::string_view str)
  : Tokenizer{tokenNames(), str, "", 0}
{
}

const std::string DefTokenizer::WordDelims = " \t\n\r()[]{};,=";

DefTokenizer::Token DefTokenizer::emitToken()
{
  while (!eof())
  {
    const auto startLine = line();
    const auto startColumn = column();
    const auto* c = curPos();
    switch (*c)
    {
    case '/': {
      if (lookAhead() == '*')
      {
        // eat all chars immediately after the '*' because it's often followed by QUAKE
        do
        {
          advance();
        } while (!eof() && !isWhitespace(curChar()));
        return Token{
          DefToken::ODefinition, c, curPos(), offset(c), startLine, startColumn};
      }
      else if (lookAhead() == '/')
      {
        discardUntil("\n\r");
        break;
      }
      // fall through and try to read as word
      switchFallthrough();
    }
    case '*': {
      if (lookAhead() == '/')
      {
        advance();
        return Token{
          DefToken::CDefinition, c, curPos(), offset(c), startLine, startColumn};
      }
      // fall through and try to read as word
      switchFallthrough();
    }
    case '(':
      advance();
      return Token{DefToken::OParenthesis, c, c + 1, offset(c), startLine, startColumn};
    case ')':
      advance();
      return Token{DefToken::CParenthesis, c, c + 1, offset(c), startLine, startColumn};
    case '{':
      advance();
      return Token{DefToken::OBrace, c, c + 1, offset(c), startLine, startColumn};
    case '}':
      advance();
      return Token{DefToken::CBrace, c, c + 1, offset(c), startLine, startColumn};
    case '=':
      advance();
      return Token{DefToken::Equality, c, c + 1, offset(c), startLine, startColumn};
    case ';':
      advance();
      return Token{DefToken::Semicolon, c, c + 1, offset(c), startLine, startColumn};
    case '\r':
      if (lookAhead() == '\n')
      {
        advance();
      }
      // handle carriage return without consecutive linefeed
      // by falling through into the line feed case
      switchFallthrough();
    case '\n':
      advance();
      return Token{DefToken::Newline, c, c + 1, offset(c), startLine, startColumn};
    case ',':
      advance();
      return Token{DefToken::Comma, c, c + 1, offset(c), startLine, startColumn};
    case ' ':
    case '\t':
      discardWhile(" \t");
      break;
    case '"': { // quoted string
      advance();
      c = curPos();
      const auto* e = readQuotedString();
      return Token{DefToken::QuotedString, c, e, offset(c), startLine, startColumn};
    }
    case '-':
      if (isWhitespace(lookAhead()))
      {
        advance();
        return Token{DefToken::Minus, c, c + 1, offset(c), startLine, startColumn};
      }
      // otherwise fallthrough, might be a negative number
      switchFallthrough();
    default: // integer, decimal or word
      if (const auto* e = readInteger(WordDelims))
      {
        return Token{DefToken::Integer, c, e, offset(c), startLine, startColumn};
      }
      if (const auto* e = readDecimal(WordDelims))
      {
        return Token{DefToken::Decimal, c, e, offset(c), startLine, startColumn};
      }
      if (const auto* e = readUntil(WordDelims))
      {
        return Token{DefToken::Word, c, e, offset(c), startLine, startColumn};
      }
      throw ParserException{
        FileLocation{startLine, startColumn}, fmt::format("Unexpected character: {}", c)};
    }
  }
  return Token{DefToken::Eof, nullptr, nullptr, length(), line(), column()};
}

DefParser::DefParser(std::string_view str, const Color& defaultEntityColor)
  : EntityDefinitionParser{defaultEntityColor}
  , m_tokenizer{DefTokenizer(str)}
{
}

std::vector<EntityDefinitionClassInfo> DefParser::parseClassInfos(ParserStatus& status)
{
  auto result = std::vector<EntityDefinitionClassInfo>{};

  auto classInfo = parseClassInfo(status);
  status.progress(m_tokenizer.progress());

  while (classInfo)
  {
    result.push_back(std::move(*classInfo));
    classInfo = parseClassInfo(status);
    status.progress(m_tokenizer.progress());
  }

  return result;
}

std::optional<EntityDefinitionClassInfo> DefParser::parseClassInfo(ParserStatus& status)
{
  auto token = m_tokenizer.skipAndNextToken(~(DefToken::Eof | DefToken::ODefinition));
  if (token.hasType(DefToken::Eof))
  {
    return std::nullopt;
  }

  auto classInfo = EntityDefinitionClassInfo{};
  classInfo.type = EntityDefinitionClassType::BaseClass;
  classInfo.location = token.location();

  token = m_tokenizer.nextToken(DefToken::Word);
  classInfo.name = token.data();

  token = m_tokenizer.peekToken(DefToken::OParenthesis | DefToken::Newline);
  if (token.type() == DefToken::OParenthesis)
  {
    classInfo.type = EntityDefinitionClassType::BrushClass;
    classInfo.color = parseColor();

    token = m_tokenizer.peekToken(DefToken::OParenthesis | DefToken::Word);
    if (token.hasType(DefToken::OParenthesis))
    {
      classInfo.size = parseBounds();
      classInfo.type = EntityDefinitionClassType::PointClass;
    }
    else if (token.data() == "?")
    {
      m_tokenizer.nextToken();
    }

    token = m_tokenizer.peekToken();
    if (token.hasType(DefToken::Word | DefToken::Minus))
    {
      if (!addPropertyDefinition(classInfo.propertyDefinitions, parseSpawnflags()))
      {
        status.warn(
          token.location(), "Skipping duplicate spawnflags property definition");
      }
    }
  }

  m_tokenizer.nextToken(DefToken::Newline);

  parseProperties(status, classInfo);
  classInfo.description = kdl::str_trim(parseDescription());

  m_tokenizer.nextToken(DefToken::CDefinition);

  return classInfo;
}

mdl::PropertyDefinition DefParser::parseSpawnflags()
{
  auto flags = std::vector<mdl::PropertyValueTypes::Flag>{};
  auto token = m_tokenizer.peekToken();
  while (token.hasType(DefToken::Word | DefToken::Minus))
  {
    token = m_tokenizer.nextToken();
    auto name = token.hasType(DefToken::Word) ? token.data() : "";
    const auto value = 1 << flags.size();
    flags.push_back(mdl::PropertyValueTypes::Flag{value, std::move(name), ""});
    token = m_tokenizer.peekToken();
  }

  return {
    mdl::EntityPropertyKeys::Spawnflags,
    mdl::PropertyValueTypes::Flags{std::move(flags)},
    "",
    ""};
}

void DefParser::parseProperties(
  ParserStatus& status, EntityDefinitionClassInfo& classInfo)
{
  if (m_tokenizer.peekToken().type() == DefToken::OBrace)
  {
    m_tokenizer.nextToken();
    while (parseProperty(status, classInfo))
    {
    }
  }
}

bool DefParser::parseProperty(ParserStatus& status, EntityDefinitionClassInfo& classInfo)
{
  auto token =
    m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::Word | DefToken::CBrace);
  if (!token.hasType(DefToken::Word))
  {
    return false;
  }

  const auto location = token.location();
  const auto typeName = token.data();
  if (typeName == "default")
  {
    // ignore these properties
    parseDefaultProperty();
  }
  else if (typeName == "base")
  {
    classInfo.superClasses.push_back(parseBaseProperty());
  }
  else if (typeName == "choice")
  {
    auto propertyDefinition = parseChoicePropertyDefinition();
    if (!addPropertyDefinition(classInfo.propertyDefinitions, propertyDefinition))
    {
      status.warn(
        location,
        fmt::format(
          "Skipping duplicate property definition: {}", propertyDefinition.key));
    }
  }
  else if (typeName == "model")
  {
    classInfo.modelDefinition = parseModelDefinition(status);
  }

  m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::Semicolon);
  return true;
}

void DefParser::parseDefaultProperty()
{
  // Token token;
  m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::OParenthesis);
  m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::QuotedString);
  // const std::string propertyName = token.data();
  m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::Comma);
  m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::QuotedString);
  // const std::string propertyValue = token.data();
  m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::CParenthesis);
}

std::string DefParser::parseBaseProperty()
{
  m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::OParenthesis);
  auto token = m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::QuotedString);
  const auto basename = token.data();
  m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::CParenthesis);

  return basename;
}

mdl::PropertyDefinition DefParser::parseChoicePropertyDefinition()
{
  auto token = m_tokenizer.nextToken(DefToken::QuotedString);
  auto propertyKey = token.data();

  auto options = std::vector<mdl::PropertyValueTypes::ChoiceOption>{};
  m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::OParenthesis);
  token = m_tokenizer.skipAndNextToken(DefToken::Newline);
  while (token.type() == DefToken::OParenthesis)
  {
    token = m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::Integer);
    auto value = token.data();

    m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::Comma);
    token = m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::QuotedString);
    auto name = token.data();
    options.push_back(
      mdl::PropertyValueTypes::ChoiceOption{std::move(value), std::move(name)});

    m_tokenizer.skipAndNextToken(DefToken::Newline, DefToken::CParenthesis);
    token = m_tokenizer.skipAndNextToken(
      DefToken::Newline, DefToken::OParenthesis | DefToken::CParenthesis);
  }

  return {
    std::move(propertyKey), mdl::PropertyValueTypes::Choice{std::move(options)}, "", ""};
}

mdl::ModelDefinition DefParser::parseModelDefinition(ParserStatus& status)
{
  m_tokenizer.nextToken(DefToken::OParenthesis);

  return io::parseModelDefinition(m_tokenizer, status, DefToken::CParenthesis)
         | kdl::if_error([](const auto& e) { throw ParserException{e.msg}; })
         | kdl::value();
}

std::string DefParser::parseDescription()
{
  auto token = m_tokenizer.peekToken();
  return token.type() != DefToken::CDefinition
           ? std::string{m_tokenizer.readRemainder(DefToken::CDefinition)}
           : "";
}

vm::vec3d DefParser::parseVector()
{
  auto vec = vm::vec3d{};
  for (size_t i = 0; i < 3; i++)
  {
    auto token = m_tokenizer.nextToken(DefToken::Integer | DefToken::Decimal);
    vec[i] = token.toFloat<double>();
  }
  return vec;
}

vm::bbox3d DefParser::parseBounds()
{
  auto bounds = vm::bbox3d{};
  m_tokenizer.nextToken(DefToken::OParenthesis);
  bounds.min = parseVector();
  m_tokenizer.nextToken(DefToken::CParenthesis);
  m_tokenizer.nextToken(DefToken::OParenthesis);
  bounds.max = parseVector();
  m_tokenizer.nextToken(DefToken::CParenthesis);
  return repair(bounds);
}

Color DefParser::parseColor()
{
  auto token = m_tokenizer.nextToken(DefToken::OParenthesis);
  const auto location = token.location();

  auto vec = vm::vec3f{};
  for (size_t i = 0; i < 3; i++)
  {
    token = m_tokenizer.nextToken(DefToken::Decimal | DefToken::Integer);
    vec[i] = token.toFloat<float>();
  }
  m_tokenizer.nextToken(DefToken::CParenthesis);

  return Color::fromVec(vec)
         | kdl::if_error([&](const auto& e) { throw ParserException{location, e.msg}; })
         | kdl::value();
}

} // namespace tb::io
