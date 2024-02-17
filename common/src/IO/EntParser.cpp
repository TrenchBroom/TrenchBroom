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

#include "EntParser.h"

#include "Assets/EntityDefinition.h"
#include "Assets/PropertyDefinition.h"
#include "EL/ELExceptions.h"
#include "EL/Expressions.h"
#include "EL/Types.h"
#include "EL/Value.h"
#include "IO/ELParser.h"
#include "IO/EntityDefinitionClassInfo.h"
#include "IO/ParserStatus.h"
#include "Model/EntityProperties.h"

#include "kdl/string_compare.h"
#include "kdl/string_utils.h"

#include "vecmath/vec_io.h"

#include <fmt/format.h>

#include <cstdlib>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <tinyxml2.h>

namespace TrenchBroom::IO
{

namespace
{

std::string_view getName(const tinyxml2::XMLElement& element)
{
  return element.Name() ? std::string_view{element.Name()} : std::string_view{};
}

bool hasAttribute(
  const tinyxml2::XMLElement& element, const std::string_view attributeName)
{
  return element.Attribute(attributeName.data()) != nullptr;
}

std::string_view getAttribute(
  const tinyxml2::XMLElement& element, const std::string_view attributeName)
{
  const auto* attribute = element.Attribute(attributeName.data());
  return attribute ? std::string_view{attribute} : std::string_view{};
}

void warn(
  const tinyxml2::XMLElement& element, const std::string_view msg, ParserStatus& status)
{
  const auto str = fmt::format("{}: {}", msg, getName(element));
  if (element.GetLineNum() > 0)
  {
    status.warn(static_cast<size_t>(element.GetLineNum()), str);
  }
  else
  {
    status.warn(str);
  }
}

bool expectAttribute(
  const tinyxml2::XMLElement& element,
  const std::string& attributeName,
  ParserStatus& status)
{
  if (!hasAttribute(element, attributeName))
  {
    warn(element, fmt::format("Expected attribute '{}'", attributeName), status);
    return false;
  }
  return true;
}

std::string getText(const tinyxml2::XMLElement& element)
{
  // I assume that only the initial and the last text is meaningful.

  auto str = std::stringstream{};
  const auto* first = element.FirstChild();
  const auto* last = element.LastChild();

  if (first && first->ToText())
  {
    str << first->Value();
  }

  if (last && last != first && last->ToText())
  {
    str << last->Value();
  }

  return str.str();
}

std::string parseString(
  const tinyxml2::XMLElement& element, const std::string& attributeName)
{
  return std::string{getAttribute(element, attributeName)};
}

std::optional<size_t> parseSize(
  const tinyxml2::XMLElement& element, const std::string& attributeName)
{
  return kdl::str_to_size(getAttribute(element, attributeName));
}

std::optional<int> parseInteger(
  const tinyxml2::XMLElement& element, const std::string& attributeName)
{
  return kdl::str_to_int(getAttribute(element, attributeName));
}

std::optional<bool> parseBoolean(
  const tinyxml2::XMLElement& element, const std::string& attributeName)
{
  const auto strValue = getAttribute(element, attributeName);
  if (kdl::ci::str_is_equal(strValue, "true"))
  {
    return true;
  }
  if (kdl::ci::str_is_equal(strValue, "false"))
  {
    return false;
  }

  const auto intValue = kdl::str_to_int(strValue);
  if (intValue != 0)
  {
    return true;
  }
  if (intValue == 0)
  {
    return false;
  }

  return std::nullopt;
}

std::optional<float> parseFloat(
  const tinyxml2::XMLElement& element, const std::string& attributeName)
{
  return kdl::str_to_float(getAttribute(element, attributeName));
}

std::optional<Color> parseColor(
  const tinyxml2::XMLElement& element, const std::string& attributeName)
{
  return Color::parse(parseString(element, attributeName));
}

std::optional<vm::bbox3> parseBounds(
  const tinyxml2::XMLElement& element,
  const std::string& attributeName,
  ParserStatus& status)
{
  const auto parts = kdl::str_split(parseString(element, attributeName), " ");
  if (parts.size() == 6)
  {
    const auto it = std::begin(parts);
    const auto mid = std::next(it, 3);
    const auto end = std::end(parts);

    const auto min = vm::parse<FloatType, 3>(kdl::str_join(it, mid, " "));
    const auto max = vm::parse<FloatType, 3>(kdl::str_join(mid, end, " "));
    if (min && max)
    {
      return vm::bbox3{*min, *max};
    }
  }

  warn(element, "Invalid bounding box", status);
  return std::nullopt;
}

std::unique_ptr<Assets::PropertyDefinition> parseListDeclaration(
  const tinyxml2::XMLElement& element, ParserStatus& status)
{
  if (expectAttribute(element, "name", status))
  {
    auto name = parseString(element, "name");
    auto options = Assets::ChoicePropertyOption::List{};

    const auto* itemElement = element.FirstChildElement("item");
    while (itemElement)
    {
      if (
        expectAttribute(*itemElement, "name", status)
        && expectAttribute(*itemElement, "value", status))
      {
        auto itemName = parseString(*itemElement, "name");
        auto itemValue = parseString(*itemElement, "value");
        options.emplace_back(std::move(itemValue), std::move(itemName));
      }
      itemElement = itemElement->NextSiblingElement("item");
    }
    return std::make_unique<Assets::ChoicePropertyDefinition>(
      std::move(name), "", "", std::move(options), false);
  }
  return nullptr;
}

std::unique_ptr<Assets::PropertyDefinition> parsePropertyDeclaration(
  const tinyxml2::XMLElement& element, ParserStatus& status)
{
  return getName(element) == "list" ? parseListDeclaration(element, status) : nullptr;
}

using PropertyDefinitionFactory =
  std::function<std::unique_ptr<Assets::PropertyDefinition>(
    std::string, std::string, std::string)>;

std::unique_ptr<Assets::PropertyDefinition> parsePropertyDefinition(
  const tinyxml2::XMLElement& element,
  const PropertyDefinitionFactory& factory,
  ParserStatus& status)
{
  if (expectAttribute(element, "key", status) && expectAttribute(element, "name", status))
  {
    auto name = parseString(element, "key");
    auto shortDesc = parseString(element, "name");
    auto longDesc = getText(element);

    return factory(std::move(name), std::move(shortDesc), std::move(longDesc));
  }
  return nullptr;
}

std::unique_ptr<Assets::PropertyDefinition> parseDeclaredPropertyDefinition(
  const tinyxml2::XMLElement& element,
  const Assets::PropertyDefinition& propertyDeclaration,
  ParserStatus& status)
{
  auto factory = [&propertyDeclaration](
                   std::string name, std::string shortDesc, std::string longDesc) {
    return propertyDeclaration.clone(name, shortDesc, longDesc, false);
  };
  return parsePropertyDefinition(element, factory, status);
}

std::unique_ptr<Assets::PropertyDefinition> parseTargetNamePropertyDefinition(
  const tinyxml2::XMLElement& element, ParserStatus& status)
{
  auto factory = [](std::string name, std::string shortDesc, std::string longDesc) {
    return std::make_unique<Assets::PropertyDefinition>(
      std::move(name),
      Assets::PropertyDefinitionType::TargetSourceProperty,
      std::move(shortDesc),
      std::move(longDesc),
      false);
  };
  return parsePropertyDefinition(element, factory, status);
}

std::unique_ptr<Assets::PropertyDefinition> parseTargetPropertyDefinition(
  const tinyxml2::XMLElement& element, ParserStatus& status)
{
  auto factory = [](std::string name, std::string shortDesc, std::string longDesc) {
    return std::make_unique<Assets::PropertyDefinition>(
      std::move(name),
      Assets::PropertyDefinitionType::TargetDestinationProperty,
      std::move(shortDesc),
      std::move(longDesc),
      false);
  };
  return parsePropertyDefinition(element, factory, status);
}

std::unique_ptr<Assets::PropertyDefinition> parseRealPropertyDefinition(
  const tinyxml2::XMLElement& element, ParserStatus& status)
{
  auto factory = [&](std::string name, std::string shortDesc, std::string longDesc)
    -> std::unique_ptr<Assets::PropertyDefinition> {
    if (hasAttribute(element, "value"))
    {
      auto floatDefaultValue = parseFloat(element, "value");
      if (floatDefaultValue)
      {
        return std::make_unique<Assets::FloatPropertyDefinition>(
          std::move(name),
          std::move(shortDesc),
          std::move(longDesc),
          false,
          std::move(floatDefaultValue));
      }

      auto strDefaultValue = parseString(element, "value");
      warn(
        element,
        fmt::format(
          "Invalid default value '{}' for float property definition", strDefaultValue),
        status);
      return std::make_unique<Assets::UnknownPropertyDefinition>(
        std::move(name),
        std::move(shortDesc),
        std::move(longDesc),
        false,
        std::move(strDefaultValue));
    }
    return std::make_unique<Assets::FloatPropertyDefinition>(
      std::move(name), std::move(shortDesc), std::move(longDesc), false);
  };
  return parsePropertyDefinition(element, factory, status);
}

std::unique_ptr<Assets::PropertyDefinition> parseIntegerPropertyDefinition(
  const tinyxml2::XMLElement& element, ParserStatus& status)
{
  auto factory = [&](std::string name, std::string shortDesc, std::string longDesc)
    -> std::unique_ptr<Assets::PropertyDefinition> {
    if (hasAttribute(element, "value"))
    {
      auto intDefaultValue = parseInteger(element, "value");
      if (intDefaultValue)
      {
        return std::make_unique<Assets::IntegerPropertyDefinition>(
          std::move(name),
          std::move(shortDesc),
          std::move(longDesc),
          false,
          std::move(intDefaultValue));
      }

      auto strDefaultValue = parseString(element, "value");
      warn(
        element,
        fmt::format(
          "Invalid default value '{}' for integer property definition", strDefaultValue),
        status);
      return std::make_unique<Assets::UnknownPropertyDefinition>(
        std::move(name),
        std::move(shortDesc),
        std::move(longDesc),
        false,
        std::move(strDefaultValue));
    }

    return std::make_unique<Assets::IntegerPropertyDefinition>(
      std::move(name), std::move(shortDesc), std::move(longDesc), false);
  };
  return parsePropertyDefinition(element, factory, status);
}

std::unique_ptr<Assets::PropertyDefinition> parseBooleanPropertyDefinition(
  const tinyxml2::XMLElement& element, ParserStatus& status)
{
  auto factory = [&](std::string name, std::string shortDesc, std::string longDesc)
    -> std::unique_ptr<Assets::PropertyDefinition> {
    if (hasAttribute(element, "value"))
    {
      if (const auto boolDefaultValue = parseBoolean(element, "value"))
      {
        return std::make_unique<Assets::BooleanPropertyDefinition>(
          std::move(name),
          std::move(shortDesc),
          std::move(longDesc),
          false,
          *boolDefaultValue);
      }

      auto strDefaultValue = parseString(element, "value");
      warn(
        element,
        fmt::format(
          "Invalid default value '{}' for boolean property definition", strDefaultValue),
        status);
      return std::make_unique<Assets::UnknownPropertyDefinition>(
        std::move(name),
        std::move(shortDesc),
        std::move(longDesc),
        false,
        std::move(strDefaultValue));
    }

    return std::make_unique<Assets::BooleanPropertyDefinition>(
      std::move(name), std::move(shortDesc), std::move(longDesc), false);
  };
  return parsePropertyDefinition(element, factory, status);
}

std::unique_ptr<Assets::PropertyDefinition> parseStringPropertyDefinition(
  const tinyxml2::XMLElement& element, ParserStatus& status)
{
  auto factory = [&](std::string name, std::string shortDesc, std::string longDesc) {
    auto defaultValue = hasAttribute(element, "value")
                          ? std::optional(parseString(element, "value"))
                          : std::nullopt;
    return std::make_unique<Assets::StringPropertyDefinition>(
      std::move(name),
      std::move(shortDesc),
      std::move(longDesc),
      false,
      std::move(defaultValue));
  };
  return parsePropertyDefinition(element, factory, status);
}

std::unique_ptr<Assets::PropertyDefinition> parseUnknownPropertyDefinition(
  const tinyxml2::XMLElement& element, ParserStatus& status)
{
  auto factory = [&](std::string name, std::string shortDesc, std::string longDesc) {
    auto defaultValue = hasAttribute(element, "value")
                          ? std::optional(parseString(element, "value"))
                          : std::nullopt;
    return std::make_unique<Assets::UnknownPropertyDefinition>(
      std::move(name),
      std::move(shortDesc),
      std::move(longDesc),
      false,
      std::move(defaultValue));
  };
  return parsePropertyDefinition(element, factory, status);
}

std::unique_ptr<Assets::PropertyDefinition> parsePropertyDefinition(
  const tinyxml2::XMLElement& element,
  const std::vector<std::shared_ptr<Assets::PropertyDefinition>>& propertyDeclarations,
  ParserStatus& status)
{
  if (getName(element) == "angle")
  {
    return parseUnknownPropertyDefinition(element, status);
  }
  if (getName(element) == "angles")
  {
    return parseUnknownPropertyDefinition(element, status);
  }
  if (getName(element) == "direction")
  {
    return parseUnknownPropertyDefinition(element, status);
  }
  if (getName(element) == "boolean")
  {
    return parseBooleanPropertyDefinition(element, status);
  }
  if (getName(element) == "integer")
  {
    return parseIntegerPropertyDefinition(element, status);
  }
  if (getName(element) == "real")
  {
    return parseRealPropertyDefinition(element, status);
  }
  if (getName(element) == "string")
  {
    return parseStringPropertyDefinition(element, status);
  }
  if (getName(element) == "target")
  {
    return parseTargetPropertyDefinition(element, status);
  }
  if (getName(element) == "targetname")
  {
    return parseTargetNamePropertyDefinition(element, status);
  }
  if (getName(element) == "texture")
  {
    return parseUnknownPropertyDefinition(element, status);
  }
  if (getName(element) == "sound")
  {
    return parseUnknownPropertyDefinition(element, status);
  }
  if (getName(element) == "model")
  {
    return parseUnknownPropertyDefinition(element, status);
  }
  if (getName(element) == "color")
  {
    return parseUnknownPropertyDefinition(element, status);
  }

  for (const auto& propertyDeclaration : propertyDeclarations)
  {
    if (getName(element) == propertyDeclaration->key())
    {
      return parseDeclaredPropertyDefinition(element, *propertyDeclaration, status);
    }
  }

  return nullptr;
}

std::vector<std::shared_ptr<Assets::PropertyDefinition>> parsePropertyDefinitions(
  const tinyxml2::XMLElement& parent,
  const std::vector<std::shared_ptr<Assets::PropertyDefinition>>& propertyDeclarations,
  ParserStatus& status)
{
  auto result = std::vector<std::shared_ptr<Assets::PropertyDefinition>>{};

  const auto* element = parent.FirstChildElement();
  while (element)
  {
    if (
      auto propertyDefinition =
        parsePropertyDefinition(*element, propertyDeclarations, status))
    {
      result.push_back(std::move(propertyDefinition));
    }
    element = element->NextSiblingElement();
  }

  return result;
}

std::unique_ptr<Assets::PropertyDefinition> parseSpawnflags(
  const tinyxml2::XMLElement& element, ParserStatus& status)
{
  if (const auto* flagElement = element.FirstChildElement("flag"))
  {
    auto result = std::make_unique<Assets::FlagsPropertyDefinition>(
      Model::EntityPropertyKeys::Spawnflags);
    do
    {
      const auto bit = parseSize(*flagElement, "bit");
      if (!bit)
      {
        const auto strValue = parseString(*flagElement, "bit");
        warn(
          *flagElement,
          fmt::format("Invalid value '{}' for bit property definition", strValue),
          status);
      }
      else
      {
        const auto value = 1 << *bit;
        auto shortDesc = parseString(*flagElement, "key");
        auto longDesc = parseString(*flagElement, "name");
        result->addOption(value, std::move(shortDesc), std::move(longDesc), false);
      }

      flagElement = flagElement->NextSiblingElement("flag");
    } while (flagElement);

    return result;
  }
  return nullptr;
}

void parsePropertyDefinitions(
  const tinyxml2::XMLElement& element,
  const std::vector<std::shared_ptr<Assets::PropertyDefinition>>& propertyDeclarations,
  EntityDefinitionClassInfo& classInfo,
  ParserStatus& status)
{
  if (auto spawnflags = parseSpawnflags(element, status))
  {
    addPropertyDefinition(classInfo.propertyDefinitions, std::move(spawnflags));
  }

  for (auto propertyDefinition :
       parsePropertyDefinitions(element, propertyDeclarations, status))
  {
    if (!addPropertyDefinition(
          classInfo.propertyDefinitions, std::move(propertyDefinition)))
    {
      const auto line = static_cast<size_t>(element.GetLineNum());
      status.warn(line, 0, "Skipping duplicate entity property definition");
    }
  }
}

Assets::ModelDefinition parseModel(const tinyxml2::XMLElement& element)
{
  if (!hasAttribute(element, "model"))
  {
    return Assets::ModelDefinition{};
  }

  const auto model = parseString(element, "model");
  try
  {
    auto parser = ELParser{ELParser::Mode::Lenient, model};
    auto expression = parser.parse();
    expression.optimize();
    return Assets::ModelDefinition{std::move(expression)};
  }
  catch (const ParserException&)
  {
    const auto lineNum = static_cast<size_t>(element.GetLineNum());
    auto expression = EL::Expression{
      EL::LiteralExpression{EL::Value{EL::MapType{{
        {Assets::ModelSpecificationKeys::Path, EL::Value{model}},
      }}}},
      lineNum,
      0};
    return Assets::ModelDefinition{std::move(expression)};
  }
  catch (const EL::EvaluationError& evaluationError)
  {
    const auto line = static_cast<size_t>(element.GetLineNum());
    throw ParserException{line, evaluationError.what()};
  }
}

EntityDefinitionClassInfo parsePointClassInfo(
  const tinyxml2::XMLElement& element,
  const std::vector<std::shared_ptr<Assets::PropertyDefinition>>& propertyDeclarations,
  ParserStatus& status)
{
  auto classInfo = EntityDefinitionClassInfo{};
  classInfo.type = EntityDefinitionClassType::PointClass;
  classInfo.line = static_cast<size_t>(element.GetLineNum());
  classInfo.column = 0;
  classInfo.name = parseString(element, "name");
  classInfo.description = getText(element);
  classInfo.color = parseColor(element, "color");
  classInfo.size = parseBounds(element, "box", status);
  classInfo.modelDefinition = parseModel(element);
  parsePropertyDefinitions(element, propertyDeclarations, classInfo, status);

  return classInfo;
}

EntityDefinitionClassInfo parseBrushClassInfo(
  const tinyxml2::XMLElement& element,
  const std::vector<std::shared_ptr<Assets::PropertyDefinition>>& propertyDeclarations,
  ParserStatus& status)
{
  auto classInfo = EntityDefinitionClassInfo{};
  classInfo.type = EntityDefinitionClassType::BrushClass;
  classInfo.line = static_cast<size_t>(element.GetLineNum());
  classInfo.column = 0;
  classInfo.name = parseString(element, "name");
  classInfo.description = getText(element);
  classInfo.color = parseColor(element, "color");
  parsePropertyDefinitions(element, propertyDeclarations, classInfo, status);

  return classInfo;
}

std::optional<EntityDefinitionClassInfo> parseClassInfo(
  const tinyxml2::XMLElement& element,
  const std::vector<std::shared_ptr<Assets::PropertyDefinition>>& propertyDeclarations,
  ParserStatus& status)
{
  if (getName(element) == "point")
  {
    return parsePointClassInfo(element, propertyDeclarations, status);
  }

  if (getName(element) == "group")
  {
    return parseBrushClassInfo(element, propertyDeclarations, status);
  }

  warn(element, "Unexpected XML element", status);
  return std::nullopt;
}

std::vector<EntityDefinitionClassInfo> parseClassInfosFromDocument(
  const tinyxml2::XMLDocument& document, ParserStatus& status)
{
  auto result = std::vector<EntityDefinitionClassInfo>{};
  auto propertyDeclarations = std::vector<std::shared_ptr<Assets::PropertyDefinition>>{};

  if (const auto* classesNode = document.FirstChildElement("classes"))
  {
    const auto* currentElement = classesNode->FirstChildElement();
    while (currentElement)
    {
      if (getName(*currentElement) == "point" || getName(*currentElement) == "group")
      {
        if (
          auto classInfo = parseClassInfo(*currentElement, propertyDeclarations, status))
        {
          result.push_back(std::move(*classInfo));
        }
      }
      else
      {
        if (auto propertyDeclaration = parsePropertyDeclaration(*currentElement, status))
        {
          if (!addPropertyDefinition(
                propertyDeclarations, std::move(propertyDeclaration)))
          {
            const auto line = static_cast<size_t>(currentElement->GetLineNum());
            status.warn(line, 0, "Skipping duplicate entity property declaration");
          }
        }
      }
      currentElement = currentElement->NextSiblingElement();
    }
  }
  return result;
}

} // namespace

EntParser::EntParser(std::string_view str, const Color& defaultEntityColor)
  : EntityDefinitionParser{defaultEntityColor}
  , m_str{str}
{
}

std::vector<EntityDefinitionClassInfo> EntParser::parseClassInfos(ParserStatus& status)
{
  auto doc = tinyxml2::XMLDocument{};
  doc.Parse(m_str.data(), m_str.length());

  if (doc.Error())
  {
    if (doc.ErrorID() == tinyxml2::XML_ERROR_EMPTY_DOCUMENT)
    {
      // we allow empty documents
      return {};
    }
    const auto lineNum = static_cast<size_t>(doc.ErrorLineNum());
    throw ParserException{lineNum, doc.ErrorStr()};
  }

  return parseClassInfosFromDocument(doc, status);
}


} // namespace TrenchBroom::IO
