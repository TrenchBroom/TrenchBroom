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

#include "Assets/AttributeDefinition.h"
#include "Assets/EntityDefinition.h"
#include "EL/Expression.h"
#include "EL/Types.h"
#include "EL/Value.h"
#include "IO/ParserStatus.h"
#include "Model/EntityAttributes.h"

#include <vecmath/bbox.h>

#include <tinyxml2/tinyxml2.h>

#include <cstdlib>

namespace TrenchBroom {
    namespace IO {
        EntParser::EntParser(const char* begin, const char* end, const Color& defaultEntityColor) :
        m_begin(begin),
        m_end(end),
        m_defaultEntityColor(defaultEntityColor) {}

        EntParser::EntParser(const String& str, const Color& defaultEntityColor) :
        m_begin(str.c_str()),
        m_end(str.c_str() + str.size()),
        m_defaultEntityColor(defaultEntityColor) {}

        Assets::EntityDefinitionList EntParser::doParseDefinitions(ParserStatus& status) {
            tinyxml2::XMLDocument doc;
            doc.Parse(m_begin, static_cast<size_t>(m_end - m_begin));
            return parseClasses(doc, status);
        }

        Assets::EntityDefinitionList EntParser::parseClasses(const tinyxml2::XMLDocument& document, ParserStatus& status) {
            Assets::EntityDefinitionList result;
            const auto* classesNode = document.FirstChildElement("classes");
            if (classesNode != nullptr) {
                const auto* currentElement = classesNode->FirstChildElement();
                while (currentElement != nullptr) {
                    auto* definition = parseClass(*currentElement, status);
                    if (definition != nullptr) {
                        result.push_back(definition);
                    }
                    currentElement = currentElement->NextSiblingElement();
                }
            }
            return result;
        }

        Assets::EntityDefinition* EntParser::parseClass(const tinyxml2::XMLElement& element, ParserStatus& status) {
            if (!std::strcmp(element.Name(), "point")) {
                return parsePointEntityDefinition(element, status);
            } else if (!std::strcmp(element.Name(), "group")) {
                return parseBrushEntityDefinition(element, status);
            } else {
                warn(element, "Unexpected XML element", status);
                return nullptr;
            }
        }

        Assets::EntityDefinition* EntParser::parsePointEntityDefinition(const tinyxml2::XMLElement& element, ParserStatus& status) {
            const auto bounds = parseBounds(element, "box", status);
            const auto color = parseColor(element, "color", status);
            const auto model = parseString(element, "model", status);
            const auto name = parseString(element, "name", status);

            Assets::AttributeDefinitionList attributeDefinitions;

            parseSpawnflags(element, attributeDefinitions, status);
            parseAttributes(element, attributeDefinitions, status);

            Assets::ModelDefinition modelDefinition(EL::LiteralExpression::create(
                EL::Value({ { "path", EL::Value(model) } }),
                static_cast<size_t>(element.GetLineNum()), 0)
            );

            return new Assets::PointEntityDefinition(name, color, bounds, getText(element), attributeDefinitions, modelDefinition);
        }

        Assets::EntityDefinition* EntParser::parseBrushEntityDefinition(const tinyxml2::XMLElement& element, ParserStatus& status) {
            const auto color = parseColor(element, "color", status);
            const auto name = parseString(element, "name", status);

            Assets::AttributeDefinitionList attributeDefinitions;

            parseSpawnflags(element, attributeDefinitions, status);
            parseAttributes(element, attributeDefinitions, status);

            return new Assets::BrushEntityDefinition(name, color, getText(element), attributeDefinitions);
        }

