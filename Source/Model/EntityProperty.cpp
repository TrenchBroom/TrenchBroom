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

#include "EntityProperty.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        bool PropertyStore::hasDuplicates() const {
            PropertyKeySet keys;
            PropertyList::const_iterator propIt, propEnd;
            for (propIt = m_properties.begin(), propEnd = m_properties.end(); propIt != propEnd; ++propIt) {
                const Property& property = *propIt;
                PropertyKeySetInsertResult result = keys.insert(property.key());
                if (!result.second)
                    return true;
            }
            return false;
        }

        bool PropertyStore::setPropertyKey(const PropertyKey& oldKey, const PropertyKey& newKey) {
            if (containsProperty(newKey))
                return false;
            
            PropertyList::iterator it, end;
            for (it = m_properties.begin(), end = m_properties.end(); it != end; ++it) {
                Property& property = *it;
                if (property.key() == oldKey) {
                    property.setKey(newKey);
                    assert(!hasDuplicates());
                    return true;
                }
            }
            
            return false;
        }

        void PropertyStore::setPropertyValue(const PropertyKey& key, const PropertyValue& value) {
            PropertyList::iterator it, end;
            for (it = m_properties.begin(), end = m_properties.end(); it != end; ++it) {
                Property& property = *it;
                if (property.key() == key) {
                    property.setValue(value);
                    return;
                }
            }
            
            m_properties.push_back(Property(key, value));
            assert(!hasDuplicates());
        }
        
        bool PropertyStore::removeProperty(const PropertyKey& key) {
            PropertyList::iterator it, end;
            for (it = m_properties.begin(), end = m_properties.end(); it != end; ++it) {
                Property& property = *it;
                if (property.key() == key) {
                    m_properties.erase(it);
                    return true;
                }
            }
            
            return false;
        }

        void PropertyStore::clear() {
            m_properties.clear();
        }
    }
}
