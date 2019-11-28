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

#ifndef TrenchBroom_EntityDefinitionParser_h
#define TrenchBroom_EntityDefinitionParser_h

#include "StringType.h"
#include "Assets/Asset_Forward.h"

#include <map>
#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class ParserStatus;
        class Path;

        class EntityDefinitionParser {
        protected:
            using EntityDefinitionList = std::vector<Assets::EntityDefinition*>;
            using AttributeDefinitionPtr = std::shared_ptr<Assets::AttributeDefinition>;
            using AttributeDefinitionList = std::vector<AttributeDefinitionPtr>;
            using AttributeDefinitionMap = std::map<String, AttributeDefinitionPtr>;
        public:
            virtual ~EntityDefinitionParser();
            EntityDefinitionList parseDefinitions(ParserStatus& status);
        private:
            virtual EntityDefinitionList doParseDefinitions(ParserStatus& status) = 0;
        };
    }
}

#endif
