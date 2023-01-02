/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "FgdParser.h"

#include "Assets/PropertyDefinition.h"
#include "EL/ELExceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/ELParser.h"
#include "IO/EntityDefinitionClassInfo.h"
#include "IO/File.h"
#include "IO/LegacyModelDefinitionParser.h"
#include "IO/ParserStatus.h"

#include <kdl/string_compare.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace IO
{
FgdTokenizer::FgdTokenizer(std::string_view str)
  : Tokenizer(std::move(str), "", 0)
{
}

const std::string FgdTokenizer::WordDelims = " \t\n\r()[]?;:,=";

FgdTokenizer::Token FgdTokenizer::emitToken()
{
  while (!eof())
  {
    auto startLine = line();
    auto startColumn = column();
    const auto* c = curPos();

    switch (*c)
    {
    case '/':
      advance();
      if (curChar() == '/')
      {
        discardUntil("\n\r");
      }
      break;
    case '(':
      advance();
      return Token{FgdToken::OParenthesis, c, c + 1, offset(c), startLine, startColumn};
    case ')':
      advance();
      return Token{FgdToken::CParenthesis, c, c + 1, offset(c), startLine, startColumn};
    case '[':
      advance();
      return Token{FgdToken::OBracket, c, c + 1, offset(c), startLine, startColumn};
    case ']':
      advance();
      return Token{FgdToken::CBracket, c, c + 1, offset(c), startLine, startColumn};
    case '=':
      advance();
      return Token{FgdToken::Equality, c, c + 1, offset(c), startLine, startColumn};
    case ',':
      advance();
      return Token{FgdToken::Comma, c, c + 1, offset(c), startLine, startColumn};
    case ':':
      advance();
      return Token{FgdToken::Colon, c, c + 1, offset(c), startLine, startColumn};
    case '"': { // quoted string
      advance();
      c = curPos();
      const auto* e = readQuotedString();
      return Token{FgdToken::String, c, e, offset(c), startLine, startColumn};
    }
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      discardWhile(Whitespace());
      break;
    case '+': { // string continuation
      const auto snapshot = this->snapshot();
      advance();

      const auto* e = curPos();
      discardWhile(Whitespace());

      if (curChar() == '"')
      {
        return Token{FgdToken::Plus, c, e, offset(c), startLine, startColumn};
      }
      else
      {
        restore(snapshot);
        // fall through to allow reading numbers
      }
      switchFallthrough();
    }
    default: {
      const auto* e = readInteger(WordDelims);
      if (e != nullptr)
      {
        return Token{FgdToken::Integer, c, e, offset(c), startLine, startColumn};
      }

      e = readDecimal(WordDelims);
      if (e != nullptr)
      {
        return Token{FgdToken::Decimal, c, e, offset(c), startLine, startColumn};
      }

      e = readUntil(WordDelims);
      if (e == nullptr)
      {
        throw ParserException(
          startLine, startColumn, "Unexpected character: '" + std::string(c, 1) + "'");
      }
      else
      {
        return Token{FgdToken::Word, c, e, offset(c), startLine, startColumn};
      }
    }
    }
  }
  return Token{FgdToken::Eof, nullptr, nullptr, length(), line(), column()};
}

FgdParser::FgdParser(
  std::string_view str, const Color& defaultEntityColor, const Path& path)
  : EntityDefinitionParser{defaultEntityColor}
  , m_tokenizer{FgdTokenizer{std::move(str)}}
{
  if (!path.isEmpty() && path.isAbsolute())
  {
    m_fs = std::make_shared<DiskFileSystem>(path.deleteLastComponent());
    pushIncludePath(path.lastComponent());
  }
}

FgdParser::FgdParser(std::string_view str, const Color& defaultEntityColor)
  : FgdParser{std::move(str), defaultEntityColor, Path{}}
{
}

FgdParser::TokenNameMap FgdParser::tokenNames() const
{
  using namespace FgdToken;

  return {
    {Integer, "integer"},
    {Decimal, "decimal"},
    {Word, "word"},
    {String, "string"},
    {OParenthesis, "'('"},
    {CParenthesis, "')'"},
    {OBracket, "'['"},
    {CBracket, "']'"},
    {Equality, "'='"},
    {Colon, "':'"},
    {Comma, "','"},
    {Plus, "'+'"},
    {Eof, "end of file"},
  };
}

class FgdParser::PushIncludePath
{
private:
  FgdParser* m_parser;

public:
  PushIncludePath(FgdParser* parser, const Path& path)
    : m_parser{parser}
  {
    m_parser->pushIncludePath(path);
  }

  ~PushIncludePath() { m_parser->popIncludePath(); }
};

void FgdParser::pushIncludePath(const Path& path)
{
  assert(!isRecursiveInclude(path));
  m_paths.push_back(path);
}

void FgdParser::popIncludePath()
{
  assert(!m_paths.empty());
  m_paths.pop_back();
}

Path FgdParser::currentRoot() const
{
  if (!m_paths.empty())
  {
    assert(!m_paths.back().isEmpty());
    return m_paths.back().deleteLastComponent();
  }
  else
  {
    return Path{};
  }
}

bool FgdParser::isRecursiveInclude(const Path& path) const
{
  return std::any_of(m_paths.begin(), m_paths.end(), [&](const auto& includedPath) {
    return includedPath == path;
  });
}

std::vector<EntityDefinitionClassInfo> FgdParser::parseClassInfos(ParserStatus& status)
{
  auto classInfos = std::vector<EntityDefinitionClassInfo>{};
  auto token = m_tokenizer.peekToken();
  while (!token.hasType(FgdToken::Eof))
  {
    parseClassInfoOrInclude(status, classInfos);
    token = m_tokenizer.peekToken();
  }
  return classInfos;
}

void FgdParser::parseClassInfoOrInclude(
  ParserStatus& status, std::vector<EntityDefinitionClassInfo>& classInfos)
{
  const auto token =
    expect(status, FgdToken::Eof | FgdToken::Word, m_tokenizer.peekToken());
  if (token.hasType(FgdToken::Eof))
  {
    return;
  }

  if (kdl::ci::str_is_equal(token.data(), "@include"))
  {
    auto includedClassInfos = parseInclude(status);
    classInfos = kdl::vec_concat(classInfos, std::move(includedClassInfos));
  }
  else
  {
    if (auto classInfo = parseClassInfo(status))
    {
      classInfos.push_back(std::move(*classInfo));
    }
    status.progress(m_tokenizer.progress());
  }
}

std::optional<EntityDefinitionClassInfo> FgdParser::parseClassInfo(ParserStatus& status)
{
  const auto token = expect(status, FgdToken::Word, m_tokenizer.nextToken());

  const auto classname = token.data();
  if (kdl::ci::str_is_equal(classname, "@SolidClass"))
  {
    return parseSolidClassInfo(status);
  }
  else if (kdl::ci::str_is_equal(classname, "@PointClass"))
  {
    return parsePointClassInfo(status);
  }
  else if (kdl::ci::str_is_equal(classname, "@BaseClass"))
  {
    return parseBaseClassInfo(status);
  }
  else if (kdl::ci::str_is_equal(classname, "@Main"))
  {
    skipMainClass(status);
    return std::nullopt;
  }
  else
  {
    const auto msg = "Unknown entity definition class '" + classname + "'";
    status.error(token.line(), token.column(), msg);
    throw ParserException{token.line(), token.column(), msg};
  }
}

EntityDefinitionClassInfo FgdParser::parseSolidClassInfo(ParserStatus& status)
{
  const auto classInfo = parseClassInfo(status, EntityDefinitionClassType::BrushClass);
  if (classInfo.size)
  {
    status.warn(
      classInfo.line, classInfo.column, "Solid entity definition must not have a size");
  }
  if (classInfo.modelDefinition)
  {
    status.warn(
      classInfo.line,
      classInfo.column,
      "Solid entity definition must not have model definitions");
  }
  return classInfo;
}

EntityDefinitionClassInfo FgdParser::parsePointClassInfo(ParserStatus& status)
{
  return parseClassInfo(status, EntityDefinitionClassType::PointClass);
}

EntityDefinitionClassInfo FgdParser::parseBaseClassInfo(ParserStatus& status)
{
  return parseClassInfo(status, EntityDefinitionClassType::BaseClass);
}

EntityDefinitionClassInfo FgdParser::parseClassInfo(
  ParserStatus& status, const EntityDefinitionClassType classType)
{
  auto token =
    expect(status, FgdToken::Word | FgdToken::Equality, m_tokenizer.nextToken());

  auto classInfo = EntityDefinitionClassInfo{};
  classInfo.type = classType;
  classInfo.line = token.line();
  classInfo.column = token.column();

  while (token.type() == FgdToken::Word)
  {
    const auto typeName = token.data();
    if (kdl::ci::str_is_equal(typeName, "base"))
    {
      if (!classInfo.superClasses.empty())
      {
        status.warn(token.line(), token.column(), "Found multiple base properties");
      }
      classInfo.superClasses = parseSuperClasses(status);
    }
    else if (kdl::ci::str_is_equal(typeName, "color"))
    {
      if (classInfo.color)
      {
        status.warn(token.line(), token.column(), "Found multiple color properties");
      }
      classInfo.color = parseColor(status);
    }
    else if (kdl::ci::str_is_equal(typeName, "size"))
    {
      if (classInfo.size)
      {
        status.warn(token.line(), token.column(), "Found multiple size properties");
      }
      classInfo.size = parseSize(status);
    }
    else if (
      kdl::ci::str_is_equal(typeName, "model")
      || kdl::ci::str_is_equal(typeName, "studio")
      || kdl::ci::str_is_equal(typeName, "studioprop")
      || kdl::ci::str_is_equal(typeName, "sprite")
      || kdl::ci::str_is_equal(typeName, "iconsprite"))
    {
      if (classInfo.modelDefinition)
      {
        status.warn(token.line(), token.column(), "Found multiple model properties");
      }
      classInfo.modelDefinition = parseModel(status);
    }
    else
    {
      status.warn(
        token.line(),
        token.column(),
        "Unknown entity definition header properties '" + typeName + "'");
      skipClassProperty(status);
    }
    token = expect(status, FgdToken::Equality | FgdToken::Word, m_tokenizer.nextToken());
  }

  token = expect(status, FgdToken::Word, m_tokenizer.nextToken());
  classInfo.name = token.data();

  token = expect(status, FgdToken::Colon | FgdToken::OBracket, m_tokenizer.peekToken());
  if (token.type() == FgdToken::Colon)
  {
    m_tokenizer.nextToken();
    const auto description = parseString(status);
    classInfo.description = kdl::str_trim(description);
  }

  classInfo.propertyDefinitions = parsePropertyDefinitions(status);

  return classInfo;
}

void FgdParser::skipMainClass(ParserStatus& status)
{
  expect(status, FgdToken::Equality, m_tokenizer.nextToken());
  expect(status, FgdToken::OBracket, m_tokenizer.nextToken());

  auto token = Token{};
  do
  {
    token = m_tokenizer.nextToken();
  } while (token.type() != FgdToken::CBracket);
}

std::vector<std::string> FgdParser::parseSuperClasses(ParserStatus& status)
{
  expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());

  auto token =
    expect(status, FgdToken::Word | FgdToken::CParenthesis, m_tokenizer.peekToken());

  auto superClasses = std::vector<std::string>{};
  if (token.type() == FgdToken::Word)
  {
    do
    {
      token = expect(status, FgdToken::Word, m_tokenizer.nextToken());
      superClasses.push_back(token.data());
      token =
        expect(status, FgdToken::Comma | FgdToken::CParenthesis, m_tokenizer.nextToken());
    } while (token.type() == FgdToken::Comma);
  }
  else
  {
    m_tokenizer.nextToken();
  }
  return superClasses;
}

