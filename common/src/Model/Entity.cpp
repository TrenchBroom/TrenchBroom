/*
 Copyright (C) 2020 Kristian Duske

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

#include "Entity.h"

#include "Assets/EntityDefinition.h"
#include "Assets/EntityModel.h"
#include "Assets/ModelDefinition.h"
#include "Model/EntityProperties.h"
#include "Model/EntityPropertiesVariableStore.h"
#include "Model/EntityRotationPolicy.h"

#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/bbox.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        const vm::bbox3 Entity::DefaultBounds = vm::bbox3{8.0};

        Entity::Entity() :
        m_pointEntity{true},
        m_model{nullptr} {
            updateCachedProperties();
        }

        Entity::Entity(std::vector<EntityProperty> properties) :
        m_properties{std::move(properties)},
        m_pointEntity{true},
        m_model{nullptr} {
            updateCachedProperties();
        }

        Entity::Entity(std::initializer_list<EntityProperty> properties) :
        m_properties{std::move(properties)},
        m_pointEntity{true},
        m_model{nullptr} {
            updateCachedProperties();
        }

        const std::vector<EntityProperty>& Entity::properties() const {
            return m_properties;
        }

        Entity::Entity(const Entity& other) = default;
        Entity::Entity(Entity&& other) = default;

        Entity& Entity::operator=(const Entity& other) = default;
        Entity& Entity::operator=(Entity&& other) = default;

        Entity::~Entity() = default;

        void Entity::setProperties(std::vector<EntityProperty> properties) {
            m_properties = std::move(properties);
            updateCachedProperties();
        }

        const std::vector<std::string>& Entity::protectedProperties() const {
            return m_protectedProperties;
        }

        void Entity::setProtectedProperties(std::vector<std::string> protectedProperties) {
            m_protectedProperties = std::move(protectedProperties);
        }

        bool Entity::pointEntity() const {
            return m_pointEntity;
        }

        void Entity::setPointEntity(const bool pointEntity) {
            if (m_pointEntity == pointEntity) {
                return;
            }

            m_pointEntity = pointEntity;
            updateCachedProperties();
        }

        Assets::EntityDefinition* Entity::definition() {
            return m_definition.get();
        }

        const Assets::EntityDefinition* Entity::definition() const {
            return m_definition.get();
        }

        const vm::bbox3& Entity::definitionBounds() const {
            return definition() && definition()->type () == Assets::EntityDefinitionType::PointEntity
                ? static_cast<const Assets::PointEntityDefinition*>(definition())->bounds() 
                : DefaultBounds;
        }

        void Entity::setDefinition(Assets::EntityDefinition* definition) {
            if (m_definition.get() == definition) {
                return;
            }

            m_definition = Assets::AssetReference{definition};
            updateCachedProperties();
        }

        const Assets::EntityModelFrame* Entity::model() const {
            return m_model;
        }

        void Entity::setModel(const Assets::EntityModelFrame* model) {
            if (m_model == model) {
                return;
            }

            m_model = model;
            updateCachedProperties();
        }

        Assets::ModelSpecification Entity::modelSpecification() const {
            if (const auto* pointDefinition = dynamic_cast<const Assets::PointEntityDefinition*>(m_definition.get())) {
                const auto variableStore = EntityPropertiesVariableStore{*this};
                return pointDefinition->model(variableStore);
            } else {
                return Assets::ModelSpecification{};
            }
        }

        const vm::mat4x4& Entity::modelTransformation() const {
            return m_cachedProperties.modelTransformation;
        }

        void Entity::addOrUpdateProperty(std::string key, std::string value, const bool defaultToProtected) {
            auto it = findProperty(key);
            if (it != std::end(m_properties)) {
                it->setValue(std::move(value));
            } else {
                m_properties.emplace_back(key, std::move(value));

                if (defaultToProtected && !kdl::vec_contains(m_protectedProperties, key)) {
                    m_protectedProperties.push_back(std::move(key));
                }
            }
            updateCachedProperties();
        }

        void Entity::renameProperty(const std::string& oldKey, std::string newKey) {
            if (oldKey == newKey) {
                return;
            }

            const auto oldIt = findProperty(oldKey);
            if (oldIt != std::end(m_properties)) {
                if (const auto protIt = std::find(std::begin(m_protectedProperties), std::end(m_protectedProperties), oldKey); protIt != std::end(m_protectedProperties)) {
                    m_protectedProperties.erase(protIt);
                    m_protectedProperties.push_back(newKey);
                }

                const auto newIt = findProperty(newKey);
                if (newIt != std::end(m_properties)) {
                    m_properties.erase(newIt);
                }

                oldIt->setKey(std::move(newKey));
                updateCachedProperties();
            }
        }

        void Entity::removeProperty(const std::string& key) {
            const auto it = findProperty(key);
            if (it != std::end(m_properties)) {
                m_properties.erase(it);
                updateCachedProperties();
            }
        }

        void Entity::removeNumberedProperty(const std::string& prefix) {
            auto it = std::begin(m_properties);
            while (it != std::end(m_properties)) {
                if (it->hasNumberedPrefix(prefix)) {
                    it = m_properties.erase(it);
                } else {
                    ++it;
                }
            }
            updateCachedProperties();
        }

        bool Entity::hasProperty(const std::string& key) const {
            return findProperty(key) != std::end(m_properties);
        }

        bool Entity::hasProperty(const std::string& key, const std::string& value) const {
            const auto it = findProperty(key);
            return it != std::end(m_properties) && it->hasValue(value);
        }

        bool Entity::hasPropertyWithPrefix(const std::string& prefix, const std::string& value) const {
            return std::any_of(std::begin(m_properties), std::end(m_properties), [&](const auto& property) {
                return property.hasPrefixAndValue(prefix, value);
            });
        }

        bool Entity::hasNumberedProperty(const std::string& prefix, const std::string& value) const {
            return std::any_of(std::begin(m_properties), std::end(m_properties), [&](const auto& property) {
                return property.hasNumberedPrefixAndValue(prefix, value);
            });
        }

        const std::string* Entity::property(const std::string& key) const {
            const auto it = findProperty(key);
            return it != std::end(m_properties) ? &it->value() : nullptr;
        }

        std::vector<std::string> Entity::propertyKeys() const {
            return kdl::vec_transform(m_properties, [](const auto& property) { return property.key(); });
        }


        const std::string& Entity::classname() const {
            return m_cachedProperties.classname;
        }

        void Entity::setClassname(const std::string& classname) {
            addOrUpdateProperty(EntityPropertyKeys::Classname, classname);
        }

        const vm::vec3& Entity::origin() const {
            return m_cachedProperties.origin;
        }

        void Entity::setOrigin(const vm::vec3& origin) {
            addOrUpdateProperty(EntityPropertyKeys::Origin, kdl::str_to_string(vm::correct(origin)));
        }

        const vm::mat4x4& Entity::rotation() const {
            return m_cachedProperties.rotation;
        }

        std::vector<EntityProperty> Entity::propertiesWithKey(const std::string& key) const {
            return kdl::vec_filter(m_properties, [&](const auto& property) { return property.hasKey(key); });
        }

        std::vector<EntityProperty> Entity::propertiesWithPrefix(const std::string& prefix) const {
            return kdl::vec_filter(m_properties, [&](const auto& property) { return property.hasPrefix(prefix); });
        }

        std::vector<EntityProperty> Entity::numberedProperties(const std::string& prefix) const {
            return kdl::vec_filter(m_properties, [&](const auto& property) { return property.hasNumberedPrefix(prefix); });
        }

        void Entity::transform(const vm::mat4x4& transformation) {
            if (m_pointEntity) {
                const auto offset = definitionBounds().center();
                const auto center = origin() + offset;
                const auto transformedCenter = transformation * center;
                const auto newOrigin = transformedCenter - offset;
                if (origin() != newOrigin) {
                    setOrigin(transformedCenter - offset);
                }

                // applying rotation has side effects (e.g. normalizing "angles")
                // so only do it if there is actually some rotation.
                const auto rotation = vm::strip_translation(transformation);
                if (rotation != vm::mat4x4::identity()) {
                    // applyRotation does not read the origin, so it's ok that it's already updated now
                    applyRotation(rotation);
                }
            }
        }

        void Entity::applyRotation(const vm::mat4x4& rotation) {
            EntityRotationPolicy::applyRotation(*this, rotation);
        }

        void Entity::updateCachedProperties() {
            const auto* classnameValue = property(EntityPropertyKeys::Classname);
            const auto* originValue = property(EntityPropertyKeys::Origin);

            // order is important here because EntityRotationPolicy::getRotation accesses classname
            m_cachedProperties.classname = classnameValue ? *classnameValue : EntityPropertyValues::NoClassname;
            m_cachedProperties.origin = originValue ? vm::parse<FloatType, 3>(*originValue).value_or(vm::vec3::zero()) : vm::vec3::zero();
            m_cachedProperties.rotation = EntityRotationPolicy::getRotation(*this);
            m_cachedProperties.modelTransformation = vm::translation_matrix(origin()) * rotation();
        }

        std::vector<EntityProperty>::const_iterator Entity::findProperty(const std::string& key) const {
            return std::find_if(std::begin(m_properties), std::end(m_properties), [&](const auto& property) { return property.hasKey(key); });
        }

        std::vector<EntityProperty>::iterator Entity::findProperty(const std::string& key) {
            return std::find_if(std::begin(m_properties), std::end(m_properties), [&](const auto& property) { return property.hasKey(key); });
        }

        bool operator==(const Entity& lhs, const Entity& rhs) {
            return lhs.properties() == rhs.properties();
        }

        bool operator!=(const Entity& lhs, const Entity& rhs) {
            return !(lhs == rhs);
        }
    }
 }
