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

#include "Model/Entity.h"
#include "Model/Map.h"
#include "Model/MapObjectsIterator.h"
#include "Model/ModelTypes.h"
#include "Model/QuakeEntityRotator.h"

namespace TrenchBroom {
    namespace Model {
        TEST(MapObjectsIteratorTest, emptyMap) {
            Map map(MFQuake);
            MapObjectsIterator::OuterIterator it = MapObjectsIterator::begin(map);
            MapObjectsIterator::OuterIterator end = MapObjectsIterator::end(map);
            ASSERT_TRUE(it == end);
        }

        TEST(MapObjectsIteratorTest, oneEmptyEntityMap) {
            Map map(MFQuake);

            Entity* entity = new ConfigurableEntity<QuakeEntityRotationPolicy>();
            map.addEntity(entity);
            
            MapObjectsIterator::OuterIterator it = MapObjectsIterator::begin(map);
            MapObjectsIterator::OuterIterator end = MapObjectsIterator::end(map);

            ASSERT_TRUE(it != end);
            ASSERT_EQ(static_cast<Object*>(entity), *it);
            ++it;
            ASSERT_TRUE(it == end);
        }
    }
}
