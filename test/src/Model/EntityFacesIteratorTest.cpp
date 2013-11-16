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

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/EntityFacesIterator.h"
#include "Model/Map.h"
#include "Model/ModelTypes.h"
#include "Model/ModelUtils.h"
#include "Model/QuakeEntityRotator.h"

namespace TrenchBroom {
    namespace Model {
        TEST(EntityFacesIteratorTest, testEmptyIterator) {
            EntityList entities;
            EntityFacesIterator::OuterIterator begin = EntityFacesIterator::begin(entities);
            EntityFacesIterator::OuterIterator end = EntityFacesIterator::end(entities);
            ASSERT_TRUE(begin == end);
        }
        
        TEST(EntityFacesIteratorTest, testOneEmptyEntityIterator) {
            EntityList entities;
            ConfigurableEntity<QuakeEntityRotationPolicy> entity;
            entities.push_back(&entity);
            
            EntityFacesIterator::OuterIterator begin = EntityFacesIterator::begin(entities);
            EntityFacesIterator::OuterIterator end = EntityFacesIterator::end(entities);
            
            ASSERT_TRUE(begin == end);
        }
        
        TEST(EntityFacesIteratorTest, testNonEmptyEntityIterator) {
            Map map(MapFormat::Quake);
            Brush* brush = createBrushFromBounds(map, BBox3(4096), BBox3(32), "");
            Entity* entity = map.createEntity();
            entity->addBrush(brush);
            map.addEntity(entity);

            const Model::EntityList& entities = map.entities();
            EntityFacesIterator::OuterIterator begin = EntityFacesIterator::begin(entities);
            EntityFacesIterator::OuterIterator end = EntityFacesIterator::end(entities);
            
            const BrushFaceList& faces = brush->faces();
            ASSERT_TRUE(begin != end);
            ASSERT_EQ(faces[0], *begin++);
            ASSERT_EQ(faces[1], *begin++);
            ASSERT_EQ(faces[2], *begin++);
            ASSERT_EQ(faces[3], *begin++);
            ASSERT_EQ(faces[4], *begin++);
            ASSERT_EQ(faces[5], *begin++);
            ASSERT_TRUE(begin == end);
        }
        
        TEST(EntityFacesIteratorTest, testEntitiesWithEmptyEntityFirst) {
            Map map(MapFormat::Quake);
            Entity* entity1 = map.createEntity();
            map.addEntity(entity1);
            
            Brush* brush2 = createBrushFromBounds(map, BBox3(4096), BBox3(32), "");
            Entity* entity2 = map.createEntity();
            entity2->addBrush(brush2);
            map.addEntity(entity2);
            
            Brush* brush3 = createBrushFromBounds(map, BBox3(4096), BBox3(32), "");
            Entity* entity3 = map.createEntity();
            entity3->addBrush(brush3);
            map.addEntity(entity3);
            
            const Model::EntityList& entities = map.entities();
            EntityFacesIterator::OuterIterator begin = EntityFacesIterator::begin(entities);
            EntityFacesIterator::OuterIterator end = EntityFacesIterator::end(entities);
            
            const BrushFaceList& faces2 = brush2->faces();
            const BrushFaceList& faces3 = brush3->faces();
            
            ASSERT_TRUE(begin != end);
            ASSERT_EQ(faces2[0], *begin++);
            ASSERT_EQ(faces2[1], *begin++);
            ASSERT_EQ(faces2[2], *begin++);
            ASSERT_EQ(faces2[3], *begin++);
            ASSERT_EQ(faces2[4], *begin++);
            ASSERT_EQ(faces2[5], *begin++);
            ASSERT_EQ(faces3[0], *begin++);
            ASSERT_EQ(faces3[1], *begin++);
            ASSERT_EQ(faces3[2], *begin++);
            ASSERT_EQ(faces3[3], *begin++);
            ASSERT_EQ(faces3[4], *begin++);
            ASSERT_EQ(faces3[5], *begin++);
            ASSERT_TRUE(begin == end);
        }
        
        TEST(EntityFacesIteratorTest, testEntitiesWithEmptyEntityInMiddle) {
            Map map(MapFormat::Quake);
            Brush* brush1 = createBrushFromBounds(map, BBox3(4096), BBox3(32), "");
            Entity* entity1 = map.createEntity();
            entity1->addBrush(brush1);
            map.addEntity(entity1);
            
            Entity* entity2 = map.createEntity();
            map.addEntity(entity2);
            
            Brush* brush3 = createBrushFromBounds(map, BBox3(4096), BBox3(32), "");
            Entity* entity3 = map.createEntity();
            entity3->addBrush(brush3);
            map.addEntity(entity3);
            
            const Model::EntityList& entities = map.entities();
            EntityFacesIterator::OuterIterator begin = EntityFacesIterator::begin(entities);
            EntityFacesIterator::OuterIterator end = EntityFacesIterator::end(entities);
            
            const BrushFaceList& faces1 = brush1->faces();
            const BrushFaceList& faces3 = brush3->faces();
            
            ASSERT_TRUE(begin != end);
            ASSERT_EQ(faces1[0], *begin++);
            ASSERT_EQ(faces1[1], *begin++);
            ASSERT_EQ(faces1[2], *begin++);
            ASSERT_EQ(faces1[3], *begin++);
            ASSERT_EQ(faces1[4], *begin++);
            ASSERT_EQ(faces1[5], *begin++);
            ASSERT_EQ(faces3[0], *begin++);
            ASSERT_EQ(faces3[1], *begin++);
            ASSERT_EQ(faces3[2], *begin++);
            ASSERT_EQ(faces3[3], *begin++);
            ASSERT_EQ(faces3[4], *begin++);
            ASSERT_EQ(faces3[5], *begin++);
            ASSERT_TRUE(begin == end);
        }
        
        TEST(EntityFacesIteratorTest, testEntitiesWithEmptyEntityLast) {
            Map map(MapFormat::Quake);
            Brush* brush1 = createBrushFromBounds(map, BBox3(4096), BBox3(32), "");
            Entity* entity1 = map.createEntity();
            entity1->addBrush(brush1);
            map.addEntity(entity1);
            
            Brush* brush2 = createBrushFromBounds(map, BBox3(4096), BBox3(32), "");
            Entity* entity2 = map.createEntity();
            entity2->addBrush(brush2);
            map.addEntity(entity2);
            
            Entity* entity3 = map.createEntity();
            map.addEntity(entity3);
            
            const Model::EntityList& entities = map.entities();
            EntityFacesIterator::OuterIterator begin = EntityFacesIterator::begin(entities);
            EntityFacesIterator::OuterIterator end = EntityFacesIterator::end(entities);
            
            const BrushFaceList& faces1 = brush1->faces();
            const BrushFaceList& faces2 = brush2->faces();
            
            ASSERT_TRUE(begin != end);
            ASSERT_EQ(faces1[0], *begin++);
            ASSERT_EQ(faces1[1], *begin++);
            ASSERT_EQ(faces1[2], *begin++);
            ASSERT_EQ(faces1[3], *begin++);
            ASSERT_EQ(faces1[4], *begin++);
            ASSERT_EQ(faces1[5], *begin++);
            ASSERT_EQ(faces2[0], *begin++);
            ASSERT_EQ(faces2[1], *begin++);
            ASSERT_EQ(faces2[2], *begin++);
            ASSERT_EQ(faces2[3], *begin++);
            ASSERT_EQ(faces2[4], *begin++);
            ASSERT_EQ(faces2[5], *begin++);
            ASSERT_TRUE(begin == end);
        }
    }
}
