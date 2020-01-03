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

#ifndef TrenchBroom_EntityDefinitionClassInfo
#define TrenchBroom_EntityDefinitionClassInfo

#include "TrenchBroom.h"
#include "Color.h"
#include "Assets/Asset_Forward.h"
#include "Assets/ModelDefinition.h"

#include <vecmath/bbox.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class EntityDefinitionClassInfo {
        private:
            size_t m_line;
            size_t m_column;
            std::string m_name;
            std::string m_description;
            bool m_hasDescription;
            Color m_color;
            bool m_hasColor;
            vm::bbox3 m_size;
            bool m_hasSize;
            std::map<std::string, std::shared_ptr<Assets::AttributeDefinition>> m_attributes;
            Assets::ModelDefinition m_modelDefinition;
            bool m_hasModelDefinition;
        public:
            EntityDefinitionClassInfo();
            EntityDefinitionClassInfo(size_t line, size_t column, const Color& defaultColor);

            size_t line() const;
            size_t column() const;
            const std::string& name() const;
            const std::string& description() const;
            bool hasDescription() const;
            const Color& color() const;
            bool hasColor() const;
            const vm::bbox3& size() const;
            bool hasSize() const;
            std::vector<std::shared_ptr<Assets::AttributeDefinition>> attributeList() const;
            const std::map<std::string, std::shared_ptr<Assets::AttributeDefinition>>& attributeMap() const;
            const Assets::ModelDefinition& modelDefinition() const;
            bool hasModelDefinition() const;

            void setName(const std::string& name);
            void setDescription(const std::string& description);
            void setColor(const Color& color);
            void setSize(const vm::bbox3& size);
            void addAttributeDefinition(std::shared_ptr<Assets::AttributeDefinition> attributeDefinition);
            void addAttributeDefinitions(const std::map<std::string, std::shared_ptr<Assets::AttributeDefinition>>& attributeDefinitions);
            void setModelDefinition(const Assets::ModelDefinition& modelDefinition);

            void resolveBaseClasses(const std::map<std::string, EntityDefinitionClassInfo>& baseClasses, const std::vector<std::string>& classnames);
        private:
            static void mergeProperties(Assets::AttributeDefinition* classAttribute, const Assets::AttributeDefinition* baseclassAttribute);
        };
    }
}

#endif /* defined(TrenchBroom_EntityDefinitionClassInfo) */
