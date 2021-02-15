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

#pragma once

#include "FloatType.h"
#include "Assets/AssetReference.h"
#include "Model/EntityProperties.h"

#include <vecmath/forward.h>
#include <vecmath/mat.h>
#include <vecmath/vec.h>

#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinition;
        class EntityModelFrame;
        struct ModelSpecification;
    }

    namespace Model {
        /**
         * An entity is essentially a collection of key / value pairs called properties. Properties can be set, renamed
         * or removed and accessed via the corresponding member functions.
         *
         * Some properties are numbered, e.g. "target1", "target2", and so on. These properties correspond to a multi valued
         * property whose name is the common prefix, e.g. "target", and whose value the union of the values of the corresponding
         * numbered properties. Note that any property counts as a numbered property even if it does not have a number as its
         * suffix, so even an property named "target" is implicitly a numbered property.
         *
         * Entity properties can be protected, which protects them from being updated by a corresponding entity in a linked group.
         * If an entity property is protected, any change to the same property from a corresponding entity in a linked group is
         * ignored. This means that the entity value remains unchanged. Note that properties can be protected even if they don't
         * exist; in this case, adding this property in a corresponding entity will not add the property here.
         *
         * Entities are geometric objects and as such can be transformed. Rotation is handled specially by a set of rules to make
         * it easier for users to apply rotation to entities. Point entities are rotated by the center of their bounding box and
         * not their origin property, which denotes their position. Furthermore, when a point entity is rotated, certain rules
         * are applied to update directional properties such as "angle" or "mangle".
         *
         * Brush entities are not subject to any of these rules. They are rotated simply by applying rotation to their constituent
         * brushes.
         */
        class Entity {
        public:
            static const vm::bbox3 DefaultBounds;
        private:
            std::vector<EntityProperty> m_properties;
            std::vector<std::string> m_protectedProperties;

            /**
             * Specifies whether this entity has children or not. This does not necessarily correspond to the
             * entity definition type because point entities can contain brushes.
             */
            bool m_pointEntity;

            Assets::AssetReference<Assets::EntityDefinition> m_definition;
            const Assets::EntityModelFrame* m_model;

            /**
             * These properties are cached for performance reasons.
             */
            struct CachedProperties {
                std::string classname;
                vm::vec3 origin;
                vm::mat4x4 rotation;
            };

            mutable std::optional<CachedProperties> m_cachedProperties;
        public:
            Entity();
            explicit Entity(std::vector<EntityProperty> properties);
            explicit Entity(std::initializer_list<EntityProperty> properties);

            Entity(const Entity& other);
            Entity(Entity&& other);

            Entity& operator=(const Entity& other);
            Entity& operator=(Entity&& other);

            ~Entity();

            const std::vector<EntityProperty>& properties() const;
            void setProperties(std::vector<EntityProperty> properties);

            /**
             * Sets the protected property keys of this entity.
             *
             * Protected entity properties are not propagated into linked groups and are not overwritten
             * when a linked group updates this entity. See also GroupNode::updateLinkedGroups
             */
            const std::vector<std::string>& protectedProperties() const;
            void setProtectedProperties(std::vector<std::string> protectedProperties);

            bool pointEntity() const;
            void setPointEntity(bool pointEntity);

            Assets::EntityDefinition* definition();
            const Assets::EntityDefinition* definition() const;
            const vm::bbox3& definitionBounds() const;
            void setDefinition(Assets::EntityDefinition* definition);

            const Assets::EntityModelFrame* model() const;
            void setModel(const Assets::EntityModelFrame* model);

            Assets::ModelSpecification modelSpecification() const;
            const vm::mat4x4 modelTransformation() const;

            void addOrUpdateProperty(std::string key, std::string value, bool defaultToProtected = false);
            void renameProperty(const std::string& oldKey, std::string newKey);
            void removeProperty(const std::string& key);
            void removeNumberedProperty(const std::string& prefix);

            bool hasProperty(const std::string& key) const;
            bool hasProperty(const std::string& key, const std::string& value) const;

            bool hasPropertyWithPrefix(const std::string& prefix, const std::string& value) const;
            bool hasNumberedProperty(const std::string& prefix, const std::string& value) const;

            const std::string* property(const std::string& key) const;
            std::vector<std::string> propertyKeys() const;

            const std::string& classname() const;
            void setClassname(const std::string& classname);

            const vm::vec3& origin() const;
            void setOrigin(const vm::vec3& origin);

            const vm::mat4x4& rotation() const;

            std::vector<EntityProperty> propertiesWithKey(const std::string& property) const;
            std::vector<EntityProperty> propertiesWithPrefix(const std::string& property) const;
            std::vector<EntityProperty> numberedProperties(const std::string& property) const;

            void transform(const vm::mat4x4& transformation);
        private:
            void applyRotation(const vm::mat4x4& rotation);
            
            void invalidateCachedProperties();
            void validateCachedProperties() const;

            std::vector<EntityProperty>::const_iterator findProperty(const std::string& property) const;
            std::vector<EntityProperty>::iterator findProperty(const std::string& property);
        };

        bool operator==(const Entity& lhs, const Entity& rhs);
        bool operator!=(const Entity& lhs, const Entity& rhs);
    }
 }
