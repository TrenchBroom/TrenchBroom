/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
                ++j;
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

        EntityProperty::EntityProperty(const PropertyKey& i_key, const PropertyValue& i_value) :
        key(i_key),
        value(i_value) {}
        
        bool EntityProperty::operator<(const EntityProperty& rhs) const {
            const int keyCmp = key.compare(rhs.key);
            if (keyCmp < 0)
                return true;
            if (keyCmp > 0)
                return false;
            return value.compare(rhs.value) < 0;
        }

        const EntityProperty::List& EntityProperties::properties() const {
            return m_properties;
        }
        
        void EntityProperties::setProperties(const EntityProperty::List& properties) {
            m_properties = properties;
        }

        const EntityProperty& EntityProperties::addOrUpdateProperty(const PropertyKey& key, const PropertyValue& value) {
            EntityProperty::List::iterator it = findProperty(key);
            if (it != m_properties.end()) {
                it->value = value;
                return *it;
            } else {
                m_properties.push_back(EntityProperty(key, value));
                return m_properties.back();
            }
        }

        void EntityProperties::renameProperty(const PropertyKey& key, const PropertyKey& newKey) {
            EntityProperty::List::iterator it = findProperty(key);
            if (it == m_properties.end())
                return;
            it->key = newKey;
        }

        void EntityProperties::removeProperty(const PropertyKey& key) {
            EntityProperty::List::iterator it = findProperty(key);
            if (it == m_properties.end())
                return;
            m_properties.erase(it);
        }

        bool EntityProperties::hasProperty(const PropertyKey& key) const {
            return findProperty(key) != m_properties.end();
        }

        const PropertyValue* EntityProperties::property(const PropertyKey& key) const {
            EntityProperty::List::const_iterator it = findProperty(key);
            if (it == m_properties.end())
                return NULL;
            return &it->value;
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
                if (isNumberedProperty(prefix, property.key))
                    result.push_back(property);
            }
            
            return result;
        }

        EntityProperty::List::const_iterator EntityProperties::findProperty(const PropertyKey& key) const {
            EntityProperty::List::const_iterator it, end;
            for (it = m_properties.begin(), end = m_properties.end(); it != end; ++it) {
                const EntityProperty& property = *it;
                if (property.key == key)
                    return it;
            }
            
            return end;
        }
        
        EntityProperty::List::iterator EntityProperties::findProperty(const PropertyKey& key) {
            EntityProperty::List::iterator it, end;
            for (it = m_properties.begin(), end = m_properties.end(); it != end; ++it) {
                const EntityProperty& property = *it;
                if (property.key == key)
                    return it;
            }
            
            return end;
        }
    }
}
