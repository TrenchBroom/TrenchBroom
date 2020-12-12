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

#include "FloatType.h"
#include "Color.h"
#include "Assets/ModelDefinition.h"

#include <vecmath/bbox.h>

#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class PropertyDefinition;
    }

    namespace IO {
        enum class EntityDefinitionClassType {
            PointClass,
            BrushClass,
            BaseClass
        };
        
        std::ostream& operator<<(std::ostream& str, EntityDefinitionClassType type);
    
        struct EntityDefinitionClassInfo {
            EntityDefinitionClassType type;
            size_t line;
            size_t column;
            std::string name;

            std::optional<std::string> description;
            std::optional<Color> color;
            std::optional<vm::bbox3> size;
            std::optional<Assets::ModelDefinition> modelDefinition;

            std::vector<std::shared_ptr<Assets::PropertyDefinition>> attributes;
            std::vector<std::string> superClasses;
        };

        bool addAttribute(std::vector<std::shared_ptr<Assets::PropertyDefinition>>& attributes, std::shared_ptr<Assets::PropertyDefinition> attribute);

        bool operator==(const EntityDefinitionClassInfo& lhs, const EntityDefinitionClassInfo& rhs);
        bool operator!=(const EntityDefinitionClassInfo& lhs, const EntityDefinitionClassInfo& rhs);
        
        std::ostream& operator<<(std::ostream& str, const EntityDefinitionClassInfo& classInfo);
    }
}

