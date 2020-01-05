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

#include "EntityAttributes.h"

#include "Assets/EntityDefinition.h"
#include "Model/EntityAttributeSnapshot.h"

#include <kdl/string_compare.h>
#include <kdl/vector_set.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        namespace AttributeNames {
            const std::string Classname         = "classname";
            const std::string Origin            = "origin";
            const std::string Wad               = "wad";
            const std::string Textures          = "_tb_textures";
            const std::string Mods              = "_tb_mod";
            const std::string Spawnflags        = "spawnflags";
            const std::string EntityDefinitions = "_tb_def";
            const std::string Angle             = "angle";
            const std::string Angles            = "angles";
            const std::string Mangle            = "mangle";
            const std::string Target            = "target";
            const std::string Targetname        = "targetname";
            const std::string Killtarget        = "killtarget";
            const std::string GroupType         = "_tb_type";
            const std::string LayerId           = "_tb_id";
            const std::string LayerName         = "_tb_name";
            const std::string Layer             = "_tb_layer";
            const std::string GroupId           = "_tb_id";
            const std::string GroupName         = "_tb_name";
            const std::string Group             = "_tb_group";
            const std::string Message           = "_tb_message";
            const std::string ValveVersion      = "mapversion";
        }

        namespace AttributeValues {
            const std::string WorldspawnClassname = "worldspawn";
            const std::string NoClassname         = "undefined";
            const std::string LayerClassname      = "func_group";
            const std::string GroupClassname      = "func_group";
            const std::string GroupTypeLayer      = "_tb_layer";
            const std::string GroupTypeGroup      = "_tb_group";
        }

        bool isNumberedAttribute(const std::string_view prefix, const std::string_view name) {
            // %* matches 0 or more digits
            const std::string pattern = std::string(prefix) + "%*";
            return kdl::cs::str_matches_glob(name, pattern);
        }

        EntityAttribute::EntityAttribute() :
        m_definition(nullptr) {}

        EntityAttribute::EntityAttribute(const std::string& name, const std::string& value, const Assets::AttributeDefinition* definition) :
        m_name(name),
        m_value(value),
        m_definition(definition) {}

        bool EntityAttribute::operator<(const EntityAttribute& rhs) const {
            return compare(rhs) < 0;
        }

        int EntityAttribute::compare(const EntityAttribute& rhs) const {
            const int nameCmp = m_name.compare(rhs.m_name);
            if (nameCmp != 0)
                return nameCmp;
            return m_value.compare(rhs.m_value);
        }

        const std::string& EntityAttribute::name() const {
            return m_name;
        }

        const std::string& EntityAttribute::value() const {
            return m_value;
        }

        const Assets::AttributeDefinition* EntityAttribute::definition() const {
            return m_definition;
        }

        bool EntityAttribute::hasName(const std::string_view name) const {
            return kdl::cs::str_is_equal(m_name, name);
        }

        bool EntityAttribute::hasValue(const std::string_view value) const {
            return kdl::cs::str_is_equal(m_value, value);
        }

        bool EntityAttribute::hasNameAndValue(const std::string_view name, const std::string_view value) const {
            return hasName(name) && hasValue(value);
        }

        bool EntityAttribute::hasPrefix(const std::string_view prefix) const {
            return kdl::cs::str_is_prefix(m_name, prefix);
        }

        bool EntityAttribute::hasPrefixAndValue(const std::string_view prefix, const std::string_view value) const {
            return hasPrefix(prefix) && hasValue(value);
        }

        bool EntityAttribute::hasNumberedPrefix(const std::string_view prefix) const {
            return isNumberedAttribute(prefix, m_name);
        }

        bool EntityAttribute::hasNumberedPrefixAndValue(const std::string_view prefix, const std::string_view value) const {
            return hasNumberedPrefix(prefix) && hasValue(value);
        }

        void EntityAttribute::setName(const std::string& name, const Assets::AttributeDefinition* definition) {
            m_name = name;
            m_definition = definition;
        }

        void EntityAttribute::setValue(const std::string& value) {
            m_value = value;
        }

        bool isLayer(const std::string& classname, const std::vector<EntityAttribute>& attributes) {
            if (classname != AttributeValues::LayerClassname) {
                return false;
            } else {
                const std::string& groupType = findAttribute(attributes, AttributeNames::GroupType);
                return groupType == AttributeValues::GroupTypeLayer;
            }
        }

        bool isGroup(const std::string& classname, const std::vector<EntityAttribute>& attributes) {
            if (classname != AttributeValues::GroupClassname) {
                return false;
            } else {
                const std::string& groupType = findAttribute(attributes, AttributeNames::GroupType);
                return groupType == AttributeValues::GroupTypeGroup;
            }
        }

        bool isWorldspawn(const std::string& classname, const std::vector<EntityAttribute>& /* attributes */) {
            return classname == AttributeValues::WorldspawnClassname;
        }

        const std::string& findAttribute(const std::vector<EntityAttribute>& attributes, const std::string& name, const std::string& defaultValue) {
            for (const EntityAttribute& attribute : attributes) {
                if (name == attribute.name()) {
                    return attribute.value();
                }
            }
            return defaultValue;
        }

        // EntityAttributes
        const std::vector<EntityAttribute>& EntityAttributes::attributes() const {
            return m_attributes;
        }

        void EntityAttributes::setAttributes(const std::vector<EntityAttribute>& attributes) {
            m_attributes.clear();

            // ensure that there are no duplicate names
            kdl::vector_set<std::string> names(attributes.size());
            for (const auto& attribute : attributes) {
                if (names.insert(attribute.name()).second) {
                    m_attributes.push_back(attribute);
                }
            }
        }

        const EntityAttribute& EntityAttributes::addOrUpdateAttribute(const std::string& name, const std::string& value, const Assets::AttributeDefinition* definition) {
            auto it = findAttribute(name);
            if (it != std::end(m_attributes)) {
                assert(it->definition() == definition);
                it->setValue(value);
                return *it;
            } else {
                m_attributes.push_back(EntityAttribute(name, value, definition));
                return m_attributes.back();
            }
        }

        void EntityAttributes::renameAttribute(const std::string& name, const std::string& newName, const Assets::AttributeDefinition* newDefinition) {
            if (!hasAttribute(name)) {
                return;
            }

            const std::string value = *attribute(name);
            removeAttribute(name);
            addOrUpdateAttribute(newName, value, newDefinition);
        }

        void EntityAttributes::removeAttribute(const std::string& name) {
            auto it = findAttribute(name);
            if (it != std::end(m_attributes)) {
                m_attributes.erase(it);
            }
        }

        void EntityAttributes::updateDefinitions(const Assets::EntityDefinition* entityDefinition) {
            for (EntityAttribute& attribute : m_attributes) {
                const std::string& name = attribute.name();
                const Assets::AttributeDefinition* attributeDefinition = Assets::EntityDefinition::safeGetAttributeDefinition(entityDefinition, name);
                attribute.setName(name, attributeDefinition);
            }
        }

        bool EntityAttributes::hasAttribute(const std::string& name) const {
            return findAttribute(name) != std::end(m_attributes);
        }

        bool EntityAttributes::hasAttribute(const std::string& name, const std::string& value) const {
            for (const auto& attribute : m_attributes) {
                if (attribute.hasNameAndValue(name, value)) {
                    return true;
                }
            }
            return false;
        }

        bool EntityAttributes::hasAttributeWithPrefix(const std::string& prefix, const std::string& value) const {
            for (const auto& attribute : m_attributes) {
                if (attribute.hasPrefixAndValue(prefix, value)) {
                    return true;
                }
            }
            return false;
        }

        bool EntityAttributes::hasNumberedAttribute(const std::string& prefix, const std::string& value) const {
            for (const auto& attribute : m_attributes) {
                if (attribute.hasNumberedPrefixAndValue(prefix, value)) {
                    return true;
                }
            }
            return false;
        }

        EntityAttributeSnapshot EntityAttributes::snapshot(const std::string& name) const {
            for (const auto& attribute : m_attributes) {
                if (attribute.hasName(name)) {
                    return EntityAttributeSnapshot(attribute.name(), attribute.value());
                }
            }
            return EntityAttributeSnapshot(name);
        }

        std::vector<std::string> EntityAttributes::names() const {
            std::vector<std::string> result;
            result.reserve(m_attributes.size());

            for (const EntityAttribute& attribute : m_attributes) {
                result.push_back(attribute.name());
            }
            return result;
        }

        const std::string* EntityAttributes::attribute(const std::string& name) const {
            auto it = findAttribute(name);
            if (it == std::end(m_attributes)) {
                return nullptr;
            } else {
                return &it->value();
            }
        }

        std::vector<EntityAttribute> EntityAttributes::attributeWithName(const std::string& name) const {
            std::vector<EntityAttribute> result;
            for (const auto& attribute : m_attributes) {
                if (attribute.hasName(name)) {
                    result.push_back(attribute);
                }
            }
            return result;
        }

        std::vector<EntityAttribute> EntityAttributes::attributesWithPrefix(const std::string& prefix) const {
            std::vector<EntityAttribute> result;
            for (const auto& attribute : m_attributes) {
                if (attribute.hasPrefix(prefix)) {
                    result.push_back(attribute);
                }
            }
            return result;
        }

        std::vector<EntityAttribute> EntityAttributes::numberedAttributes(const std::string& prefix) const {
            std::vector<EntityAttribute> result;
            for (const auto& attribute : m_attributes) {
                if (attribute.hasNumberedPrefix(prefix)) {
                    result.push_back(attribute);
                }
            }
            return result;
        }

        std::vector<EntityAttribute>::const_iterator EntityAttributes::findAttribute(const std::string& name) const {
            for (auto it = std::begin(m_attributes), end = std::end(m_attributes); it != end; ++it) {
                if (it->hasName(name)) {
                    return it;
                }
            }
            return std::end(m_attributes);
        }

        std::vector<EntityAttribute>::iterator EntityAttributes::findAttribute(const std::string& name) {
            for (auto it = std::begin(m_attributes), end = std::end(m_attributes); it != end; ++it) {
                if (it->hasName(name)) {
                    return it;
                }
            }
            return std::end(m_attributes);
        }
    }
}
