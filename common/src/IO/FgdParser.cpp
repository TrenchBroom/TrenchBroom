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
#include "EL/Expressions.h"
#include "Error.h"
#include "IO/DiskFileSystem.h"
#include "IO/ELParser.h"
#include "IO/EntityDefinitionClassInfo.h"
#include "IO/File.h"
#include "IO/LegacyModelDefinitionParser.h"
#include "IO/ParserStatus.h"

#include "kdl/invoke.h"
#include "kdl/result.h"
#include "kdl/string_compare.h"
#include "kdl/string_format.h"
#include "kdl/string_utils.h"
#include "kdl/vector_utils.h"

#include <fmt/format.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom::IO
{

FgdTokenizer::FgdTokenizer(const std::string_view str)
  : Tokenizer{str, "", 0}
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
        throw ParserException{
          startLine, startColumn, fmt::format("Unexpected character: '{}'", c)};
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
  const std::string_view str,
  const Color& defaultEntityColor,
  const std::filesystem::path& path)
  : EntityDefinitionParser{defaultEntityColor}
  , m_tokenizer{FgdTokenizer{str}}
{
  if (!path.empty() && path.is_absolute())
  {
    m_fs = std::make_unique<DiskFileSystem>(path.parent_path());
    pushIncludePath(path.filename());
  }
}

FgdParser::FgdParser(std::string_view str, const Color& defaultEntityColor)
  : FgdParser{std::move(str), defaultEntityColor, {}}
{
}

FgdParser::~FgdParser() = default;

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


PropertyTypeMap FgdParser::propertyTypeNames() const
{
  using PropDefType = Assets::PropertyDefinitionType;

  return {
    {"target_source", PropDefType::TargetSourceProperty},
    {"target_destination", PropDefType::TargetDestinationProperty},
    {"string", PropDefType::StringProperty},
    {"boolean", PropDefType::BooleanProperty},
    {"integer", PropDefType::IntegerProperty},
    {"float", PropDefType::FloatProperty},
    {"flags", PropDefType::FlagsProperty},
    {"choices", PropDefType::ChoiceProperty},
    {"input", PropDefType::InputProperty},
    {"output", PropDefType::OutputProperty}};
}

IOTypeMap FgdParser::ioTypeNames() const
{
  using IOType = Assets::IOType;

  return {
    {"void", IOType::Void},
    {"bool", IOType::Bool},
    {"float", IOType::Float},
    {"string", IOType::String},
    {"integer", IOType::Integer}};
}


class FgdParser::PushIncludePath
{
private:
  FgdParser& m_parser;

public:
  PushIncludePath(FgdParser& parser, const std::filesystem::path& path)
    : m_parser{parser}
  {
    m_parser.pushIncludePath(path);
  }

  ~PushIncludePath() { m_parser.popIncludePath(); }
};

void FgdParser::pushIncludePath(std::filesystem::path path)
{
  assert(!isRecursiveInclude(path));
  m_paths.push_back(std::move(path));
}

void FgdParser::popIncludePath()
{
  assert(!m_paths.empty());
  m_paths.pop_back();
}

std::filesystem::path FgdParser::currentRoot() const
{
  assert(m_paths.empty() || !m_paths.back().empty());
  return !m_paths.empty() ? m_paths.back().parent_path() : std::filesystem::path{};
}

