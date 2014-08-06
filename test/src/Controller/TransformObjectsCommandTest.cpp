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
#include "TestUtils.h"
#include "Controller/TransformObjectsCommand.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/Map.h"
#include "Model/ModelTypes.h"
#include "Model/Object.h"
#include "Model/SelectionResult.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        TEST(TransformObjectsCommandTest, transformBrush) {
            const BBox3d worldBounds(8192.0);
            View::MapDocumentSPtr doc = makeDocument(worldBounds);
            
            const Vec3 offset(1.0, 2.0, 3.0);
            
            const Model::BrushBuilder builder(doc->map(), worldBounds);
            Model::Brush* brush = builder.createCube(128.0, "someName");
            ASSERT_EQ(Vec3::Null, brush->bounds().center());
            
            doc->addObject(brush);
            doc->objectsWereAddedNotifier(Model::ObjectList(1, brush));
            
            Model::ObjectList objects(1, brush);
            
            TransformObjectsCommand::Ptr command = TransformObjectsCommand::translateObjects(doc, offset, true, objects);
            
            MockObserver1<const Model::ObjectList&> objectsWillChange(doc->objectsWillChangeNotifier);
            MockObserver1<const Model::ObjectList&> objectsDidChange(doc->objectsDidChangeNotifier);

            Model::ObjectList changedObjects;
            changedObjects.push_back(brush->parent());
            changedObjects.push_back(brush);
            
            objectsWillChange.expect(changedObjects);
            objectsDidChange.expect(changedObjects);

            ASSERT_TRUE(command->performDo());
            ASSERT_EQ(offset, brush->bounds().center());

            objectsWillChange.expect(changedObjects);
            objectsDidChange.expect(changedObjects);

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
            doc->objectsWereAddedNotifier(Model::ObjectList(1, brush));
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
