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

#include "Exceptions.h"
#include "FileLocation.h"
#include "el/ELExceptions.h"
#include "io/ELParser.h"
#include "io/EntityDefinitionClassInfo.h"
#include "io/LegacyModelDefinitionParser.h"
#include "io/ParserStatus.h"
#include "mdl/EntityProperties.h"
#include "mdl/ModelDefinition.h"
#include "mdl/PropertyDefinition.h"

#include "kdl/string_format.h"

#include <fmt/format.h>

#include <memory>
#include <string>
#include <vector>

namespace tb::io
{

DefTokenizer::DefTokenizer(std::string_view str)
  : Tokenizer{std::move(str), "", 0}
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

DefParser::TokenNameMap DefParser::tokenNames() const
{
  using namespace DefToken;

  return TokenNameMap{
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
  auto token = m_tokenizer.nextToken();
  while (token.type() != DefToken::Eof && token.type() != DefToken::ODefinition)
  {
    token = m_tokenizer.nextToken();
  }
  if (token.type() == DefToken::Eof)
  {
    return std::nullopt;
  }

  expect(status, DefToken::ODefinition, token);

  auto classInfo = EntityDefinitionClassInfo{};
  classInfo.type = EntityDefinitionClassType::BaseClass;
  classInfo.location = token.location();

  token = expect(status, DefToken::Word, m_tokenizer.nextToken());
  classInfo.name = token.data();

  token =
    expect(status, DefToken::OParenthesis | DefToken::Newline, m_tokenizer.peekToken());
  if (token.type() == DefToken::OParenthesis)
  {
    classInfo.type = EntityDefinitionClassType::BrushClass;
    classInfo.color = parseColor(status);

    token =
      expect(status, DefToken::OParenthesis | DefToken::Word, m_tokenizer.peekToken());
    if (token.hasType(DefToken::OParenthesis))
    {
      classInfo.size = parseBounds(status);
      classInfo.type = EntityDefinitionClassType::PointClass;
    }
    else if (token.data() == "?")
    {
      m_tokenizer.nextToken();
    }

    token = m_tokenizer.peekToken();
    if (token.hasType(DefToken::Word | DefToken::Minus))
    {
      if (!addPropertyDefinition(classInfo.propertyDefinitions, parseSpawnflags(status)))
      {
        status.warn(
          token.location(), "Skipping duplicate spawnflags property definition");
      }
    }
  }

  expect(status, DefToken::Newline, m_tokenizer.nextToken());

  parseProperties(status, classInfo);
  classInfo.description = kdl::str_trim(parseDescription());

  expect(status, DefToken::CDefinition, m_tokenizer.nextToken());

  return classInfo;
}

std::unique_ptr<mdl::PropertyDefinition> DefParser::parseSpawnflags(
  ParserStatus& /* status */)
{
  auto definition =
    std::make_unique<mdl::FlagsPropertyDefinition>(mdl::EntityPropertyKeys::Spawnflags);
  size_t numOptions = 0;

  auto token = m_tokenizer.peekToken();
  while (token.hasType(DefToken::Word | DefToken::Minus))
  {
    token = m_tokenizer.nextToken();
    const auto name = token.hasType(DefToken::Word) ? token.data() : "";
    const auto value = 1 << numOptions++;
    definition->addOption(value, name, "", false);
    token = m_tokenizer.peekToken();
  }

  return definition;
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
    expect(status, DefToken::Word | DefToken::CBrace, nextTokenIgnoringNewlines());
  if (token.type() != DefToken::Word)
  {
    return false;
  }

  const auto location = token.location();
  const auto typeName = token.data();
  if (typeName == "default")
  {
    // ignore these properties
    parseDefaultProperty(status);
  }
  else if (typeName == "base")
  {
    classInfo.superClasses.push_back(parseBaseProperty(status));
  }
  else if (typeName == "choice")
  {
    auto propertyDefinition =
      std::shared_ptr<mdl::PropertyDefinition>{parseChoicePropertyDefinition(status)};
    if (!addPropertyDefinition(classInfo.propertyDefinitions, propertyDefinition))
    {
      status.warn(
        location,
        fmt::format(
          "Skipping duplicate property definition: {}", propertyDefinition->key()));
    }
  }
  else if (typeName == "model")
  {
    classInfo.modelDefinition = parseModelDefinition(status);
  }

  expect(status, DefToken::Semicolon, nextTokenIgnoringNewlines());
  return true;
}

void DefParser::parseDefaultProperty(ParserStatus& status)
{
  // Token token;
  expect(status, DefToken::OParenthesis, nextTokenIgnoringNewlines());
  expect(status, DefToken::QuotedString, nextTokenIgnoringNewlines());
  // const std::string propertyName = token.data();
  expect(status, DefToken::Comma, nextTokenIgnoringNewlines());
  expect(status, DefToken::QuotedString, nextTokenIgnoringNewlines());
  // const std::string propertyValue = token.data();
  expect(status, DefToken::CParenthesis, nextTokenIgnoringNewlines());
}

std::string DefParser::parseBaseProperty(ParserStatus& status)
{
  expect(status, DefToken::OParenthesis, nextTokenIgnoringNewlines());
  auto token = expect(status, DefToken::QuotedString, nextTokenIgnoringNewlines());
  const auto basename = token.data();
  expect(status, DefToken::CParenthesis, nextTokenIgnoringNewlines());

  return basename;
}

std::unique_ptr<mdl::PropertyDefinition> DefParser::parseChoicePropertyDefinition(
  ParserStatus& status)
{
  auto token = expect(status, DefToken::QuotedString, m_tokenizer.nextToken());
  auto propertyKey = token.data();

  mdl::ChoicePropertyOption::List options;
  expect(status, DefToken::OParenthesis, nextTokenIgnoringNewlines());
  token = nextTokenIgnoringNewlines();
  while (token.type() == DefToken::OParenthesis)
  {
    token = expect(status, DefToken::Integer, nextTokenIgnoringNewlines());
    auto name = token.data();

    expect(status, DefToken::Comma, nextTokenIgnoringNewlines());
    token = expect(status, DefToken::QuotedString, nextTokenIgnoringNewlines());
    auto value = token.data();
    options.emplace_back(std::move(name), std::move(value));

    expect(status, DefToken::CParenthesis, nextTokenIgnoringNewlines());
    token = nextTokenIgnoringNewlines();
  }

  expect(status, DefToken::CParenthesis, token);

  return std::make_unique<mdl::ChoicePropertyDefinition>(
    std::move(propertyKey), "", "", std::move(options), false);
}

mdl::ModelDefinition DefParser::parseModelDefinition(ParserStatus& status)
{
  expect(status, DefToken::OParenthesis, m_tokenizer.nextToken());

  const auto snapshot = m_tokenizer.snapshot();
  const auto line = m_tokenizer.line();
  const auto column = m_tokenizer.column();
  const auto location = m_tokenizer.location();

  try
  {
    auto parser =
      ELParser{ELParser::Mode::Lenient, m_tokenizer.remainder(), line, column};
    auto expression = parser.parse();

    // advance our tokenizer by the amount that the `parser` parsed
    m_tokenizer.adoptState(parser.tokenizerState());
    expect(status, DefToken::CParenthesis, m_tokenizer.nextToken());

    expression.optimize();
    return mdl::ModelDefinition{std::move(expression)};
  }
  catch (const ParserException& e)
  {
    try
    {
      m_tokenizer.restore(snapshot);

      auto parser = LegacyModelDefinitionParser{m_tokenizer.remainder(), line, column};
      auto expression = parser.parse(status);

      // advance our tokenizer by the amount that `parser` parsed
      m_tokenizer.adoptState(parser.tokenizerState());
      expect(status, DefToken::CParenthesis, m_tokenizer.nextToken());

      expression.optimize();
      status.warn(
        location,
        fmt::format(
          "Legacy model expressions are deprecated, replace with '{}'",
          expression.asString()));
      return mdl::ModelDefinition{std::move(expression)};
    }
    catch (const ParserException&)
    {
      m_tokenizer.restore(snapshot);
      throw e;
    }
  }
  catch (const el::EvaluationError& evaluationError)
  {
    throw ParserException{location, evaluationError.what()};
  }
}

std::string DefParser::parseDescription()
{
  auto token = m_tokenizer.peekToken();
  return token.type() != DefToken::CDefinition
           ? std::string{m_tokenizer.readRemainder(DefToken::CDefinition)}
           : "";
}

vm::vec3d DefParser::parseVector(ParserStatus& status)
{
  auto vec = vm::vec3d{};
  for (size_t i = 0; i < 3; i++)
  {
    auto token =
      expect(status, DefToken::Integer | DefToken::Decimal, m_tokenizer.nextToken());
    vec[i] = token.toFloat<double>();
  }
  return vec;
}

vm::bbox3d DefParser::parseBounds(ParserStatus& status)
{
  auto bounds = vm::bbox3d{};
  expect(status, DefToken::OParenthesis, m_tokenizer.nextToken());
  bounds.min = parseVector(status);
  expect(status, DefToken::CParenthesis, m_tokenizer.nextToken());
  expect(status, DefToken::OParenthesis, m_tokenizer.nextToken());
  bounds.max = parseVector(status);
  expect(status, DefToken::CParenthesis, m_tokenizer.nextToken());
  return repair(bounds);
}

Color DefParser::parseColor(ParserStatus& status)
{
  auto color = Color{};
  expect(status, DefToken::OParenthesis, m_tokenizer.nextToken());
  for (size_t i = 0; i < 3; i++)
  {
    const auto token =
      expect(status, DefToken::Decimal | DefToken::Integer, m_tokenizer.nextToken());
    color[i] = token.toFloat<float>();
    if (color[i] > 1.0f)
    {
      color[i] /= 255.0f;
    }
  }
  expect(status, DefToken::CParenthesis, m_tokenizer.nextToken());
  color[3] = 1.0f;
  return color;
}

DefParser::Token DefParser::nextTokenIgnoringNewlines()
{
  auto token = m_tokenizer.nextToken();
  while (token.type() == DefToken::Newline)
  {
    token = m_tokenizer.nextToken();
  }
  return token;
}

} // namespace tb::io
