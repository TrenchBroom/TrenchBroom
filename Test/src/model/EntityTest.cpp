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

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace Model {
        TEST(EntityTest, GetProperty) {
            EntityPtr entity = Entity::newEntity();
            const PropertyKey key("key");
            const PropertyKey key2("asdf");
            const PropertyValue value("value");
            const PropertyValue defaultValue("default");
            entity->addOrUpdateProperty(key, value);
            
            ASSERT_EQ(value, entity->property(key, defaultValue));
            ASSERT_EQ(defaultValue, entity->property(key2, defaultValue));
        }
        
        TEST(EntityTest, AddProperty) {
            EntityPtr entity = Entity::newEntity();
            const PropertyKey key("key");
            const PropertyValue value("value");
            
            entity->addOrUpdateProperty(key, value);

            const EntityPropertyList& properties = entity->properties();
            ASSERT_EQ(1, properties.size());
            ASSERT_EQ(key, properties[0].key);
            ASSERT_EQ(value, properties[0].value);
        }
        
        TEST(EntityTest, UpdateProperty) {
            EntityPtr entity = Entity::newEntity();
            const PropertyKey key("key");
            const PropertyValue value("value");
            const PropertyValue newValue("value");
            entity->addOrUpdateProperty(key, value);
            
            entity->addOrUpdateProperty(key, newValue);
            
            const EntityPropertyList& properties = entity->properties();
            ASSERT_EQ(1, properties.size());
            ASSERT_EQ(key, properties[0].key);
            ASSERT_EQ(newValue, properties[0].value);
        }
        
        TEST(EntityTest, GetClassname) {
            EntityPtr entity = Entity::newEntity();
            const PropertyValue classname = "classname";
            const PropertyValue defaultClassname = "asdf";
            
            ASSERT_EQ(PropertyValues::NoClassname, entity->classname());
            ASSERT_EQ(defaultClassname, entity->classname(defaultClassname));
            
            entity->addOrUpdateProperty(PropertyKeys::Classname, classname);
            ASSERT_EQ(classname, entity->classname());
            ASSERT_EQ(classname, entity->classname(defaultClassname));
        }

        TEST(EntityTest, AddBrush) {
            EntityPtr entity = Entity::newEntity();
            BrushPtr brush = Brush::newBrush(EmptyBrushFaceList);
            
            entity->addBrush(brush);

            const BrushList& brushes = entity->brushes();
            ASSERT_EQ(1, brushes.size());
            ASSERT_EQ(brush, brushes[0]);
        }
        
        TEST(EntityTest, RemoveBrush) {
            EntityPtr entity = Entity::newEntity();
            BrushPtr brush = Brush::newBrush(EmptyBrushFaceList);
            entity->addBrush(brush);

            entity->removeBrush(brush);
            
            const BrushList& brushes = entity->brushes();
            ASSERT_TRUE(brushes.empty());
        }
    }
}
