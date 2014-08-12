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

#ifndef __TrenchBroom__EntityProperties__
#define __TrenchBroom__EntityProperties__

#include "StringUtils.h"
#include "Model/ModelTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinition;
        class PropertyDefinition;
    }
    
    namespace Model {
        namespace PropertyKeys {
            extern const PropertyKey Classname;
            extern const PropertyKey Origin;
            extern const PropertyKey Wad;
            extern const PropertyKey Wal;
            extern const PropertyKey Mods;
            extern const PropertyKey Spawnflags;
            extern const PropertyKey EntityDefinitions;
            extern const PropertyKey Angle;
            extern const PropertyKey Angles;
            extern const PropertyKey Mangle;
            extern const PropertyKey Target;
            extern const PropertyKey Targetname;
            extern const PropertyKey Killtarget;
        }
        
        namespace PropertyValues {
            extern const PropertyValue WorldspawnClassname;
            extern const PropertyValue NoClassname;
        }

        bool isPropertyKeyMutable(const PropertyKey& key);
        bool isPropertyValueMutable(const PropertyKey& key);
        String numberedPropertyPrefix(const String& key);
        bool isNumberedProperty(const String& prefix, const PropertyKey& key);
        
        class EntityProperty {
        private:
            PropertyKey m_key;
            PropertyValue m_value;
            const Assets::PropertyDefinition* m_definition;
        public:
            typedef std::vector<EntityProperty> List;
            EntityProperty();
            EntityProperty(const PropertyKey& key, const PropertyValue& value, const Assets::PropertyDefinition* definition);
            bool operator<(const EntityProperty& rhs) const;
            int compare(const EntityProperty& rhs) const;
            
            const PropertyKey& key() const;
            const PropertyValue& value() const;
            const Assets::PropertyDefinition* definition() const;
            
            void setKey(const PropertyKey& key, const Assets::PropertyDefinition* definition);
            void setValue(const PropertyValue& value);
        };

        class EntityProperties {
        private:
            EntityProperty::List m_properties;
        public:
            const EntityProperty::List& properties() const;
            
            void setProperties(const EntityProperty::List& properties);

            template <typename T>
            const EntityProperty& addOrUpdateProperty(const PropertyKey& key, const T& value, const Assets::PropertyDefinition* definition) {
                StringStream str;
                str << value;
                return addOrUpdateProperty(key, str.str(), definition);
            }
            
            const EntityProperty& addOrUpdateProperty(const PropertyKey& key, const PropertyValue& value, const Assets::PropertyDefinition* definition);
            void renameProperty(const PropertyKey& key, const PropertyKey& newKey, const Assets::PropertyDefinition* newDefinition);
            void removeProperty(const PropertyKey& key);
            void updateDefinitions(const Assets::EntityDefinition* entityDefinition);
            
            bool hasProperty(const PropertyKey& key) const;
            const PropertyValue* property(const PropertyKey& key) const;
            const PropertyValue safeProperty(const PropertyKey& key, const PropertyValue& defaultValue) const;
            
            EntityProperty::List numberedProperties(const String& prefix) const;
        private:
            EntityProperty::List::const_iterator findProperty(const PropertyKey& key) const;
            EntityProperty::List::iterator findProperty(const PropertyKey& key);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityProperties__) */
