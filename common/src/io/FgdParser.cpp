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

#include "FgdParser.h"

#include "el/ELExceptions.h"
#include "el/Expression.h"
#include "io/DiskFileSystem.h"
#include "io/ELParser.h"
#include "io/EntityDefinitionClassInfo.h"
#include "io/LegacyModelDefinitionParser.h"
#include "io/ParserStatus.h"
#include "mdl/PropertyDefinition.h"

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

namespace tb::io
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
          FileLocation{startLine, startColumn},
          fmt::format("Unexpected character: '{}'", c)};
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
  status.error(token.location(), msg);
  throw ParserException{token.location(), msg};
}

EntityDefinitionClassInfo FgdParser::parseSolidClassInfo(ParserStatus& status)
{
  const auto classInfo = parseClassInfo(status, EntityDefinitionClassType::BrushClass);
  if (classInfo.size)
  {
    status.warn(classInfo.location, "Solid entity definition must not have a size");
  }
  if (classInfo.modelDefinition)
  {
    status.warn(
      classInfo.location, "Solid entity definition must not have model definitions");
  }
  if (classInfo.decalDefinition)
  {
    status.warn(
      classInfo.location, "Solid entity definition must not have decal definitions");
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
  classInfo.location = token.location();

  while (token.type() == FgdToken::Word)
  {
    const auto typeName = token.data();
    if (kdl::ci::str_is_equal(typeName, "base"))
    {
      if (!classInfo.superClasses.empty())
      {
        status.warn(token.location(), "Found multiple base properties");
      }
      classInfo.superClasses = parseSuperClasses(status);
    }
    else if (kdl::ci::str_is_equal(typeName, "color"))
    {
      if (classInfo.color)
      {
        status.warn(token.location(), "Found multiple color properties");
      }
      classInfo.color = parseColor(status);
    }
    else if (kdl::ci::str_is_equal(typeName, "size"))
    {
      if (classInfo.size)
      {
        status.warn(token.location(), "Found multiple size properties");
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
        status.warn(token.location(), "Found multiple model properties");
      }
      classInfo.modelDefinition =
        parseModel(status, kdl::ci::str_is_equal(typeName, "sprite"));
    }
    else if (kdl::ci::str_is_equal(typeName, "decal"))
    {
      if (classInfo.decalDefinition)
      {
        status.warn(token.location(), "Found multiple decal properties");
      }
      classInfo.decalDefinition = parseDecal(status);
    }
    else
    {
      status.warn(
        token.location(),
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

mdl::ModelDefinition FgdParser::parseModel(
  ParserStatus& status, const bool allowEmptyExpression)
{
  expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());

  const auto line = m_tokenizer.line();
  const auto column = m_tokenizer.column();
  const auto location = m_tokenizer.location();

  if (allowEmptyExpression && m_tokenizer.peekToken().hasType(FgdToken::CParenthesis))
  {
    m_tokenizer.skipToken();

    auto defaultModel = el::MapExpression{{
      {"path", el::ExpressionNode{el::VariableExpression{"model"}, location}},
      {"scale", el::ExpressionNode{el::VariableExpression{"scale"}, location}},
    }};
    auto defaultExp = el::ExpressionNode{std::move(defaultModel), location};
    return mdl::ModelDefinition{std::move(defaultExp)};
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
      expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());

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

mdl::DecalDefinition FgdParser::parseDecal(ParserStatus& status)
{
  expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());

  const auto snapshot = m_tokenizer.snapshot();
  const auto line = m_tokenizer.line();
  const auto column = m_tokenizer.column();
  const auto location = m_tokenizer.location();

  // Accept "decal()" and give it a default expression of `{ texture: texture }`
  const auto token = m_tokenizer.peekToken();
  if (token.hasType(FgdToken::CParenthesis))
  {
    expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());
    auto defaultDecal = el::MapExpression{
      {{"texture", el::ExpressionNode{el::VariableExpression{"texture"}, location}}}};
    auto defaultExp = el::ExpressionNode{std::move(defaultDecal), location};
    return mdl::DecalDefinition{std::move(defaultExp)};
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
    return mdl::DecalDefinition{std::move(expression)};
  }
  catch (const ParserException&)
  {
    m_tokenizer.restore(snapshot);
    throw;
  }
  catch (const el::EvaluationError& evaluationError)
  {
    throw ParserException{location, evaluationError.what()};
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

std::vector<std::shared_ptr<mdl::PropertyDefinition>> FgdParser::parsePropertyDefinitions(
  ParserStatus& status)
{
  auto propertyDefinitions = std::vector<std::shared_ptr<mdl::PropertyDefinition>>{};

  expect(status, FgdToken::OBracket, m_tokenizer.nextToken());
  auto token = expect(
    status,
    FgdToken::Word | FgdToken::Integer | FgdToken::CBracket,
    m_tokenizer.nextToken());

  while (token.type() != FgdToken::CBracket)
  {
    const auto propertyKey = token.data();
    const auto location = token.location();

    expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());
    token = expect(status, FgdToken::Word, m_tokenizer.nextToken());
    const auto typeName = token.data();
    expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());

    auto propertyDefinition =
      parsePropertyDefinition(status, propertyKey, typeName, location);
    if (!addPropertyDefinition(propertyDefinitions, std::move(propertyDefinition)))
    {
      status.warn(
        location,
        fmt::format("Skipping duplicate property definition: '{}'", propertyKey));
    }

    token = expect(
      status,
      FgdToken::Word | FgdToken::Integer | FgdToken::CBracket,
      m_tokenizer.nextToken());
  }

  return propertyDefinitions;
}

std::unique_ptr<mdl::PropertyDefinition> FgdParser::parsePropertyDefinition(
  ParserStatus& status,
  std::string propertyKey,
  const std::string& typeName,
  const FileLocation& location)
{
  if (kdl::ci::str_is_equal(typeName, "target_source"))
  {
    return parseTargetSourcePropertyDefinition(status, std::move(propertyKey));
  }
  if (kdl::ci::str_is_equal(typeName, "target_destination"))
  {
    return parseTargetDestinationPropertyDefinition(status, std::move(propertyKey));
  }
  if (kdl::ci::str_is_equal(typeName, "string"))
  {
    return parseStringPropertyDefinition(status, std::move(propertyKey));
  }
  if (kdl::ci::str_is_equal(typeName, "integer"))
  {
    return parseIntegerPropertyDefinition(status, std::move(propertyKey));
  }
  if (kdl::ci::str_is_equal(typeName, "float"))
  {
    return parseFloatPropertyDefinition(status, std::move(propertyKey));
  }
  if (kdl::ci::str_is_equal(typeName, "choices"))
  {
    return parseChoicesPropertyDefinition(status, std::move(propertyKey));
  }
  if (kdl::ci::str_is_equal(typeName, "flags"))
  {
    return parseFlagsPropertyDefinition(status, std::move(propertyKey));
  }

  status.debug(
    location,
    fmt::format(
      "Unknown property definition type '{}' for property '{}'", typeName, propertyKey));
  return parseUnknownPropertyDefinition(status, std::move(propertyKey));
}

std::unique_ptr<mdl::PropertyDefinition> FgdParser::parseTargetSourcePropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription(status);
  parseDefaultStringValue(status);
  auto longDescription = parsePropertyDescription(status);
  return std::make_unique<mdl::PropertyDefinition>(
    std::move(propertyKey),
    mdl::PropertyDefinitionType::TargetSourceProperty,
    std::move(shortDescription),
    std::move(longDescription),
    readOnly);
}

std::unique_ptr<mdl::PropertyDefinition> FgdParser::
  parseTargetDestinationPropertyDefinition(ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription(status);
  parseDefaultStringValue(status);
  auto longDescription = parsePropertyDescription(status);
  return std::make_unique<mdl::PropertyDefinition>(
    std::move(propertyKey),
    mdl::PropertyDefinitionType::TargetDestinationProperty,
    std::move(shortDescription),
    std::move(longDescription),
    readOnly);
}

std::unique_ptr<mdl::PropertyDefinition> FgdParser::parseStringPropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription(status);
  auto defaultValue = parseDefaultStringValue(status);
  auto longDescription = parsePropertyDescription(status);
  return std::make_unique<mdl::StringPropertyDefinition>(
    std::move(propertyKey),
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    std::move(defaultValue));
}

std::unique_ptr<mdl::PropertyDefinition> FgdParser::parseIntegerPropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription(status);
  auto defaultValue = parseDefaultIntegerValue(status);
  auto longDescription = parsePropertyDescription(status);
  return std::make_unique<mdl::IntegerPropertyDefinition>(
    std::move(propertyKey),
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    std::move(defaultValue));
}

