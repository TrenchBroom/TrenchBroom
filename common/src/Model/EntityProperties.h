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

#include "EL/Expression.h"

#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        namespace EntityPropertyKeys {
            extern const std::string Classname;
            extern const std::string Origin;
            extern const std::string Wad;
            extern const std::string Textures;
            extern const std::string Mods;
            extern const std::string Spawnflags;
            extern const std::string EntityDefinitions;
            extern const std::string Angle;
            extern const std::string Angles;
            extern const std::string Mangle;
            extern const std::string Target;
            extern const std::string Targetname;
            extern const std::string Killtarget;
            extern const std::string ProtectedEntityProperties;
            extern const std::string GroupType;
            extern const std::string LayerId;
            extern const std::string LayerName;
            extern const std::string LayerSortIndex;
            extern const std::string LayerColor;
            extern const std::string LayerLocked;
            extern const std::string LayerHidden;
            extern const std::string LayerOmitFromExport;
            extern const std::string Layer;
            extern const std::string GroupId;
            extern const std::string GroupName;
            extern const std::string Group;
            extern const std::string GroupTransformation;
            extern const std::string LinkedGroupId;
            extern const std::string Message;
            extern const std::string ValveVersion;
            extern const std::string SoftMapBounds;
        }

        namespace EntityPropertyValues {
            extern const std::string WorldspawnClassname;
            extern const std::string NoClassname;
            extern const std::string LayerClassname;
            extern const std::string GroupClassname;
            extern const std::string GroupTypeLayer;
            extern const std::string GroupTypeGroup;
            extern const std::string DefaultValue;
            extern const std::string NoSoftMapBounds;
            extern const std::string LayerLockedValue;
            extern const std::string LayerHiddenValue;
            extern const std::string LayerOmitFromExportValue;
        }

        struct EntityPropertyConfig {
            std::optional<EL::Expression> defaultModelScaleExpression;
        };

        bool operator==(const EntityPropertyConfig& lhs, const EntityPropertyConfig& rhs);
        bool operator!=(const EntityPropertyConfig& lhs, const EntityPropertyConfig& rhs);

        std::ostream& operator<<(std::ostream& lhs, const EntityPropertyConfig& rhs);

        bool isNumberedProperty(std::string_view prefix, std::string_view key);

        class EntityProperty {
        private:
            std::string m_key;
            std::string m_value;
        public:
            EntityProperty();
            EntityProperty(const std::string& key, const std::string& value);
            
            int compare(const EntityProperty& rhs) const;

            const std::string& key() const;
            const std::string& value() const;

            bool hasKey(std::string_view key) const;
            bool hasValue(std::string_view value) const;
            bool hasKeyAndValue(std::string_view key, std::string_view value) const;
            bool hasPrefix(std::string_view prefix) const;
            bool hasPrefixAndValue(std::string_view prefix, std::string_view value) const;
            bool hasNumberedPrefix(std::string_view prefix) const;
            bool hasNumberedPrefixAndValue(std::string_view prefix, std::string_view value) const;

            void setKey(const std::string& key);
            void setValue(const std::string& value);
        };

        bool operator<(const EntityProperty& lhs, const EntityProperty& rhs);
        bool operator<=(const EntityProperty& lhs, const EntityProperty& rhs);
        bool operator>(const EntityProperty& lhs, const EntityProperty& rhs);
        bool operator>=(const EntityProperty& lhs, const EntityProperty& rhs);
        bool operator==(const EntityProperty& lhs, const EntityProperty& rhs);
        bool operator!=(const EntityProperty& lhs, const EntityProperty& rhs);
        std::ostream& operator<<(std::ostream& str, const EntityProperty& prop);

        bool isLayer(const std::string& classname, const std::vector<EntityProperty>& properties);
        bool isGroup(const std::string& classname, const std::vector<EntityProperty>& properties);
        bool isWorldspawn(const std::string& classname, const std::vector<EntityProperty>& properties);
        const std::string& findProperty(const std::vector<EntityProperty>& properties, const std::string& key, const std::string& defaultValue = EntityPropertyValues::DefaultValue);

        class EntityProperties {
        private:
            std::vector<EntityProperty> m_properties;
        public:
            EntityProperties();
            explicit EntityProperties(std::vector<EntityProperty> properties);

            std::vector<EntityProperty> releaseProperties();
            const std::vector<EntityProperty>& properties() const;
            void setProperties(const std::vector<EntityProperty>& properties);

            const EntityProperty& addOrUpdateProperty(const std::string& key, const std::string& value);
            void renameProperty(const std::string& key, const std::string& newKey);
            void removeProperty(const std::string& key);

            bool hasProperty(const std::string& key) const;
            bool hasProperty(const std::string& key, const std::string& value) const;
            bool hasPropertyWithPrefix(const std::string& prefix, const std::string& value) const;
            bool hasNumberedProperty(const std::string& prefix, const std::string& value) const;
        public:
            std::vector<std::string> keys() const;
            const std::string* properties(const std::string& key) const;

            std::vector<EntityProperty> propertiesWithKey(const std::string& key) const;
            std::vector<EntityProperty> propertiesWithPrefix(const std::string& prefix) const;
            std::vector<EntityProperty> numberedProperties(const std::string& prefix) const;
        private:
            std::vector<EntityProperty>::const_iterator findProperty(const std::string& key) const;
            std::vector<EntityProperty>::iterator findProperty(const std::string& key);
        };
    }
}

