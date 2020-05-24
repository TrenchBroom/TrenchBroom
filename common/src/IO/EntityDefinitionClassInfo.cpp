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

#include "Assets/AttributeDefinition.h"
#include "Model/EntityAttributes.h"

#include <kdl/map_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        EntityDefinitionClassInfo::EntityDefinitionClassInfo() :
        m_line(0),
        m_column(0),
        m_hasDescription(false),
        m_hasColor(false),
        m_size(vm::bbox3(-8.0, 8.0)),
        m_hasSize(false),
        m_hasModelDefinition(false) {}

        EntityDefinitionClassInfo::EntityDefinitionClassInfo(const size_t line, const size_t column, const Color& defaultColor) :
        m_line(line),
        m_column(column),
        m_hasDescription(false),
        m_color(defaultColor),
        m_hasColor(false),
        m_size(vm::bbox3(-8.0, 8.0)),
        m_hasSize(false),
        m_hasModelDefinition(false) {}


        size_t EntityDefinitionClassInfo::line() const {
            return m_line;
        }

        size_t EntityDefinitionClassInfo::column() const {
            return m_column;
        }

        const std::string& EntityDefinitionClassInfo::name() const {
            return m_name;
        }

        const std::string& EntityDefinitionClassInfo::description() const {
            return m_description;
        }

        bool EntityDefinitionClassInfo::hasDescription() const {
            return m_hasDescription;
        }

        const Color& EntityDefinitionClassInfo::color() const {
            return m_color;
        }

        bool EntityDefinitionClassInfo::hasColor() const {
            return m_hasColor;
        }

        const vm::bbox3& EntityDefinitionClassInfo::size() const {
            return m_size;
        }

        bool EntityDefinitionClassInfo::hasSize() const {
            return m_hasSize;
        }

        std::vector<std::shared_ptr<Assets::AttributeDefinition>> EntityDefinitionClassInfo::attributeList() const {
            return kdl::map_values(m_attributes);
        }

        const std::map<std::string, std::shared_ptr<Assets::AttributeDefinition>>& EntityDefinitionClassInfo::attributeMap() const {
            return m_attributes;
        }

        const Assets::ModelDefinition& EntityDefinitionClassInfo::modelDefinition() const {
            return m_modelDefinition;
        }

        bool EntityDefinitionClassInfo::hasModelDefinition() const {
            return m_hasModelDefinition;
        }

        void EntityDefinitionClassInfo::setName(const std::string& name) {
            m_name = name;
        }

        void EntityDefinitionClassInfo::setDescription(const std::string& description) {
            m_description = description;
            m_hasDescription = true;
        }

        void EntityDefinitionClassInfo::setColor(const Color& color) {
            m_color = color;
            m_hasColor = true;
        }

        void EntityDefinitionClassInfo::setSize(const vm::bbox3& size) {
            m_size = size;
            m_hasSize = true;
        }

        void EntityDefinitionClassInfo::addAttributeDefinition(std::shared_ptr<Assets::AttributeDefinition> attributeDefinition) {
            m_attributes[attributeDefinition->name()] = attributeDefinition;
        }

        void EntityDefinitionClassInfo::addAttributeDefinitions(const std::map<std::string, std::shared_ptr<Assets::AttributeDefinition>>& attributeDefinitions) {
            m_attributes.insert(std::begin(attributeDefinitions), std::end(attributeDefinitions));
        }

        void EntityDefinitionClassInfo::setModelDefinition(const Assets::ModelDefinition& modelDefinition) {
            m_modelDefinition = modelDefinition;
            m_hasModelDefinition = true;
        }

        void EntityDefinitionClassInfo::resolveBaseClasses(const std::map<std::string, EntityDefinitionClassInfo>& baseClasses, const std::vector<std::string>& classnames) {
            for (auto classnameIt = classnames.rbegin(), classnameEnd = classnames.rend(); classnameIt != classnameEnd; ++classnameIt) {
                const std::string& classname = *classnameIt;
                const auto baseClassIt = baseClasses.find(classname);
                if (baseClassIt != std::end(baseClasses)) {
                    const EntityDefinitionClassInfo& baseClass = baseClassIt->second;
                    if (!hasDescription() && baseClass.hasDescription())
                        setDescription(baseClass.description());
                    if (!hasColor() && baseClass.hasColor())
                        setColor(baseClass.color());
                    if (!hasSize() && baseClass.hasSize())
                        setSize(baseClass.size());

                    const auto& baseProperties = baseClass.attributeMap();
                    for (const auto& entry : baseProperties) {
                        const std::shared_ptr<Assets::AttributeDefinition> baseAttribute = entry.second;

                        auto classAttributeIt = m_attributes.find(baseAttribute->name());
                        if (classAttributeIt != std::end(m_attributes)) {
                            // the class already has a definition for this attribute, attempt merging them
                            mergeProperties(classAttributeIt->second.get(), baseAttribute.get());
                        } else {
                            // the class doesn't have a definition for this attribute, add the base class attribute
                            addAttributeDefinition(baseAttribute);
                        }
                    }

                    m_modelDefinition.append(baseClass.modelDefinition());
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
    }
}