std::unique_ptr<mdl::PropertyDefinition> FgdParser::parseFloatPropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription(status);
  auto defaultValue = parseDefaultFloatValue(status);
  auto longDescription = parsePropertyDescription(status);
  return std::make_unique<mdl::FloatPropertyDefinition>(
    std::move(propertyKey),
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    std::move(defaultValue));
}

std::unique_ptr<mdl::PropertyDefinition> FgdParser::parseChoicesPropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription(status);
  auto defaultValue = parseDefaultChoiceValue(status);
  auto longDescription = parsePropertyDescription(status);

  expect(status, FgdToken::Equality, m_tokenizer.nextToken());
  expect(status, FgdToken::OBracket, m_tokenizer.nextToken());

  auto token = expect(
    status,
    FgdToken::Integer | FgdToken::Decimal | FgdToken::String | FgdToken::CBracket,
    m_tokenizer.nextToken());

  auto options = mdl::ChoicePropertyOption::List{};
  while (token.type() != FgdToken::CBracket)
  {
    auto value = token.data();
    expect(status, FgdToken::Colon, m_tokenizer.nextToken());
    auto caption = parseString(status);

    options.emplace_back(std::move(value), std::move(caption));
    token = expect(
      status,
      FgdToken::Integer | FgdToken::Decimal | FgdToken::String | FgdToken::CBracket,
      m_tokenizer.nextToken());
  }

  return std::make_unique<mdl::ChoicePropertyDefinition>(
    std::move(propertyKey),
    std::move(shortDescription),
    std::move(longDescription),
    std::move(options),
    readOnly,
    std::move(defaultValue));
}

