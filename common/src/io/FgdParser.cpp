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

#include "ParserException.h"
#include "ParserStatus.h"
#include "el/Expression.h"
#include "fs/DiskFileSystem.h"
#include "io/EntityDefinitionClassInfo.h"
#include "io/LegacyModelDefinitionParser.h"
#include "io/ParseModelDefinition.h"
#include "mdl/PropertyDefinition.h"

#include "kd/contracts.h"
#include "kd/invoke.h"
#include "kd/result.h"
#include "kd/string_compare.h"
#include "kd/string_format.h"
#include "kd/string_utils.h"
#include "kd/vector_utils.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace tb::io
{
namespace
{

auto tokenNames()
{
  using namespace FgdToken;

  return FgdTokenizer::TokenNameMap{
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

Result<mdl::PropertyValueTypes::IOParameterType> parseIOParameterType(
  const std::string_view typeName)
{
  using mdl::PropertyValueTypes::IOParameterType;

  // see https://developer.valvesoftware.com/wiki/FGD#Inputs_and_Outputs
  static const auto nameToTypeMap = std::unordered_map<std::string_view, IOParameterType>{
    {"void", IOParameterType::Void},
    {"string", IOParameterType::String},
    {"integer", IOParameterType::Integer},
    {"float", IOParameterType::Float},
    {"bool", IOParameterType::Boolean},
    {"boolean", IOParameterType::Boolean}, // for better compatibility
    {"ehandle", IOParameterType::EHandle},
  };

  if (const auto iParameterType = nameToTypeMap.find(kdl::str_to_lower(typeName));
      iParameterType != nameToTypeMap.end())
  {
    return iParameterType->second;
  }

  return Error{fmt::format("Unknown IO parameter type: {}", typeName)};
}

} // namespace


FgdTokenizer::FgdTokenizer(const std::string_view str)
  : Tokenizer{tokenNames(), str, "", 0}
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
    m_fs = std::make_unique<fs::DiskFileSystem>(path.parent_path());
    pushIncludePath(path.filename());
  }
}

FgdParser::FgdParser(std::string_view str, const Color& defaultEntityColor)
  : FgdParser{std::move(str), defaultEntityColor, {}}
{
}

FgdParser::~FgdParser() = default;

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
  contract_pre(!isRecursiveInclude(path));

  m_paths.push_back(std::move(path));
}

void FgdParser::popIncludePath()
{
  contract_pre(!m_paths.empty());

  m_paths.pop_back();
}

std::filesystem::path FgdParser::currentRoot() const
{
  contract_pre(m_paths.empty() || !m_paths.back().empty());

  return !m_paths.empty() ? m_paths.back().parent_path() : std::filesystem::path{};
}

