/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef MapDocumentTest_h
#define MapDocumentTest_h

#include <gtest/gtest.h>

#include "Model/MapFormat.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Assets {
        class PointEntityDefinition;
        class BrushEntityDefinition;
    }
    namespace Model {
        class Brush;
    }
    
    namespace View {
        class MapDocumentTest : public ::testing::Test {
        private:
            Model::MapFormat m_mapFormat;
        protected:
            MapDocumentSPtr document;
            Assets::PointEntityDefinition* m_pointEntityDef;
            Assets::BrushEntityDefinition* m_brushEntityDef;
        protected:
            MapDocumentTest();
            MapDocumentTest(Model::MapFormat mapFormat);
            
            void SetUp() override;
            void TearDown() override;
            
            Model::Brush* createBrush(const String& textureName = "texture");
        };
        
        class ValveMapDocumentTest : public MapDocumentTest {
        protected:
            ValveMapDocumentTest();
        };
    }
}

#endif /* MapDocumentTest_h */
