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

#include "MockObserver.h"
#include "VecMath.h"
#include "Controller/TransformObjectsCommand.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/Map.h"
#include "Model/ModelTypes.h"
#include "Model/MockGame.h"
#include "Model/Object.h"
#include "Model/SelectionResult.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        static View::MapDocumentSPtr makeDocument(const BBox3d& worldBounds) {
            using namespace testing;
            
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
            return doc;
        }
        
        TEST(TransformObjectsCommandTest, transformBrush) {
            const BBox3d worldBounds(8192.0);
            View::MapDocumentSPtr doc = makeDocument(worldBounds);
            
            const Vec3 offset(1.0, 2.0, 3.0);
            
            const Model::BrushBuilder builder(doc->map(), worldBounds);
            Model::Brush* brush = builder.createCube(128.0, "someName");
            ASSERT_EQ(Vec3::Null, brush->bounds().center());
            
            doc->addObject(brush);
            doc->objectWasAddedNotifier(brush);
            
            Model::ObjectList objects(1, brush);
            
            TransformObjectsCommand::Ptr command = TransformObjectsCommand::translateObjects(doc, offset, true, objects);
            
            MockObserver1<Model::Object*> objectWillChange(doc->objectWillChangeNotifier);
            MockObserver1<Model::Object*> objectDidChange(doc->objectDidChangeNotifier);

            objectWillChange.expect(brush->parent());
            objectWillChange.expect(brush);
            objectDidChange.expect(brush->parent());
            objectDidChange.expect(brush);

            ASSERT_TRUE(command->performDo());
            ASSERT_EQ(offset, brush->bounds().center());

            objectWillChange.expect(brush->parent());
            objectWillChange.expect(brush);
            objectDidChange.expect(brush->parent());
            objectDidChange.expect(brush);

            ASSERT_TRUE(command->performUndo());
            ASSERT_EQ(Vec3::Null, brush->bounds().center());
        }
        
        TEST(TransformObjectsCommandTest, collateWith) {
            const BBox3d worldBounds(8192.0);
            View::MapDocumentSPtr doc = makeDocument(worldBounds);
            
            const Model::BrushBuilder builder(doc->map(), worldBounds);
            Model::Brush* brush = builder.createCube(128.0, "someName");
            ASSERT_EQ(Vec3::Null, brush->bounds().center());
            
            doc->addObject(brush);
            doc->objectWasAddedNotifier(brush);
            Model::ObjectList objects(1, brush);
            
            TransformObjectsCommand::Ptr translate1 = TransformObjectsCommand::translateObjects(doc, Vec3::PosX, true, objects);
            TransformObjectsCommand::Ptr translate2 = TransformObjectsCommand::translateObjects(doc, Vec3::PosY, true, objects);
            TransformObjectsCommand::Ptr rotate1    = TransformObjectsCommand::rotateObjects(doc, Vec3::Null, Vec3::PosZ, Math::radians(10.0), true, objects);
            TransformObjectsCommand::Ptr rotate2    = TransformObjectsCommand::rotateObjects(doc, Vec3::Null, Vec3::PosY, Math::radians(12.0), true, objects);
            TransformObjectsCommand::Ptr flip1      = TransformObjectsCommand::flipObjects(doc, Vec3::Null, Math::Axis::AX, true, objects);
            TransformObjectsCommand::Ptr flip2      = TransformObjectsCommand::flipObjects(doc, Vec3::Null, Math::Axis::AY, true, objects);
            
            ASSERT_FALSE(translate1->collateWith(rotate1));
            ASSERT_FALSE(translate1->collateWith(flip1));
            ASSERT_FALSE(rotate1->collateWith(translate1));
            ASSERT_FALSE(rotate1->collateWith(flip1));
            ASSERT_FALSE(flip1->collateWith(translate1));
            ASSERT_FALSE(flip1->collateWith(rotate1));
            
            ASSERT_TRUE(translate1->collateWith(translate2));
            ASSERT_TRUE(rotate1->collateWith(rotate2));
            ASSERT_TRUE(flip1->collateWith(flip2));
        }
    }
}
