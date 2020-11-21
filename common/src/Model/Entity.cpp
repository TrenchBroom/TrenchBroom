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
#include "Model/EntityAttributes.h"
#include "Model/EntityAttributesVariableStore.h"
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
        const vm::bbox3 Entity::DefaultBounds = vm::bbox3(8.0);

        Entity::Entity() :
        m_pointEntity(true),
        m_model(nullptr) {}

        Entity::Entity(std::vector<EntityAttribute> attributes) :
        m_attributes(std::move(attributes)),
        m_pointEntity(true),
        m_model(nullptr) {}

        Entity::Entity(std::initializer_list<EntityAttribute> attributes) :
        m_attributes(attributes),
        m_pointEntity(true),
        m_model(nullptr) {}

        const std::vector<EntityAttribute>& Entity::attributes() const {
            return m_attributes;
        }

        Entity::Entity(const Entity& other) = default;
        Entity::Entity(Entity&& other) = default;

        Entity& Entity::operator=(const Entity& other) = default;
        Entity& Entity::operator=(Entity&& other) = default;

        Entity::~Entity() = default;

        void Entity::setAttributes(std::vector<EntityAttribute> attributes) {
            m_attributes = std::move(attributes);
            invalidateCachedAttributes();
        }

        bool Entity::pointEntity() const {
            return m_pointEntity;
        }

        void Entity::setPointEntity(const bool pointEntity) {
            if (m_pointEntity == pointEntity) {
                return;
            }

            m_pointEntity = pointEntity;
            invalidateCachedAttributes();
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

            m_definition = Assets::AssetReference(definition);
            invalidateCachedAttributes();
        }

        const Assets::EntityModelFrame* Entity::model() const {
            return m_model;
        }

        void Entity::setModel(const Assets::EntityModelFrame* model) {
            if (m_model == model) {
                return;
            }

            m_model = model;
            invalidateCachedAttributes();
        }

        Assets::ModelSpecification Entity::modelSpecification() const {
            if (const auto* pointDefinition = dynamic_cast<const Assets::PointEntityDefinition*>(m_definition.get())) {
                const auto variableStore = EntityAttributesVariableStore(*this);
                return pointDefinition->model(variableStore);
            } else {
                return Assets::ModelSpecification();
            }
        }

        const vm::mat4x4 Entity::modelTransformation() const {
            return vm::translation_matrix(origin()) * rotation();
        }

        void Entity::addOrUpdateAttribute(std::string name, std::string value) {
            auto it = findAttribute(name);
            if (it != std::end(m_attributes)) {
                it->setValue(value);
            } else {
                m_attributes.emplace_back(name, value);
            }
            invalidateCachedAttributes();
        }

        void Entity::renameAttribute(const std::string& oldName, std::string newName) {
            if (oldName == newName) {
                return;
            }

            const auto oldIt = findAttribute(oldName);
            if (oldIt != std::end(m_attributes)) {
                const auto newIt = findAttribute(newName);
                if (newIt != std::end(m_attributes)) {
                    m_attributes.erase(newIt);
                }
                
                oldIt->setName(std::move(newName));
                invalidateCachedAttributes();
            }
        }

        void Entity::removeAttribute(const std::string& name) {
            const auto it = findAttribute(name);
            if (it != std::end(m_attributes)) {
                m_attributes.erase(it);
                invalidateCachedAttributes();
            }
        }

        void Entity::removeNumberedAttribute(const std::string& prefix) {
            auto it = std::begin(m_attributes);
            while (it != std::end(m_attributes)) {
                if (it->hasNumberedPrefix(prefix)) {
                    it = m_attributes.erase(it);
                } else {
                    ++it;
                }
            }
            invalidateCachedAttributes();
        }

        bool Entity::hasAttribute(const std::string& name) const {
            return findAttribute(name) != std::end(m_attributes);
        }

        bool Entity::hasAttribute(const std::string& name, const std::string& value) const {
            const auto it = findAttribute(name);
            return it != std::end(m_attributes) && it->hasValue(value);
        }

        bool Entity::hasAttributeWithPrefix(const std::string& prefix, const std::string& value) const {
            for (const auto& attribute : m_attributes) {
                if (attribute.hasPrefixAndValue(prefix, value)) {
                    return true;
                }
            }
            return false;
        }

        bool Entity::hasNumberedAttribute(const std::string& prefix, const std::string& value) const {
            for (const auto& attribute : m_attributes) {
                if (attribute.hasNumberedPrefixAndValue(prefix, value)) {
                    return true;
                }
            }
            return false;
        }

        const std::string* Entity::attribute(const std::string& name) const {
            const auto it = findAttribute(name);
            return it != std::end(m_attributes) ? &it->value() : nullptr;
        }

        std::vector<std::string> Entity::attributeNames() const {
            return kdl::vec_transform(m_attributes, [](const auto& attribute) { return attribute.name(); });
        }


        const std::string& Entity::classname() const {
            validateCachedAttributes();
            return m_cachedAttributes->classname;
        }

        void Entity::setClassname(const std::string& classname) {
            addOrUpdateAttribute(AttributeNames::Classname, classname);
        }

        const vm::vec3& Entity::origin() const {
            validateCachedAttributes();
            return m_cachedAttributes->origin;
        }

        void Entity::setOrigin(const vm::vec3& origin) {
            addOrUpdateAttribute(AttributeNames::Origin, kdl::str_to_string(vm::correct(origin)));
        }

        const vm::mat4x4& Entity::rotation() const {
            validateCachedAttributes();
            return m_cachedAttributes->rotation;
        }

        std::vector<EntityAttribute> Entity::attributeWithName(const std::string& name) const {
            return kdl::vec_filter(m_attributes, [&](const auto& attribute) { return attribute.hasName(name); });
        }

        std::vector<EntityAttribute> Entity::attributesWithPrefix(const std::string& prefix) const {
            return kdl::vec_filter(m_attributes, [&](const auto& attribute) { return attribute.hasPrefix(prefix); });
        }

        std::vector<EntityAttribute> Entity::numberedAttributes(const std::string& prefix) const {
            return kdl::vec_filter(m_attributes, [&](const auto& attribute) { return attribute.hasNumberedPrefix(prefix); });
        }

        void Entity::transform(const vm::mat4x4& transformation) {
            if (m_pointEntity) {
                const auto offset = definitionBounds().center();
                const auto center = origin() + offset;
                const auto transformedCenter = transformation * center;
                setOrigin(transformedCenter - offset);

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

        void Entity::invalidateCachedAttributes() {
            m_cachedAttributes = std::nullopt;
        }
        
        void Entity::validateCachedAttributes() const {
            if (!m_cachedAttributes.has_value()) {
                const auto* classnameValue = attribute(AttributeNames::Classname);
                const auto* originValue = attribute(AttributeNames::Origin);

                // order is important here because EntityRotationPolicy::getRotation accesses classname
                m_cachedAttributes = CachedAttributes{};
                m_cachedAttributes->classname = classnameValue ? *classnameValue : AttributeValues::NoClassname;
                m_cachedAttributes->origin = originValue ? vm::parse<FloatType, 3>(*originValue, vm::vec3::zero()) : vm::vec3::zero();
                m_cachedAttributes->rotation = EntityRotationPolicy::getRotation(*this);
            }
        }

        std::vector<EntityAttribute>::const_iterator Entity::findAttribute(const std::string& name) const {
            return std::find_if(std::begin(m_attributes), std::end(m_attributes), [&](const auto& attribute) { return attribute.hasName(name); });
        }

        std::vector<EntityAttribute>::iterator Entity::findAttribute(const std::string& name) {
            return std::find_if(std::begin(m_attributes), std::end(m_attributes), [&](const auto& attribute) { return attribute.hasName(name); });
        }

        bool operator==(const Entity& lhs, const Entity& rhs) {
            return lhs.attributes() == rhs.attributes();
        }

        bool operator!=(const Entity& lhs, const Entity& rhs) {
            return !(lhs == rhs);
        }
    }
 }