Assets::ModelDefinition FgdParser::parseModel(ParserStatus& status)
{
  expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());

  const auto snapshot = m_tokenizer.snapshot();
  const auto line = m_tokenizer.line();
  const auto column = m_tokenizer.column();

  try
  {
    auto parser =
      ELParser{ELParser::Mode::Lenient, m_tokenizer.remainder(), line, column};
    auto expression = parser.parse();

    // advance our tokenizer by the amount that `parser` parsed
    m_tokenizer.adoptState(parser.tokenizerState());
    expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());

    expression.optimize();
    return Assets::ModelDefinition{expression};
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
      expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());

      expression.optimize();
      status.warn(
        line,
        column,
        "Legacy model expressions are deprecated, replace with '" + expression.asString()
          + "'");
      return Assets::ModelDefinition{expression};
    }
    catch (const ParserException&)
    {
      m_tokenizer.restore(snapshot);
      throw e;
    }
  }
  catch (const EL::EvaluationError& evaluationError)
  {
    throw ParserException{
      m_tokenizer.line(), m_tokenizer.column(), evaluationError.what()};
  }
}

void FgdParser::skipClassProperty(ParserStatus& /* status */)
{
  // We have already consumed the property name.
  // We assume that the next token we should encounter is
  // an open parenthesis. If the next token is not a
  // parenthesis, it forms part of the next property
  // (which we should not skip).
  if (m_tokenizer.peekToken().type() != FgdToken::OParenthesis)
  {
    return;
  }

  size_t depth = 0;
  auto token = Token{};
  do
  {
    token = m_tokenizer.nextToken();
    if (token.type() == FgdToken::OParenthesis)
    {
      ++depth;
    }
    else if (token.type() == FgdToken::CParenthesis)
    {
      --depth;
    }
  } while (depth > 0 && token.type() != FgdToken::Eof);
}

