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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "Exceptions.h"
#include "Model/EntityProperties.h"

namespace TrenchBroom {
    namespace Model {
        TEST(EntityPropertiesTest, AddProperty) {
            EntityProperties entityProperties;
            const PropertyKey key("key");
            const PropertyValue value("value");
            entityProperties.addOrUpdateProperty(key, value);
            
            const EntityPropertyList& propertyList = entityProperties.properties();
            ASSERT_EQ(1, propertyList.size());
            ASSERT_EQ(key, propertyList[0].key);
            ASSERT_EQ(value, propertyList[0].value);
        }
        
        TEST(EntityPropertiesTest, UpdateProperty) {
            EntityProperties entityProperties;
            const PropertyKey key("key");
            const PropertyValue value("value");
            const PropertyValue newValue("value");
            entityProperties.addOrUpdateProperty(key, value);
            entityProperties.addOrUpdateProperty(key, newValue);
            
            const EntityPropertyList& propertyList = entityProperties.properties();
            ASSERT_EQ(1, propertyList.size());
            ASSERT_EQ(key, propertyList[0].key);
            ASSERT_EQ(newValue, propertyList[0].value);
        }
    
        TEST(EntityPropertiesTest, HasProperty) {
            EntityProperties entityProperties;
            const PropertyKey key("key");
            const PropertyValue value("value");
            entityProperties.addOrUpdateProperty(key, value);
            
            ASSERT_TRUE(entityProperties.hasProperty(key));
            ASSERT_FALSE(entityProperties.hasProperty("asfd"));
        }
        
        TEST(EntityPropertiesTest, GetExistingProperty) {
            EntityProperties entityProperties;
            const PropertyKey key("key");
            const PropertyValue value("value");
            entityProperties.addOrUpdateProperty(key, value);
            
            const PropertyValue* result = entityProperties.property(key);
            ASSERT_EQ(value, *result);
        }
        
        TEST(EntityPropertiesTest, GetNonExistingProperty) {
            EntityProperties entityProperties;
            ASSERT_EQ(NULL, entityProperties.property("key"));
            
            const PropertyKey key("key");
            const PropertyValue value("value");
            entityProperties.addOrUpdateProperty(key, value);
            
            ASSERT_EQ(NULL, entityProperties.property("key2"));
        }
    }
}
