/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
        namespace AttributeNames {
            const AttributeName Classname         = "classname";
            const AttributeName Origin            = "origin";
            const AttributeName Wad               = "wad";
            const AttributeName Wal               = "_tb_wals";
            const AttributeName Mods              = "_tb_mod";
            const AttributeName Spawnflags        = "spawnflags";
            const AttributeName EntityDefinitions = "_tb_def";
            const AttributeName Angle             = "angle";
            const AttributeName Angles            = "angles";
            const AttributeName Mangle            = "mangle";
            const AttributeName Target            = "target";
            const AttributeName Targetname        = "targetname";
            const AttributeName Killtarget        = "killtarget";
            const AttributeName GroupType         = "_tb_type";
            const AttributeName LayerName         = "_tb_name";
            const AttributeName Layer             = "_tb_layer";
            const AttributeName GroupName         = "_tb_name";
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
        m_definition(NULL) {}
        
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

        const EntityAttribute::List& EntityAttributes::attributes() const {
            return m_attributes;
        }
        
        void EntityAttributes::setAttributes(const EntityAttribute::List& attributes) {
            m_attributes = attributes;
        }

        const EntityAttribute& EntityAttributes::addOrUpdateAttribute(const AttributeName& name, const AttributeValue& value, const Assets::AttributeDefinition* definition) {
            EntityAttribute::List::iterator it = findAttribute(name);
            if (it != m_attributes.end()) {
                assert(it->definition() == definition);
                it->setValue(value);
                return *it;
            } else {
                m_attributes.push_back(EntityAttribute(name, value, definition));
                return m_attributes.back();
            }
        }

        void EntityAttributes::renameAttribute(const AttributeName& name, const AttributeName& newName, const Assets::AttributeDefinition* newDefinition) {
            EntityAttribute::List::iterator it = findAttribute(name);
            if (it == m_attributes.end())
                return;
            it->setName(newName, newDefinition);
        }

        void EntityAttributes::removeAttribute(const AttributeName& name) {
            EntityAttribute::List::iterator it = findAttribute(name);
            if (it == m_attributes.end())
                return;
            m_attributes.erase(it);
        }

        void EntityAttributes::updateDefinitions(const Assets::EntityDefinition* entityDefinition) {
            EntityAttribute::List::iterator it, end;
            for (it = m_attributes.begin(), end = m_attributes.end(); it != end; ++it) {
                EntityAttribute& attribute = *it;
                const AttributeName& name = attribute.name();
                const Assets::AttributeDefinition* attributeDefinition = Assets::EntityDefinition::safeGetAttributeDefinition(entityDefinition, name);
                attribute.setName(name, attributeDefinition);
            }
        }

        bool EntityAttributes::hasAttribute(const AttributeName& name) const {
            return findAttribute(name) != m_attributes.end();
        }

        const AttributeValue* EntityAttributes::attribute(const AttributeName& name) const {
            EntityAttribute::List::const_iterator it = findAttribute(name);
            if (it == m_attributes.end())
                return NULL;
            return &it->value();
        }

        const AttributeValue& EntityAttributes::safeAttribute(const AttributeName& name, const AttributeValue& defaultValue) const {
            const AttributeValue* value = attribute(name);
            if (value == NULL)
                return defaultValue;
            return *value;
        }

        EntityAttribute::List EntityAttributes::numberedAttributes(const String& prefix) const {
            EntityAttribute::List result;
            
            EntityAttribute::List::const_iterator it, end;
            for (it = m_attributes.begin(), end = m_attributes.end(); it != end; ++it) {
                const EntityAttribute& attribute = *it;
                if (isNumberedAttribute(prefix, attribute.name()))
                    result.push_back(attribute);
            }
            
            return result;
        }

        EntityAttribute::List::const_iterator EntityAttributes::findAttribute(const AttributeName& name) const {
            EntityAttribute::List::const_iterator it, end;
            for (it = m_attributes.begin(), end = m_attributes.end(); it != end; ++it) {
                const EntityAttribute& attribute = *it;
                if (attribute.name() == name)
                    return it;
            }
            
            return end;
        }
        
        EntityAttribute::List::iterator EntityAttributes::findAttribute(const AttributeName& name) {
            EntityAttribute::List::iterator it, end;
            for (it = m_attributes.begin(), end = m_attributes.end(); it != end; ++it) {
                const EntityAttribute& attribute = *it;
                if (attribute.name() == name)
                    return it;
            }
            
            return end;
        }
    }
}