bool FgdParser::isRecursiveInclude(const std::filesystem::path& path) const
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
  if (kdl::ci::str_is_equal(classname, "@PointClass"))
  {
    return parsePointClassInfo(status);
  }
  if (kdl::ci::str_is_equal(classname, "@BaseClass"))
  {
    return parseBaseClassInfo(status);
  }
  if (kdl::ci::str_is_equal(classname, "@Main"))
  {
    skipMainClass(status);
    return std::nullopt;
  }

  const auto msg = fmt::format("Unknown entity definition class '{}'", classname);
  status.error(token.line(), token.column(), msg);
  throw ParserException{token.line(), token.column(), msg};
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
  if (classInfo.decalDefinition)
  {
    status.warn(
      classInfo.line,
      classInfo.column,
      "Solid entity definition must not have decal definitions");
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
      classInfo.modelDefinition =
        parseModel(status, kdl::ci::str_is_equal(typeName, "sprite"));
    }
    else if (kdl::ci::str_is_equal(typeName, "decal"))
    {
      if (classInfo.decalDefinition)
      {
        status.warn(token.line(), token.column(), "Found multiple decal properties");
      }
      classInfo.decalDefinition = parseDecal(status);
    }
    else
    {
      status.warn(
        token.line(),
        token.column(),
        fmt::format("Unknown entity definition header properties '{}'", typeName));
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
    classInfo.description = kdl::str_trim(parseString(status));
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

Assets::ModelDefinition FgdParser::parseModel(
  ParserStatus& status, const bool allowEmptyExpression)
{
  expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());

  const auto line = m_tokenizer.line();
  const auto column = m_tokenizer.column();

  if (allowEmptyExpression && m_tokenizer.peekToken().hasType(FgdToken::CParenthesis))
  {
    m_tokenizer.skipToken();

    auto defaultModel = EL::MapExpression{{
      {"path", {EL::VariableExpression{"model"}, line, column}},
      {"scale", {EL::VariableExpression{"scale"}, line, column}},
    }};
    auto defaultExp = EL::Expression{std::move(defaultModel), line, column};
    return Assets::ModelDefinition{std::move(defaultExp)};
  }

  const auto snapshot = m_tokenizer.snapshot();
  try
  {
    auto parser =
      ELParser{ELParser::Mode::Lenient, m_tokenizer.remainder(), line, column};
    auto expression = parser.parse();

    // advance our tokenizer by the amount that `parser` parsed
    m_tokenizer.adoptState(parser.tokenizerState());
    expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());

    expression.optimize();
    return Assets::ModelDefinition{std::move(expression)};
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
        fmt::format(
          "Legacy model expressions are deprecated, replace with '{}'",
          expression.asString()));
      return Assets::ModelDefinition{std::move(expression)};
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

Assets::DecalDefinition FgdParser::parseDecal(ParserStatus& status)
{
  expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());

  const auto snapshot = m_tokenizer.snapshot();
  const auto line = m_tokenizer.line();
  const auto column = m_tokenizer.column();

  // Accept "decal()" and give it a default expression of `{ texture: texture }`
  const auto token = m_tokenizer.peekToken();
  if (token.hasType(FgdToken::CParenthesis))
  {
    expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());
    auto defaultDecal =
      EL::MapExpression{{{"texture", {EL::VariableExpression{"texture"}, line, column}}}};
    auto defaultExp = EL::Expression{std::move(defaultDecal), line, column};
    return Assets::DecalDefinition{std::move(defaultExp)};
  }

  try
  {
    auto parser =
      ELParser{ELParser::Mode::Lenient, m_tokenizer.remainder(), line, column};
    auto expression = parser.parse();

    // advance our tokenizer by the amount that `parser` parsed
    m_tokenizer.adoptState(parser.tokenizerState());
    expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());

    expression = expression.optimize();
    return Assets::DecalDefinition{std::move(expression)};
  }
  catch (const ParserException&)
  {
    m_tokenizer.restore(snapshot);
    throw;
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

PropertyDefinitionType FgdParser::getPropertyType(std::string typeName)
{
  const auto propTypeMap = propertyTypeNames();
  const auto it = propTypeMap.find(kdl::str_to_lower(typeName));

  if (it != propTypeMap.end())
  {
    return it->second;
  }

  return PropertyDefinitionType::UnknownProperty;
}

Assets::IOType FgdParser::getIOType(ParserStatus& status)
{
  expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());
  auto token = expect(status, FgdToken::Word, m_tokenizer.nextToken());
  const auto line = token.line();
  const auto column = token.column();

  const auto typeName = token.data();
  expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());
  const auto ioTypeMap = ioTypeNames();
  auto it = ioTypeMap.find(kdl::str_to_lower(typeName));
  const auto propertyType = (it == ioTypeMap.end()) ? Assets::IOType::Void : it->second;

  if (it == ioTypeMap.end())
  {
    status.warn(
      line, column, std::format("Unknown input/output argument type {}", typeName));
  }

  return propertyType;
}


