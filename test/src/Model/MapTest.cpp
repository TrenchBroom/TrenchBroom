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
#include "Model/ModelTypes.h"
#include "Model/QuakeEntityRotator.h"

namespace TrenchBroom {
    namespace Model {
        TEST(MapTest, addEntity) {
            Map map(MapFormat::Quake);
            Entity* entity = new ConfigurableEntity<QuakeEntityRotationPolicy>();
            map.addEntity(entity);
            
            const EntityList& entities = map.entities();
            ASSERT_EQ(1u, entities.size());
            ASSERT_EQ(entity, entities[0]);
        }
        
        TEST(MapTest, getNonExistingWorldspawn) {
            Map map(MapFormat::Quake);
            ASSERT_EQ(NULL, map.worldspawn());

            Entity* worldspawn = new ConfigurableEntity<QuakeEntityRotationPolicy>();
            map.addEntity(worldspawn);
            ASSERT_EQ(NULL, map.worldspawn());
        }
        
        TEST(MapTest, getExistingWorldspawn) {
            Map map(MapFormat::Quake);
            Entity* worldspawn = new ConfigurableEntity<QuakeEntityRotationPolicy>();
            worldspawn->addOrUpdateProperty(PropertyKeys::Classname, PropertyValues::WorldspawnClassname);
            map.addEntity(worldspawn);
            
            ASSERT_EQ(worldspawn, map.worldspawn());
        }
    }
}
