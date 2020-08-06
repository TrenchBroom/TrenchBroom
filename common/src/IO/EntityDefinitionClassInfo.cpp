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

#include "EntityDefinitionClassInfo.h"

#include "Macros.h"
#include "Assets/AttributeDefinition.h"
#include "Model/EntityAttributes.h"

#include <kdl/opt_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/bbox_io.h>
#include <vecmath/vec_io.h>

#include <iostream>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        std::ostream& operator<<(std::ostream& str, const EntityDefinitionClassType type) {
            switch (type) {
                case EntityDefinitionClassType::BaseClass:
                    str << "BaseClass";
                    break;
                case EntityDefinitionClassType::PointClass:
                    str << "PointClass";
                    break;
                case EntityDefinitionClassType::BrushClass:
                    str << "BrushClass";
                    break;
                switchDefault();
            }
            return str;
        }

        void EntityDefinitionClassInfo::resolveBaseClasses(const std::map<std::string, EntityDefinitionClassInfo>& baseClasses) {
            for (auto classnameIt = superClasses.rbegin(), classnameEnd = superClasses.rend(); classnameIt != classnameEnd; ++classnameIt) {
                const std::string& classname = *classnameIt;
                const auto baseClassIt = baseClasses.find(classname);
                if (baseClassIt != std::end(baseClasses)) {
                    const EntityDefinitionClassInfo& baseClass = baseClassIt->second;
                    if (!description && baseClass.description) {
                        description = baseClass.description;
                    }
                    if (!color && baseClass.color) {
                        color = baseClass.color;
                    }
                    if (!size && baseClass.size) {
                        size = baseClass.size;
                    }

                    for (const auto& baseAttribute : baseClass.attributes) {
                        auto classAttributeIt = std::find_if(std::begin(attributes), std::end(attributes), [&](const auto& a) { return a->name() == baseAttribute->name(); });
                        if (classAttributeIt != std::end(attributes)) {
                            // the class already has a definition for this attribute, attempt merging them
                            mergeProperties(classAttributeIt->get(), baseAttribute.get());
                        } else {
                            // the class doesn't have a definition for this attribute, add the base class attribute
                            attributes.push_back(baseAttribute);
                        }
                    }

                    if (modelDefinition && baseClass.modelDefinition) {
                        modelDefinition->append(*baseClass.modelDefinition);
                    } else if (!modelDefinition) {
                        modelDefinition = baseClass.modelDefinition;
                    }
                }
            }
        }

        void EntityDefinitionClassInfo::mergeProperties(Assets::AttributeDefinition* classAttribute, const Assets::AttributeDefinition* baseclassAttribute) {
            // for now, only merge spawnflags
            if (baseclassAttribute->type() == Assets::AttributeDefinitionType::FlagsAttribute &&
                classAttribute->type() == Assets::AttributeDefinitionType::FlagsAttribute &&
                baseclassAttribute->name() == Model::AttributeNames::Spawnflags &&
                classAttribute->name() == Model::AttributeNames::Spawnflags) {

                const Assets::FlagsAttributeDefinition* baseclassFlags = static_cast<const Assets::FlagsAttributeDefinition*>(baseclassAttribute);
                Assets::FlagsAttributeDefinition* classFlags = static_cast<Assets::FlagsAttributeDefinition*>(classAttribute);

                for (int i = 0; i < 24; ++i) {
                    const Assets::FlagsAttributeOption* baseclassFlag = baseclassFlags->option(static_cast<int>(1 << i));
                    const Assets::FlagsAttributeOption* classFlag = classFlags->option(static_cast<int>(1 << i));

                    if (baseclassFlag != nullptr && classFlag == nullptr)
                        classFlags->addOption(baseclassFlag->value(), baseclassFlag->shortDescription(), baseclassFlag->longDescription(), baseclassFlag->isDefault());
                }
            }
        }

        bool addAttribute(std::vector<std::shared_ptr<Assets::AttributeDefinition>>& attributes, std::shared_ptr<Assets::AttributeDefinition> attribute) {
            assert(attribute != nullptr);
            if (kdl::vec_contains(attributes, [&](const auto& a) { return a->name() == attribute->name(); })) {
                return false;
            }
            
            attributes.push_back(std::move(attribute));
            return true;
        }

        bool operator==(const EntityDefinitionClassInfo& lhs, const EntityDefinitionClassInfo& rhs) {
            return lhs.type == rhs.type
                && lhs.line == rhs.line
                && lhs.column == rhs.column
                && lhs.name == rhs.name
                && lhs.description == rhs.description
                && lhs.color == rhs.color
                && lhs.size == rhs.size
                && lhs.modelDefinition == rhs.modelDefinition
                && lhs.attributes == rhs.attributes
                && lhs.superClasses == rhs.superClasses;
        }
        
        bool operator!=(const EntityDefinitionClassInfo& lhs, const EntityDefinitionClassInfo& rhs) {
            return !(lhs == rhs);
        }

        std::ostream& operator<<(std::ostream& str, const EntityDefinitionClassInfo& classInfo) {
            str << "EntityDefinitionClassInfo{ "
                << "type: " << classInfo.type << ", "
                << "line: " << classInfo.line << ", "
                << "column: " << classInfo.column << ", "
                << "name: " << classInfo.name << ", "
                << "description: " << kdl::opt_to_string(classInfo.description) << ", "
                << "color: " << kdl::opt_to_string(classInfo.color) << ", "
                << "size: " << kdl::opt_to_string(classInfo.size) << ", "
                << "modelDefinition: " << kdl::opt_to_string(classInfo.modelDefinition) << ", "
                << "attributes: {";
            for (const auto& attribute : classInfo.attributes) {
                str << "'" << attribute->name() << "', ";
            }
            str << "}, "
                << "superClasses: { ";
            for (const auto& superClass : classInfo.superClasses) {
                str << superClass << ", ";
            }
            str << " } "
                << " }";
            
            return str;
        }
    }
}
