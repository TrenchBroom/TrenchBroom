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

#include <vecmath/forward.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tinyxml2 {
    class XMLDocument;
    class XMLElement;
}

namespace TrenchBroom {
    namespace Assets {
        class ModelDefinition;
        class SpriteDefinition;
    }

    namespace IO {
        struct EntityDefinitionClassInfo;
        class ParserStatus;

        class EntParser : public EntityDefinitionParser {
        private:
            using PropertyDefinitionFactory = std::function<std::shared_ptr<Assets::PropertyDefinition>(const std::string&, const std::string&, const std::string&)>;

            const char* m_begin;
            const char* m_end;
        public:
            EntParser(std::string_view str, const Color& defaultEntityColor);
        private:
            std::vector<EntityDefinitionClassInfo> parseClassInfos(ParserStatus& status) override;
            
            std::vector<EntityDefinitionClassInfo> parseClassInfos(const tinyxml2::XMLDocument& document, ParserStatus& status);
            std::optional<EntityDefinitionClassInfo> parseClassInfo(const tinyxml2::XMLElement& element, const PropertyDefinitionList& propertyDeclarations, ParserStatus& status);
            EntityDefinitionClassInfo parsePointClassInfo(const tinyxml2::XMLElement& element, const PropertyDefinitionList& propertyDeclarations, ParserStatus& status);
            EntityDefinitionClassInfo parseBrushClassInfo(const tinyxml2::XMLElement& element, const PropertyDefinitionList& propertyDeclarations, ParserStatus& status);

            Assets::ModelDefinition parseModel(const tinyxml2::XMLElement& element, ParserStatus& status);
            Assets::SpriteDefinition parseSprite(const tinyxml2::XMLElement& element, ParserStatus& status);

            void parseSpawnflags(const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions, ParserStatus& status);

            void parsePropertyDefinitions(const tinyxml2::XMLElement& parent, const PropertyDefinitionList& propertyDeclarations, PropertyDefinitionList& propertyDefinitions, ParserStatus& status);
            void parseUnknownPropertyDefinition(const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions, ParserStatus& status);
            void parseStringPropertyDefinition(const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions, ParserStatus& status);
            void parseBooleanPropertyDefinition(const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions, ParserStatus& status);
            void parseIntegerPropertyDefinition(const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions, ParserStatus& status);
            void parseRealPropertyDefinition(const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions, ParserStatus& status);
            void parseTargetPropertyDefinition(const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions, ParserStatus& status);
            void parseTargetNamePropertyDefinition(const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDefinitions, ParserStatus& status);

            void parseDeclaredPropertyDefinition(const tinyxml2::XMLElement& element, const std::shared_ptr<Assets::PropertyDefinition>& propertyDeclaration, PropertyDefinitionList& propertyDefinitions, ParserStatus& status);
            void parsePropertyDefinition(const tinyxml2::XMLElement& element, PropertyDefinitionFactory factory, PropertyDefinitionList& propertyDefinitions, ParserStatus& status);

            void parsePropertyDeclaration(const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDeclarations, ParserStatus& status);
            void parseListDeclaration(const tinyxml2::XMLElement& element, PropertyDefinitionList& propertyDeclarations, ParserStatus& status);

            vm::bbox3 parseBounds(const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status);
            Color parseColor(const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status);
            std::optional<int> parseInteger(const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status);
            std::optional<float> parseFloat(const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status);
            std::optional<size_t> parseSize(const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status);
            std::string parseString(const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status);
            std::string getText(const tinyxml2::XMLElement& element);

            bool expectAttribute(const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status);
            bool hasAttribute(const tinyxml2::XMLElement& element, const std::string& attributeName);
            void warn(const tinyxml2::XMLElement& element, const std::string& msg, ParserStatus& status);
        };
    }
}



