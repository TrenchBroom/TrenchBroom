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
        class PropertyDefinition;
        class FlagsPropertyDefinition;
        class FlagsPropertyOption;

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
            using PropertyDefinitionPtr = std::shared_ptr<PropertyDefinition>;
            using PropertyDefinitionList = std::vector<PropertyDefinitionPtr>;
        private:
            size_t m_index;
            std::string m_name;
            Color m_color;
            std::string m_description;
            size_t m_usageCount;
            PropertyDefinitionList m_propertyDefinitions;
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

            const FlagsPropertyDefinition* spawnflags() const;
            const PropertyDefinitionList& propertyDefinitions() const;
            const PropertyDefinition* propertyDefinition(const std::string& propertyKey) const;

            static const PropertyDefinition* safeGetPropertyDefinition(const EntityDefinition* entityDefinition, const std::string& propertyKey);
            static const FlagsPropertyDefinition* safeGetFlagsPropertyDefinition(const EntityDefinition* entityDefinition, const std::string& propertyKey);

            static std::vector<EntityDefinition*> filterAndSort(const std::vector<EntityDefinition*>& definitions, EntityDefinitionType type, EntityDefinitionSortOrder prder = EntityDefinitionSortOrder::Name);
        protected:
            EntityDefinition(const std::string& name, const Color& color, const std::string& description, const PropertyDefinitionList& propertyDefinitions);
        };

        class PointEntityDefinition : public EntityDefinition {
        private:
            vm::bbox3 m_bounds;
            ModelDefinition m_modelDefinition;
        public:
            PointEntityDefinition(const std::string& name, const Color& color, const vm::bbox3& bounds, const std::string& description, const PropertyDefinitionList& propertyDefinitions, const ModelDefinition& modelDefinition);

            EntityDefinitionType type() const override;
            const vm::bbox3& bounds() const;
            ModelSpecification model(const EL::VariableStore& variableStore) const;
            ModelSpecification defaultModel() const;
            const ModelDefinition& modelDefinition() const;
        };

        class BrushEntityDefinition : public EntityDefinition {
        public:
            BrushEntityDefinition(const std::string& name, const Color& color, const std::string& description, const PropertyDefinitionList& propertyDefinitions);
            EntityDefinitionType type() const override;
        };
    }
}

