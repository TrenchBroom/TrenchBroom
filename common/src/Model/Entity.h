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

#ifndef TrenchBroom_Entity
#define TrenchBroom_Entity

#include "FloatType.h"
#include "Assets/AssetReference.h"
#include "Model/EntityAttributes.h"

#include <vecmath/forward.h>
#include <vecmath/mat.h>
#include <vecmath/vec.h>

#include <optional>

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinition;
        class EntityModelFrame;
        struct ModelSpecification;
    }

    namespace Model {
        /**
         * An entity is essentially a collection of key / value pairs called attributes. Attributes can be set, renamed
         * or removed and accessed via the corresponding member functions.
         *
         * Some attributes are numbered, e.g. "target1", "target2", and so on. These attributes correspond to a multi valued 
         * attribute whose name is the common prefix, e.g. "target", and whose value the union of the values of the corresponding
         * numbered attributes. Note that any attribute counts as a numbered attribute even if it does not have a number as its
         * suffix, so even an attribute named "target" is implicitly a numbered attribute.
         *
         * Entities are geometric objects and as such can be transformed. Rotation is handled specially by a set of rules to make
         * it easier for users to apply rotation to entities. Point entities are rotated by the center of their bounding box and
         * not their origin attribute, which denotes their position. Furthermore, when a point entity is rotated, certain rules
         * are applied to update directional attributes such as "angle" or "mangle".
         *
         * Brush entities are not subject to any of these rules. They are rotated simply by applying rotation to their constituent
         * brushes.
         */
        class Entity {
        public:
            static const vm::bbox3 DefaultBounds;
        private:
            std::vector<EntityAttribute> m_attributes;

            /**
             * Specifies whether this entity has children or not. This does not necessarily correspond to the
             * entity definition type because point entities can contain brushes.
             */
            bool m_pointEntity;

            Assets::AssetReference<Assets::EntityDefinition> m_definition;
            const Assets::EntityModelFrame* m_model;

            /**
             * These attributes are cached for performance reasons.
             */
            struct CachedAttributes {
                std::string classname;
                vm::vec3 origin;
                vm::mat4x4 rotation;
            };

            mutable std::optional<CachedAttributes> m_cachedAttributes;
        public:
            Entity();
            explicit Entity(std::vector<EntityAttribute> attributes);
            explicit Entity(std::initializer_list<EntityAttribute> attributes);

            Entity(const Entity& other);
            Entity(Entity&& other);

            Entity& operator=(const Entity& other);
            Entity& operator=(Entity&& other);

            ~Entity();

            const std::vector<EntityAttribute>& attributes() const;
            void setAttributes(std::vector<EntityAttribute> attributes);

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

            void addOrUpdateAttribute(std::string name, std::string value);
            void renameAttribute(const std::string& oldName, std::string newName);
            void removeAttribute(const std::string& name);
            void removeNumberedAttribute(const std::string& prefix);

            bool hasAttribute(const std::string& name) const;
            bool hasAttribute(const std::string& name, const std::string& value) const;

            bool hasAttributeWithPrefix(const std::string& prefix, const std::string& value) const;
            bool hasNumberedAttribute(const std::string& prefix, const std::string& value) const;

            const std::string* attribute(const std::string& name) const;
            std::vector<std::string> attributeNames() const;

            const std::string& classname() const;
            void setClassname(const std::string& classname);

            const vm::vec3& origin() const;
            void setOrigin(const vm::vec3& origin);

            const vm::mat4x4& rotation() const;

            std::vector<EntityAttribute> attributeWithName(const std::string& name) const;
            std::vector<EntityAttribute> attributesWithPrefix(const std::string& prefix) const;
            std::vector<EntityAttribute> numberedAttributes(const std::string& prefix) const;

            void transform(const vm::mat4x4& transformation);
        private:
            void applyRotation(const vm::mat4x4& rotation);
            
            void invalidateCachedAttributes();
            void validateCachedAttributes() const;

            std::vector<EntityAttribute>::const_iterator findAttribute(const std::string& name) const;
            std::vector<EntityAttribute>::iterator findAttribute(const std::string& name);
        };

        bool operator==(const Entity& lhs, const Entity& rhs);
        bool operator!=(const Entity& lhs, const Entity& rhs);
    }
 }
 
 #endif