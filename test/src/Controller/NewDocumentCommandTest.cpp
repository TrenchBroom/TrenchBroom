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
#include <gmock/gmock.h>

#include "Controller/NewDocumentCommand.h"
#include "IO/Path.h"
#include "Model/MockGame.h"
#include "Model/Map.h"
#include "Model/ModelTypes.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        TEST(NewDocumentCommandTest, newDocumentInEmptyDocument) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            const BBox3d worldBounds(-8192.0, 8192.0);
            Model::MockGamePtr game = Model::MockGame::newGame();
            EXPECT_CALL(*game, doNewMap()).WillOnce(Return(new Model::Map(Model::MFQuake)));
            EXPECT_CALL(*game, doExtractEntityDefinitionFile(_)).WillOnce(Return(IO::Path("")));
            EXPECT_CALL(*game, doLoadEntityDefinitions(IO::Path(""))).WillOnce(Return(Assets::EntityDefinitionList()));
            EXPECT_CALL(*game, doFindBuiltinTextureCollections()).WillOnce(Return(IO::Path::List()));
            
            View::MapDocumentPtr doc = View::MapDocument::newMapDocument();
            
            Command::Ptr command = Command::Ptr(new NewDocumentCommand(doc, worldBounds, game));
            ASSERT_FALSE(command->undoable());
            ASSERT_TRUE(command->performDo());
            ASSERT_EQ(IO::Path("unnamed.map"), doc->path());
            ASSERT_FALSE(doc->modified());
        }

        TEST(NewDocumentCommandTest, newDocumentInExistingDocument) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;

            const BBox3d worldBounds(-8192.0, 8192.0);
            Model::MockGamePtr game = Model::MockGame::newGame();
            const IO::Path path("data/Controller/NewDocumentCommandTest/Cube.map");

            Model::Map* map = new Model::Map(Model::MFQuake);
            EXPECT_CALL(*game, doLoadMap(worldBounds, path)).WillOnce(Return(map));
            EXPECT_CALL(*game, doExtractEntityDefinitionFile(map)).WillOnce(Return(IO::Path("")));
            EXPECT_CALL(*game, doLoadEntityDefinitions(IO::Path(""))).WillOnce(Return(Assets::EntityDefinitionList()));
            EXPECT_CALL(*game, doFindBuiltinTextureCollections()).WillOnce(Return(IO::Path::List()));
            EXPECT_CALL(*game, doExtractTexturePaths(map)).WillOnce(Return(IO::Path::List()));
            EXPECT_CALL(*game, doNewMap()).WillOnce(Return(new Model::Map(Model::MFQuake)));
            EXPECT_CALL(*game, doExtractEntityDefinitionFile(_)).WillOnce(Return(IO::Path("")));
            EXPECT_CALL(*game, doLoadEntityDefinitions(IO::Path(""))).WillOnce(Return(Assets::EntityDefinitionList()));
            EXPECT_CALL(*game, doFindBuiltinTextureCollections()).WillOnce(Return(IO::Path::List()));

            View::MapDocumentPtr doc = View::MapDocument::newMapDocument();
            doc->openDocument(worldBounds, game, path);
            
            Command::Ptr command = Command::Ptr(new NewDocumentCommand(doc, worldBounds, game));
            ASSERT_FALSE(command->undoable());
            ASSERT_TRUE(command->performDo());
            ASSERT_EQ(IO::Path("unnamed.map"), doc->path());
            ASSERT_FALSE(doc->modified());
            
            ASSERT_TRUE(doc->map()->entities().empty());
            ASSERT_TRUE(doc->map()->worldspawn() == NULL);
        }
    }
}
