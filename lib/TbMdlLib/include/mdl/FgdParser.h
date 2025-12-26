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

#pragma once

#include "Color.h"
#include "Parser.h"
#include "Tokenizer.h"
#include "mdl/EntityDefinitionParser.h"

#include "vm/bbox.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace tb
{
class ParserStatus;

struct FileLocation;

namespace fs
{
class FileSystem;
} // namespace fs

namespace mdl
{
class DecalDefinition;
class ModelDefinition;

enum class ColorType
{
  Color1,
  Color255,
};

enum class EntityDefinitionClassType;

struct EntityDefinitionClassInfo;

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

  std::vector<std::filesystem::path> m_paths;
  std::unique_ptr<fs::FileSystem> m_fs;

  FgdTokenizer m_tokenizer;

public:
  FgdParser(
    std::string_view str,
    const Color& defaultEntityColor,
    const std::filesystem::path& path);
  FgdParser(std::string_view str, const Color& defaultEntityColor);

  ~FgdParser() override;

private:
  class PushIncludePath;
  void pushIncludePath(std::filesystem::path path);
  void popIncludePath();

  std::filesystem::path currentRoot() const;
  bool isRecursiveInclude(const std::filesystem::path& path) const;

private:
  std::vector<EntityDefinitionClassInfo> parseClassInfos(ParserStatus& status) override;

  void parseClassInfoOrInclude(
    ParserStatus& status, std::vector<EntityDefinitionClassInfo>& classInfos);

  std::optional<EntityDefinitionClassInfo> parseClassInfo(ParserStatus& status);
  EntityDefinitionClassInfo parseSolidClassInfo(ParserStatus& status);
  EntityDefinitionClassInfo parsePointClassInfo(ParserStatus& status);
  EntityDefinitionClassInfo parseBaseClassInfo(ParserStatus& status);
  EntityDefinitionClassInfo parseClassInfo(
    ParserStatus& status, EntityDefinitionClassType classType);
  void skipMainClass();

  std::vector<std::string> parseSuperClasses();
  ModelDefinition parseModel(ParserStatus& status, bool allowEmptyExpression);
  DecalDefinition parseDecal();
  std::string parseNamedValue(ParserStatus& status, const std::string& name);
  void skipClassProperty(ParserStatus& status);

  std::vector<PropertyDefinition> parsePropertyDefinitions(ParserStatus& status);
  PropertyDefinition parsePropertyDefinition(ParserStatus& status);
  PropertyDefinition parseTargetSourcePropertyDefinition(
    ParserStatus& status, std::string propertyKey);
  PropertyDefinition parseTargetDestinationPropertyDefinition(
    ParserStatus& status, std::string propertyKey);
  PropertyDefinition parseStringPropertyDefinition(
    ParserStatus& status, std::string propertyKey);
  PropertyDefinition parseIntegerPropertyDefinition(
    ParserStatus& status, std::string propertyKey);
  PropertyDefinition parseFloatPropertyDefinition(
    ParserStatus& status, std::string propertyKey);
  PropertyDefinition parseChoicesPropertyDefinition(
    ParserStatus& status, std::string propertyKey);
  PropertyDefinition parseFlagsPropertyDefinition(std::string propertyKey);
  PropertyDefinition parseOriginPropertyDefinition(
    ParserStatus& status, std::string propertyKey);
  PropertyDefinition parseInputPropertyDefinition(ParserStatus& status);
  PropertyDefinition parseOutputPropertyDefinition(ParserStatus& status);
  PropertyDefinition parseColorPropertyDefinition(
    ParserStatus& status, ColorType colorType, std::string propertyKey);
  PropertyDefinition parseUnknownPropertyDefinition(
    ParserStatus& status, std::string propertyKey);

  bool parseReadOnlyFlag(ParserStatus& status);
  std::string parsePropertyDescription();
  std::optional<std::string> parseDefaultStringValue(ParserStatus& status);
  std::optional<int> parseDefaultIntegerValue(ParserStatus& status);
  std::optional<float> parseDefaultFloatValue(ParserStatus& status);
  std::optional<std::string> parseDefaultChoiceValue();
  std::tuple<std::string, PropertyValueTypes::IOParameterType> parseIOProperty(
    ParserStatus& status);

  vm::vec3d parseVector();
  vm::bbox3d parseSize();
  Color parseColor();
  std::string parseString();

  std::vector<EntityDefinitionClassInfo> parseInclude(ParserStatus& status);
  std::vector<EntityDefinitionClassInfo> handleInclude(
    ParserStatus& status, const std::filesystem::path& path);
};

} // namespace mdl
} // namespace tb
