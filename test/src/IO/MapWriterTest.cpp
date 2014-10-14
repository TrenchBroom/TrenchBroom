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

#include "IO/MapWriter.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/MapFormat.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace IO {
        TEST(MapWriterTest, writeEmptyMap) {
            Model::World map(Model::MapFormat::Standard, NULL);
            
            StringStream str;
            MapWriter::writeToStream(&map, str);
            
            const String result = str.str();
            ASSERT_STREQ("{\n}\n", result.c_str());
        }

        TEST(MapWriterTest, writeWorldspawn) {
            Model::World map(Model::MapFormat::Standard, NULL);
            map.addOrUpdateAttribute("classname", "worldspawn");
            map.addOrUpdateAttribute("message", "holy damn");
            
            StringStream str;
            MapWriter::writeToStream(&map, str);
            
            const String result = str.str();
            ASSERT_STREQ("{\n"
                         "\"classname\" \"worldspawn\"\n"
                         "\"message\" \"holy damn\"\n"
                         "}\n", result.c_str());
        }
        
        TEST(MapWriterTest, writeWorldspawnWithBrushInDefaultLayer) {
            const BBox3 worldBounds(8192.0);
            
            Model::World map(Model::MapFormat::Standard, NULL);
            map.addOrUpdateAttribute("classname", "worldspawn");
            
            Model::BrushBuilder builder(&map, worldBounds);
            Model::Brush* brush = builder.createCube(64.0, "none");
            map.defaultLayer()->addChild(brush);
            
            StringStream str;
            MapWriter::writeToStream(&map, str);
            
            const String result = str.str();
            ASSERT_STREQ("{\n"
                         "\"classname\" \"worldspawn\"\n"
                         "{\n"
                         "( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1\n"
                         "( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1\n"
                         "( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1\n"
                         "( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1\n"
                         "( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1\n"
                         "( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1\n"
                         "}\n"
                         "}\n", result.c_str());
        }
    }
}
