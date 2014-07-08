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

#include "CollectionUtils.h"
#include "VecMath.h"
#include "MockObserver.h"
#include "TestUtils.h"
#include "Controller/AddRemoveObjectsCommand.h"
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
        TEST(AddRemoveObjectsCommandTest, addEntity) {
            const BBox3d worldBounds(8192.0);
            View::MapDocumentSPtr doc = makeDocument(worldBounds);
            
            Model::Entity* entity = doc->map()->createEntity();
            const Model::ObjectParentList objects(1, Model::ObjectParentPair(entity));
            
            AddRemoveObjectsCommand::Ptr command = AddRemoveObjectsCommand::addObjects(doc, objects);
            
            MockObserver1<Model::Object*> objectWasAdded(doc->objectWasAddedNotifier);
            MockObserver1<Model::Object*> objectWillBeRemoved(doc->objectWillBeRemovedNotifier);
            MockObserver1<Model::Object*> objectWasRemoved(doc->objectWasRemovedNotifier);
            
            objectWasAdded.expect(entity);
            ASSERT_TRUE(command->performDo());
            ASSERT_TRUE(VectorUtils::contains(doc->map()->entities(), entity));
            
            objectWillBeRemoved.expect(entity);
            objectWasRemoved.expect(entity);
            ASSERT_TRUE(command->performUndo());
            ASSERT_FALSE(VectorUtils::contains(doc->map()->entities(), entity));
        }

        TEST(AddRemoveObjectsCommandTest, addBrushesToWorldspawn) {
            const BBox3d worldBounds(8192.0);
            View::MapDocumentSPtr doc = makeDocument(worldBounds);
            doc->worldspawn(); // make sure worldspawn exists
            
            const Model::BrushBuilder builder(doc->map(), worldBounds);
            Model::Brush* brush1 = builder.createCube(128.0, "someName");
            Model::Brush* brush2 = builder.createCube(64.0, "someName");

            Model::ObjectParentList objects;
            objects.push_back(Model::ObjectParentPair(brush1, doc->worldspawn()));
            objects.push_back(Model::ObjectParentPair(brush2, doc->worldspawn()));
            
            AddRemoveObjectsCommand::Ptr command = AddRemoveObjectsCommand::addObjects(doc, objects);
            
            MockObserver1<Model::Object*> objectWillChange(doc->objectWillChangeNotifier);
            MockObserver1<Model::Object*> objectDidChange(doc->objectDidChangeNotifier);
            
            MockObserver1<Model::Object*> objectWasAdded(doc->objectWasAddedNotifier);
            MockObserver1<Model::Object*> objectWillBeRemoved(doc->objectWillBeRemovedNotifier);
            MockObserver1<Model::Object*> objectWasRemoved(doc->objectWasRemovedNotifier);

            objectWillChange.expect(doc->worldspawn());
            objectWasAdded.expect(brush1);
            objectWasAdded.expect(brush2);
            objectDidChange.expect(doc->worldspawn());
            ASSERT_TRUE(command->performDo());
            ASSERT_TRUE(VectorUtils::contains(doc->map()->worldspawn()->brushes(), brush1));
            ASSERT_TRUE(VectorUtils::contains(doc->map()->worldspawn()->brushes(), brush2));
            
            objectWillChange.expect(doc->worldspawn());
            objectWillBeRemoved.expect(brush1);
            objectWillBeRemoved.expect(brush2);
            objectWasRemoved.expect(brush1);
            objectWasRemoved.expect(brush2);
            objectDidChange.expect(doc->worldspawn());
            ASSERT_TRUE(command->performUndo());
            ASSERT_FALSE(VectorUtils::contains(doc->map()->worldspawn()->brushes(), brush1));
            ASSERT_FALSE(VectorUtils::contains(doc->map()->worldspawn()->brushes(), brush2));
        }
        
        TEST(AddRemoveObjectsCommandTest, addBrushToEntity) {
            const BBox3d worldBounds(8192.0);
            View::MapDocumentSPtr doc = makeDocument(worldBounds);
            doc->worldspawn(); // make sure worldspawn exists

            Model::Entity* entity = doc->map()->createEntity();
            doc->addObject(entity);
            doc->objectWasAddedNotifier(entity);

            const Model::BrushBuilder builder(doc->map(), worldBounds);
            Model::Brush* brush = builder.createCube(128.0, "someName");
            
            Model::ObjectParentList objects;
            objects.push_back(Model::ObjectParentPair(brush, entity));
            
            AddRemoveObjectsCommand::Ptr command = AddRemoveObjectsCommand::addObjects(doc, objects);
            
            MockObserver1<Model::Object*> objectWillChange(doc->objectWillChangeNotifier);
            MockObserver1<Model::Object*> objectDidChange(doc->objectDidChangeNotifier);

            MockObserver1<Model::Object*> objectWasAdded(doc->objectWasAddedNotifier);
            MockObserver1<Model::Object*> objectWillBeRemoved(doc->objectWillBeRemovedNotifier);
            MockObserver1<Model::Object*> objectWasRemoved(doc->objectWasRemovedNotifier);
            
            objectWillChange.expect(entity);
            objectWasAdded.expect(brush);
            objectDidChange.expect(entity);
            ASSERT_TRUE(command->performDo());
            ASSERT_TRUE(VectorUtils::contains(entity->brushes(), brush));
            
            objectWillChange.expect(entity);
            objectWillBeRemoved.expect(brush);
            objectWasRemoved.expect(brush);
            objectDidChange.expect(entity);
            ASSERT_TRUE(command->performUndo());
            ASSERT_FALSE(VectorUtils::contains(entity->brushes(), brush));
        }
    }
}