bool FgdParser::isRecursiveInclude(const std::filesystem::path& path) const
{
  return std::ranges::any_of(
    m_paths, [&](const auto& includedPath) { return includedPath == path; });
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
  const auto token = m_tokenizer.peekToken(FgdToken::Eof | FgdToken::Word);
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
  const auto token = m_tokenizer.nextToken(FgdToken::Word);

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
    skipMainClass();
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
  auto token = m_tokenizer.nextToken(FgdToken::Word | FgdToken::Equality);

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
      classInfo.superClasses = parseSuperClasses();
    }
    else if (kdl::ci::str_is_equal(typeName, "color"))
    {
      if (classInfo.color)
      {
        status.warn(token.location(), "Found multiple color properties");
      }
      classInfo.color = parseColor();
    }
    else if (kdl::ci::str_is_equal(typeName, "size"))
    {
      if (classInfo.size)
      {
        status.warn(token.location(), "Found multiple size properties");
      }
      classInfo.size = parseSize();
    }
    else if (
      kdl::ci::str_is_equal(typeName, "model")
      || kdl::ci::str_is_equal(typeName, "studio")
      || kdl::ci::str_is_equal(typeName, "studioprop")
      || kdl::ci::str_is_equal(typeName, "lightprop")
      || kdl::ci::str_is_equal(typeName, "sprite")
      || kdl::ci::str_is_equal(typeName, "iconsprite"))
    {
      if (classInfo.modelDefinition)
      {
        status.warn(token.location(), "Found multiple model properties");
      }
      classInfo.modelDefinition = parseModel(status);
    }
    else if (kdl::ci::str_is_equal(typeName, "decal"))
    {
      if (classInfo.decalDefinition)
      {
        status.warn(token.location(), "Found multiple decal properties");
      }
      classInfo.decalDefinition = parseDecal();
    }
    else
    {
      status.warn(
        token.location(),
        fmt::format("Unknown entity definition header properties '{}'", typeName));
      skipClassProperty(status);
    }
    token = m_tokenizer.nextToken(FgdToken::Equality | FgdToken::Word);
  }

  token = m_tokenizer.nextToken(FgdToken::Word);
  classInfo.name = token.data();

  token = m_tokenizer.peekToken(FgdToken::Colon | FgdToken::OBracket);
  if (token.type() == FgdToken::Colon)
  {
    m_tokenizer.nextToken();
    classInfo.description = kdl::str_trim(parseString());
  }

  classInfo.propertyDefinitions = parsePropertyDefinitions(status);

  return classInfo;
}

void FgdParser::skipMainClass()
{
  m_tokenizer.nextToken(FgdToken::Equality);
  m_tokenizer.nextToken(FgdToken::OBracket);

  auto token = Token{};
  do
  {
    token = m_tokenizer.nextToken();
  } while (token.type() != FgdToken::CBracket);
}

std::vector<std::string> FgdParser::parseSuperClasses()
{
  m_tokenizer.nextToken(FgdToken::OParenthesis);

  auto token = m_tokenizer.peekToken(FgdToken::Word | FgdToken::CParenthesis);

  auto superClasses = std::vector<std::string>{};
  if (token.type() == FgdToken::Word)
  {
    do
    {
      token = m_tokenizer.nextToken(FgdToken::Word);
      superClasses.push_back(token.data());
      token = m_tokenizer.nextToken(FgdToken::Comma | FgdToken::CParenthesis);
    } while (token.type() == FgdToken::Comma);
  }
  else
  {
    m_tokenizer.nextToken();
  }
  return superClasses;
}

mdl::ModelDefinition FgdParser::parseModel(
  ParserStatus& status)
{
  m_tokenizer.nextToken(FgdToken::OParenthesis);

  if (m_tokenizer.peekToken().hasType(FgdToken::CParenthesis))
  {
    m_tokenizer.skipToken();
    return mdl::ModelDefinition();
  }

  return io::parseModelDefinition(m_tokenizer, status, FgdToken::CParenthesis)
         | kdl::if_error([](const auto& e) { throw ParserException{e.msg}; })
         | kdl::value();
}

