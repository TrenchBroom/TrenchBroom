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

#include "EntityProperties.h"

#include "Exceptions.h"
#include "Assets/EntityDefinition.h"

namespace TrenchBroom {
    namespace Model {
        namespace PropertyKeys {
            const PropertyKey Classname         = "classname";
            const PropertyKey Origin            = "origin";
            const PropertyKey Wad               = "wad";
            const PropertyKey Wal               = "_textures";
            const PropertyKey Mods              = "_mod";
            const PropertyKey Spawnflags        = "spawnflags";
            const PropertyKey EntityDefinitions = "_def";
            const PropertyKey Angle             = "angle";
            const PropertyKey Angles            = "angles";
            const PropertyKey Mangle            = "mangle";
            const PropertyKey Target            = "target";
            const PropertyKey Targetname        = "targetname";
            const PropertyKey Killtarget        = "killtarget";
        }
        
        namespace PropertyValues {
            const PropertyValue WorldspawnClassname = "worldspawn";
            const PropertyValue NoClassname         = "undefined";
        }

        bool isPropertyKeyMutable(const PropertyKey& key) {
            if (key == PropertyKeys::Mods)
                return false;
            if (key == PropertyKeys::EntityDefinitions)
                return false;
            if (key == PropertyKeys::Wad)
                return false;
            if (key == PropertyKeys::Wal)
                return false;
            return true;
        }
        
        bool isPropertyValueMutable(const PropertyKey& key) {
            if (key == PropertyKeys::Classname)
                return false;
            if (key == PropertyKeys::Origin)
                return false;
            if (key == PropertyKeys::Mods)
                return false;
            if (key == PropertyKeys::EntityDefinitions)
                return false;
            if (key == PropertyKeys::Wad)
                return false;
            if (key == PropertyKeys::Wal)
                return false;
            return true;
        }

        String numberedPropertyPrefix(const String& key) {
            size_t i = 0;
            while (i < key.size() && key[i] < '0' && key[i] > '9')
                ++i;
            if (i == key.size())
                return "";
            for (size_t j = i; j < key.size(); ++j) {
                if (key[j] < '0' || key[j] > '9')
                    return "";
            }
            return key.substr(0, i);
        }

        bool isNumberedProperty(const String& prefix, const PropertyKey& key) {
            if (key.size() < prefix.size())
                return false;
            for (size_t i = 0; i < prefix.size(); ++i)
                if (key[i] != prefix[i])
                    return false;
            for (size_t i = prefix.size(); i < key.size(); ++i)
                if (key[i] < '0' || key[i] > '9')
                    return false;
            return true;
        }

        EntityProperty::EntityProperty() :
        m_definition(NULL) {}
        
        EntityProperty::EntityProperty(const PropertyKey& key, const PropertyValue& value, const Assets::PropertyDefinition* definition) :
        m_key(key),
        m_value(value),
        m_definition(definition) {}
        
        bool EntityProperty::operator<(const EntityProperty& rhs) const {
            const int keyCmp = m_key.compare(rhs.m_key);
            if (keyCmp < 0)
                return true;
            if (keyCmp > 0)
                return false;
            return m_value.compare(rhs.m_value) < 0;
        }

        const PropertyKey& EntityProperty::key() const {
            return m_key;
        }
        
        const PropertyValue& EntityProperty::value() const {
            return m_value;
        }
        
        const Assets::PropertyDefinition* EntityProperty::definition() const {
            return m_definition;
        }

        void EntityProperty::setKey(const PropertyKey& key, const Assets::PropertyDefinition* definition) {
            m_key = key;
            m_definition = definition;
        }
        
        void EntityProperty::setValue(const PropertyValue& value) {
            m_value = value;
        }

        const EntityProperty::List& EntityProperties::properties() const {
            return m_properties;
        }
        
        void EntityProperties::setProperties(const EntityProperty::List& properties) {
            m_properties = properties;
        }

        const EntityProperty& EntityProperties::addOrUpdateProperty(const PropertyKey& key, const PropertyValue& value, const Assets::PropertyDefinition* definition) {
            EntityProperty::List::iterator it = findProperty(key);
            if (it != m_properties.end()) {
                assert(it->definition() == definition);
                it->setValue(value);
                return *it;
            } else {
                m_properties.push_back(EntityProperty(key, value, definition));
                return m_properties.back();
            }
        }

        void EntityProperties::renameProperty(const PropertyKey& key, const PropertyKey& newKey, const Assets::PropertyDefinition* newDefinition) {
            EntityProperty::List::iterator it = findProperty(key);
            if (it == m_properties.end())
                return;
            it->setKey(newKey, newDefinition);
        }

        void EntityProperties::removeProperty(const PropertyKey& key) {
            EntityProperty::List::iterator it = findProperty(key);
            if (it == m_properties.end())
                return;
            m_properties.erase(it);
        }

        void EntityProperties::updateDefinitions(const Assets::EntityDefinition* entityDefinition) {
            EntityProperty::List::iterator it, end;
            for (it = m_properties.begin(), end = m_properties.end(); it != end; ++it) {
                EntityProperty& property = *it;
                const PropertyKey& key = property.key();
                const Assets::PropertyDefinition* propertyDefinition = Assets::EntityDefinition::safeGetPropertyDefinition(entityDefinition, key);
                property.setKey(key, propertyDefinition);
            }
        }

        bool EntityProperties::hasProperty(const PropertyKey& key) const {
            return findProperty(key) != m_properties.end();
        }

        const PropertyValue* EntityProperties::property(const PropertyKey& key) const {
            EntityProperty::List::const_iterator it = findProperty(key);
            if (it == m_properties.end())
                return NULL;
            return &it->value();
        }

        const PropertyValue EntityProperties::safeProperty(const PropertyKey& key, const PropertyValue& defaultValue) const {
            const PropertyValue* value = property(key);
            if (value == NULL)
                return defaultValue;
            return *value;
        }

        EntityProperty::List EntityProperties::numberedProperties(const String& prefix) const {
            EntityProperty::List result;
            
            EntityProperty::List::const_iterator it, end;
            for (it = m_properties.begin(), end = m_properties.end(); it != end; ++it) {
                const EntityProperty& property = *it;
                if (isNumberedProperty(prefix, property.key()))
                    result.push_back(property);
            }
            
            return result;
        }

        EntityProperty::List::const_iterator EntityProperties::findProperty(const PropertyKey& key) const {
            EntityProperty::List::const_iterator it, end;
            for (it = m_properties.begin(), end = m_properties.end(); it != end; ++it) {
                const EntityProperty& property = *it;
                if (property.key() == key)
                    return it;
            }
            
            return end;
        }
        
        EntityProperty::List::iterator EntityProperties::findProperty(const PropertyKey& key) {
            EntityProperty::List::iterator it, end;
            for (it = m_properties.begin(), end = m_properties.end(); it != end; ++it) {
                const EntityProperty& property = *it;
                if (property.key() == key)
                    return it;
            }
            
            return end;
        }
    }
}
