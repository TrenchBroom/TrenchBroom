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

#include "IO/Path.h"
#include "Model/EntityProperties.h"
#include "Model/ModelTypes.h"
#include "Model/ModelDefinition.h"

namespace TrenchBroom {
    namespace Model {
        TEST(ModelDefinitionTest, testStaticModelDefinition) {
            const IO::Path path("maps/shell.bsp");
            const size_t skinIndex = 1;
            const size_t frameIndex = 2;
            ModelDefinitionPtr definition = ModelDefinitionPtr(new StaticModelDefinition(path, skinIndex, frameIndex));
            
            EntityProperties properties;
            ASSERT_TRUE(definition->matches(properties));
            
            const ModelSpecification spec = definition->modelSpecification(properties);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(skinIndex, spec.skinIndex);
            ASSERT_EQ(frameIndex, spec.frameIndex);
        }

        TEST(ModelDefinitionTest, testStaticModelDefinitionWithProperty) {
            const IO::Path path("maps/shell.bsp");
            const size_t skinIndex = 1;
            const size_t frameIndex = 2;
            const PropertyKey key = "testKey";
            const PropertyValue value = "testValue";
            ModelDefinitionPtr definition = ModelDefinitionPtr(new StaticModelDefinition(path, skinIndex, frameIndex, key, value));
            
            EntityProperties properties;
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(key, "blah");
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(key, value);
            ASSERT_TRUE(definition->matches(properties));
            
            const ModelSpecification spec = definition->modelSpecification(properties);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(skinIndex, spec.skinIndex);
            ASSERT_EQ(frameIndex, spec.frameIndex);
        }
        
        TEST(ModelDefinitionTest, testStaticModelDefinitionWithFlag) {
            const IO::Path path("maps/shell.bsp");
            const size_t skinIndex = 1;
            const size_t frameIndex = 2;
            const PropertyKey key = "testKey";
            const int value = 4;
            ModelDefinitionPtr definition = ModelDefinitionPtr(new StaticModelDefinition(path, skinIndex, frameIndex, key, value));
            
            EntityProperties properties;
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(key, "blah");
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(key, "1");
            ASSERT_FALSE(definition->matches(properties));

            properties.addOrUpdateProperty(key, "4");
            ASSERT_TRUE(definition->matches(properties));
            
            properties.addOrUpdateProperty(key, "5");
            ASSERT_TRUE(definition->matches(properties));

            const ModelSpecification spec = definition->modelSpecification(properties);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(skinIndex, spec.skinIndex);
            ASSERT_EQ(frameIndex, spec.frameIndex);
        }
        
        TEST(ModelDefinitionTest, testDynamicModelDefinition) {
            const PropertyKey pathKey = "model";
            const PropertyValue pathValue = "maps/shell.bsp";
            
            const IO::Path path(pathValue);
            ModelDefinitionPtr definition = ModelDefinitionPtr(new DynamicModelDefinition(pathKey));
            
            EntityProperties properties;
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(pathKey, "");
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(pathKey, pathValue);
            ASSERT_TRUE(definition->matches(properties));
            
            const ModelSpecification spec = definition->modelSpecification(properties);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(0, spec.skinIndex);
            ASSERT_EQ(0, spec.frameIndex);
        }
        
        TEST(ModelDefinitionTest, testDynamicModelDefinitionWithSkinKey) {
            const PropertyKey pathKey = "model";
            const PropertyValue pathValue = "maps/shell.bsp";
            const PropertyKey skinKey = "skin";
            const PropertyValue skinValue = "1";
            
            const IO::Path path(pathValue);
            const size_t skinIndex = 1;
            
            ModelDefinitionPtr definition = ModelDefinitionPtr(new DynamicModelDefinition(pathKey, skinKey));
            
            EntityProperties properties;
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(pathKey, pathValue);
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(skinKey, "");
            ASSERT_FALSE(definition->matches(properties));

            properties.addOrUpdateProperty(skinKey, skinValue);
            ASSERT_TRUE(definition->matches(properties));
            
            const ModelSpecification spec = definition->modelSpecification(properties);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(skinIndex, spec.skinIndex);
            ASSERT_EQ(0, spec.frameIndex);
        }
        
        TEST(ModelDefinitionTest, testDynamicModelDefinitionWithSkinAndFrameKey) {
            const PropertyKey pathKey = "model";
            const PropertyValue pathValue = "maps/shell.bsp";
            const PropertyKey skinKey = "skin";
            const PropertyValue skinValue = "1";
            const PropertyKey frameKey = "frame";
            const PropertyValue frameValue = "2";
            
            const IO::Path path(pathValue);
            const size_t skinIndex = 1;
            const size_t frameIndex = 2;
            
            ModelDefinitionPtr definition = ModelDefinitionPtr(new DynamicModelDefinition(pathKey, skinKey, frameKey));
            
            EntityProperties properties;
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(pathKey, pathValue);
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(skinKey, skinValue);
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(frameKey, "");
            ASSERT_FALSE(definition->matches(properties));

            properties.addOrUpdateProperty(frameKey, frameValue);
            ASSERT_TRUE(definition->matches(properties));

            const ModelSpecification spec = definition->modelSpecification(properties);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(skinIndex, spec.skinIndex);
            ASSERT_EQ(frameIndex, spec.frameIndex);
        }
    }
}