        void EntParser::parseSpawnflags(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status) {
            const auto* flagElement = element.FirstChildElement("flag");
            if (flagElement != nullptr) {
                auto result = std::make_shared<Assets::FlagsAttributeDefinition>(Model::AttributeNames::Spawnflags);
                do {
                    const auto [success, bit] = parseSize(element, "bit", status);
                    if (!success) {
                        const auto strValue = parseString(element, "bit", status);
                        warn(element, "Invalid value '" + strValue + "' for bit attribute", status);
                    } else {
                        const auto value = 1 << bit;
                        const auto shortDesc = parseString(element, "key", status);
                        const auto longDesc = parseString(element, "name", status);
                        result->addOption(value, shortDesc, longDesc, false);
                    }

                    flagElement = flagElement->NextSiblingElement("flag");
                } while (flagElement != nullptr);
                attributeDefinitions.push_back(result);
            }
        }

        void EntParser::parseAttributes(const tinyxml2::XMLElement& parent, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status) {
            const auto* element = parent.FirstChildElement();
            while (element != nullptr) {
                if (!std::strcmp(element->Name(), "angle")) {
                    parseAngleAttribute(*element, attributeDefinitions, status);
                } else if (!std::strcmp(element->Name(), "angles")) {
                    parseAnglesAttribute(*element, attributeDefinitions, status);
                } else if (!std::strcmp(element->Name(), "direction")) {
                    parseDirectionAttribute(*element, attributeDefinitions, status);
                } else if (!std::strcmp(element->Name(), "boolean")) {
                    parseBooleanAttribute(*element, attributeDefinitions, status);
                } else if (!std::strcmp(element->Name(), "integer")) {
                    parseIntegerAttribute(*element, attributeDefinitions, status);
                } else if (!std::strcmp(element->Name(), "real")) {
                    parseRealAttribute(*element, attributeDefinitions, status);
                } else if (!std::strcmp(element->Name(), "string")) {
                    parseStringAttribute(*element, attributeDefinitions, status);
                } else if (!std::strcmp(element->Name(), "target")) {
                    parseTargetAttribute(*element, attributeDefinitions, status);
                } else if (!std::strcmp(element->Name(), "targetname")) {
                    parseTargetNameAttribute(*element, attributeDefinitions, status);
                } else if (!std::strcmp(element->Name(), "texture")) {
                    parseTextureAttribute(*element, attributeDefinitions, status);
                } else if (!std::strcmp(element->Name(), "sound")) {
                    parseStringAttribute(*element, attributeDefinitions, status);
                } else if (!std::strcmp(element->Name(), "model")) {
                    parseStringAttribute(*element, attributeDefinitions, status);
                } else if (!std::strcmp(element->Name(), "color")) {
                    parseStringAttribute(*element, attributeDefinitions, status);
                }
                element = element->NextSiblingElement();
            }
        }

        void EntParser::parseStringAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status) {
            auto factory = [this, &element, &status](const String& name, const String& shortDesc, const String& longDesc) {
                if (hasAttribute(element, "value")) {
                    const auto value = parseString(element, "value", status);
                    return std::make_shared<Assets::StringAttributeDefinition>(name, shortDesc, longDesc, value);
                } else {
                    return std::make_shared<Assets::StringAttributeDefinition>(name, shortDesc, longDesc);
                }
            };
            parseAttributeDefinition(element, factory, attributeDefinitions, status);
        }