mdl::DecalDefinition FgdParser::parseDecal()
{
  m_tokenizer.nextToken(FgdToken::OParenthesis);

  const auto location = m_tokenizer.location();

  // Accept "decal()" and give it a default expression of `{ texture: texture }`
  const auto token = m_tokenizer.peekToken();
  if (token.hasType(FgdToken::CParenthesis))
  {
    m_tokenizer.nextToken(FgdToken::CParenthesis);
    auto defaultDecal = el::MapExpression{
      {{"texture", el::ExpressionNode{el::VariableExpression{"texture"}, location}}}};
    auto defaultExp = el::ExpressionNode{std::move(defaultDecal), location};
    return mdl::DecalDefinition{std::move(defaultExp)};
  }

  return parseElModelExpression(m_tokenizer, location, FgdToken::CParenthesis)
         | kdl::and_then(
           [&](const auto& expression) { return optimizeModelExpression(expression); })
         | kdl::transform(
           [](auto expression) { return mdl::DecalDefinition{std::move(expression)}; })
         | kdl::if_error([](const auto& e) { throw ParserException{e.msg}; })
         | kdl::value();
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

std::vector<mdl::PropertyDefinition> FgdParser::parsePropertyDefinitions(
  ParserStatus& status)
{
  auto propertyDefinitions = std::vector<mdl::PropertyDefinition>{};

  m_tokenizer.nextToken(FgdToken::OBracket);
  auto token =
    m_tokenizer.peekToken(FgdToken::Word | FgdToken::Integer | FgdToken::CBracket);

  while (token.type() != FgdToken::CBracket)
  {
    const auto propertyKey = token.data();
    const auto location = token.location();

    auto propertyDefinition = parsePropertyDefinition(status);
    if (!addPropertyDefinition(propertyDefinitions, std::move(propertyDefinition)))
    {
      status.warn(
        location,
        fmt::format("Skipping duplicate property definition: '{}'", propertyKey));
    }

    token =
      m_tokenizer.peekToken(FgdToken::Word | FgdToken::Integer | FgdToken::CBracket);
  }
  m_tokenizer.nextToken(FgdToken::CBracket);

  return propertyDefinitions;
}

mdl::PropertyDefinition FgdParser::parsePropertyDefinition(ParserStatus& status)
{
  auto token = m_tokenizer.nextToken(FgdToken::Word | FgdToken::Integer);

  const auto location = token.location();
  auto propertyKeyOrIOKeyword = token.data();

  token = m_tokenizer.peekToken(FgdToken::OParenthesis | FgdToken::Word);
  if (kdl::ci::str_is_equal(propertyKeyOrIOKeyword, "input"))
  {
    if (token.hasType(FgdToken::Word))
    {
      return parseInputPropertyDefinition(status);
    }
  }
  else if (kdl::ci::str_is_equal(propertyKeyOrIOKeyword, "output"))
  {
    if (token.hasType(FgdToken::Word))
    {
      return parseOutputPropertyDefinition(status);
    }
  }

  auto propertyKey = std::move(propertyKeyOrIOKeyword);
  m_tokenizer.nextToken(FgdToken::OParenthesis);
  token = m_tokenizer.nextToken(FgdToken::Word);
  const auto typeName = token.data();
  m_tokenizer.nextToken(FgdToken::CParenthesis);

  if (kdl::ci::str_is_equal(typeName, "target_destination"))
  {
    return parseTargetDestinationPropertyDefinition(status, std::move(propertyKey));
  }
  if (kdl::ci::str_is_equal(typeName, "target_source"))
  {
    return parseTargetSourcePropertyDefinition(status, std::move(propertyKey));
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
    return parseFlagsPropertyDefinition(std::move(propertyKey));
  }
  if (kdl::ci::str_is_equal(typeName, "origin"))
  {
    return parseOriginPropertyDefinition(status, std::move(propertyKey));
  }
  if (kdl::ci::str_is_equal(typeName, "color1"))
  {
    return parseColorPropertyDefinition(
      status, ColorType::Color1, std::move(propertyKey));
  }
  if (kdl::ci::str_is_equal(typeName, "color255"))
  {
    return parseColorPropertyDefinition(
      status, ColorType::Color255, std::move(propertyKey));
  }

  status.debug(
    location,
    fmt::format(
      "Unknown property definition type '{}' for property '{}'", typeName, propertyKey));
  return parseUnknownPropertyDefinition(status, std::move(propertyKey));
}

mdl::PropertyDefinition FgdParser::parseTargetSourcePropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription();
  parseDefaultStringValue(status);
  auto longDescription = parsePropertyDescription();
  return {
    std::move(propertyKey),
    mdl::PropertyValueTypes::LinkTarget{},
    std::move(shortDescription),
    std::move(longDescription),
    readOnly};
}

