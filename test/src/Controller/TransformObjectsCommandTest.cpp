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
#include <gmock/gmock.h>

#include "VecMath.h"
#include "Controller/TransformObjectsCommand.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/Map.h"
#include "Model/ModelTypes.h"
#include "Model/MockGame.h"
#include "Model/SelectionResult.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        TEST(TransformObjectsCommandTest, transformBrush) {
            using namespace testing;
            
            const BBox3d worldBounds(-8192.0, 8192.0);
            Model::MockGamePtr game = Model::MockGame::newGame();
            const Model::MapFormat::Type mapFormat = Model::MapFormat::Quake;
            
            EXPECT_CALL(*game, doNewMap(Model::MapFormat::Quake)).WillOnce(Return(new Model::Map(Model::MapFormat::Quake)));
            const Model::GameConfig::FlagsConfig contentFlags;
            EXPECT_CALL(*game, doContentFlags()).WillOnce(ReturnRef(contentFlags));
            EXPECT_CALL(*game, doExtractEntityDefinitionFile(_)).WillOnce(Return(Model::EntityDefinitionFileSpec::external(IO::Path("/somefile.def"))));
            EXPECT_CALL(*game, doGamePath()).WillOnce(Return(IO::Path("")));
            EXPECT_CALL(*game, doFindEntityDefinitionFile(_, _)).WillOnce(Return(IO::Path("/somefile.def")));
            EXPECT_CALL(*game, doLoadEntityDefinitions(IO::Path("/somefile.def"))).WillOnce(Return(Assets::EntityDefinitionList()));
            EXPECT_CALL(*game, doFindBuiltinTextureCollections()).WillOnce(Return(IO::Path::List()));
            
            View::MapDocumentSPtr doc = View::MapDocument::newMapDocument();
            doc->newDocument(worldBounds, game, mapFormat);
            
            const Mat4x4d transformation = translationMatrix(Vec3(1.0, 2.0, 3.0));
            
            const Model::BrushBuilder builder(doc->map(), worldBounds);
            Model::Brush* brush = builder.createCube(128.0, "someName");
            ASSERT_EQ(Vec3::Null, brush->bounds().center());
            
            doc->addObject(brush);
            doc->objectWasAddedNotifier(brush);
            
            Model::ObjectList objects(1, brush);
            
            TransformObjectsCommand::Ptr command = TransformObjectsCommand::transformObjects(doc, transformation, true, "Translate", objects);
            
            ASSERT_TRUE(command->performDo());
            ASSERT_EQ(Vec3(1.0, 2.0, 3.0), brush->bounds().center());
        }
    }
}
