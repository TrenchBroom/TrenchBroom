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

#include <kdl/string_utils.h>

#include <vecmath/vec_io.h>

#include <tinyxml2.h>

#include <cstdlib>
#include <memory>
#include <sstream>
#include <string>

namespace TrenchBroom {
namespace IO {
EntParser::EntParser(std::string_view str, const Color& defaultEntityColor)
  : EntityDefinitionParser(defaultEntityColor)
  , m_begin(str.data())
  , m_end(str.data() + str.size()) {}

std::vector<EntityDefinitionClassInfo> EntParser::parseClassInfos(ParserStatus& status) {
  tinyxml2::XMLDocument doc;
  doc.Parse(m_begin, static_cast<size_t>(m_end - m_begin));
  if (doc.Error()) {
    if (doc.ErrorID() == tinyxml2::XML_ERROR_EMPTY_DOCUMENT) {
      // we allow empty documents
      return {};
    } else {
      const auto lineNum = static_cast<size_t>(doc.ErrorLineNum());
      const auto error = std::string(doc.ErrorStr());
      throw ParserException(lineNum, error);
    }
  }
  return parseClassInfos(doc, status);
}

std::vector<EntityDefinitionClassInfo> EntParser::parseClassInfos(
  const tinyxml2::XMLDocument& document, ParserStatus& status) {
  std::vector<EntityDefinitionClassInfo> result;
  PropertyDefinitionList propertyDeclarations;

  const auto* classesNode = document.FirstChildElement("classes");
  if (classesNode != nullptr) {
    const auto* currentElement = classesNode->FirstChildElement();
    while (currentElement != nullptr) {
      if (
        !std::strcmp(currentElement->Name(), "point") ||
        !std::strcmp(currentElement->Name(), "group")) {
        if (auto classInfo = parseClassInfo(*currentElement, propertyDeclarations, status)) {
          result.push_back(std::move(*classInfo));
        }
      } else {
        // interpret this as an property declaration
        parsePropertyDeclaration(*currentElement, propertyDeclarations, status);
      }
      currentElement = currentElement->NextSiblingElement();
    }
  }
  return result;
}

std::optional<EntityDefinitionClassInfo> EntParser::parseClassInfo(
  const tinyxml2::XMLElement& element, const PropertyDefinitionList& propertyDeclarations,
  ParserStatus& status) {
  if (!std::strcmp(element.Name(), "point")) {
    return parsePointClassInfo(element, propertyDeclarations, status);
  } else if (!std::strcmp(element.Name(), "group")) {
    return parseBrushClassInfo(element, propertyDeclarations, status);
  } else {
    warn(element, "Unexpected XML element", status);
    return std::nullopt;
  }
}

EntityDefinitionClassInfo EntParser::parsePointClassInfo(
  const tinyxml2::XMLElement& element, const PropertyDefinitionList& propertyDeclarations,
  ParserStatus& status) {
  EntityDefinitionClassInfo classInfo;
  ;
  classInfo.type = EntityDefinitionClassType::PointClass;
  classInfo.line = static_cast<size_t>(element.GetLineNum());
  classInfo.column = 0;
  classInfo.name = parseString(element, "name", status);
  classInfo.description = getText(element);
  classInfo.color = parseColor(element, "color", status);
  classInfo.size = parseBounds(element, "box", status);
  classInfo.modelDefinition = parseModel(element, status);

  parseSpawnflags(element, classInfo.propertyDefinitions, status);
  parsePropertyDefinitions(element, propertyDeclarations, classInfo.propertyDefinitions, status);

  return classInfo;
}

EntityDefinitionClassInfo EntParser::parseBrushClassInfo(
  const tinyxml2::XMLElement& element, const PropertyDefinitionList& propertyDeclarations,
  ParserStatus& status) {
  EntityDefinitionClassInfo classInfo;
  ;
  classInfo.type = EntityDefinitionClassType::BrushClass;
  classInfo.line = static_cast<size_t>(element.GetLineNum());
  classInfo.column = 0;
  classInfo.name = parseString(element, "name", status);
  classInfo.description = getText(element);
  classInfo.color = parseColor(element, "color", status);

  parseSpawnflags(element, classInfo.propertyDefinitions, status);
  parsePropertyDefinitions(element, propertyDeclarations, classInfo.propertyDefinitions, status);

  return classInfo;
}

Assets::ModelDefinition EntParser::parseModel(
  const tinyxml2::XMLElement& element, ParserStatus& status) {
  if (!hasAttribute(element, "model")) {
    return Assets::ModelDefinition();
  }

  const auto model = parseString(element, "model", status);

  try {
    ELParser parser(ELParser::Mode::Lenient, model);
    auto expression = parser.parse();
    expression.optimize();
    return Assets::ModelDefinition(expression);
  } catch (const ParserException&) {
    const auto lineNum = static_cast<size_t>(element.GetLineNum());
    auto expression = EL::Expression(
      EL::LiteralExpression(
        EL::Value(EL::MapType({{Assets::ModelSpecificationKeys::Path, EL::Value{model}}}))),
      lineNum, 0);
    return Assets::ModelDefinition(expression);
  } catch (const EL::EvaluationError& evaluationError) {
    const auto line = static_cast<size_t>(element.GetLineNum());
    throw ParserException{line, evaluationError.what()};
  }
}

void EntParser::parseSpawnflags(
  const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions,
  ParserStatus& status) {
  const auto* flagElement = element.FirstChildElement("flag");
  if (flagElement != nullptr) {
    auto result =
      std::make_shared<Assets::FlagsPropertyDefinition>(Model::EntityPropertyKeys::Spawnflags);
    do {
      const auto bit = parseSize(*flagElement, "bit", status);
      if (!bit.has_value()) {
        const auto strValue = parseString(*flagElement, "bit", status);
        warn(*flagElement, "Invalid value '" + strValue + "' for bit property definition", status);
      } else {
        const auto value = 1 << *bit;
        const auto shortDesc = parseString(*flagElement, "key", status);
        const auto longDesc = parseString(*flagElement, "name", status);
        result->addOption(value, shortDesc, longDesc, false);
      }

      flagElement = flagElement->NextSiblingElement("flag");
    } while (flagElement != nullptr);

    if (!addPropertyDefinition(propertyDefinitions, std::move(result))) {
      const auto line = static_cast<size_t>(element.GetLineNum());
      status.warn(line, 0, "Skipping duplicate spawnflags property definition");
    }
  }
}

void EntParser::parsePropertyDefinitions(
  const tinyxml2::XMLElement& parent, const PropertyDefinitionList& propertyDeclarations,
  PropertyDefinitionList& propertyDefinitions, ParserStatus& status) {
  const auto* element = parent.FirstChildElement();
  while (element != nullptr) {
    if (!std::strcmp(element->Name(), "angle")) {
      parseUnknownPropertyDefinition(*element, propertyDefinitions, status);
    } else if (!std::strcmp(element->Name(), "angles")) {
      parseUnknownPropertyDefinition(*element, propertyDefinitions, status);
    } else if (!std::strcmp(element->Name(), "direction")) {
      parseUnknownPropertyDefinition(*element, propertyDefinitions, status);
    } else if (!std::strcmp(element->Name(), "boolean")) {
      parseBooleanPropertyDefinition(*element, propertyDefinitions, status);
    } else if (!std::strcmp(element->Name(), "integer")) {
      parseIntegerPropertyDefinition(*element, propertyDefinitions, status);
    } else if (!std::strcmp(element->Name(), "real")) {
      parseRealPropertyDefinition(*element, propertyDefinitions, status);
    } else if (!std::strcmp(element->Name(), "string")) {
      parseStringPropertyDefinition(*element, propertyDefinitions, status);
    } else if (!std::strcmp(element->Name(), "target")) {
      parseTargetPropertyDefinition(*element, propertyDefinitions, status);
    } else if (!std::strcmp(element->Name(), "targetname")) {
      parseTargetNamePropertyDefinition(*element, propertyDefinitions, status);
    } else if (!std::strcmp(element->Name(), "texture")) {
      parseUnknownPropertyDefinition(*element, propertyDefinitions, status);
    } else if (!std::strcmp(element->Name(), "sound")) {
      parseUnknownPropertyDefinition(*element, propertyDefinitions, status);
    } else if (!std::strcmp(element->Name(), "model")) {
      parseUnknownPropertyDefinition(*element, propertyDefinitions, status);
    } else if (!std::strcmp(element->Name(), "color")) {
      parseUnknownPropertyDefinition(*element, propertyDefinitions, status);
    } else {
      const auto* name = element->Name();
      if (name) {
        for (const auto& propertyDeclaration : propertyDeclarations) {
          if (!std::strcmp(name, propertyDeclaration->key().c_str())) {
            parseDeclaredPropertyDefinition(
              *element, propertyDeclaration, propertyDefinitions, status);
          }
        }
      }
    }
    element = element->NextSiblingElement();
  }
}

void EntParser::parseUnknownPropertyDefinition(
  const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions,
  ParserStatus& status) {
  auto factory =
    [this, &element,
     &status](const std::string& name, const std::string& shortDesc, const std::string& longDesc) {
      auto defaultValue = hasAttribute(element, "value")
                            ? std::make_optional(parseString(element, "value", status))
                            : std::nullopt;
      return std::make_shared<Assets::UnknownPropertyDefinition>(
        name, shortDesc, longDesc, false, std::move(defaultValue));
    };
  parsePropertyDefinition(element, factory, propertyDefinitions, status);
}

void EntParser::parseStringPropertyDefinition(
  const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions,
  ParserStatus& status) {
  auto factory =
    [this, &element,
     &status](const std::string& name, const std::string& shortDesc, const std::string& longDesc) {
      auto defaultValue = hasAttribute(element, "value")
                            ? std::make_optional(parseString(element, "value", status))
                            : std::nullopt;
      return std::make_shared<Assets::StringPropertyDefinition>(
        name, shortDesc, longDesc, false, std::move(defaultValue));
    };
  parsePropertyDefinition(element, factory, propertyDefinitions, status);
}

void EntParser::parseBooleanPropertyDefinition(
  const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions,
  ParserStatus& status) {
  auto factory = [this, &element, &status](
                   const std::string& name, const std::string& shortDesc,
                   const std::string& longDesc) -> std::shared_ptr<Assets::PropertyDefinition> {
    if (hasAttribute(element, "value")) {
      const auto boolDefaultValue = parseInteger(element, "value", status);
      if (boolDefaultValue.has_value()) {
        return std::make_shared<Assets::BooleanPropertyDefinition>(
          name, shortDesc, longDesc, false, *boolDefaultValue != 0);
      } else {
        auto strDefaultValue = parseString(element, "value", status);
        warn(
          element,
          "Invalid default value '" + strDefaultValue + "' for boolean property definition",
          status);
        return std::make_shared<Assets::UnknownPropertyDefinition>(
          name, shortDesc, longDesc, false, std::move(strDefaultValue));
      }
    } else {
      return std::make_shared<Assets::BooleanPropertyDefinition>(name, shortDesc, longDesc, false);
    }
  };
  parsePropertyDefinition(element, factory, propertyDefinitions, status);
}

void EntParser::parseIntegerPropertyDefinition(
  const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions,
  ParserStatus& status) {
  auto factory = [this, &element, &status](
                   const std::string& name, const std::string& shortDesc,
                   const std::string& longDesc) -> std::shared_ptr<Assets::PropertyDefinition> {
    if (hasAttribute(element, "value")) {
      auto intDefaultValue = parseInteger(element, "value", status);
      if (intDefaultValue.has_value()) {
        return std::make_shared<Assets::IntegerPropertyDefinition>(
          name, shortDesc, longDesc, false, std::move(intDefaultValue));
      } else {
        auto strDefaultValue = parseString(element, "value", status);
        warn(
          element,
          "Invalid default value '" + strDefaultValue + "' for integer property definition",
          status);
        return std::make_shared<Assets::UnknownPropertyDefinition>(
          name, shortDesc, longDesc, false, std::move(strDefaultValue));
      }
    } else {
      return std::make_shared<Assets::IntegerPropertyDefinition>(name, shortDesc, longDesc, false);
    }
  };
  parsePropertyDefinition(element, factory, propertyDefinitions, status);
}

void EntParser::parseRealPropertyDefinition(
  const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions,
  ParserStatus& status) {
  auto factory = [this, &element, &status](
                   const std::string& name, const std::string& shortDesc,
                   const std::string& longDesc) -> std::shared_ptr<Assets::PropertyDefinition> {
    if (hasAttribute(element, "value")) {
      auto floatDefaultValue = parseFloat(element, "value", status);
      if (floatDefaultValue.has_value()) {
        return std::make_shared<Assets::FloatPropertyDefinition>(
          name, shortDesc, longDesc, false, std::move(floatDefaultValue));
      } else {
        auto strDefaultValue = parseString(element, "value", status);
        warn(
          element, "Invalid default value '" + strDefaultValue + "' for float property definition",
          status);
        return std::make_shared<Assets::UnknownPropertyDefinition>(
          name, shortDesc, longDesc, false, std::move(strDefaultValue));
      }
    } else {
      return std::make_shared<Assets::FloatPropertyDefinition>(name, shortDesc, longDesc, false);
    }
  };
  parsePropertyDefinition(element, factory, propertyDefinitions, status);
}

void EntParser::parseTargetPropertyDefinition(
  const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions,
  ParserStatus& status) {
  auto factory =
    [](const std::string& name, const std::string& shortDesc, const std::string& longDesc) {
      return std::make_shared<Assets::PropertyDefinition>(
        name, Assets::PropertyDefinitionType::TargetDestinationProperty, shortDesc, longDesc,
        false);
    };
  parsePropertyDefinition(element, factory, propertyDefinitions, status);
}

void EntParser::parseTargetNamePropertyDefinition(
  const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions,
  ParserStatus& status) {
  auto factory =
    [](const std::string& name, const std::string& shortDesc, const std::string& longDesc) {
      return std::make_shared<Assets::PropertyDefinition>(
        name, Assets::PropertyDefinitionType::TargetSourceProperty, shortDesc, longDesc, false);
    };
  parsePropertyDefinition(element, factory, propertyDefinitions, status);
}

void EntParser::parseDeclaredPropertyDefinition(
  const tinyxml2::XMLElement& element,
  const std::shared_ptr<Assets::PropertyDefinition>& propertyDeclaration,
  PropertyDefinitionList& propertyDefinitions, ParserStatus& status) {
  auto factory = [&propertyDeclaration](
                   const std::string& name, const std::string& shortDesc,
                   const std::string& longDesc) {
    return std::shared_ptr<Assets::PropertyDefinition>(
      propertyDeclaration->clone(name, shortDesc, longDesc, false));
  };
  parsePropertyDefinition(element, factory, propertyDefinitions, status);
}

void EntParser::parsePropertyDefinition(
  const tinyxml2::XMLElement& element, EntParser::PropertyDefinitionFactory factory,
  PropertyDefinitionList& propertyDefinitions, ParserStatus& status) {
  if (expectAttribute(element, "key", status) && expectAttribute(element, "name", status)) {
    const auto name = parseString(element, "key", status);
    const auto shortDesc = parseString(element, "name", status);
    const auto longDesc = getText(element);

    if (!addPropertyDefinition(propertyDefinitions, factory(name, shortDesc, longDesc))) {
      const auto line = static_cast<size_t>(element.GetLineNum());
      status.warn(line, 0, "Skipping duplicate property definition: '" + name + "'");
    }
  }
}

void EntParser::parsePropertyDeclaration(
  const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDeclarations,
  ParserStatus& status) {
  const auto* name = element.Name();
  if (name && !std::strcmp(name, "list")) {
    parseListDeclaration(element, propertyDeclarations, status);
  }
}

void EntParser::parseListDeclaration(
  const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDeclarations,
  ParserStatus& status) {
  if (expectAttribute(element, "name", status)) {
    const auto name = parseString(element, "name", status);
    Assets::ChoicePropertyOption::List options;

    const auto* itemElement = element.FirstChildElement("item");
    while (itemElement != nullptr) {
      if (
        expectAttribute(*itemElement, "name", status) &&
        expectAttribute(*itemElement, "value", status)) {
        const auto itemName = parseString(*itemElement, "name", status);
        const auto itemValue = parseString(*itemElement, "value", status);
        options.push_back(Assets::ChoicePropertyOption(itemValue, itemName));
      }
      itemElement = itemElement->NextSiblingElement("item");
    }
    propertyDeclarations.push_back(
      std::make_shared<Assets::ChoicePropertyDefinition>(name, "", "", options, false));
  }
}

vm::bbox3 EntParser::parseBounds(
  const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status) {
  const auto parts = kdl::str_split(parseString(element, attributeName, status), " ");
  if (parts.size() != 6) {
    warn(element, "Invalid bounding box", status);
  }

  const auto it = std::begin(parts);
  vm::bbox3 result;
  result.min =
    vm::parse<FloatType, 3>(kdl::str_join(it, std::next(it, 3), " ")).value_or(vm::vec3::zero());
  result.max = vm::parse<FloatType, 3>(kdl::str_join(std::next(it, 3), std::end(parts), " "))
                 .value_or(vm::vec3::zero());
  return result;
}

Color EntParser::parseColor(
  const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status) {
  return Color::parse(parseString(element, attributeName, status)).value_or(Color());
}

std::optional<int> EntParser::parseInteger(
  const tinyxml2::XMLElement& element, const std::string& attributeName,
  ParserStatus& /* status */) {
  const auto* strValue = element.Attribute(attributeName.c_str());
  if (strValue != nullptr) {
    char* end;
    const auto longValue = std::strtol(strValue, &end, 10);
    if (
      *end == '\0' && errno != ERANGE && longValue >= std::numeric_limits<int>::min() &&
      longValue <= std::numeric_limits<int>::max()) {
      return {static_cast<int>(longValue)};
    }
  }
  return std::nullopt;
}

std::optional<float> EntParser::parseFloat(
  const tinyxml2::XMLElement& element, const std::string& attributeName,
  ParserStatus& /* status */) {
  const auto* strValue = element.Attribute(attributeName.c_str());
  if (strValue != nullptr) {
    char* end;
    const auto floatValue = std::strtof(strValue, &end);
    if (*end == '\0' && errno != ERANGE) {
      return floatValue;
    }
  }
  return std::nullopt;
}

std::optional<size_t> EntParser::parseSize(
  const tinyxml2::XMLElement& element, const std::string& attributeName,
  ParserStatus& /* status */) {
  const auto* strValue = element.Attribute(attributeName.c_str());
  if (strValue != nullptr) {
    char* end;
    const auto intValue = std::strtoul(strValue, &end, 10);
    if (*end == '\0' && errno != ERANGE) {
      return static_cast<size_t>(intValue);
    }
  }
  return std::nullopt;
}

std::string EntParser::parseString(
  const tinyxml2::XMLElement& element, const std::string& attributeName,
  ParserStatus& /* status */) {
  const auto* value = element.Attribute(attributeName.c_str());
  if (value == nullptr) {
    return std::string();
  } else {
    return std::string(value);
  }
}

std::string EntParser::getText(const tinyxml2::XMLElement& element) {
  // I assume that only the initial and the last text is meaningful.

  std::stringstream str;
  const auto* first = element.FirstChild();
  const auto* last = element.LastChild();

  if (first && first->ToText()) {
    str << first->Value();
  }

  if (last && last != first && last->ToText()) {
    str << last->Value();
  }

  return str.str();
}

bool EntParser::expectAttribute(
  const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status) {
  if (!hasAttribute(element, attributeName)) {
    warn(element, "Expected attribute '" + attributeName + "'", status);
    return false;
  } else {
    return true;
  }
}

bool EntParser::hasAttribute(
  const tinyxml2::XMLElement& element, const std::string& attributeName) {
  return element.Attribute(attributeName.c_str()) != nullptr;
}

void EntParser::warn(
  const tinyxml2::XMLElement& element, const std::string& msg, ParserStatus& status) {
  const auto str = msg + std::string(": ") + std::string(element.Name());
  if (element.GetLineNum() > 0) {
    status.warn(static_cast<size_t>(element.GetLineNum()), str);
  } else {
    status.warn(str);
  }
}
} // namespace IO
} // namespace TrenchBroom
