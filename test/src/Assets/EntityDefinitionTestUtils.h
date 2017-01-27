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

#ifndef EntityDefinitionTestUtils_h
#define EntityDefinitionTestUtils_h

#include "Color.h"
#include "StringUtils.h"

namespace TrenchBroom {
    namespace IO {
        class EntityDefinitionParser;
    }
    
    namespace Assets {
        struct ModelSpecification;
        class ModelDefinition;
        class EntityDefinition;
        
        void assertModelDefinition(const ModelSpecification& expected, IO::EntityDefinitionParser& parser, const String& entityPropertiesStr = "{}");
        void assertModelDefinition(const ModelSpecification& expected, const EntityDefinition* definition, const String& entityPropertiesStr = "{}");
        void assertModelDefinition(const ModelSpecification& expected, const ModelDefinition& actual, const String& entityPropertiesStr = "{}");
        
        template <typename Parser>
        void assertModelDefinition(const ModelSpecification& expected, const String& modelStr, const String& templateStr, const String& entityPropertiesStr = "{}") {
            const String defStr = StringUtils::replaceAll(templateStr, "${MODEL}", modelStr);
            Parser parser(defStr, Color(1.0f, 1.0f, 1.0f, 1.0f));
            assertModelDefinition(expected, parser, entityPropertiesStr);
        }
    }
}

#endif /* EntityDefinitionTestUtils_h */