        void EntParser::parseAngleAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status) {
            // we currently do not have special support for angle attributes
            parseStringAttribute(element, attributeDefinitions, status);
        }

        void EntParser::parseAnglesAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status) {
            // we currently do not have special support for angles attributes
            parseStringAttribute(element, attributeDefinitions, status);
        }

        void EntParser::parseDirectionAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status) {
            // we currently do not have special support for direction attributes
            parseStringAttribute(element, attributeDefinitions, status);
        }

        void EntParser::parseBooleanAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status) {
            auto factory = [this, &element, &status](const String& name, const String& shortDesc, const String& longDesc) -> Assets::AttributeDefinitionPtr {
                if (hasAttribute(element, "value")) {
                    const auto [success, value] = parseInteger(element, "value", status);
                    if (success) {
                        return std::make_shared<Assets::BooleanAttributeDefinition>(name, shortDesc, longDesc, value != 0, false);
                    } else {
                        const auto strValue = parseString(element, "value", status);
                        warn(element, "Invalid default value '" + strValue + "' for boolean attribute definition, converting to string attribute definition", status);
                        return std::make_shared<Assets::StringAttributeDefinition>(name, shortDesc, longDesc, strValue);
                    }
                } else {
                    return std::make_shared<Assets::BooleanAttributeDefinition>(name, shortDesc, longDesc);
                }
            };
            parseAttributeDefinition(element, factory, attributeDefinitions, status);
        }

        void EntParser::parseIntegerAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status) {
            auto factory = [this, &element, &status](const String& name, const String& shortDesc, const String& longDesc) -> Assets::AttributeDefinitionPtr {
                if (hasAttribute(element, "value")) {
                    const auto [success, value] = parseInteger(element, "value", status);
                    if (success) {
                        return std::make_shared<Assets::IntegerAttributeDefinition>(name, shortDesc, longDesc, value, false);
                    } else {
                        const auto strValue = parseString(element, "value", status);
                        warn(element, "Invalid default value '" + strValue + "' for integer attribute definition, converting to string attribute definition", status);
                        return std::make_shared<Assets::StringAttributeDefinition>(name, shortDesc, longDesc, strValue);
                    }
                } else {
                    return std::make_shared<Assets::IntegerAttributeDefinition>(name, shortDesc, longDesc);
                }
            };
            parseAttributeDefinition(element, factory, attributeDefinitions, status);
        }

        void EntParser::parseRealAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status) {
            auto factory = [this, &element, &status](const String& name, const String& shortDesc, const String& longDesc) -> Assets::AttributeDefinitionPtr {
                if (hasAttribute(element, "value")) {
                    const auto [success, value] = parseFloat(element, "value", status);
                    if (success) {
                        return std::make_shared<Assets::FloatAttributeDefinition>(name, shortDesc, longDesc, value, false);
                    } else {
                        const auto strValue = parseString(element, "value", status);
                        warn(element, "Invalid default value '" + strValue + "' for float attribute definition, converting to string attribute definition", status);
                        return std::make_shared<Assets::StringAttributeDefinition>(name, shortDesc, longDesc, strValue);
                    }
                } else {
                    return std::make_shared<Assets::FloatAttributeDefinition>(name, shortDesc, longDesc);
                }
            };
            parseAttributeDefinition(element, factory, attributeDefinitions, status);
        }

        void EntParser::parseTargetAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status) {
            auto factory = [](const String& name, const String& shortDesc, const String& longDesc) {
                return std::make_shared<Assets::AttributeDefinition>(name, Assets::AttributeDefinition::Type_TargetDestinationAttribute, shortDesc, longDesc, false);
            };
            parseAttributeDefinition(element, factory, attributeDefinitions, status);
        }

        void EntParser::parseTargetNameAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status) {
            auto factory = [](const String& name, const String& shortDesc, const String& longDesc) {
                return std::make_shared<Assets::AttributeDefinition>(name, Assets::AttributeDefinition::Type_TargetSourceAttribute, shortDesc, longDesc, false);
            };
            parseAttributeDefinition(element, factory, attributeDefinitions, status);
        }

        void EntParser::parseTextureAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status) {
            // we currently do not have special support for texture attributes
            parseStringAttribute(element, attributeDefinitions, status);
        }

        void EntParser::parseAttributeDefinition(const tinyxml2::XMLElement& element, EntParser::AttributeFactory factory, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status) {
            if (expectAttribute(element, "key", status) && expectAttribute(element, "name", status)) {
                const auto name = parseString(element, "key", status);
                const auto shortDesc = parseString(element, "name", status);
                const auto longDesc = getText(element);

                attributeDefinitions.push_back(factory(name, shortDesc, longDesc));
            }
        }

        vm::bbox3 EntParser::parseBounds(const tinyxml2::XMLElement& element, const String& attributeName, ParserStatus& status) {
            const auto parts = StringUtils::split(parseString(element, attributeName, status), " ");
            if (parts.size() != 6) {
                warn(element, "Invalid bounding box", status);
            }

            const auto it = std::begin(parts);
            vm::bbox3 result;
            result.min = vm::vec3::parse(StringUtils::join(it, std::next(it, 3), " ", " ", " ", StringUtils::StringToString()));
            result.max = vm::vec3::parse(StringUtils::join(std::next(it, 3), std::end(parts), " ", " ", " ", StringUtils::StringToString()));
            return result;
        }

        Color EntParser::parseColor(const tinyxml2::XMLElement& element, const String& attributeName, ParserStatus& status) {
            return Color::parse(parseString(element, attributeName, status));
        }

        std::tuple<bool, int> EntParser::parseInteger(const tinyxml2::XMLElement& element, const String& attributeName, ParserStatus& status) {
            const auto* strValue = element.Attribute(attributeName.c_str());
            if (strValue == nullptr) {
                return std::make_tuple(false, 0);
            } else {
                char* end;
                const auto intValue = std::strtol(strValue, &end, 10);
                if (*end != '\0' || errno == ERANGE) {
                    return std::make_tuple(false, 0);
                }
                return std::make_tuple(true, intValue);
            }
        }

        std::tuple<bool, float> EntParser::parseFloat(const tinyxml2::XMLElement& element, const String& attributeName, ParserStatus& status) {
            const auto* strValue = element.Attribute(attributeName.c_str());
            if (strValue == nullptr) {
                return std::make_tuple(false, 0.0f);
            } else {
                char* end;
                const auto floatValue = std::strtof(strValue, &end);
                if (*end != '\0' || errno == ERANGE) {
                    return std::make_tuple(false, 0.0f);
                }
                return std::make_tuple(true, floatValue);
            }
        }

        std::tuple<bool, size_t> EntParser::parseSize(const tinyxml2::XMLElement& element, const String& attributeName, ParserStatus& status) {
            const auto* strValue = element.Attribute(attributeName.c_str());
            if (strValue == nullptr) {
                return std::make_tuple(false, 0);
            } else {
                char* end;
                const auto intValue = std::strtoul(strValue, &end, 10);
                if (*end != '\0' || errno == ERANGE) {
                    return std::make_tuple(false, 0);
                }
                return std::make_tuple(true, static_cast<size_t>(intValue));
            }
        }

        String EntParser::parseString(const tinyxml2::XMLElement& element, const String& attributeName, ParserStatus& status) {
            const auto* value = element.Attribute(attributeName.c_str());
            if (value == nullptr) {
                return String();
            } else {
                return String(value);
            }
        }

        String EntParser::getText(const tinyxml2::XMLElement& element) {
            // I assume that only the initial and the last text is meaningful.

            StringStream str;
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


        bool EntParser::expectAttribute(const tinyxml2::XMLElement& element, const String& attributeName, ParserStatus& status) {
            if (!hasAttribute(element, attributeName)) {
                warn(element, "Expected attribute '" + attributeName + "'", status);
                return false;
            } else {
                return true;
            }
        }

        bool EntParser::hasAttribute(const tinyxml2::XMLElement& element, const String& attributeName) {
            return element.Attribute(attributeName.c_str()) != nullptr;
        }

        void EntParser::warn(const tinyxml2::XMLElement& element, const String& msg, ParserStatus& status) {
            const auto str = msg + String(": ") + String(element.Name());
            if (element.GetLineNum() > 0) {
                status.warn(static_cast<size_t>(element.GetLineNum()), str);
            } else {
                status.warn(str);
            }
        }
    }
}
