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

#include "EntityDefinitionTestUtils.h"

#include <gtest/gtest.h>

#include "Assets/EntityDefinition.h"
#include "EL/EvaluationContext.h"
#include "EL/Types.h"
#include "IO/ELParser.h"
#include "IO/EntityDefinitionParser.h"
#include "IO/TestParserStatus.h"
#include "VectorUtilsMinimal.h"

namespace TrenchBroom {
    namespace Assets {
        void assertModelDefinition(const ModelSpecification& expected, IO::EntityDefinitionParser& parser, const String& entityPropertiesStr) {
            IO::TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());

            assertModelDefinition(expected, definition, entityPropertiesStr);

            VectorUtils::clearAndDelete(definitions);
        }

        void assertModelDefinition(const ModelSpecification& expected, const EntityDefinition* definition, const String& entityPropertiesStr) {
            assert(definition->type() == EntityDefinition::Type_PointEntity);

            const PointEntityDefinition* pointDefinition = static_cast<const PointEntityDefinition*>(definition);
            const ModelDefinition& modelDefinition = pointDefinition->modelDefinition();
            assertModelDefinition(expected, modelDefinition, entityPropertiesStr);
        }

        void assertModelDefinition(const ModelSpecification& expected, const ModelDefinition& actual, const String& entityPropertiesStr) {
            const EL::MapType entityPropertiesMap = IO::ELParser::parseStrict(entityPropertiesStr).evaluate(EL::EvaluationContext()).mapValue();

            Model::EntityAttributes attributes;
            for (const auto& entry : entityPropertiesMap) {
                const String& key = entry.first;
                const EL::Value& value = entry.second;
                attributes.addOrUpdateAttribute(key, value.convertTo(EL::ValueType::Type_String).stringValue(), nullptr);
            }

            ASSERT_EQ(expected, actual.modelSpecification(attributes));
        }
    }
}