FgdParser::PropertyDefinitionList FgdParser::parsePropertyDefinitions(
  ParserStatus& status)
{
  auto propertyDefinitions = PropertyDefinitionList{};

  expect(status, FgdToken::OBracket, m_tokenizer.nextToken());
  auto token =
    expect(status, FgdToken::Word | FgdToken::CBracket, m_tokenizer.nextToken());

  while (token.type() != FgdToken::CBracket)
  {
    const auto propertyKey = token.data();
    const auto line = token.line();
    const auto column = token.column();

    expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());
    token = expect(status, FgdToken::Word, m_tokenizer.nextToken());
    const auto typeName = token.data();
    expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());

    auto propertyDefinition =
      parsePropertyDefinition(status, propertyKey, typeName, line, column);
    if (!addPropertyDefinition(propertyDefinitions, std::move(propertyDefinition)))
    {
      status.warn(
        line, column, "Skipping duplicate property definition: '" + propertyKey + "'");
    }

    token = expect(status, FgdToken::Word | FgdToken::CBracket, m_tokenizer.nextToken());
  }

  return propertyDefinitions;
}

FgdParser::PropertyDefinitionPtr FgdParser::parsePropertyDefinition(
  ParserStatus& status,
  const std::string& propertyKey,
  const std::string& typeName,
  const size_t line,
  const size_t column)
{
  if (kdl::ci::str_is_equal(typeName, "target_source"))
  {
    return parseTargetSourcePropertyDefinition(status, propertyKey);
  }
  if (kdl::ci::str_is_equal(typeName, "target_destination"))
  {
    return parseTargetDestinationPropertyDefinition(status, propertyKey);
  }
  if (kdl::ci::str_is_equal(typeName, "string"))
  {
    return parseStringPropertyDefinition(status, propertyKey);
  }
  if (kdl::ci::str_is_equal(typeName, "integer"))
  {
    return parseIntegerPropertyDefinition(status, propertyKey);
  }
  if (kdl::ci::str_is_equal(typeName, "float"))
  {
    return parseFloatPropertyDefinition(status, propertyKey);
  }
  if (kdl::ci::str_is_equal(typeName, "choices"))
  {
    return parseChoicesPropertyDefinition(status, propertyKey);
  }
  if (kdl::ci::str_is_equal(typeName, "flags"))
  {
    return parseFlagsPropertyDefinition(status, propertyKey);
  }

  status.debug(
    line,
    column,
    kdl::str_to_string(
      "Unknown property definition type '",
      typeName,
      "' for property '",
      propertyKey,
      "'"));
  return parseUnknownPropertyDefinition(status, propertyKey);
}

