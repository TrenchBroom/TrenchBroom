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

#include <iosfwd>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        namespace AttributeNames {
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
            extern const std::string Message;
            extern const std::string ValveVersion;
            extern const std::string SoftMapBounds;
        }

        namespace AttributeValues {
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

        bool isNumberedAttribute(std::string_view prefix, std::string_view name);

        class EntityAttributeSnapshot;

        class EntityAttribute {
        private:
            std::string m_name;
            std::string m_value;
        public:
            EntityAttribute();
            EntityAttribute(const std::string& name, const std::string& value);
            
            int compare(const EntityAttribute& rhs) const;

            const std::string& name() const;
            const std::string& value() const;

            bool hasName(std::string_view name) const;
            bool hasValue(std::string_view value) const;
            bool hasNameAndValue(std::string_view name, std::string_view value) const;
            bool hasPrefix(std::string_view prefix) const;
            bool hasPrefixAndValue(std::string_view prefix, std::string_view value) const;
            bool hasNumberedPrefix(std::string_view prefix) const;
            bool hasNumberedPrefixAndValue(std::string_view prefix, std::string_view value) const;

            void setName(const std::string& name);
            void setValue(const std::string& value);
        };

        bool operator<(const EntityAttribute& lhs, const EntityAttribute& rhs);
        bool operator<=(const EntityAttribute& lhs, const EntityAttribute& rhs);
        bool operator>(const EntityAttribute& lhs, const EntityAttribute& rhs);
        bool operator>=(const EntityAttribute& lhs, const EntityAttribute& rhs);
        bool operator==(const EntityAttribute& lhs, const EntityAttribute& rhs);
        bool operator!=(const EntityAttribute& lhs, const EntityAttribute& rhs);
        std::ostream& operator<<(std::ostream& str, const EntityAttribute& attr);

        bool isLayer(const std::string& classname, const std::vector<EntityAttribute>& attributes);
        bool isGroup(const std::string& classname, const std::vector<EntityAttribute>& attributes);
        bool isWorldspawn(const std::string& classname, const std::vector<EntityAttribute>& attributes);
        const std::string& findAttribute(const std::vector<EntityAttribute>& attributes, const std::string& name, const std::string& defaultValue = AttributeValues::DefaultValue);

        class EntityAttributes {
        private:
            std::vector<EntityAttribute> m_attributes;
        public:
            EntityAttributes();
            explicit EntityAttributes(std::vector<EntityAttribute> attributes);

            std::vector<EntityAttribute> releaseAttributes();
            const std::vector<EntityAttribute>& attributes() const;
            void setAttributes(const std::vector<EntityAttribute>& attributes);

            const EntityAttribute& addOrUpdateAttribute(const std::string& name, const std::string& value);
            void renameAttribute(const std::string& name, const std::string& newName);
            void removeAttribute(const std::string& name);

            bool hasAttribute(const std::string& name) const;
            bool hasAttribute(const std::string& name, const std::string& value) const;
            bool hasAttributeWithPrefix(const std::string& prefix, const std::string& value) const;
            bool hasNumberedAttribute(const std::string& prefix, const std::string& value) const;

            EntityAttributeSnapshot snapshot(const std::string& name) const;
        public:
            std::vector<std::string> names() const;
            const std::string* attribute(const std::string& name) const;

            std::vector<EntityAttribute> attributeWithName(const std::string& name) const;
            std::vector<EntityAttribute> attributesWithPrefix(const std::string& prefix) const;
            std::vector<EntityAttribute> numberedAttributes(const std::string& prefix) const;
        private:
            std::vector<EntityAttribute>::const_iterator findAttribute(const std::string& name) const;
            std::vector<EntityAttribute>::iterator findAttribute(const std::string& name);
        };
    }
}

#endif /* defined(TrenchBroom_EntityProperties) */
