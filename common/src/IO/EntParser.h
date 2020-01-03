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

#ifndef TRENCHBROOM_ENTPARSER_H
#define TRENCHBROOM_ENTPARSER_H

#include "Color.h"
#include "FloatType.h"
#include "IO/EntityDefinitionParser.h"

#include <vecmath/forward.h>

#include <functional>
#include <memory>
#include <nonstd/optional.hpp>
#include <string>
#include <vector>

namespace tinyxml2 {
    class XMLDocument;
    class XMLElement;
}

namespace TrenchBroom {
    namespace Assets {
        class ModelDefinition;
    }

    namespace IO {
        class ParserStatus;

        class EntParser : public EntityDefinitionParser {
        private:
            using AttributeFactory = std::function<std::shared_ptr<Assets::AttributeDefinition>(const std::string&, const std::string&, const std::string&)>;

            const char* m_begin;
            const char* m_end;
            const Color m_defaultEntityColor;
        public:
            EntParser(const char* begin, const char* end, const Color& defaultEntityColor);
            EntParser(const std::string& str, const Color& defaultEntityColor);
        private:
            EntityDefinitionList doParseDefinitions(ParserStatus& status) override;
            EntityDefinitionList parseClasses(const tinyxml2::XMLDocument& document, ParserStatus& status);
            Assets::EntityDefinition* parseClass(const tinyxml2::XMLElement& element, const AttributeDefinitionList& attributeDeclarations, ParserStatus& status);
            Assets::EntityDefinition* parsePointEntityDefinition(const tinyxml2::XMLElement& element, const AttributeDefinitionList& attributeDeclarations, ParserStatus& status);
            Assets::EntityDefinition* parseBrushEntityDefinition(const tinyxml2::XMLElement& element, const AttributeDefinitionList& attributeDeclarations, ParserStatus& status);

            Assets::ModelDefinition parseModel(const tinyxml2::XMLElement& element, ParserStatus& status);

            void parseSpawnflags(const tinyxml2::XMLElement& element, AttributeDefinitionList& attributeDefinitions, ParserStatus& status);

            void parseAttributes(const tinyxml2::XMLElement& parent, const AttributeDefinitionList& attributeDeclarations, AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseUnknownAttribute(const tinyxml2::XMLElement& element, AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseStringAttribute(const tinyxml2::XMLElement& element, AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseBooleanAttribute(const tinyxml2::XMLElement& element, AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseIntegerAttribute(const tinyxml2::XMLElement& element, AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseRealAttribute(const tinyxml2::XMLElement& element, AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseTargetAttribute(const tinyxml2::XMLElement& element, AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseTargetNameAttribute(const tinyxml2::XMLElement& element, AttributeDefinitionList& attributeDefinitions, ParserStatus& status);

            void parseDeclaredAttributeDefinition(const tinyxml2::XMLElement& element, const std::shared_ptr<Assets::AttributeDefinition>& attributeDeclaration, AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseAttributeDefinition(const tinyxml2::XMLElement& element, AttributeFactory factory, AttributeDefinitionList& attributeDefinitions, ParserStatus& status);

            void parseAttributeDeclaration(const tinyxml2::XMLElement& element, AttributeDefinitionList& attributeDeclarations, ParserStatus& status);
            void parseListDeclaration(const tinyxml2::XMLElement& element, AttributeDefinitionList& attributeDeclarations, ParserStatus& status);

            vm::bbox3 parseBounds(const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status);
            Color parseColor(const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status);
            nonstd::optional<int> parseInteger(const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status);
            nonstd::optional<float> parseFloat(const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status);
            nonstd::optional<size_t> parseSize(const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status);
            std::string parseString(const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status);
            std::string getText(const tinyxml2::XMLElement& element);

            bool expectAttribute(const tinyxml2::XMLElement& element, const std::string& attributeName, ParserStatus& status);
            bool hasAttribute(const tinyxml2::XMLElement& element, const std::string& attributeName);
            void warn(const tinyxml2::XMLElement& element, const std::string& msg, ParserStatus& status);
        };
    }
}



#endif //TRENCHBROOM_ENTPARSER_H