FgdParser::PropertyDefinitionPtr FgdParser::parseTargetSourcePropertyDefinition(
  ParserStatus& status, const std::string& propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  const auto shortDescription = parsePropertyDescription(status);
  parseDefaultStringValue(status);
  const auto longDescription = parsePropertyDescription(status);
  return std::make_shared<Assets::PropertyDefinition>(
    propertyKey,
    Assets::PropertyDefinitionType::TargetSourceProperty,
    shortDescription,
    longDescription,
    readOnly);
}

FgdParser::PropertyDefinitionPtr FgdParser::parseTargetDestinationPropertyDefinition(
  ParserStatus& status, const std::string& propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  const auto shortDescription = parsePropertyDescription(status);
  parseDefaultStringValue(status);
  const auto longDescription = parsePropertyDescription(status);
  return std::make_shared<Assets::PropertyDefinition>(
    propertyKey,
    Assets::PropertyDefinitionType::TargetDestinationProperty,
    shortDescription,
    longDescription,
    readOnly);
}

FgdParser::PropertyDefinitionPtr FgdParser::parseStringPropertyDefinition(
  ParserStatus& status, const std::string& propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  const auto shortDescription = parsePropertyDescription(status);
  const auto defaultValue = parseDefaultStringValue(status);
  const auto longDescription = parsePropertyDescription(status);
  return std::make_shared<Assets::StringPropertyDefinition>(
    propertyKey, shortDescription, longDescription, readOnly, defaultValue);
}

