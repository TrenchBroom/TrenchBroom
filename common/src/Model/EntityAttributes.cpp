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

#include "Exceptions.h"
#include "Assets/EntityDefinition.h"

namespace TrenchBroom {
    namespace Model {
        const String AttributeEscapeChars = "\"\n\\";
        
        namespace AttributeNames {
            const AttributeName Classname         = "classname";
            const AttributeName Origin            = "origin";
            const AttributeName Wad               = "wad";
            const AttributeName Textures          = "_tb_textures";
            const AttributeName Mods              = "_tb_mod";
            const AttributeName GameEngineParameterSpecs = "_tb_engines";
            const AttributeName Spawnflags        = "spawnflags";
            const AttributeName EntityDefinitions = "_tb_def";
            const AttributeName Angle             = "angle";
            const AttributeName Angles            = "angles";
            const AttributeName Mangle            = "mangle";
            const AttributeName Target            = "target";
            const AttributeName Targetname        = "targetname";
            const AttributeName Killtarget        = "killtarget";
            const AttributeName GroupType         = "_tb_type";
            const AttributeName LayerId           = "_tb_id";
            const AttributeName LayerName         = "_tb_name";
            const AttributeName Layer             = "_tb_layer";
            const AttributeName GroupId           = "_tb_id";
            const AttributeName GroupName         = "_tb_name";
            const AttributeName Group             = "_tb_group";
            const AttributeName Message           = "_tb_message";
            const AttributeName ValveVersion      = "mapversion";
            const AttributeName ValveMaxRange     = "MaxRange";
        }
        
        namespace AttributeValues {
            const AttributeValue WorldspawnClassname = "worldspawn";
            const AttributeValue NoClassname         = "undefined";
            const AttributeValue LayerClassname      = "func_group";
            const AttributeValue GroupClassname      = "func_group";
            const AttributeValue GroupTypeLayer      = "_tb_layer";
            const AttributeValue GroupTypeGroup      = "_tb_group";
        }

        String numberedAttributePrefix(const String& name) {
            size_t i = 0;
            while (i < name.size() && name[i] < '0' && name[i] > '9')
                ++i;
            if (i == name.size())
                return "";
            for (size_t j = i; j < name.size(); ++j) {
                if (name[j] < '0' || name[j] > '9')
                    return "";
            }
            return name.substr(0, i);
        }

        bool isNumberedAttribute(const String& prefix, const AttributeName& name) {
            if (name.size() < prefix.size())
                return false;
            for (size_t i = 0; i < prefix.size(); ++i)
                if (name[i] != prefix[i])
                    return false;
            for (size_t i = prefix.size(); i < name.size(); ++i)
                if (name[i] < '0' || name[i] > '9')
                    return false;
            return true;
        }

        const EntityAttribute::List EntityAttribute::EmptyList(0);
        
        EntityAttribute::EntityAttribute() :
        m_definition(nullptr) {}
        
        EntityAttribute::EntityAttribute(const AttributeName& name, const AttributeValue& value, const Assets::AttributeDefinition* definition) :
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

        const AttributeName& EntityAttribute::name() const {
            return m_name;
        }
        
        const AttributeValue& EntityAttribute::value() const {
            return m_value;
        }
        
        const Assets::AttributeDefinition* EntityAttribute::definition() const {
            return m_definition;
        }

        void EntityAttribute::setName(const AttributeName& name, const Assets::AttributeDefinition* definition) {
            m_name = name;
            m_definition = definition;
        }
        
        void EntityAttribute::setValue(const AttributeValue& value) {
            m_value = value;
        }

        bool isLayer(const String& classname, const EntityAttribute::List& attributes) {
            if (classname != AttributeValues::LayerClassname)
                return false;
            const AttributeValue& groupType = findAttribute(attributes, AttributeNames::GroupType);
            return groupType == AttributeValues::GroupTypeLayer;
        }
        
        bool isGroup(const String& classname, const EntityAttribute::List& attributes) {
            if (classname != AttributeValues::GroupClassname)
                return false;
            const AttributeValue& groupType = findAttribute(attributes, AttributeNames::GroupType);
            return groupType == AttributeValues::GroupTypeGroup;
        }
        
        bool isWorldspawn(const String& classname, const EntityAttribute::List& attributes) {
            return classname == AttributeValues::WorldspawnClassname;
        }
        
        const AttributeValue& findAttribute(const EntityAttribute::List& attributes, const AttributeName& name, const AttributeValue& defaultValue) {
            for (const EntityAttribute& attribute : attributes) {
                if (name == attribute.name())
                    return attribute.value();
            }
            return defaultValue;
        }

        const EntityAttribute::List& EntityAttributes::attributes() const {
            return m_attributes;
        }
        
        void EntityAttributes::setAttributes(const EntityAttribute::List& attributes) {
            m_attributes = attributes;
            rebuildIndex();
        }

        const EntityAttribute& EntityAttributes::addOrUpdateAttribute(const AttributeName& name, const AttributeValue& value, const Assets::AttributeDefinition* definition) {
            EntityAttribute::List::iterator it = findAttribute(name);
            if (it != std::end(m_attributes)) {
                assert(it->definition() == definition);
                it->setValue(value);
                return *it;
            } else {
                m_attributes.push_back(EntityAttribute(name, value, definition));
                m_index.insert(name, --std::end(m_attributes));
                return m_attributes.back();
            }
        }

