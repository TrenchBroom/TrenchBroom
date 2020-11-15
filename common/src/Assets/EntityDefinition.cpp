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

#include "EntityDefinition.h"

#include "Assets/AttributeDefinition.h"
#include "Model/EntityAttributes.h"

#include <kdl/string_compare.h>

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        EntityDefinition::~EntityDefinition() {}

        size_t EntityDefinition::index() const {
            return m_index;
        }

        void EntityDefinition::setIndex(const size_t index) {
            m_index = index;
        }

        const std::string& EntityDefinition::name() const {
            return m_name;
        }

        std::string EntityDefinition::shortName() const {
            const size_t index = m_name.find_first_of('_');
            if (index == std::string::npos)
                return m_name;
            return m_name.substr(index+1);
        }

        std::string EntityDefinition::groupName() const {
            const size_t index = m_name.find_first_of('_');
            if (index == std::string::npos)
                return m_name;
            return m_name.substr(0, index);
        }

        const Color& EntityDefinition::color() const {
            return m_color;
        }

        const std::string& EntityDefinition::description() const {
            return m_description;
        }

        size_t EntityDefinition::usageCount() const {
            return m_usageCount;
        }

        void EntityDefinition::incUsageCount() {
            ++m_usageCount;
            usageCountDidChangeNotifier();
        }

        void EntityDefinition::decUsageCount() {
            assert(m_usageCount > 0);
            --m_usageCount;
            usageCountDidChangeNotifier();
        }

        const FlagsAttributeDefinition* EntityDefinition::spawnflags() const {
            for (const auto& attributeDefinition : m_attributeDefinitions) {
                if (attributeDefinition->type() == AttributeDefinitionType::FlagsAttribute &&
                    attributeDefinition->name() == Model::AttributeNames::Spawnflags) {
                    return static_cast<FlagsAttributeDefinition*>(attributeDefinition.get());
                }
            }
            return nullptr;
        }

        const EntityDefinition::AttributeDefinitionList& EntityDefinition::attributeDefinitions() const {
            return m_attributeDefinitions;
        }

        const AttributeDefinition* EntityDefinition::attributeDefinition(const std::string& attributeKey) const {
            for (const auto& attributeDefinition : m_attributeDefinitions) {
                if (attributeDefinition->name() == attributeKey) {
                    return attributeDefinition.get();
                }
            }
            return nullptr;
        }

        const AttributeDefinition* EntityDefinition::safeGetAttributeDefinition(const EntityDefinition* entityDefinition, const std::string& attributeName) {
            if (entityDefinition == nullptr)
                return nullptr;
            return entityDefinition->attributeDefinition(attributeName);
        }

        const FlagsAttributeDefinition* EntityDefinition::safeGetFlagsAttributeDefinition(const EntityDefinition* entityDefinition, const std::string& attributeName) {
            if (entityDefinition == nullptr)
                return nullptr;
            const AttributeDefinition* attributeDefinition = entityDefinition->attributeDefinition(attributeName);
            if (attributeDefinition == nullptr || attributeDefinition->type() != AttributeDefinitionType::FlagsAttribute) {
                return nullptr;
            }
            return static_cast<const FlagsAttributeDefinition*>(attributeDefinition);
        }

        std::vector<EntityDefinition*> EntityDefinition::filterAndSort(const std::vector<EntityDefinition*>& definitions, const EntityDefinitionType type, const EntityDefinitionSortOrder order) {
            std::vector<EntityDefinition*> result;
            for (const auto& definition : definitions) {
                if (definition->type() == type) {
                    result.push_back(definition);
                }
            }

            if (order == EntityDefinitionSortOrder::Usage) {
                std::sort(std::begin(result), std::end(result),
                    [](const EntityDefinition* lhs, const EntityDefinition* rhs) {
                        if (lhs->usageCount() == rhs->usageCount()) {
                            return lhs->name() < rhs->name();
                        } else {
                            return lhs->usageCount() > rhs->usageCount();
                        }
                    });
            } else {
                std::sort(std::begin(result), std::end(result),
                    [](const EntityDefinition* lhs, const EntityDefinition* rhs) {
                        const int strCmp = kdl::ci::str_compare(lhs->name(), rhs->name());
                        if (strCmp == 0) {
                            return lhs->usageCount() > rhs->usageCount();
                        } else {
                            return strCmp < 0;
                        }
                    });
            }

            return result;
        }

        EntityDefinition::EntityDefinition(const std::string& name, const Color& color, const std::string& description, const AttributeDefinitionList& attributeDefinitions) :
        m_index(0),
        m_name(name),
        m_color(color),
        m_description(description),
        m_usageCount(0),
        m_attributeDefinitions(attributeDefinitions) {}

        PointEntityDefinition::PointEntityDefinition(const std::string& name, const Color& color, const vm::bbox3& bounds, const std::string& description, const AttributeDefinitionList& attributeDefinitions, const ModelDefinition& modelDefinition) :
        EntityDefinition(name, color, description, attributeDefinitions),
        m_bounds(bounds),
        m_modelDefinition(modelDefinition) {}

        EntityDefinitionType PointEntityDefinition::type() const {
            return EntityDefinitionType::PointEntity;
        }

        const vm::bbox3& PointEntityDefinition::bounds() const {
            return m_bounds;
        }

        ModelSpecification PointEntityDefinition::model(const EL::VariableStore& variableStore) const {
            return m_modelDefinition.modelSpecification(variableStore);
        }

        ModelSpecification PointEntityDefinition::defaultModel() const {
            return m_modelDefinition.defaultModelSpecification();
        }

        const ModelDefinition& PointEntityDefinition::modelDefinition() const {
            return m_modelDefinition;
        }

        BrushEntityDefinition::BrushEntityDefinition(const std::string& name, const Color& color, const std::string& description, const AttributeDefinitionList& attributeDefinitions) :
        EntityDefinition(name, color, description, attributeDefinitions) {}

        EntityDefinitionType BrushEntityDefinition::type() const {
            return EntityDefinitionType::BrushEntity;
        }
    }
}
