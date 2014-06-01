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

#include "Controller/OpenDocumentCommand.h"
#include "IO/Path.h"
#include "Model/Entity.h"
#include "Model/EntityDefinitionFileSpec.h"
#include "Model/Map.h"
#include "Model/MockGame.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        TEST(OpenDocumentCommandTest, openDocumentInEmptyDocument) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;

            View::MapDocumentSPtr doc = View::MapDocument::newMapDocument();
            const BBox3d worldBounds(-8192.0, 8192.0);
            const Model::GameConfig::FlagsConfig contentFlags;
            Model::MockGamePtr game = Model::MockGame::newGame();
            const IO::Path path("data/Controller/OpenDocumentCommandTest/Cube.map");
            
            Model::Map* map = new Model::Map(Model::MapFormat::Quake);
            EXPECT_CALL(*game, doLoadMap(worldBounds, path)).WillOnce(Return(map));
            EXPECT_CALL(*game, doContentFlags()).WillOnce(ReturnRef(contentFlags));
            EXPECT_CALL(*game, doExtractEnabledMods(map)).WillOnce(Return(StringList()));
            EXPECT_CALL(*game, doSetAdditionalSearchPaths(IO::Path::List()));

            EXPECT_CALL(*game, doExtractEntityDefinitionFile(map)).WillOnce(Return(Model::EntityDefinitionFileSpec::external(IO::Path("/somefile.def"))));
            EXPECT_CALL(*game, doGamePath()).WillOnce(Return(IO::Path("")));
            EXPECT_CALL(*game, doFindEntityDefinitionFile(_, _)).WillOnce(Return(IO::Path("/somefile.def")));
            EXPECT_CALL(*game, doLoadEntityDefinitions(IO::Path("/somefile.def"))).WillOnce(Return(Assets::EntityDefinitionList()));

            EXPECT_CALL(*game, doFindBuiltinTextureCollections()).WillOnce(Return(IO::Path::List()));
            
            EXPECT_CALL(*game, doExtractExternalTextureCollections(map)).WillOnce(Return(EmptyStringList));
            EXPECT_CALL(*game, doGamePath()).WillOnce(Return(IO::Path("Quake")));

            Command::Ptr command = Command::Ptr(new OpenDocumentCommand(doc, worldBounds, game, path));
            ASSERT_FALSE(command->undoable());
            ASSERT_TRUE(command->performDo());
            ASSERT_EQ(path, doc->path());
            ASSERT_FALSE(doc->modified());
            ASSERT_EQ(map, doc->map());
        }

        TEST(OpenDocumentCommandTest, openDocumentInExistingDocument) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            View::MapDocumentSPtr doc = View::MapDocument::newMapDocument();
            const BBox3d worldBounds(-8192.0, 8192.0);
            const Model::GameConfig::FlagsConfig contentFlags;
            Model::MockGamePtr game = Model::MockGame::newGame();
            const IO::Path path1("data/Controller/OpenDocumentCommandTest/2Cubes.map");
            const IO::Path path2("data/Controller/OpenDocumentCommandTest/Cube.map");

            Model::Map* map1 = new Model::Map(Model::MapFormat::Quake);
            Model::Map* map2 = new Model::Map(Model::MapFormat::Quake);
            EXPECT_CALL(*game, doLoadMap(worldBounds, path1)).WillOnce(Return(map1));
            EXPECT_CALL(*game, doContentFlags()).WillOnce(ReturnRef(contentFlags));
            EXPECT_CALL(*game, doExtractEnabledMods(map1)).WillOnce(Return(StringList()));
            EXPECT_CALL(*game, doSetAdditionalSearchPaths(IO::Path::List()));

            EXPECT_CALL(*game, doExtractEntityDefinitionFile(map1)).WillOnce(Return(Model::EntityDefinitionFileSpec::external(IO::Path("/somefile.def"))));
            EXPECT_CALL(*game, doGamePath()).WillOnce(Return(IO::Path("")));
            EXPECT_CALL(*game, doFindEntityDefinitionFile(_, _)).WillOnce(Return(IO::Path("/somefile.def")));
            EXPECT_CALL(*game, doLoadEntityDefinitions(IO::Path("/somefile.def"))).WillOnce(Return(Assets::EntityDefinitionList()));

            EXPECT_CALL(*game, doFindBuiltinTextureCollections()).WillOnce(Return(IO::Path::List()));
            
            EXPECT_CALL(*game, doExtractExternalTextureCollections(map1)).WillOnce(Return(EmptyStringList));
            EXPECT_CALL(*game, doGamePath()).WillOnce(Return(IO::Path("Quake")));

            EXPECT_CALL(*game, doLoadMap(worldBounds, path2)).WillOnce(Return(map2));
            EXPECT_CALL(*game, doContentFlags()).WillOnce(ReturnRef(contentFlags));
            EXPECT_CALL(*game, doExtractEnabledMods(map2)).WillOnce(Return(StringList()));
            EXPECT_CALL(*game, doSetAdditionalSearchPaths(IO::Path::List()));

            EXPECT_CALL(*game, doExtractEntityDefinitionFile(map2)).WillOnce(Return(Model::EntityDefinitionFileSpec::external(IO::Path("/someotherfile.def"))));
            EXPECT_CALL(*game, doGamePath()).WillOnce(Return(IO::Path("")));
            EXPECT_CALL(*game, doFindEntityDefinitionFile(_, _)).WillOnce(Return(IO::Path("/someotherfile.def")));
            EXPECT_CALL(*game, doLoadEntityDefinitions(IO::Path("/someotherfile.def"))).WillOnce(Return(Assets::EntityDefinitionList()));
            
            EXPECT_CALL(*game, doFindBuiltinTextureCollections()).WillOnce(Return(IO::Path::List()));
            
            EXPECT_CALL(*game, doExtractExternalTextureCollections(map2)).WillOnce(Return(EmptyStringList));
            EXPECT_CALL(*game, doGamePath()).WillOnce(Return(IO::Path("Quake")));

            doc->openDocument(worldBounds, game, path1);
            
            Command::Ptr command = Command::Ptr(new OpenDocumentCommand(doc, worldBounds, game, path2));
            ASSERT_FALSE(command->undoable());
            ASSERT_TRUE(command->performDo());
            ASSERT_EQ(path2, doc->path());
            ASSERT_FALSE(doc->modified());
            ASSERT_EQ(map2, doc->map());
        }
    }
}
