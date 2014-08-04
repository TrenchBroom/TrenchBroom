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

#include <gtest/gtest.h>

#include "IO/Path.h"
#include "Assets/AssetTypes.h"
#include "Model/EntityProperties.h"
#include "Model/ModelTypes.h"
#include "Assets/ModelDefinition.h"

namespace TrenchBroom {
    namespace Assets {
        TEST(ModelDefinitionTest, testStaticModelDefinition) {
            const IO::Path path("maps/shell.bsp");
            const size_t skinIndex = 1;
            const size_t frameIndex = 2;
            ModelDefinitionPtr definition = ModelDefinitionPtr(new StaticModelDefinition(path, skinIndex, frameIndex));
            
            Model::EntityProperties properties;
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
            const Model::PropertyKey key = "testKey";
            const Model::PropertyValue value = "testValue";
            ModelDefinitionPtr definition = ModelDefinitionPtr(new StaticModelDefinition(path, skinIndex, frameIndex, key, value));
            
            Model::EntityProperties properties;
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(key, "blah", NULL);
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(key, value, NULL);
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
            const Model::PropertyKey key = "testKey";
            const int value = 4;
            ModelDefinitionPtr definition = ModelDefinitionPtr(new StaticModelDefinition(path, skinIndex, frameIndex, key, value));
            
            Model::EntityProperties properties;
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(key, "blah", NULL);
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(key, "1", NULL);
            ASSERT_FALSE(definition->matches(properties));

            properties.addOrUpdateProperty(key, "4", NULL);
            ASSERT_TRUE(definition->matches(properties));
            
            properties.addOrUpdateProperty(key, "5", NULL);
            ASSERT_TRUE(definition->matches(properties));

            const ModelSpecification spec = definition->modelSpecification(properties);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(skinIndex, spec.skinIndex);
            ASSERT_EQ(frameIndex, spec.frameIndex);
        }
        
        TEST(ModelDefinitionTest, testDynamicModelDefinition) {
            const Model::PropertyKey pathKey = "model";
            const Model::PropertyValue pathValue = "maps/shell.bsp";
            
            const IO::Path path(pathValue);
            ModelDefinitionPtr definition = ModelDefinitionPtr(new DynamicModelDefinition(pathKey));
            
            Model::EntityProperties properties;
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(pathKey, "", NULL);
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(pathKey, pathValue, NULL);
            ASSERT_TRUE(definition->matches(properties));
            
            const ModelSpecification spec = definition->modelSpecification(properties);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(0u, spec.skinIndex);
            ASSERT_EQ(0u, spec.frameIndex);
        }
        
        TEST(ModelDefinitionTest, testDynamicModelDefinitionWithSkinKey) {
            const Model::PropertyKey pathKey = "model";
            const Model::PropertyValue pathValue = "maps/shell.bsp";
            const Model::PropertyKey skinKey = "skin";
            const Model::PropertyValue skinValue = "1";
            
            const IO::Path path(pathValue);
            const size_t skinIndex = 1;
            
            ModelDefinitionPtr definition = ModelDefinitionPtr(new DynamicModelDefinition(pathKey, skinKey));
            
            Model::EntityProperties properties;
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(pathKey, pathValue, NULL);
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(skinKey, "", NULL);
            ASSERT_FALSE(definition->matches(properties));

            properties.addOrUpdateProperty(skinKey, skinValue, NULL);
            ASSERT_TRUE(definition->matches(properties));
            
            const ModelSpecification spec = definition->modelSpecification(properties);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(skinIndex, spec.skinIndex);
            ASSERT_EQ(0u, spec.frameIndex);
        }
        
        TEST(ModelDefinitionTest, testDynamicModelDefinitionWithSkinAndFrameKey) {
            const Model::PropertyKey pathKey = "model";
            const Model::PropertyValue pathValue = "maps/shell.bsp";
            const Model::PropertyKey skinKey = "skin";
            const Model::PropertyValue skinValue = "1";
            const Model::PropertyKey frameKey = "frame";
            const Model::PropertyValue frameValue = "2";
            
            const IO::Path path(pathValue);
            const size_t skinIndex = 1;
            const size_t frameIndex = 2;
            
            ModelDefinitionPtr definition = ModelDefinitionPtr(new DynamicModelDefinition(pathKey, skinKey, frameKey));
            
            Model::EntityProperties properties;
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(pathKey, pathValue, NULL);
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(skinKey, skinValue, NULL);
            ASSERT_FALSE(definition->matches(properties));
            
            properties.addOrUpdateProperty(frameKey, "", NULL);
            ASSERT_FALSE(definition->matches(properties));

            properties.addOrUpdateProperty(frameKey, frameValue, NULL);
            ASSERT_TRUE(definition->matches(properties));

            const ModelSpecification spec = definition->modelSpecification(properties);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(skinIndex, spec.skinIndex);
            ASSERT_EQ(frameIndex, spec.frameIndex);
        }
    }
}
