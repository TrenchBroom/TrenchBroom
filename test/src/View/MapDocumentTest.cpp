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

#include "IO/Path.h"
#include "Model/MockGame.h"
#include "Model/Map.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"

#include <wx/frame.h>

namespace TrenchBroom {
    namespace View {
        TEST(MapDocumentTest, newDocument) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;

            const BBox3d worldBounds(-8192.0, 8192.0);
            Model::MockGamePtr game = Model::MockGame::newGame();
            EXPECT_CALL(*game, doNewMap()).WillOnce(Return(new Model::Map(Model::MFQuake)));
            EXPECT_CALL(*game, doExtractEntityDefinitionFile(_)).WillOnce(Return(IO::Path("")));
            EXPECT_CALL(*game, doLoadEntityDefinitions(IO::Path(""))).WillOnce(Return(Assets::EntityDefinitionList()));
            EXPECT_CALL(*game, doFindBuiltinTextureCollections()).WillOnce(Return(IO::Path::List()));

            MapDocumentPtr document = MapDocument::newMapDocument();
            document->newDocument(worldBounds, game);
            
            ASSERT_EQ(IO::Path("unnamed.map"), document->path());
            ASSERT_EQ(String("unnamed.map"), document->filename());
        }
        
        TEST(MapDocumentTest, openDocument) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            const BBox3d worldBounds(-8192.0, 8192.0);
            Model::MockGamePtr game = Model::MockGame::newGame();
            const IO::Path path("data/View/FrameManager/TestDoc1.map");
            
            Model::Map* map = new Model::Map(Model::MFQuake);
            EXPECT_CALL(*game, doLoadMap(worldBounds, path)).WillOnce(Return(map));
            EXPECT_CALL(*game, doExtractEntityDefinitionFile(map)).WillOnce(Return(IO::Path("")));
            EXPECT_CALL(*game, doLoadEntityDefinitions(IO::Path(""))).WillOnce(Return(Assets::EntityDefinitionList()));
            EXPECT_CALL(*game, doFindBuiltinTextureCollections()).WillOnce(Return(IO::Path::List()));
            EXPECT_CALL(*game, doExtractTexturePaths(map)).WillOnce(Return(IO::Path::List()));
            
            MapDocumentPtr document = MapDocument::newMapDocument();
            document->openDocument(worldBounds, game, path);
            
            ASSERT_EQ(path, document->path());
            ASSERT_EQ(String("TestDoc1.map"), document->filename());
        }
    }
}

