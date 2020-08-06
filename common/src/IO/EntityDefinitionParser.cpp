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

#include "EntityDefinitionParser.h"

#include "Macros.h"
#include "Assets/AttributeDefinition.h"
#include "Assets/EntityDefinition.h"
#include "IO/EntityDefinitionClassInfo.h"

#include <map>
#include <optional>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        EntityDefinitionParser::EntityDefinitionParser(const Color& defaultEntityColor) :
        m_defaultEntityColor(defaultEntityColor) {}
        
        EntityDefinitionParser::~EntityDefinitionParser() {}

        static void resolveSuperClasses(ParserStatus&, std::vector<EntityDefinitionClassInfo>& classInfos) {
            std::map<std::string, EntityDefinitionClassInfo> baseClasses;
            for (auto& classInfo : classInfos) {
                classInfo.resolveBaseClasses(baseClasses);
                if (classInfo.type == EntityDefinitionClassType::BaseClass) {
                    baseClasses.insert({ classInfo.name, classInfo });
                }
            }
        }

        std::unique_ptr<Assets::EntityDefinition> EntityDefinitionParser::createDefinition(const EntityDefinitionClassInfo& classInfo) const {
            switch (classInfo.type) {
                case EntityDefinitionClassType::PointClass:
                    return std::make_unique<Assets::PointEntityDefinition>(
                        classInfo.name,
                        classInfo.color.value_or(m_defaultEntityColor),
                        classInfo.size.value_or(vm::bbox3(-8, 8)),
                        classInfo.description.value_or(""),
                        classInfo.attributes,
                        classInfo.modelDefinition.value_or(Assets::ModelDefinition()));
                case EntityDefinitionClassType::BrushClass:
                    return std::make_unique<Assets::BrushEntityDefinition>(
                        classInfo.name,
                        classInfo.color.value_or(m_defaultEntityColor),
                        classInfo.description.value_or(""),
                        classInfo.attributes);
                case EntityDefinitionClassType::BaseClass:
                    return nullptr;
                switchDefault()
            };
        }

        std::vector<Assets::EntityDefinition*> EntityDefinitionParser::createDefinitions(ParserStatus& status, std::vector<EntityDefinitionClassInfo> classInfos) const {
            resolveSuperClasses(status, classInfos);

            std::vector<Assets::EntityDefinition*> result;
            for (const auto& classInfo : classInfos) {
                if (auto definition = createDefinition(classInfo)) {
                    result.push_back(definition.release());
                }
            }
            
            return result;
        }

        EntityDefinitionParser::EntityDefinitionList EntityDefinitionParser::parseDefinitions(ParserStatus& status) {
            auto classInfos = parseClassInfos(status);
            return createDefinitions(status, std::move(classInfos));
        }
    }
}
