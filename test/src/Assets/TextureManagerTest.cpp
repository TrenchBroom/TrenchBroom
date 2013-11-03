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

#include "TestUtils.h"
#include "Exceptions.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureManager.h"
#include "Model/MockGame.h"

namespace TrenchBroom {
    namespace Assets {
        TEST(TextureManagerTest, addNonExistingTextureCollection) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;

            Model::MockGamePtr game = Model::MockGame::newGame();
            TextureManager textureManager;
            textureManager.reset(game);
            
            const IO::Path path("./_does_not_exist.wad");
            EXPECT_CALL(*game, doLoadTextureCollection(path)).WillOnce(Throw(FileSystemException("")));
            
            ASSERT_THROW(textureManager.addTextureCollection(path), FileSystemException);
        }
        
        TEST(TextureCollectionTest, addExistingTextureCollection) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            Model::MockGamePtr game = Model::MockGame::newGame();
            TextureManager textureManager;
            textureManager.reset(game);

            TextureCollection* collection = new TextureCollection("name", TextureList());
            
            const IO::Path path("./some_collection.wad");
            EXPECT_CALL(*game, doLoadTextureCollection(path)).WillOnce(Return(collection));
            
            textureManager.addTextureCollection(path);
            const TextureCollectionList& collections = textureManager.collections();
            ASSERT_EQ(1u, collections.size());
            ASSERT_EQ(collection, collections.front());
        }
    }
}
