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
#include "TrenchBroom.h"
#include "IO/EntityDefinitionParser.h"
#include "IO/Path.h"

#include <vecmath/forward.h>

namespace tinyxml2 {
    class XMLDocument;
    class XMLElement;
}

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinition;
    }

    namespace IO {
        class ParserStatus;

        class EntParser : public EntityDefinitionParser {
        private:
            using AttributeFactory = std::function<Assets::AttributeDefinitionPtr(const String&, const String&, const String&)>;

            const char* m_begin;
            const char* m_end;
            const Color m_defaultEntityColor;
        public:
            EntParser(const char* begin, const char* end, const Color& defaultEntityColor);
            EntParser(const String& str, const Color& defaultEntityColor);
        private:
            Assets::EntityDefinitionList doParseDefinitions(ParserStatus& status) override;
            Assets::EntityDefinitionList parseClasses(const tinyxml2::XMLDocument& document, ParserStatus& status);
            Assets::EntityDefinition* parseClass(const tinyxml2::XMLElement& element, ParserStatus& status);
            Assets::EntityDefinition* parsePointEntityDefinition(const tinyxml2::XMLElement& element, ParserStatus& status);
            Assets::EntityDefinition* parseBrushEntityDefinition(const tinyxml2::XMLElement& element, ParserStatus& status);

            void parseSpawnflags(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status);

            void parseAttributes(const tinyxml2::XMLElement& parent, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseStringAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseAngleAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseAnglesAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseDirectionAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseBooleanAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseIntegerAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseRealAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseTargetAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseTargetNameAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status);
            void parseTextureAttribute(const tinyxml2::XMLElement& element, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status);

            void parseAttributeDefinition(const tinyxml2::XMLElement& element, AttributeFactory factory, Assets::AttributeDefinitionList& attributeDefinitions, ParserStatus& status);

            vm::bbox3 parseBounds(const tinyxml2::XMLElement& element, const String& attributeName, ParserStatus& status);
            Color parseColor(const tinyxml2::XMLElement& element, const String& attributeName, ParserStatus& status);
            int parseInteger(const tinyxml2::XMLElement& element, const String& attributeName, ParserStatus& status);
            size_t parseSize(const tinyxml2::XMLElement& element, const String& attributeName, ParserStatus& status);
            String parseString(const tinyxml2::XMLElement& element, const String& attributeName, ParserStatus& status);

            bool expectAttribute(const tinyxml2::XMLElement& element, const String& attributeName, ParserStatus& status);
            bool hasAttribute(const tinyxml2::XMLElement& element, const String& attributeName);
            void warn(const tinyxml2::XMLElement& element, const String& msg, ParserStatus& status);
        };
    }
}



#endif //TRENCHBROOM_ENTPARSER_H