FgdParser::PropertyDefinitionPtr FgdParser::parseIntegerPropertyDefinition(
  ParserStatus& status, const std::string& propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  const auto shortDescription = parsePropertyDescription(status);
  const auto defaultValue = parseDefaultIntegerValue(status);
  const auto longDescription = parsePropertyDescription(status);
  return std::make_shared<Assets::IntegerPropertyDefinition>(
    propertyKey, shortDescription, longDescription, readOnly, defaultValue);
}

FgdParser::PropertyDefinitionPtr FgdParser::parseFloatPropertyDefinition(
  ParserStatus& status, const std::string& propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  const auto shortDescription = parsePropertyDescription(status);
  const auto defaultValue = parseDefaultFloatValue(status);
  const auto longDescription = parsePropertyDescription(status);
  return std::make_shared<Assets::FloatPropertyDefinition>(
    propertyKey, shortDescription, longDescription, readOnly, defaultValue);
}

FgdParser::PropertyDefinitionPtr FgdParser::parseChoicesPropertyDefinition(
  ParserStatus& status, const std::string& propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  const auto shortDescription = parsePropertyDescription(status);
  const auto defaultValue = parseDefaultChoiceValue(status);
  const auto longDescription = parsePropertyDescription(status);

  expect(status, FgdToken::Equality, m_tokenizer.nextToken());
  expect(status, FgdToken::OBracket, m_tokenizer.nextToken());

  auto token = expect(
    status,
    FgdToken::Integer | FgdToken::Decimal | FgdToken::String | FgdToken::CBracket,
    m_tokenizer.nextToken());

  auto options = Assets::ChoicePropertyOption::List{};
  while (token.type() != FgdToken::CBracket)
  {
    const auto value = token.data();
    expect(status, FgdToken::Colon, m_tokenizer.nextToken());
    const auto caption = parseString(status);

    options.emplace_back(value, caption);
    token = expect(
      status,
      FgdToken::Integer | FgdToken::Decimal | FgdToken::String | FgdToken::CBracket,
      m_tokenizer.nextToken());
  }

  return std::make_shared<Assets::ChoicePropertyDefinition>(
    propertyKey, shortDescription, longDescription, options, readOnly, defaultValue);
}

FgdParser::PropertyDefinitionPtr FgdParser::parseFlagsPropertyDefinition(
  ParserStatus& status, const std::string& propertyKey)
{
  // Flag property definitions do not have descriptions or defaults, see
  // https://developer.valvesoftware.com/wiki/FGD

  expect(status, FgdToken::Equality, m_tokenizer.nextToken());
  expect(status, FgdToken::OBracket, m_tokenizer.nextToken());

  auto token =
    expect(status, FgdToken::Integer | FgdToken::CBracket, m_tokenizer.nextToken());

  auto definition = std::make_shared<Assets::FlagsPropertyDefinition>(propertyKey);

  while (token.type() != FgdToken::CBracket)
  {
    const auto value = token.toInteger<int>();
    expect(status, FgdToken::Colon, m_tokenizer.nextToken());
    const auto shortDescription = parseString(status);

    auto defaultValue = false;
    token = expect(
      status,
      FgdToken::Colon | FgdToken::Integer | FgdToken::CBracket,
      m_tokenizer.peekToken());
    if (token.type() == FgdToken::Colon)
    {
      m_tokenizer.nextToken();
      token = expect(status, FgdToken::Integer, m_tokenizer.nextToken());
      defaultValue = token.toInteger<int>() != 0;
    }

    token = expect(
      status,
      FgdToken::Integer | FgdToken::CBracket | FgdToken::Colon,
      m_tokenizer.nextToken());

    auto longDescription = std::string{};
    if (token.type() == FgdToken::Colon)
    {
      longDescription = parseString(status);
      token =
        expect(status, FgdToken::Integer | FgdToken::CBracket, m_tokenizer.nextToken());
    }

    definition->addOption(value, shortDescription, longDescription, defaultValue);
  }
  return definition;
}

