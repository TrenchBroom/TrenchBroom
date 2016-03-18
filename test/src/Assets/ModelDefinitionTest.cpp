/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
#include "Model/EntityAttributes.h"
#include "Model/ModelTypes.h"
#include "Assets/ModelDefinition.h"

namespace TrenchBroom {
    namespace Assets {
        TEST(ModelDefinitionTest, testStaticModelDefinition) {
            const IO::Path path("maps/shell.bsp");
            const size_t skinIndex = 1;
            const size_t frameIndex = 2;
            ModelDefinitionPtr definition = ModelDefinitionPtr(new StaticModelDefinition(path, skinIndex, frameIndex));
            
            Model::EntityAttributes attributes;
            ASSERT_TRUE(definition->matches(attributes));
            
            const ModelSpecification spec = definition->modelSpecification(attributes);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(skinIndex, spec.skinIndex);
            ASSERT_EQ(frameIndex, spec.frameIndex);
        }

        TEST(ModelDefinitionTest, testStaticModelDefinitionWithAttribute) {
            const IO::Path path("maps/shell.bsp");
            const size_t skinIndex = 1;
            const size_t frameIndex = 2;
            const Model::AttributeName key = "testKey";
            const Model::AttributeValue value = "testValue";
            ModelDefinitionPtr definition = ModelDefinitionPtr(new StaticModelDefinition(path, skinIndex, frameIndex, key, value));
            
            Model::EntityAttributes attributes;
            ASSERT_FALSE(definition->matches(attributes));
            
            attributes.addOrUpdateAttribute(key, "blah", NULL);
            ASSERT_FALSE(definition->matches(attributes));
            
            attributes.addOrUpdateAttribute(key, value, NULL);
            ASSERT_TRUE(definition->matches(attributes));
            
            const ModelSpecification spec = definition->modelSpecification(attributes);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(skinIndex, spec.skinIndex);
            ASSERT_EQ(frameIndex, spec.frameIndex);
        }
        
        TEST(ModelDefinitionTest, testStaticModelDefinitionWithFlag) {
            const IO::Path path("maps/shell.bsp");
            const size_t skinIndex = 1;
            const size_t frameIndex = 2;
            const Model::AttributeName key = "testKey";
            const int value = 4;
            ModelDefinitionPtr definition = ModelDefinitionPtr(new StaticModelDefinition(path, skinIndex, frameIndex, key, value));
            
            Model::EntityAttributes attributes;
            ASSERT_FALSE(definition->matches(attributes));
            
            attributes.addOrUpdateAttribute(key, "blah", NULL);
            ASSERT_FALSE(definition->matches(attributes));
            
            attributes.addOrUpdateAttribute(key, "1", NULL);
            ASSERT_FALSE(definition->matches(attributes));

            attributes.addOrUpdateAttribute(key, "4", NULL);
            ASSERT_TRUE(definition->matches(attributes));
            
            attributes.addOrUpdateAttribute(key, "5", NULL);
            ASSERT_TRUE(definition->matches(attributes));

            const ModelSpecification spec = definition->modelSpecification(attributes);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(skinIndex, spec.skinIndex);
            ASSERT_EQ(frameIndex, spec.frameIndex);
        }
        
        TEST(ModelDefinitionTest, testDynamicModelDefinition) {
            const Model::AttributeName pathKey = "model";
            const Model::AttributeValue pathValue = "maps/shell.bsp";
            
            const IO::Path path(pathValue);
            ModelDefinitionPtr definition = ModelDefinitionPtr(new DynamicModelDefinition(pathKey));
            
            Model::EntityAttributes attributes;
            ASSERT_FALSE(definition->matches(attributes));
            
            attributes.addOrUpdateAttribute(pathKey, "", NULL);
            ASSERT_FALSE(definition->matches(attributes));
            
            attributes.addOrUpdateAttribute(pathKey, pathValue, NULL);
            ASSERT_TRUE(definition->matches(attributes));
            
            const ModelSpecification spec = definition->modelSpecification(attributes);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(0u, spec.skinIndex);
            ASSERT_EQ(0u, spec.frameIndex);
        }
        
        TEST(ModelDefinitionTest, testDynamicModelDefinitionWithSkinKey) {
            const Model::AttributeName pathKey = "model";
            const Model::AttributeValue pathValue = "maps/shell.bsp";
            const Model::AttributeName skinKey = "skin";
            const Model::AttributeValue skinValue = "1";
            
            const IO::Path path(pathValue);
            const size_t skinIndex = 1;
            
            ModelDefinitionPtr definition = ModelDefinitionPtr(new DynamicModelDefinition(pathKey, skinKey));
            
            Model::EntityAttributes attributes;
            ASSERT_FALSE(definition->matches(attributes));
            
            attributes.addOrUpdateAttribute(pathKey, pathValue, NULL);
            ASSERT_FALSE(definition->matches(attributes));
            
            attributes.addOrUpdateAttribute(skinKey, "", NULL);
            ASSERT_FALSE(definition->matches(attributes));

            attributes.addOrUpdateAttribute(skinKey, skinValue, NULL);
            ASSERT_TRUE(definition->matches(attributes));
            
            const ModelSpecification spec = definition->modelSpecification(attributes);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(skinIndex, spec.skinIndex);
            ASSERT_EQ(0u, spec.frameIndex);
        }
        
        TEST(ModelDefinitionTest, testDynamicModelDefinitionWithSkinAndFrameKey) {
            const Model::AttributeName pathKey = "model";
            const Model::AttributeValue pathValue = "maps/shell.bsp";
            const Model::AttributeName skinKey = "skin";
            const Model::AttributeValue skinValue = "1";
            const Model::AttributeName frameKey = "frame";
            const Model::AttributeValue frameValue = "2";
            
            const IO::Path path(pathValue);
            const size_t skinIndex = 1;
            const size_t frameIndex = 2;
            
            ModelDefinitionPtr definition = ModelDefinitionPtr(new DynamicModelDefinition(pathKey, skinKey, frameKey));
            
            Model::EntityAttributes attributes;
            ASSERT_FALSE(definition->matches(attributes));
            
            attributes.addOrUpdateAttribute(pathKey, pathValue, NULL);
            ASSERT_FALSE(definition->matches(attributes));
            
            attributes.addOrUpdateAttribute(skinKey, skinValue, NULL);
            ASSERT_FALSE(definition->matches(attributes));
            
            attributes.addOrUpdateAttribute(frameKey, "", NULL);
            ASSERT_FALSE(definition->matches(attributes));

            attributes.addOrUpdateAttribute(frameKey, frameValue, NULL);
            ASSERT_TRUE(definition->matches(attributes));

            const ModelSpecification spec = definition->modelSpecification(attributes);
            ASSERT_EQ(path, spec.path);
            ASSERT_EQ(skinIndex, spec.skinIndex);
            ASSERT_EQ(frameIndex, spec.frameIndex);
        }
    }
}