std::vector<std::shared_ptr<Assets::PropertyDefinition>> FgdParser::
  parsePropertyDefinitions(ParserStatus& status)
{
  auto propertyDefinitions = std::vector<std::shared_ptr<Assets::PropertyDefinition>>{};

  // Parse opening [ of class
  expect(status, FgdToken::OBracket, m_tokenizer.nextToken());
  auto token =
    expect(status, FgdToken::Word | FgdToken::CBracket, m_tokenizer.nextToken());

  while (token.type() != FgdToken::CBracket)
  {
    auto propertyKey = token.data();
    const auto line = token.line();
    const auto column = token.line();

    std::string typeName = {};

    if (
      (kdl::str_to_lower(propertyKey) == "input"
       || kdl::str_to_lower(propertyKey) == "output")
      && m_tokenizer.peekToken(1).type() == FgdToken::Word)
    {
      token = expect(status, FgdToken::Word, m_tokenizer.nextToken());
      typeName = propertyKey;
      propertyKey = token.data();
    }
    else
    {
      expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());
      token = expect(status, FgdToken::Word, m_tokenizer.nextToken());
      typeName = token.data();
      expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());
    }


    const auto propertyType = getPropertyType(typeName);
    std::unique_ptr<Assets::PropertyDefinition> propertyDefinition;

    switch (propertyType)
    {
    case PropertyDefinitionType::StringProperty:
      propertyDefinition =
        parsePropertyDefinition<PropertyDefinitionType::StringProperty>(
          status, propertyKey);
      break;
    case PropertyDefinitionType::FloatProperty:
      propertyDefinition = parsePropertyDefinition<PropertyDefinitionType::FloatProperty>(
        status, propertyKey);
      break;
    case PropertyDefinitionType::IntegerProperty:
      propertyDefinition =
        parsePropertyDefinition<PropertyDefinitionType::IntegerProperty>(
          status, propertyKey);
      break;
    case PropertyDefinitionType::BooleanProperty:
      propertyDefinition =
        parsePropertyDefinition<PropertyDefinitionType::BooleanProperty>(
          status, propertyKey);
      break;
    case PropertyDefinitionType::TargetDestinationProperty:
      propertyDefinition =
        parsePropertyDefinition<PropertyDefinitionType::TargetDestinationProperty>(
          status, propertyKey);
      break;
    case PropertyDefinitionType::TargetSourceProperty:
      propertyDefinition =
        parsePropertyDefinition<PropertyDefinitionType::TargetSourceProperty>(
          status, propertyKey);
      break;
    case PropertyDefinitionType::ChoiceProperty:
      propertyDefinition =
        parsePropertyDefinition<PropertyDefinitionType::ChoiceProperty>(
          status, propertyKey);
      break;
    case PropertyDefinitionType::FlagsProperty:
      propertyDefinition = parsePropertyDefinition<PropertyDefinitionType::FlagsProperty>(
        status, propertyKey);
      break;
    case PropertyDefinitionType::OutputProperty:
      propertyDefinition =
        parsePropertyDefinition<PropertyDefinitionType::OutputProperty>(
          status, propertyKey);
      break;
    case PropertyDefinitionType::InputProperty:
      propertyDefinition = parsePropertyDefinition<PropertyDefinitionType::InputProperty>(
        status, propertyKey);
      break;
    case PropertyDefinitionType::UnknownProperty:
      propertyDefinition =
        parsePropertyDefinition<PropertyDefinitionType::UnknownProperty>(
          status, propertyKey);
      break;
    }

    if (!addPropertyDefinition(propertyDefinitions, std::move(propertyDefinition)))
    {
      status.warn(
        line,
        column,
        fmt::format("Skipping duplicate property definition: '{}'", propertyKey));
    }

    token = expect(status, FgdToken::CBracket | FgdToken::Word, m_tokenizer.nextToken());
  }

  return propertyDefinitions;
}