FgdParser::PropertyDefinitionPtr FgdParser::parseUnknownPropertyDefinition(
  ParserStatus& status, const std::string& propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  const auto shortDescription = parsePropertyDescription(status);
  const auto defaultValue = parseDefaultStringValue(status);
  const auto longDescription = parsePropertyDescription(status);
  return std::make_shared<Assets::UnknownPropertyDefinition>(
    propertyKey, shortDescription, longDescription, readOnly, defaultValue);
}

bool FgdParser::parseReadOnlyFlag(ParserStatus& /* status */)
{
  auto token = m_tokenizer.peekToken();
  if (token.hasType(FgdToken::Word) && token.data() == "readonly")
  {
    m_tokenizer.nextToken();
    return true;
  }
  return false;
}

std::string FgdParser::parsePropertyDescription(ParserStatus& status)
{
  auto token = m_tokenizer.peekToken();
  if (token.type() == FgdToken::Colon)
  {
    m_tokenizer.nextToken();
    token = expect(status, FgdToken::String | FgdToken::Colon, m_tokenizer.peekToken());
    if (token.type() == FgdToken::String)
    {
      return parseString(status);
    }
  }
  return "";
}

std::optional<std::string> FgdParser::parseDefaultStringValue(ParserStatus& status)
{
  auto token = m_tokenizer.peekToken();
  if (token.type() == FgdToken::Colon)
  {
    m_tokenizer.nextToken();
    token = expect(
      status,
      FgdToken::String | FgdToken::Colon | FgdToken::Integer | FgdToken::Decimal,
      m_tokenizer.peekToken());
    if (token.type() == FgdToken::String)
    {
      token = m_tokenizer.nextToken();
      return token.data();
    }
    if (token.type() == FgdToken::Integer || token.type() == FgdToken::Decimal)
    {
      token = m_tokenizer.nextToken();
      status.warn(
        token.line(), token.column(), "Found numeric default value for string property");
      return token.data();
    }
  }
  return std::nullopt;
}

std::optional<int> FgdParser::parseDefaultIntegerValue(ParserStatus& status)
{
  auto token = m_tokenizer.peekToken();
  if (token.type() == FgdToken::Colon)
  {
    m_tokenizer.nextToken();
    token = expect(
      status,
      FgdToken::Integer | FgdToken::Decimal | FgdToken::Colon,
      m_tokenizer.peekToken());
    if (token.type() == FgdToken::Integer)
    {
      token = m_tokenizer.nextToken();
      return token.toInteger<int>();
    }
    if (token.type() == FgdToken::Decimal)
    { // be graceful for DaZ
      token = m_tokenizer.nextToken();
      status.warn(
        token.line(), token.column(), "Found float default value for integer property");
      return static_cast<int>(token.toFloat<float>());
    }
  }
  return std::nullopt;
}

std::optional<float> FgdParser::parseDefaultFloatValue(ParserStatus& status)
{
  auto token = m_tokenizer.peekToken();
  if (token.type() == FgdToken::Colon)
  {
    m_tokenizer.nextToken();
    // the default value should have quotes around it, but sometimes they're missing
    token = expect(
      status,
      FgdToken::String | FgdToken::Decimal | FgdToken::Integer | FgdToken::Colon,
      m_tokenizer.peekToken());
    if (token.type() != FgdToken::Colon)
    {
      token = m_tokenizer.nextToken();
      if (token.type() != FgdToken::String)
      {
        status.warn(
          token.line(), token.column(), "Unquoted float default value " + token.data());
      }
      return token.toFloat<float>();
    }
  }
  return std::nullopt;
}

