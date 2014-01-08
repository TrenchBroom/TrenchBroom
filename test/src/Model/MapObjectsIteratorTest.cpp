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

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Map.h"
#include "Model/MapObjectsIterator.h"
#include "Model/ModelTypes.h"
#include "Model/QuakeEntityRotationPolicy.h"

namespace TrenchBroom {
    namespace Model {
        TEST(MapObjectsIteratorTest, emptyMap) {
            Map map(MapFormat::Quake);
            MapObjectsIterator::OuterIterator it = MapObjectsIterator::begin(map);
            MapObjectsIterator::OuterIterator end = MapObjectsIterator::end(map);
            ASSERT_TRUE(it == end);
        }

        TEST(MapObjectsIteratorTest, oneEmptyEntityMap) {
            Map map(MapFormat::Quake);

            Entity* entity = new ConfigurableEntity<QuakeEntityRotationPolicy>();
            map.addEntity(entity);
            
            MapObjectsIterator::OuterIterator it = MapObjectsIterator::begin(map);
            MapObjectsIterator::OuterIterator end = MapObjectsIterator::end(map);

            ASSERT_TRUE(it != end);
            ASSERT_EQ(entity, *it);
            ++it;
            ASSERT_TRUE(it == end);
        }
        
        TEST(MapObjectsIteratorTest, severalEntitiesMap) {
            Map map(MapFormat::Quake);
            
            const BBox3 worldbounds(8192.0);
            Brush* brush1 = new Brush(worldbounds, EmptyBrushFaceList);
            Brush* brush2 = new Brush(worldbounds, EmptyBrushFaceList);
            Brush* brush3 = new Brush(worldbounds, EmptyBrushFaceList);
            Brush* brush4 = new Brush(worldbounds, EmptyBrushFaceList);
            Brush* brush5 = new Brush(worldbounds, EmptyBrushFaceList);
            

            Entity* entity1 = new ConfigurableEntity<QuakeEntityRotationPolicy>();
            Entity* entity2 = new ConfigurableEntity<QuakeEntityRotationPolicy>();
            Entity* entity3 = new ConfigurableEntity<QuakeEntityRotationPolicy>();
            Entity* entity4 = new ConfigurableEntity<QuakeEntityRotationPolicy>();
            Entity* entity5 = new ConfigurableEntity<QuakeEntityRotationPolicy>();
            
            map.addEntity(entity1);
            entity1->addBrush(brush1);
            
            map.addEntity(entity2);
            
            map.addEntity(entity3);
            entity3->addBrush(brush2);
            entity3->addBrush(brush3);
            entity3->addBrush(brush4);
            
            map.addEntity(entity4);
            
            map.addEntity(entity5);
            entity5->addBrush(brush5);
            
            
            MapObjectsIterator::OuterIterator it = MapObjectsIterator::begin(map);
            MapObjectsIterator::OuterIterator end = MapObjectsIterator::end(map);
            
            ASSERT_TRUE(it != end);
            ASSERT_EQ(entity1, *it);
            ++it;
            ASSERT_EQ(brush1, *it);
            ++it;
            ASSERT_EQ(entity2, *it);
            ++it;
            ASSERT_EQ(entity3, *it);
            ++it;
            ASSERT_EQ(brush2, *it);
            ++it;
            ASSERT_EQ(brush3, *it);
            ++it;
            ASSERT_EQ(brush4, *it);
            ++it;
            ASSERT_EQ(entity4, *it);
            ++it;
            ASSERT_EQ(entity5, *it);
            ++it;
            ASSERT_EQ(brush5, *it);
            ++it;
            ASSERT_TRUE(it == end);
        }
    }
}
