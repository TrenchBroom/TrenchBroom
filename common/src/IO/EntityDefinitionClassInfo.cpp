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
#include "Assets/PropertyDefinition.h"
#include "Model/EntityProperties.h"

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

        bool addAttribute(std::vector<std::shared_ptr<Assets::PropertyDefinition>>& attributes, std::shared_ptr<Assets::PropertyDefinition> attribute) {
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
