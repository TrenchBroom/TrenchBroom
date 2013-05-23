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

#include "StringUtils.h"
#include "IO/QuakeMapParser.h"
#include "Model/Entity.h"
#include "Model/EntityPropertyTypes.h"
#include "Model/Map.h"

namespace TrenchBroom {
    namespace IO {
        TEST(QuakeMapParserTest, ParseEmptyMap) {
            const String data("");
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::MapPtr map = parser.parseMap(worldBounds);
            
            ASSERT_TRUE(map->entities().empty());
        }
        
        TEST(QuakeMapParserTest, ParseMapWithEmptyEntity) {
            const String data("{}");
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::MapPtr map = parser.parseMap(worldBounds);

            ASSERT_EQ(1, map->entities().size());
        }
        
        TEST(QuakeMapParserTest, ParseMapWithWorldspawn) {
            const String data("{"
                              "\"classname\" \"worldspawn\""
                              "}");
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::MapPtr map = parser.parseMap(worldBounds);
            
            const Model::EntityList& entities = map->entities();
            ASSERT_EQ(1, entities.size());
            
            const Model::EntityPtr entity = entities.front();
            ASSERT_TRUE(entity->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(Model::PropertyValues::WorldspawnClassname, entity->property(Model::PropertyKeys::Classname));
            
        }
    }
}