std::unique_ptr<mdl::PropertyDefinition> FgdParser::parseFlagsPropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  // Flag property definitions do not have descriptions or defaults, see
  // https://developer.valvesoftware.com/wiki/FGD

  expect(status, FgdToken::Equality, m_tokenizer.nextToken());
  expect(status, FgdToken::OBracket, m_tokenizer.nextToken());

  auto token =
    expect(status, FgdToken::Integer | FgdToken::CBracket, m_tokenizer.nextToken());

  auto definition =
    std::make_unique<mdl::FlagsPropertyDefinition>(std::move(propertyKey));

  while (token.type() != FgdToken::CBracket)
  {
    const auto value = token.toInteger<int>();
    expect(status, FgdToken::Colon, m_tokenizer.nextToken());
    auto shortDescription = parseString(status);

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

    definition->addOption(
      value, std::move(shortDescription), std::move(longDescription), defaultValue);
  }
  return definition;
}

std::unique_ptr<mdl::PropertyDefinition> FgdParser::parseUnknownPropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription(status);
  auto defaultValue = parseDefaultStringValue(status);
  auto longDescription = parsePropertyDescription(status);
  return std::make_unique<mdl::UnknownPropertyDefinition>(
    std::move(propertyKey),
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    std::move(defaultValue));
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
      status.warn(token.location(), "Found numeric default value for string property");
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
      status.warn(token.location(), "Found float default value for integer property");
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
          token.location(), fmt::format("Unquoted float default value {}", token.data()));
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

vm::vec3d FgdParser::parseVector(ParserStatus& status)
{
  auto vec = vm::vec3d{};
  for (size_t i = 0; i < 3; i++)
  {
    auto token =
      expect(status, FgdToken::Integer | FgdToken::Decimal, m_tokenizer.nextToken());
    vec[i] = token.toFloat<double>();
  }
  return vec;
}

vm::bbox3d FgdParser::parseSize(ParserStatus& status)
{
  auto size = vm::bbox3d{};
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
      m_tokenizer.location(),
      kdl::str_to_string("Cannot include file without host file path"));
    return {};
  }

  const auto restoreSnapshot =
    kdl::invoke_later{[&, snapshot = m_tokenizer.snapshotStateAndSource()]() {
      m_tokenizer.restoreStateAndSource(snapshot);
    }};

  status.debug(
    m_tokenizer.location(), fmt::format("Parsing included file '{}'", path.string()));

  const auto filePath = currentRoot() / path;
  return m_fs->openFile(filePath) | kdl::transform([&](auto file) {
           status.debug(
             m_tokenizer.location(),
             fmt::format("Resolved '{}' to '{}'", path.string(), filePath.string()));

           if (isRecursiveInclude(filePath))
           {
             status.error(
               m_tokenizer.location(),
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
         | kdl::transform_error([&](auto e) {
             status.error(
               m_tokenizer.location(),
               fmt::format("Failed to parse included file: {}", e.msg));
             return std::vector<EntityDefinitionClassInfo>{};
           })
         | kdl::value();
}

} // namespace tb::io
