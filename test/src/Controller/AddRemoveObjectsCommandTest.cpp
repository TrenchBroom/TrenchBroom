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
#include "Model/Layer.h"
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
            Model::Layer* layer = doc->map()->defaultLayer();
            const Model::ObjectParentList objects(1, Model::ObjectParentPair(entity));
            
            AddRemoveObjectsCommand::Ptr command = AddRemoveObjectsCommand::addObjects(doc, objects, layer);
            
            MockObserver1<const Model::ObjectList&> objectsWereAdded(doc->objectsWereAddedNotifier);
            MockObserver1<const Model::ObjectList&> objectsWillBeRemoved(doc->objectsWillBeRemovedNotifier);
            MockObserver1<const Model::ObjectParentList&> objectsWereRemoved(doc->objectsWereRemovedNotifier);
            
            objectsWereAdded.expect(Model::ObjectList(1, entity));
            ASSERT_TRUE(command->performDo());
            ASSERT_TRUE(VectorUtils::contains(doc->map()->entities(), entity));
            ASSERT_EQ(layer, entity->layer());
            ASSERT_TRUE(VectorUtils::contains(layer->objects(), static_cast<Model::Object*>(entity)));
            
            objectsWillBeRemoved.expect(Model::ObjectList(1, entity));
            objectsWereRemoved.expect(objects);
            ASSERT_TRUE(command->performUndo());
            ASSERT_FALSE(VectorUtils::contains(doc->map()->entities(), entity));
            ASSERT_EQ(NULL, entity->layer());
            ASSERT_FALSE(VectorUtils::contains(layer->objects(), static_cast<Model::Object*>(entity)));
        }

        TEST(AddRemoveObjectsCommandTest, addBrushesToWorldspawn) {
            const BBox3d worldBounds(8192.0);
            View::MapDocumentSPtr doc = makeDocument(worldBounds);
            doc->worldspawn(); // make sure worldspawn exists
            
            Model::Layer* layer = doc->map()->defaultLayer();
            
            const Model::BrushBuilder builder(doc->map(), worldBounds);
            Model::Brush* brush1 = builder.createCube(128.0, "someName");
            Model::Brush* brush2 = builder.createCube(64.0, "someName");
            Model::ObjectList addedObjects;
            addedObjects.push_back(brush1);
            addedObjects.push_back(brush2);
            
            Model::ObjectParentList objects;
            objects.push_back(Model::ObjectParentPair(brush1, doc->worldspawn()));
            objects.push_back(Model::ObjectParentPair(brush2, doc->worldspawn()));
            
            AddRemoveObjectsCommand::Ptr command = AddRemoveObjectsCommand::addObjects(doc, objects, layer);
            
            MockObserver1<const Model::ObjectList&> objectsWillChange(doc->objectsWillChangeNotifier);
            MockObserver1<const Model::ObjectList&> objectsDidChange(doc->objectsDidChangeNotifier);
            
            MockObserver1<const Model::ObjectList&> objectsWereAdded(doc->objectsWereAddedNotifier);
            MockObserver1<const Model::ObjectList&> objectsWillBeRemoved(doc->objectsWillBeRemovedNotifier);
            MockObserver1<const Model::ObjectParentList&> objectsWereRemoved(doc->objectsWereRemovedNotifier);

            objectsWillChange.expect(Model::ObjectList(1, doc->worldspawn()));
            objectsWereAdded.expect(addedObjects);
            objectsDidChange.expect(Model::ObjectList(1, doc->worldspawn()));
            ASSERT_TRUE(command->performDo());
            ASSERT_TRUE(VectorUtils::contains(doc->map()->worldspawn()->brushes(), brush1));
            ASSERT_TRUE(VectorUtils::contains(doc->map()->worldspawn()->brushes(), brush2));
            ASSERT_EQ(layer, brush1->layer());
            ASSERT_EQ(layer, brush2->layer());
            ASSERT_TRUE(VectorUtils::contains(layer->objects(), static_cast<Model::Object*>(brush1)));
            ASSERT_TRUE(VectorUtils::contains(layer->objects(), static_cast<Model::Object*>(brush2)));
            
            objectsWillChange.expect(Model::ObjectList(1, doc->worldspawn()));
            objectsWillBeRemoved.expect(addedObjects);
            objectsWereRemoved.expect(objects);
            objectsDidChange.expect(Model::ObjectList(1, doc->worldspawn()));
            ASSERT_TRUE(command->performUndo());
            ASSERT_FALSE(VectorUtils::contains(doc->map()->worldspawn()->brushes(), brush1));
            ASSERT_FALSE(VectorUtils::contains(doc->map()->worldspawn()->brushes(), brush2));
            ASSERT_EQ(NULL, brush1->layer());
            ASSERT_EQ(NULL, brush2->layer());
            ASSERT_FALSE(VectorUtils::contains(layer->objects(), static_cast<Model::Object*>(brush1)));
            ASSERT_FALSE(VectorUtils::contains(layer->objects(), static_cast<Model::Object*>(brush2)));
        }
        
        TEST(AddRemoveObjectsCommandTest, addBrushToEntity) {
            const BBox3d worldBounds(8192.0);
            View::MapDocumentSPtr doc = makeDocument(worldBounds);
            doc->worldspawn(); // make sure worldspawn exists

            Model::Layer* layer = doc->map()->createLayer("Test");
            doc->map()->addLayer(layer);
            
            Model::Entity* entity = doc->map()->createEntity();
            
            doc->addObject(entity);
            doc->objectsWereAddedNotifier(Model::ObjectList(1, entity));
            entity->setLayer(layer);

            const Model::BrushBuilder builder(doc->map(), worldBounds);
            Model::Brush* brush = builder.createCube(128.0, "someName");
            
            Model::ObjectParentList objects;
            objects.push_back(Model::ObjectParentPair(brush, entity));
            
            AddRemoveObjectsCommand::Ptr command = AddRemoveObjectsCommand::addObjects(doc, objects, layer);
            
            MockObserver1<const Model::ObjectList&> objectsWillChange(doc->objectsWillChangeNotifier);
            MockObserver1<const Model::ObjectList&> objectsDidChange(doc->objectsDidChangeNotifier);

            MockObserver1<const Model::ObjectList&> objectsWereAdded(doc->objectsWereAddedNotifier);
            MockObserver1<const Model::ObjectList&> objectsWillBeRemoved(doc->objectsWillBeRemovedNotifier);
            MockObserver1<const Model::ObjectParentList&> objectsWereRemoved(doc->objectsWereRemovedNotifier);
            
            objectsWillChange.expect(Model::ObjectList(1, entity));
            objectsWereAdded.expect(Model::ObjectList(1, brush));
            objectsDidChange.expect(Model::ObjectList(1, entity));
            ASSERT_TRUE(command->performDo());
            ASSERT_TRUE(VectorUtils::contains(entity->brushes(), brush));
            ASSERT_EQ(layer, brush->layer());
            ASSERT_TRUE(VectorUtils::contains(layer->objects(), static_cast<Model::Object*>(brush)));
            
            objectsWillChange.expect(Model::ObjectList(1, entity));
            objectsWillBeRemoved.expect(Model::ObjectList(1, brush));
            objectsWereRemoved.expect(objects);
            objectsDidChange.expect(Model::ObjectList(1, entity));
            ASSERT_TRUE(command->performUndo());
            ASSERT_FALSE(VectorUtils::contains(entity->brushes(), brush));
            ASSERT_EQ(NULL, brush->layer());
            ASSERT_FALSE(VectorUtils::contains(layer->objects(), static_cast<Model::Object*>(brush)));
        }
    }
}