mdl::PropertyDefinition FgdParser::parseTargetDestinationPropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription();
  parseDefaultStringValue(status);
  auto longDescription = parsePropertyDescription();
  return {
    std::move(propertyKey),
    mdl::PropertyValueTypes::LinkSource{},
    std::move(shortDescription),
    std::move(longDescription),
    readOnly};
}

mdl::PropertyDefinition FgdParser::parseStringPropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription();
  auto defaultValue = parseDefaultStringValue(status);
  auto longDescription = parsePropertyDescription();
  return {
    std::move(propertyKey),
    mdl::PropertyValueTypes::String{std::move(defaultValue)},
    std::move(shortDescription),
    std::move(longDescription),
    readOnly};
}

mdl::PropertyDefinition FgdParser::parseIntegerPropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription();
  auto defaultValue = parseDefaultIntegerValue(status);
  auto longDescription = parsePropertyDescription();
  return {
    std::move(propertyKey),
    mdl::PropertyValueTypes::Integer{std::move(defaultValue)},
    std::move(shortDescription),
    std::move(longDescription),
    readOnly};
}

mdl::PropertyDefinition FgdParser::parseFloatPropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription();
  auto defaultValue = parseDefaultFloatValue(status);
  auto longDescription = parsePropertyDescription();
  return {
    std::move(propertyKey),
    mdl::PropertyValueTypes::Float{std::move(defaultValue)},
    std::move(shortDescription),
    std::move(longDescription),
    readOnly};
}

mdl::PropertyDefinition FgdParser::parseChoicesPropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription();
  auto defaultValue = parseDefaultChoiceValue();
  auto longDescription = parsePropertyDescription();

  m_tokenizer.nextToken(FgdToken::Equality);
  m_tokenizer.nextToken(FgdToken::OBracket);

  auto token = m_tokenizer.nextToken(
    FgdToken::Integer | FgdToken::Decimal | FgdToken::String | FgdToken::CBracket);

  auto options = std::vector<mdl::PropertyValueTypes::ChoiceOption>{};
  while (token.type() != FgdToken::CBracket)
  {
    auto value = token.data();
    m_tokenizer.nextToken(FgdToken::Colon);
    auto caption = parseString();

    options.push_back(
      mdl::PropertyValueTypes::ChoiceOption{std::move(value), std::move(caption)});
    token = m_tokenizer.nextToken(
      FgdToken::Integer | FgdToken::Decimal | FgdToken::String | FgdToken::CBracket);
  }

  return {
    std::move(propertyKey),
    mdl::PropertyValueTypes::Choice{std::move(options), std::move(defaultValue)},
    std::move(shortDescription),
    std::move(longDescription),
    readOnly};
}

mdl::PropertyDefinition FgdParser::parseFlagsPropertyDefinition(std::string propertyKey)
{
  // Flag property definitions do not have descriptions or defaults, see
  // https://developer.valvesoftware.com/wiki/FGD

  m_tokenizer.nextToken(FgdToken::Equality);
  m_tokenizer.nextToken(FgdToken::OBracket);

  auto token = m_tokenizer.nextToken(FgdToken::Integer | FgdToken::CBracket);

  auto flags = std::vector<mdl::PropertyValueTypes::Flag>{};
  auto defaultValue = 0;

  while (token.type() != FgdToken::CBracket)
  {
    const auto value = token.toInteger<int>();
    m_tokenizer.nextToken(FgdToken::Colon);
    auto shortDescription = parseString();

    token =
      m_tokenizer.peekToken(FgdToken::Colon | FgdToken::Integer | FgdToken::CBracket);
    if (token.type() == FgdToken::Colon)
    {
      m_tokenizer.nextToken();
      token = m_tokenizer.nextToken(FgdToken::Integer);
      if (token.toInteger<int>() != 0)
      {
        defaultValue = defaultValue | value;
      }
    }

    token =
      m_tokenizer.nextToken(FgdToken::Integer | FgdToken::CBracket | FgdToken::Colon);

    auto longDescription = std::string{};
    if (token.type() == FgdToken::Colon)
    {
      longDescription = parseString();
      token = m_tokenizer.nextToken(FgdToken::Integer | FgdToken::CBracket);
    }

    flags.push_back(mdl::PropertyValueTypes::Flag{
      value, std::move(shortDescription), std::move(longDescription)});
  }

  return {
    std::move(propertyKey),
    mdl::PropertyValueTypes::Flags{std::move(flags), defaultValue},
    "",
    "",
    false};
}

