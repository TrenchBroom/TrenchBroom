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

#include <gtest/gtest.h>

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/BrushFace.h"
#include "Model/QuakeEntityRotator.h"

namespace TrenchBroom {
    namespace Model {
        typedef ConfigurableEntity<QuakeEntityRotationPolicy> QuakeEntity;
        
        TEST(EntityTest, hasProperty) {
            QuakeEntity entity;
            const PropertyKey key("key");
            const PropertyValue value("value");
            entity.addOrUpdateProperty(key, value);
            
            ASSERT_TRUE(entity.hasProperty("key"));
            ASSERT_FALSE(entity.hasProperty("asfd"));
        }
        
        TEST(EntityTest, getProperty) {
            QuakeEntity entity;
            const PropertyKey key("key");
            const PropertyKey key2("asdf");
            const PropertyValue value("value");
            const PropertyValue defaultValue("default");
            entity.addOrUpdateProperty(key, value);
            
            ASSERT_EQ(value, entity.property(key, defaultValue));
            ASSERT_EQ(defaultValue, entity.property(key2, defaultValue));
        }
        
        TEST(EntityTest, addProperty) {
            QuakeEntity entity;
            const PropertyKey key("key");
            const PropertyValue value("value");
            
            entity.addOrUpdateProperty(key, value);
            
            const EntityProperty::List& properties = entity.properties();
            ASSERT_EQ(1u, properties.size());
            ASSERT_EQ(key, properties[0].key);
            ASSERT_EQ(value, properties[0].value);
        }
        
        TEST(EntityTest, updateProperty) {
            QuakeEntity entity;
            const PropertyKey key("key");
            const PropertyValue value("value");
            const PropertyValue newValue("value");
            entity.addOrUpdateProperty(key, value);
            
            entity.addOrUpdateProperty(key, newValue);
            
            const EntityProperty::List& properties = entity.properties();
            ASSERT_EQ(1u, properties.size());
            ASSERT_EQ(key, properties[0].key);
            ASSERT_EQ(newValue, properties[0].value);
        }
        
        TEST(EntityTest, getClassname) {
            QuakeEntity entity;
            const PropertyValue classname = "classname";
            const PropertyValue defaultClassname = "asdf";
            
            ASSERT_EQ(PropertyValues::NoClassname, entity.classname());
            ASSERT_EQ(defaultClassname, entity.classname(defaultClassname));
            
            entity.addOrUpdateProperty(PropertyKeys::Classname, classname);
            ASSERT_EQ(classname, entity.classname());
            ASSERT_EQ(classname, entity.classname(defaultClassname));
        }
        
        TEST(EntityTest, addBrush) {
            const BBox3 worldBounds(Vec3(-4096.0, -4096.0, -4096.0),
                                    Vec3( 4096.0,  4096.0,  4096.0));
            QuakeEntity entity;
            Brush* brush = new Brush(worldBounds, EmptyBrushFaceList);
            
            entity.addBrush(brush);
            
            const BrushList& brushes = entity.brushes();
            ASSERT_EQ(1u, brushes.size());
            ASSERT_EQ(brush, brushes[0]);
        }
        
        TEST(EntityTest, removeBrush) {
            const BBox3 worldBounds(Vec3(-4096.0, -4096.0, -4096.0),
                                    Vec3( 4096.0,  4096.0,  4096.0));
            QuakeEntity entity;
            Brush* brush = new Brush(worldBounds, EmptyBrushFaceList);
            entity.addBrush(brush);
            
            entity.removeBrush(brush);
            
            const BrushList& brushes = entity.brushes();
            ASSERT_TRUE(brushes.empty());
        }
        
        TEST(EntityTest, partialSelectionAfterAdd) {
            const BBox3 worldBounds(Vec3(-4096.0, -4096.0, -4096.0),
                                    Vec3( 4096.0,  4096.0,  4096.0));
            QuakeEntity entity;
            Brush* brush1 = new Brush(worldBounds, EmptyBrushFaceList);
            Brush* brush2 = new Brush(worldBounds, EmptyBrushFaceList);
            entity.addBrush(brush1);
            entity.addBrush(brush2);
            ASSERT_FALSE(entity.partiallySelected());
            brush1->select();
            ASSERT_TRUE(entity.partiallySelected());
            brush2->select();
            ASSERT_TRUE(entity.partiallySelected());
            brush1->deselect();
            ASSERT_TRUE(entity.partiallySelected());
            brush2->deselect();
            ASSERT_FALSE(entity.partiallySelected());
        }
        
        TEST(EntityTest, partialSelectionBeforeAdd) {
            const BBox3 worldBounds(Vec3(-4096.0, -4096.0, -4096.0),
                                    Vec3( 4096.0,  4096.0,  4096.0));
            QuakeEntity entity;
            Brush* brush1 = new Brush(worldBounds, EmptyBrushFaceList);
            Brush* brush2 = new Brush(worldBounds, EmptyBrushFaceList);
            brush1->select();
            entity.addBrush(brush1);
            entity.addBrush(brush2);
            ASSERT_TRUE(entity.partiallySelected());
            brush2->select();
            ASSERT_TRUE(entity.partiallySelected());
            brush2->deselect();
            ASSERT_TRUE(entity.partiallySelected());
            entity.removeBrush(brush2);
            ASSERT_TRUE(entity.partiallySelected());
            entity.removeBrush(brush1);
            ASSERT_FALSE(entity.partiallySelected());
        }
    }
}
