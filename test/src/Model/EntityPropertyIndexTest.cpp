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

#include "CollectionUtils.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/EntityPropertyIndex.h"
#include "Model/QuakeEntityRotationPolicy.h"

namespace TrenchBroom {
    namespace Model {
        typedef ConfigurableEntity<QuakeEntityRotationPolicy> QuakeEntity;

        TEST(EntityPropertyIndexTest, addEntity) {
            EntityPropertyIndex index;
            
            Entity* entity1 = new QuakeEntity();
            entity1->addOrUpdateProperty("test", "somevalue");
            
            Entity* entity2 = new QuakeEntity();
            entity2->addOrUpdateProperty("test", "somevalue");
            entity2->addOrUpdateProperty("other", "someothervalue");
            
            index.addEntity(entity1);
            index.addEntity(entity2);
            
            ASSERT_TRUE(index.findEntitiesWithProperty("test", "notfound").empty());
            
            EntityList entities = index.findEntitiesWithProperty("test", "somevalue");
            ASSERT_EQ(2u, entities.size());
            ASSERT_TRUE(VectorUtils::contains(entities, entity1));
            ASSERT_TRUE(VectorUtils::contains(entities, entity2));
            
            entities = index.findEntitiesWithProperty("other", "someothervalue");
            ASSERT_EQ(1u, entities.size());
            ASSERT_TRUE(VectorUtils::contains(entities, entity2));
            
            delete entity1;
            delete entity2;
        }
        
        TEST(EntityPropertyIndexTest, removeEntity) {
            EntityPropertyIndex index;
            
            Entity* entity1 = new QuakeEntity();
            entity1->addOrUpdateProperty("test", "somevalue");
            
            Entity* entity2 = new QuakeEntity();
            entity2->addOrUpdateProperty("test", "somevalue");
            entity2->addOrUpdateProperty("other", "someothervalue");
            
            index.addEntity(entity1);
            index.addEntity(entity2);
            
            index.removeEntity(entity2);
            
            const EntityList& entities = index.findEntitiesWithProperty("test", "somevalue");
            ASSERT_EQ(1u, entities.size());
            ASSERT_EQ(entity1, entities.front());
            
            delete entity1;
            delete entity2;
        }
        
        TEST(EntityPropertyIndexTest, addEntityProperty) {
            EntityPropertyIndex index;
            
            Entity* entity1 = new QuakeEntity();
            entity1->addOrUpdateProperty("test", "somevalue");
            
            Entity* entity2 = new QuakeEntity();
            entity2->addOrUpdateProperty("test", "somevalue");
            
            index.addEntity(entity1);
            index.addEntity(entity2);
            
            entity2->addOrUpdateProperty("other", "someothervalue");
            index.addEntityProperty(entity2, Model::EntityProperty("other", "someothervalue"));
            
            ASSERT_TRUE(index.findEntitiesWithProperty("test", "notfound").empty());
            
            EntityList entities = index.findEntitiesWithProperty("test", "somevalue");
            ASSERT_EQ(2u, entities.size());
            ASSERT_TRUE(VectorUtils::contains(entities, entity1));
            ASSERT_TRUE(VectorUtils::contains(entities, entity2));
            
            entities = index.findEntitiesWithProperty("other", "someothervalue");
            ASSERT_EQ(1u, entities.size());
            ASSERT_TRUE(VectorUtils::contains(entities, entity2));
            
            delete entity1;
            delete entity2;
        }

        TEST(EntityPropertyIndexTest, removeEntityProperty) {
            EntityPropertyIndex index;
            
            Entity* entity1 = new QuakeEntity();
            entity1->addOrUpdateProperty("test", "somevalue");
            
            Entity* entity2 = new QuakeEntity();
            entity2->addOrUpdateProperty("test", "somevalue");
            entity2->addOrUpdateProperty("other", "someothervalue");
            
            index.addEntity(entity1);
            index.addEntity(entity2);
            
            index.removeEntityProperty(entity2, EntityProperty("other", "someothervalue"));
            
            const EntityList& entities = index.findEntitiesWithProperty("test", "somevalue");
            ASSERT_EQ(2u, entities.size());
            ASSERT_TRUE(VectorUtils::contains(entities, entity1));
            ASSERT_TRUE(VectorUtils::contains(entities, entity2));
            
            ASSERT_TRUE(index.findEntitiesWithProperty("other", "someothervalue").empty());

            delete entity1;
            delete entity2;
        }

        TEST(EntityPropertyIndexTest, addNumberedEntityProperty) {
            EntityPropertyIndex index;
            
            Entity* entity1 = new QuakeEntity();
            entity1->addOrUpdateProperty("test1", "somevalue");
            entity1->addOrUpdateProperty("test2", "somevalue");
            
            index.addEntity(entity1);
            
            ASSERT_TRUE(index.findEntitiesWithNumberedProperty("test", "notfound").empty());
            
            EntityList entities = index.findEntitiesWithNumberedProperty("test", "somevalue");
            ASSERT_EQ(1u, entities.size());
            ASSERT_TRUE(VectorUtils::contains(entities, entity1));
            
            delete entity1;
        }
        
    }
}