mdl::PropertyDefinition FgdParser::parseOriginPropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription();
  auto defaultValue = parseDefaultStringValue(status);
  auto longDescription = parsePropertyDescription();
  return {
    std::move(propertyKey),
    mdl::PropertyValueTypes::Origin{std::move(defaultValue)},
    std::move(shortDescription),
    std::move(longDescription),
    readOnly};
}

mdl::PropertyDefinition FgdParser::parseInputPropertyDefinition(ParserStatus& status)
{
  auto [propertyKey, parameterType] = parseIOProperty(status);
  auto description = parsePropertyDescription();

  return {
    std::move(propertyKey),
    mdl::PropertyValueTypes::Input{parameterType},
    std::move(description),
    {},
    false};
}

mdl::PropertyDefinition FgdParser::parseOutputPropertyDefinition(ParserStatus& status)
{
  auto [propertyKey, parameterType] = parseIOProperty(status);
  auto description = parsePropertyDescription();

  return {
    std::move(propertyKey),
    mdl::PropertyValueTypes::Output{parameterType},
    std::move(description),
    {},
    false};
}

mdl::PropertyDefinition FgdParser::parseColorPropertyDefinition(
  ParserStatus& status, const ColorType colorType, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription();
  auto defaultValue = parseDefaultStringValue(status);
  auto longDescription = parsePropertyDescription();

  switch (colorType)
  {
  case ColorType::Color1:
    return {
      std::move(propertyKey),
      mdl::PropertyValueTypes::Color<RgbF>{std::move(defaultValue)},
      std::move(shortDescription),
      std::move(longDescription),
      readOnly};
  case ColorType::Color255:
    return {
      std::move(propertyKey),
      mdl::PropertyValueTypes::Color<RgbB>{std::move(defaultValue)},
      std::move(shortDescription),
      std::move(longDescription),
      readOnly};
    switchDefault();
  }
}