template <>
std::unique_ptr<
  Assets::PropertyDefinitionT<PropertyDefinitionType::FlagsProperty>> inline FgdParser::
  parsePropertyDefinition(ParserStatus& status, std::string propertyKey)
{
  // Flag property definitions do not have descriptions or defaults, see
  // https://developer.valvesoftware.com/wiki/FGD

  expect(status, FgdToken::Equality, m_tokenizer.nextToken());
  expect(status, FgdToken::OBracket, m_tokenizer.nextToken());

  auto token =
    expect(status, FgdToken::Integer | FgdToken::CBracket, m_tokenizer.nextToken());
  auto definition =
    std::make_unique<Assets::PropertyDefinitionT<PropertyDefinitionType::FlagsProperty>>(
      propertyKey, std::string{}, std::string{}, false);

  auto options = Assets::FlagOption::List{};

  while (token.type() != FgdToken::CBracket)
  {
    const auto value = token.toInteger<int>();
    expect(status, FgdToken::Colon, m_tokenizer.nextToken());
    auto shortDescription = parseString(status);
    auto defaultState = false;

    token = expect(
      status,
      FgdToken::CBracket | FgdToken::Colon | FgdToken::Integer,
      m_tokenizer.peekToken());

    if (token.type() == FgdToken::Colon)
    {
      m_tokenizer.nextToken();
      token = expect(status, FgdToken::Integer, m_tokenizer.nextToken());
      defaultState = token.toInteger<int>() != 0;
    }

    token = expect(
      status,
      FgdToken::CBracket | FgdToken::Colon | FgdToken::Integer,
      m_tokenizer.nextToken());

    auto longDescription = std::string{};
    if (token.type() == FgdToken::Colon)
    {
      longDescription = parseString(status);
      token =
        expect(status, FgdToken::CBracket | FgdToken::Integer, m_tokenizer.nextToken());
    }

    options.emplace_back(value, shortDescription, longDescription, defaultState);
  }

  definition->setOptions(std::move(options));
  return definition;
}

template <>
std::unique_ptr<
  Assets::PropertyDefinitionT<PropertyDefinitionType::ChoiceProperty>> inline FgdParser::
  parsePropertyDefinition(ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription(status);
  auto defaultValue = parseDefaultValue<std::string>(status);
  auto longDescription = parsePropertyDescription(status);

  auto definition =
    std::make_unique<Assets::PropertyDefinitionT<PropertyDefinitionType::ChoiceProperty>>(
      propertyKey, shortDescription, longDescription, readOnly, defaultValue);

  expect(status, FgdToken::Equality, m_tokenizer.nextToken());
  expect(status, FgdToken::OBracket, m_tokenizer.nextToken());

  auto token = expect(
    status,
    FgdToken::CBracket | FgdToken::Decimal | FgdToken::Integer | FgdToken::String,
    m_tokenizer.nextToken());

  auto options = Assets::ChoiceOption::List{};
  while (token.type() != FgdToken::CBracket)
  {
    auto value = token.data();
    expect(status, FgdToken::Colon, m_tokenizer.nextToken());
    auto caption = parseString(status);

    options.emplace_back(std::move(value), std::move(caption));
    token = expect(
      status,
      FgdToken::CBracket | FgdToken::Decimal | FgdToken::Integer | FgdToken::String,
      m_tokenizer.nextToken());
  }

  definition->setOptions(std::move(options));
  return definition;
}

template <>
std::unique_ptr<
  Assets::PropertyDefinitionT<PropertyDefinitionType::InputProperty>> inline FgdParser::
  parsePropertyDefinition(ParserStatus& status, std::string propertyKey)
{
  auto ioType = getIOType(status);
  auto shortDescription = parsePropertyDescription(status);

  auto definition =
    std::make_unique<Assets::PropertyDefinitionT<PropertyDefinitionType::InputProperty>>(
      propertyKey, shortDescription, std::string{}, false, ioType);

  return definition;
}

template <>
std::unique_ptr<
  Assets::PropertyDefinitionT<PropertyDefinitionType::OutputProperty>> inline FgdParser::
  parsePropertyDefinition(ParserStatus& status, std::string propertyKey)
{
  auto ioType = getIOType(status);
  auto shortDescription = parsePropertyDescription(status);

  auto definition =
    std::make_unique<Assets::PropertyDefinitionT<PropertyDefinitionType::OutputProperty>>(
      propertyKey, shortDescription, std::string{}, false, ioType);

  return definition;
}

bool FgdParser::parseReadOnlyFlag(ParserStatus& status)
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
    token = expect(status, FgdToken::Colon | FgdToken::String, m_tokenizer.peekToken());
    if (token.type() == FgdToken::String)
    {
      return parseString(status);
    }
  }
  return "";
}

