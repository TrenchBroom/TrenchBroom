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

#pragma once

#include "Color.h"
#include "FloatType.h"
#include "IO/EntityDefinitionParser.h"
#include "IO/Parser.h"
#include "IO/Tokenizer.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Assets
{
class ModelDefinition;
}

namespace IO
{
struct EntityDefinitionClassInfo;
enum class EntityDefinitionClassType;
class FileSystem;
class ParserStatus;
class Path;

namespace FgdToken
{
using Type = unsigned int;
static const Type Integer = 1 << 0;      // integer number
static const Type Decimal = 1 << 1;      // decimal number
static const Type Word = 1 << 2;         // letter or digits, no whitespace
static const Type String = 1 << 3;       // "anything but quotes"
static const Type OParenthesis = 1 << 4; // opening parenthesis: (
static const Type CParenthesis = 1 << 5; // closing parenthesis: )
static const Type OBracket = 1 << 6;     // opening bracket: [
static const Type CBracket = 1 << 7;     // closing bracket: ]
static const Type Equality = 1 << 8;     // equality sign: =
static const Type Colon = 1 << 9;        // colon: :
static const Type Comma = 1 << 10;       // comma: ,
static const Type Plus = 1 << 11;        // plus: + (not used in string continuations)
static const Type Eof = 1 << 12;         // end of file
} // namespace FgdToken

class FgdTokenizer : public Tokenizer<FgdToken::Type>
{
public:
  explicit FgdTokenizer(std::string_view str);

private:
  static const std::string WordDelims;
  Token emitToken() override;
};

class FgdParser : public EntityDefinitionParser, public Parser<FgdToken::Type>
{
private:
  using Token = FgdTokenizer::Token;

  std::vector<Path> m_paths;
  std::shared_ptr<FileSystem> m_fs;

  FgdTokenizer m_tokenizer;

public:
  FgdParser(std::string_view str, const Color& defaultEntityColor, const Path& path);
  FgdParser(std::string_view str, const Color& defaultEntityColor);

private:
  class PushIncludePath;
  void pushIncludePath(const Path& path);
  void popIncludePath();

  Path currentRoot() const;
  bool isRecursiveInclude(const Path& path) const;

private:
  TokenNameMap tokenNames() const override;

  std::vector<EntityDefinitionClassInfo> parseClassInfos(ParserStatus& status) override;

  void parseClassInfoOrInclude(
    ParserStatus& status, std::vector<EntityDefinitionClassInfo>& classInfos);

  std::optional<EntityDefinitionClassInfo> parseClassInfo(ParserStatus& status);
  EntityDefinitionClassInfo parseSolidClassInfo(ParserStatus& status);
  EntityDefinitionClassInfo parsePointClassInfo(ParserStatus& status);
  EntityDefinitionClassInfo parseBaseClassInfo(ParserStatus& status);
  EntityDefinitionClassInfo parseClassInfo(
    ParserStatus& status, EntityDefinitionClassType classType);
  void skipMainClass(ParserStatus& status);

  std::vector<std::string> parseSuperClasses(ParserStatus& status);
  Assets::ModelDefinition parseModel(ParserStatus& status);
  std::string parseNamedValue(ParserStatus& status, const std::string& name);
  void skipClassProperty(ParserStatus& status);

  PropertyDefinitionList parsePropertyDefinitions(ParserStatus& status);
  PropertyDefinitionPtr parsePropertyDefinition(
    ParserStatus& status,
    const std::string& propertyKey,
    const std::string& typeName,
    size_t line,
    size_t column);
  PropertyDefinitionPtr parseTargetSourcePropertyDefinition(
    ParserStatus& status, const std::string& propertyKey);
  PropertyDefinitionPtr parseTargetDestinationPropertyDefinition(
    ParserStatus& status, const std::string& propertyKey);
  PropertyDefinitionPtr parseStringPropertyDefinition(
    ParserStatus& status, const std::string& propertyKey);
  PropertyDefinitionPtr parseIntegerPropertyDefinition(
    ParserStatus& status, const std::string& propertyKey);
  PropertyDefinitionPtr parseFloatPropertyDefinition(
    ParserStatus& status, const std::string& propertyKey);
  PropertyDefinitionPtr parseChoicesPropertyDefinition(
    ParserStatus& status, const std::string& propertyKey);
  PropertyDefinitionPtr parseFlagsPropertyDefinition(
    ParserStatus& status, const std::string& propertyKey);
  PropertyDefinitionPtr parseUnknownPropertyDefinition(
    ParserStatus& status, const std::string& propertyKey);

  bool parseReadOnlyFlag(ParserStatus& status);
  std::string parsePropertyDescription(ParserStatus& status);
  std::optional<std::string> parseDefaultStringValue(ParserStatus& status);
  std::optional<int> parseDefaultIntegerValue(ParserStatus& status);
  std::optional<float> parseDefaultFloatValue(ParserStatus& status);
  std::optional<std::string> parseDefaultChoiceValue(ParserStatus& status);

  vm::vec3 parseVector(ParserStatus& status);
  vm::bbox3 parseSize(ParserStatus& status);
  Color parseColor(ParserStatus& status);
  std::string parseString(ParserStatus& status);

  std::vector<EntityDefinitionClassInfo> parseInclude(ParserStatus& status);
  std::vector<EntityDefinitionClassInfo> handleInclude(
    ParserStatus& status, const Path& path);
};
} // namespace IO
} // namespace TrenchBroom