mdl::PropertyDefinition FgdParser::parseUnknownPropertyDefinition(
  ParserStatus& status, std::string propertyKey)
{
  const auto readOnly = parseReadOnlyFlag(status);
  auto shortDescription = parsePropertyDescription();
  auto defaultValue = parseDefaultStringValue(status);
  auto longDescription = parsePropertyDescription();
  return {
    std::move(propertyKey),
    mdl::PropertyValueTypes::Unknown{std::move(defaultValue)},
    std::move(shortDescription),
    std::move(longDescription),
    readOnly};
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

std::string FgdParser::parsePropertyDescription()
{
  auto token = m_tokenizer.peekToken();
  if (token.type() == FgdToken::Colon)
  {
    m_tokenizer.nextToken();
    token = m_tokenizer.peekToken(FgdToken::String | FgdToken::Colon);
    if (token.type() == FgdToken::String)
    {
      return parseString();
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
    token = m_tokenizer.peekToken(
      FgdToken::String | FgdToken::Colon | FgdToken::Integer | FgdToken::Decimal);
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
    token =
      m_tokenizer.peekToken(FgdToken::Integer | FgdToken::Decimal | FgdToken::Colon);
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
    token = m_tokenizer.peekToken(
      FgdToken::String | FgdToken::Decimal | FgdToken::Integer | FgdToken::Colon);
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

std::optional<std::string> FgdParser::parseDefaultChoiceValue()
{
  auto token = m_tokenizer.peekToken();
  if (token.type() == FgdToken::Colon)
  {
    m_tokenizer.nextToken();
    token = m_tokenizer.peekToken(
      FgdToken::String | FgdToken::Integer | FgdToken::Decimal | FgdToken::Colon);
    if (token.hasType(FgdToken::String | FgdToken::Integer | FgdToken::Decimal))
    {
      token = m_tokenizer.nextToken();
      return token.data();
    }
  }
  return std::nullopt;
}

std::tuple<std::string, mdl::PropertyValueTypes::IOParameterType> FgdParser::
  parseIOProperty(ParserStatus&)
{
  auto token = m_tokenizer.nextToken(FgdToken::Word);
  auto propertyKey = token.data();

  m_tokenizer.nextToken(FgdToken::OParenthesis);

  token = m_tokenizer.nextToken(FgdToken::Word);
  const auto parameterTypeName = token.data();
  const auto parameterTypeLocation = token.location();

  m_tokenizer.nextToken(FgdToken::CParenthesis);

  return parseIOParameterType(parameterTypeName)
         | kdl::transform([&](const auto parameterType) {
             return std::tuple{std::move(propertyKey), parameterType};
           })
         | kdl::if_error(
           [&](const auto& e) { throw ParserException{parameterTypeLocation, e.msg}; })
         | kdl::value();
}

vm::vec3d FgdParser::parseVector()
{
  auto vec = vm::vec3d{};
  for (size_t i = 0; i < 3; i++)
  {
    auto token = m_tokenizer.nextToken(FgdToken::Integer | FgdToken::Decimal);
    vec[i] = token.toFloat<double>();
  }
  return vec;
}

vm::bbox3d FgdParser::parseSize()
{
  auto size = vm::bbox3d{};
  m_tokenizer.nextToken(FgdToken::OParenthesis);
  size.min = parseVector();

  auto token = m_tokenizer.nextToken(FgdToken::CParenthesis | FgdToken::Comma);
  if (token.type() == FgdToken::Comma)
  {
    size.max = parseVector();
    m_tokenizer.nextToken(FgdToken::CParenthesis);
  }
  else
  {
    const auto halfSize = size.min / 2.0;
    size.min = -halfSize;
    size.max = halfSize;
  }
  return repair(size);
}

Color FgdParser::parseColor()
{
  auto token = m_tokenizer.nextToken(FgdToken::OParenthesis);
  const auto location = token.location();

  auto vec = vm::vec3f{};
  for (size_t i = 0; i < 3; i++)
  {
    token = m_tokenizer.nextToken(FgdToken::Decimal | FgdToken::Integer);
    vec[i] = token.toFloat<float>();
  }
  m_tokenizer.nextToken(FgdToken::CParenthesis);

  return Color::fromVec(vec)
         | kdl::if_error([&](const auto& e) { throw ParserException{location, e.msg}; })
         | kdl::value();
}

std::string FgdParser::parseString()
{
  auto token = m_tokenizer.nextToken(FgdToken::String);
  if (m_tokenizer.peekToken().hasType(FgdToken::Plus))
  {
    auto str = std::stringstream{};
    str << token.data();
    do
    {
      m_tokenizer.nextToken();
      token = m_tokenizer.nextToken(FgdToken::String);
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
  auto token = m_tokenizer.nextToken(FgdToken::Word);
  contract_assert(kdl::ci::str_is_equal(token.data(), "@include"));

  token = m_tokenizer.nextToken(FgdToken::String);
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

  status.debug(m_tokenizer.location(), fmt::format("Parsing included file '{}'", path));

  const auto filePath = currentRoot() / path;
  return m_fs->openFile(filePath) | kdl::transform([&](auto file) {
           status.debug(
             m_tokenizer.location(),
             fmt::format("Resolved '{}' to '{}'", path, filePath));

           if (isRecursiveInclude(filePath))
           {
             status.error(
               m_tokenizer.location(),
               fmt::format(
                 "Skipping recursively included file: {} ({})", path, filePath));
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
