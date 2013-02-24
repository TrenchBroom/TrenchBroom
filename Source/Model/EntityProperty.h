/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__EntityProperty__
#define __TrenchBroom__EntityProperty__

#include "Utility/String.h"

#include <map>
#include <set>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        typedef String PropertyKey;
        typedef String PropertyValue;
        typedef std::vector<PropertyKey> PropertyKeyList;
        typedef std::set<PropertyKey> PropertyKeySet;
        typedef std::pair<PropertyKeySet::iterator, bool> PropertyKeySetInsertResult;
        typedef std::vector<PropertyValue> PropertyValueList;

        class Property {
        private:
            PropertyKey m_key;
            PropertyValue m_value;
        public:
            Property() {}
            
            Property(const PropertyKey& key, const PropertyValue& value) :
            m_key(key),
            m_value(value) {}
            
            inline const PropertyKey& key() const {
                return m_key;
            }
            
            inline void setKey(const PropertyKey& key) {
                m_key = key;
            }
            
            inline const PropertyValue& value() const {
                return m_value;
            }
            
            inline void setValue(const PropertyValue& value) {
                m_value = value;
            }
        };
        
        typedef std::vector<Property> PropertyList;
        static const PropertyList EmptyPropertyList;

        typedef std::map<PropertyKey, Property> PropertyMap;
        static const PropertyMap EmptyPropertyMap;
        
        class PropertyStore {
        private:
            PropertyList m_properties;
            
            bool hasDuplicates() const;
        public:
            inline bool containsProperty(const PropertyKey& key) const {
                PropertyList::const_iterator it, end;
                for (it = m_properties.begin(), end = m_properties.end(); it != end; ++it) {
                    const Property& property = *it;
                    if (property.key() == key)
                        return true;
                }
                
                return false;
            }

            inline const Property* property(const PropertyKey& key) const {
                PropertyList::const_iterator it, end;
                for (it = m_properties.begin(), end = m_properties.end(); it != end; ++it) {
                    const Property& property = *it;
                    if (property.key() == key)
                        return &property;
                }
                
                return NULL;
            }
            
            inline const PropertyValue* propertyValue(const PropertyKey& key) const {
                const Property* prop = property(key);
                if (prop == NULL)
                    return NULL;
                return &prop->value();
            }
            
            inline const PropertyList& properties() const {
                return m_properties;
            }
            
            bool setPropertyKey(const PropertyKey& oldKey, const PropertyKey& newKey);
            void setPropertyValue(const PropertyKey& key, const PropertyValue& value);
            bool removeProperty(const PropertyKey& key);
            void clear();
        };
    }
}


#endif /* defined(__TrenchBroom__EntityProperty__) */