std::optional<std::string> FgdParser::parseDefaultChoiceValue(ParserStatus& status)
{
  auto token = m_tokenizer.peekToken();
  if (token.type() == FgdToken::Colon)
  {
    m_tokenizer.nextToken();
    token = expect(
      status,
      FgdToken::String | FgdToken::Integer | FgdToken::Decimal | FgdToken::Colon,
      m_tokenizer.peekToken());
    if (token.hasType(FgdToken::String | FgdToken::Integer | FgdToken::Decimal))
    {
      token = m_tokenizer.nextToken();
      return token.data();
    }
  }
  return std::nullopt;
}

vm::vec3 FgdParser::parseVector(ParserStatus& status)
{
  auto vec = vm::vec3{};
  for (size_t i = 0; i < 3; i++)
  {
    auto token =
      expect(status, FgdToken::Integer | FgdToken::Decimal, m_tokenizer.nextToken());
    vec[i] = token.toFloat<double>();
  }
  return vec;
}

vm::bbox3 FgdParser::parseSize(ParserStatus& status)
{
  auto size = vm::bbox3{};
  expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());
  size.min = parseVector(status);

  auto token =
    expect(status, FgdToken::CParenthesis | FgdToken::Comma, m_tokenizer.nextToken());
  if (token.type() == FgdToken::Comma)
  {
    size.max = parseVector(status);
    expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());
  }
  else
  {
    const auto halfSize = size.min / 2.0;
    size.min = -halfSize;
    size.max = halfSize;
  }
  return repair(size);
}

Color FgdParser::parseColor(ParserStatus& status)
{
  auto color = Color{};
  expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());
  for (size_t i = 0; i < 3; i++)
  {
    auto token =
      expect(status, FgdToken::Decimal | FgdToken::Integer, m_tokenizer.nextToken());
    color[i] = token.toFloat<float>();
    if (color[i] > 1.0f)
    {
      color[i] /= 255.0f;
    }
  }
  expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());
  color[3] = 1.0f;
  return color;
}

std::string FgdParser::parseString(ParserStatus& status)
{
  auto token = expect(status, FgdToken::String, m_tokenizer.nextToken());
  if (m_tokenizer.peekToken().hasType(FgdToken::Plus))
  {
    auto str = std::stringstream{};
    str << token.data();
    do
    {
      m_tokenizer.nextToken();
      token = expect(status, FgdToken::String, m_tokenizer.nextToken());
      str << token.data();
    } while (m_tokenizer.peekToken().hasType(FgdToken::Plus));
    return str.str();
  }
  else
  {
    return token.data();
  }
}

std::vector<EntityDefinitionClassInfo> FgdParser::parseInclude(ParserStatus& status)
{
  auto token = expect(status, FgdToken::Word, m_tokenizer.nextToken());
  assert(kdl::ci::str_is_equal(token.data(), "@include"));

  expect(status, FgdToken::String, token = m_tokenizer.nextToken());
  const auto path = Path(token.data());
  return handleInclude(status, path);
}

std::vector<EntityDefinitionClassInfo> FgdParser::handleInclude(
  ParserStatus& status, const Path& path)
{
  if (!m_fs)
  {
    status.error(
      m_tokenizer.line(),
      kdl::str_to_string("Cannot include file without host file path"));
    return {};
  }

  const auto snapshot = m_tokenizer.snapshotStateAndSource();
  auto result = std::vector<EntityDefinitionClassInfo>{};
  try
  {
    status.debug(m_tokenizer.line(), "Parsing included file '" + path.asString() + "'");
    const auto file = m_fs->openFile(currentRoot() + path);
    const auto filePath = file->path();
    status.debug(
      m_tokenizer.line(),
      "Resolved '" + path.asString() + "' to '" + filePath.asString() + "'");

    if (!isRecursiveInclude(filePath))
    {
      const auto pushIncludePath = PushIncludePath{this, filePath};
      auto reader = file->reader().buffer();
      m_tokenizer.replaceState(reader.stringView());
      result = parseClassInfos(status);
    }
    else
    {
      status.error(
        m_tokenizer.line(),
        kdl::str_to_string(
          "Skipping recursively included file: ", path.asString(), " (", filePath, ")"));
    }
  }
  catch (const Exception& e)
  {
    status.error(
      m_tokenizer.line(),
      kdl::str_to_string("Failed to parse included file: ", e.what()));
  }

  m_tokenizer.restoreStateAndSource(snapshot);
  return result;
}
} // namespace IO
} // namespace TrenchBroom