template <>
std::optional<float> FgdParser::parseDefaultValue(ParserStatus& status)
{
  auto token = m_tokenizer.peekToken();
  if (token.type() == FgdToken::Colon)
  {
    m_tokenizer.nextToken();
    token = expect(
      status,
      FgdToken::Colon | FgdToken::Decimal | FgdToken::String | FgdToken::Integer,
      m_tokenizer.peekToken());
    if (token.type() != FgdToken::Colon)
    {
      token = m_tokenizer.nextToken();
      if (token.type() != FgdToken::String)
      {
        status.warn(
          token.line(),
          token.column(),
          fmt::format("Unquoted float default value {}", token.data()));
      }
    }
  }
  return std::nullopt;
}

template <>
std::optional<int> FgdParser::parseDefaultValue(ParserStatus& status)
{
  auto token = m_tokenizer.peekToken();
  if (token.type() == FgdToken::Colon)
  {
    m_tokenizer.nextToken();
    token = expect(
      status,
      FgdToken::Colon | FgdToken::Decimal | FgdToken::Integer,
      m_tokenizer.peekToken());
    if (token.type() == FgdToken::Integer)
    {
      token = m_tokenizer.nextToken();
      return token.toInteger<int>();
    }
    if (token.type() == FgdToken::Decimal)
    {
      token = m_tokenizer.nextToken();
      status.warn(
        token.line(), token.column(), "Found float default value for integer property");
      return static_cast<int>(token.toFloat<float>());
    }
  }
  return std::nullopt;
}

template <>
std::optional<std::string> FgdParser::parseDefaultValue(ParserStatus& status)
{
  auto token = m_tokenizer.peekToken();
  if (token.type() == FgdToken::Colon)
  {
    m_tokenizer.nextToken();
    token = expect(
      status,
      FgdToken::Colon | FgdToken::Decimal | FgdToken::String | FgdToken::Integer,
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
        token.line(),
        token.column(),
        "Found numeric default for string or choice property");
      return token.data();
    }
  }
  return std::nullopt;
}

template <>
std::optional<bool> FgdParser::parseDefaultValue<bool>(ParserStatus& status)
{
  auto token = m_tokenizer.peekToken();
  if (token.type() == FgdToken::Colon)
  {
    m_tokenizer.nextToken();
    token = expect(status, FgdToken::Integer, m_tokenizer.peekToken());
    return token.toInteger<bool>();
  }
  return std::nullopt;
}

template <>
std::optional<char> FgdParser::parseDefaultValue<char>(ParserStatus& status)
{
  return std::nullopt;
};

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
  return handleInclude(status, token.data());
}

std::vector<EntityDefinitionClassInfo> FgdParser::handleInclude(
  ParserStatus& status, const std::filesystem::path& path)
{
  if (!m_fs)
  {
    status.error(
      m_tokenizer.line(),
      kdl::str_to_string("Cannot include file without host file path"));
    return {};
  }

  const auto restoreSnapshot =
    kdl::invoke_later{[&, snapshot = m_tokenizer.snapshotStateAndSource()]() {
      m_tokenizer.restoreStateAndSource(snapshot);
    }};

  status.debug(
    m_tokenizer.line(), fmt::format("Parsing included file '{}'", path.string()));

  const auto filePath = currentRoot() / path;
  return m_fs->openFile(filePath)
    .transform([&](auto file) {
      status.debug(
        m_tokenizer.line(),
        fmt::format("Resolved '{}' to '{}'", path.string(), filePath.string()));

      if (isRecursiveInclude(filePath))
      {
        status.error(
          m_tokenizer.line(),
          fmt::format(
            "Skipping recursively included file: {} ({})",
            path.string(),
            filePath.string()));
        return std::vector<EntityDefinitionClassInfo>{};
      }

      const auto pushIncludePath = PushIncludePath{*this, filePath};
      auto reader = file->reader().buffer();
      m_tokenizer.replaceState(reader.stringView());
      return parseClassInfos(status);
    })
    .transform_error([&](auto e) {
      status.error(
        m_tokenizer.line(), fmt::format("Failed to parse included file: {}", e.msg));
      return std::vector<EntityDefinitionClassInfo>{};
    })
    .value();
}

} // namespace TrenchBroom::IO
