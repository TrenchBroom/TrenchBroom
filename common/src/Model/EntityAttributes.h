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

#ifndef TrenchBroom_EntityProperties
#define TrenchBroom_EntityProperties

#include "Assets/Asset_Forward.h"
#include "Model/Model_Forward.h"

#include <list>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace TrenchBroom {
    template <typename V>
    class StringMapValueContainer;

    template <typename V, typename P>
    class StringMap;

    namespace Model {
        extern const std::string AttributeEscapeChars;

        namespace AttributeNames {
            extern const AttributeName Classname;
            extern const AttributeName Origin;
            extern const AttributeName Wad;
            extern const AttributeName Textures;
            extern const AttributeName Mods;
            extern const AttributeName GameEngineParameterSpecs;
            extern const AttributeName Spawnflags;
            extern const AttributeName EntityDefinitions;
            extern const AttributeName Angle;
            extern const AttributeName Angles;
            extern const AttributeName Mangle;
            extern const AttributeName Target;
            extern const AttributeName Targetname;
            extern const AttributeName Killtarget;
            extern const AttributeName GroupType;
            extern const AttributeName LayerId;
            extern const AttributeName LayerName;
            extern const AttributeName Layer;
            extern const AttributeName GroupId;
            extern const AttributeName GroupName;
            extern const AttributeName Group;
            extern const AttributeName Message;
            extern const AttributeName ValveVersion;
        }

        namespace AttributeValues {
            extern const AttributeValue WorldspawnClassname;
            extern const AttributeValue NoClassname;
            extern const AttributeValue LayerClassname;
            extern const AttributeValue GroupClassname;
            extern const AttributeValue GroupTypeLayer;
            extern const AttributeValue GroupTypeGroup;
        }

        std::string numberedAttributePrefix(const std::string& name);
        bool isNumberedAttribute(const std::string& prefix, const AttributeName& name);

        class EntityAttributeSnapshot;

        class EntityAttribute {
        private:
            AttributeName m_name;
            AttributeValue m_value;
            const Assets::AttributeDefinition* m_definition;
        public:
            EntityAttribute();
            EntityAttribute(const AttributeName& name, const AttributeValue& value, const Assets::AttributeDefinition* definition = nullptr);
            bool operator<(const EntityAttribute& rhs) const;
            int compare(const EntityAttribute& rhs) const;

            const AttributeName& name() const;
            const AttributeValue& value() const;
            const Assets::AttributeDefinition* definition() const;

            void setName(const AttributeName& name, const Assets::AttributeDefinition* definition);
            void setValue(const AttributeValue& value);
        };

        bool isLayer(const std::string& classname, const std::list<EntityAttribute>& attributes);
        bool isGroup(const std::string& classname, const std::list<EntityAttribute>& attributes);
        bool isWorldspawn(const std::string& classname, const std::list<EntityAttribute>& attributes);
        const AttributeValue& findAttribute(const std::list<EntityAttribute>& attributes, const AttributeName& name, const AttributeValue& defaultValue = "");

        class EntityAttributes {
        private:
            std::list<EntityAttribute> m_attributes;

            using IndexValue = std::list<EntityAttribute>::iterator;
            using IndexValueContainer = StringMapValueContainer<IndexValue>;
            using AttributeIndex = StringMap<IndexValue, IndexValueContainer>;
            std::unique_ptr<AttributeIndex> m_index;
        public:
            explicit EntityAttributes();
            ~EntityAttributes();

            const std::list<EntityAttribute>& attributes() const;
            void setAttributes(const std::list<EntityAttribute>& attributes);

            const EntityAttribute& addOrUpdateAttribute(const AttributeName& name, const AttributeValue& value, const Assets::AttributeDefinition* definition);
            void renameAttribute(const AttributeName& name, const AttributeName& newName, const Assets::AttributeDefinition* newDefinition);
            void removeAttribute(const AttributeName& name);
            void updateDefinitions(const Assets::EntityDefinition* entityDefinition);

            bool hasAttribute(const AttributeName& name) const;
            bool hasAttribute(const AttributeName& name, const AttributeValue& value) const;
            bool hasAttributeWithPrefix(const AttributeName& prefix, const AttributeValue& value) const;
            bool hasNumberedAttribute(const AttributeName& prefix, const AttributeValue& value) const;

            EntityAttributeSnapshot snapshot(const AttributeName& name) const;
        private:
            bool containsValue(const std::vector<IndexValue>& matches, const AttributeValue& value) const;
            std::list<EntityAttribute> listFromQueryResult(const std::vector<IndexValue>& matches) const;
        public:
            const std::vector<AttributeName> names() const;
            const AttributeValue* attribute(const AttributeName& name) const;
            const AttributeValue& safeAttribute(const AttributeName& name, const AttributeValue& defaultValue) const;

            std::list<EntityAttribute> attributeWithName(const AttributeName& name) const;
            std::list<EntityAttribute> attributesWithPrefix(const AttributeName& prefix) const;
            std::list<EntityAttribute> numberedAttributes(const std::string& prefix) const;
        private:
            std::list<EntityAttribute>::const_iterator findAttribute(const AttributeName& name) const;
            std::list<EntityAttribute>::iterator findAttribute(const AttributeName& name);

            void rebuildIndex();
        };
    }
}

#endif /* defined(TrenchBroom_EntityProperties) */
