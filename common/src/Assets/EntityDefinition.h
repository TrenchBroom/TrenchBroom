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

#ifndef TrenchBroom_EntityDefinition
#define TrenchBroom_EntityDefinition

#include "Color.h"
#include "FloatType.h"
#include "Notifier.h"
#include "Assets/ModelDefinition.h"

#include <vecmath/bbox.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace EL {
        class VariableStore;
    }

    namespace Assets {
        class AttributeDefinition;
        class FlagsAttributeDefinition;
        class FlagsAttributeOption;

        enum class EntityDefinitionType {
            PointEntity,
            BrushEntity
        };

        enum class EntityDefinitionSortOrder {
            Name,
            Usage
        };

        class EntityDefinition {
        protected:
            using AttributeDefinitionPtr = std::shared_ptr<AttributeDefinition>;
            using AttributeDefinitionList = std::vector<AttributeDefinitionPtr>;
        private:
            size_t m_index;
            std::string m_name;
            Color m_color;
            std::string m_description;
            size_t m_usageCount;
            AttributeDefinitionList m_attributeDefinitions;
        public:
            Notifier<> usageCountDidChangeNotifier;
        public:
            virtual ~EntityDefinition();

            size_t index() const;
            void setIndex(size_t index);

            virtual EntityDefinitionType type() const = 0;
            const std::string& name() const;
            std::string shortName() const;
            std::string groupName() const;
            const Color& color() const;
            const std::string& description() const;
            size_t usageCount() const;
            void incUsageCount();
            void decUsageCount();

            const FlagsAttributeDefinition* spawnflags() const;
            const AttributeDefinitionList& attributeDefinitions() const;
            const AttributeDefinition* attributeDefinition(const std::string& attributeKey) const;

            static const AttributeDefinition* safeGetAttributeDefinition(const EntityDefinition* entityDefinition, const std::string& attributeName);
            static const FlagsAttributeDefinition* safeGetFlagsAttributeDefinition(const EntityDefinition* entityDefinition, const std::string& attributeName);

            static std::vector<EntityDefinition*> filterAndSort(const std::vector<EntityDefinition*>& definitions, EntityDefinitionType type, EntityDefinitionSortOrder prder = EntityDefinitionSortOrder::Name);
        protected:
            EntityDefinition(const std::string& name, const Color& color, const std::string& description, const AttributeDefinitionList& attributeDefinitions);
        };

        class PointEntityDefinition : public EntityDefinition {
        private:
            vm::bbox3 m_bounds;
            ModelDefinition m_modelDefinition;
        public:
            PointEntityDefinition(const std::string& name, const Color& color, const vm::bbox3& bounds, const std::string& description, const AttributeDefinitionList& attributeDefinitions, const ModelDefinition& modelDefinition);

            EntityDefinitionType type() const override;
            const vm::bbox3& bounds() const;
            ModelSpecification model(const EL::VariableStore& variableStore) const;
            ModelSpecification defaultModel() const;
            const ModelDefinition& modelDefinition() const;
        };

        class BrushEntityDefinition : public EntityDefinition {
        public:
            BrushEntityDefinition(const std::string& name, const Color& color, const std::string& description, const AttributeDefinitionList& attributeDefinitions);
            EntityDefinitionType type() const override;
        };
    }
}

#endif /* defined(TrenchBroom_EntityDefinition) */