        void EntityAttributes::renameAttribute(const AttributeName& name, const AttributeName& newName, const Assets::AttributeDefinition* newDefinition) {
            if (!hasAttribute(name))
                return;
            
            const AttributeValue value = *attribute(name);
            removeAttribute(name);
            addOrUpdateAttribute(newName, value, newDefinition);
        }

        void EntityAttributes::removeAttribute(const AttributeName& name) {
            EntityAttribute::List::iterator it = findAttribute(name);
            if (it == std::end(m_attributes))
                return;
            m_index.remove(name, it);
            m_attributes.erase(it);
        }

        void EntityAttributes::updateDefinitions(const Assets::EntityDefinition* entityDefinition) {
            for (EntityAttribute& attribute : m_attributes) {
                const AttributeName& name = attribute.name();
                const Assets::AttributeDefinition* attributeDefinition = Assets::EntityDefinition::safeGetAttributeDefinition(entityDefinition, name);
                attribute.setName(name, attributeDefinition);
            }
        }

        bool EntityAttributes::hasAttribute(const AttributeName& name) const {
            return findAttribute(name) != std::end(m_attributes);
        }

        bool EntityAttributes::hasAttribute(const AttributeName& name, const AttributeValue& value) const {
            const EntityAttribute::List::const_iterator it = findAttribute(name);
            if (it == std::end(m_attributes))
                return false;
            return it->value() == value;
        }
        
        bool EntityAttributes::hasAttributeWithPrefix(const AttributeName& prefix, const AttributeValue& value) const {
            return containsValue(m_index.queryPrefixMatches(prefix), value);
        }
        
        bool EntityAttributes::hasNumberedAttribute(const AttributeName& prefix, const AttributeValue& value) const {
            return containsValue(m_index.queryNumberedMatches(prefix), value);
        }

        EntityAttributeSnapshot EntityAttributes::snapshot(const AttributeName& name) const {
            const AttributeIndex::QueryResult matches = m_index.queryExactMatches(name);
            if (matches.empty())
                return EntityAttributeSnapshot(name);
            
            assert(matches.size() == 1);
            return EntityAttributeSnapshot(name, matches.front()->value());
        }

        bool EntityAttributes::containsValue(const AttributeIndex::QueryResult& matches, const AttributeValue& value) const {
            if (matches.empty())
                return false;
            
            for (auto attrIt : matches) {
                const EntityAttribute& attribute = *attrIt;
                if (attribute.value() == value)
                    return true;
            }
            
            return false;
        }
        
        EntityAttribute::List EntityAttributes::listFromQueryResult(const AttributeIndex::QueryResult& matches) const {
            EntityAttribute::List result;
            
            for (auto attrIt : matches) {
                const EntityAttribute& attribute = *attrIt;
                result.push_back(attribute);
            }
            
            return result;
        }

        const AttributeNameSet EntityAttributes::names() const {
            AttributeNameSet result;
            for (const EntityAttribute& attribute : m_attributes)
                result.insert(attribute.name());
            return result;
        }

        const AttributeValue* EntityAttributes::attribute(const AttributeName& name) const {
            EntityAttribute::List::const_iterator it = findAttribute(name);
            if (it == std::end(m_attributes))
                return nullptr;
            return &it->value();
        }

        const AttributeValue& EntityAttributes::safeAttribute(const AttributeName& name, const AttributeValue& defaultValue) const {
            const AttributeValue* value = attribute(name);
            if (value == nullptr)
                return defaultValue;
            return *value;
        }

        EntityAttribute::List EntityAttributes::attributeWithName(const AttributeName& name) const {
            return listFromQueryResult(m_index.queryExactMatches(name));
        }
        
        EntityAttribute::List EntityAttributes::attributesWithPrefix(const AttributeName& prefix) const{
            return listFromQueryResult(m_index.queryPrefixMatches(prefix));
        }
        
        EntityAttribute::List EntityAttributes::numberedAttributes(const String& prefix) const {
            EntityAttribute::List result;

            for (const EntityAttribute& attribute : m_attributes) {
                if (isNumberedAttribute(prefix, attribute.name()))
                    result.push_back(attribute);
            }
            
            return result;
        }

        EntityAttribute::List::const_iterator EntityAttributes::findAttribute(const AttributeName& name) const {
            const AttributeIndex::QueryResult matches = m_index.queryExactMatches(name);
            if (matches.empty())
                return std::end(m_attributes);
            
            assert(matches.size() == 1);
            return matches.front();
        }
        
        EntityAttribute::List::iterator EntityAttributes::findAttribute(const AttributeName& name) {
            const AttributeIndex::QueryResult matches = m_index.queryExactMatches(name);
            if (matches.empty())
                return std::end(m_attributes);
            
            assert(matches.size() == 1);
            return matches.front();
        }

        void EntityAttributes::rebuildIndex() {
            m_index.clear();
            
            for (auto it = std::begin(m_attributes), end = std::end(m_attributes); it != end; ++it) {
                const EntityAttribute& attribute = *it;
                m_index.insert(attribute.name(), it);
            }
        }
    }
}
