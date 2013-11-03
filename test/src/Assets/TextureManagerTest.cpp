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
        class TestTextureCollection : public TextureCollection {
        private:
            bool& m_deleted;
        public:
            TestTextureCollection(const String& name, const TextureList& textures, bool& deleted) :
            TextureCollection(name, textures),
            m_deleted(deleted) {
                m_deleted = false;
            }
            
            ~TestTextureCollection() {
                m_deleted = true;
            }
        };
        
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
        
        TEST(TextureManagerTest, addExistingTextureCollections) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            Model::MockGamePtr game = Model::MockGame::newGame();
            TextureManager textureManager;
            textureManager.reset(game);
            
            TextureCollectionList collections;
            collections.push_back(new TextureCollection("name1", TextureList()));
            collections.push_back(new TextureCollection("name2", TextureList()));
            collections.push_back(new TextureCollection("name3", TextureList()));
            
            IO::Path::List paths;
            paths.push_back(IO::Path("./some_collection1.wad"));
            paths.push_back(IO::Path("./some_collection2.wad"));
            paths.push_back(IO::Path("./some_collection3.wad"));
            
            for (size_t i = 0; i < 3; ++i)
                EXPECT_CALL(*game, doLoadTextureCollection(paths[i])).WillOnce(Return(collections[i]));
            
            textureManager.addTextureCollections(paths);
            
            const TextureCollectionList& managerCollections = textureManager.collections();
            ASSERT_EQ(3u, managerCollections.size());
            for (size_t i = 0; i < 3; ++i)
                ASSERT_EQ(collections[i], managerCollections[i]);
        }
        
        TEST(TextureManagerTest, addNonExistingTextureCollections) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            Model::MockGamePtr game = Model::MockGame::newGame();
            TextureManager textureManager;
            textureManager.reset(game);
            
            bool deleted[2];
            TextureCollectionList collections;
            collections.push_back(new TestTextureCollection("name1", TextureList(), deleted[0]));
            collections.push_back(new TestTextureCollection("name3", TextureList(), deleted[1]));
            
            IO::Path::List paths;
            paths.push_back(IO::Path("./some_collection1.wad"));
            paths.push_back(IO::Path("./some_collection2.wad"));
            paths.push_back(IO::Path("./some_collection3.wad"));

            EXPECT_CALL(*game, doLoadTextureCollection(paths[0])).WillOnce(Return(collections[0]));
            EXPECT_CALL(*game, doLoadTextureCollection(paths[1])).WillOnce(Throw(FileSystemException("")));

            ASSERT_THROW(textureManager.addTextureCollections(paths), FileSystemException);
            ASSERT_TRUE(textureManager.collections().empty());
            ASSERT_TRUE(deleted[0]);
            ASSERT_FALSE(deleted[1]); // because it has not been constructed when the exception is thrown
            
            delete collections[1];
        }

        TEST(TextureManagerTest, removeTextureCollection) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            Model::MockGamePtr game = Model::MockGame::newGame();
            TextureManager textureManager;
            textureManager.reset(game);
            
            TextureCollectionList collections;
            collections.push_back(new TextureCollection("name1", TextureList()));
            collections.push_back(new TextureCollection("name2", TextureList()));
            collections.push_back(new TextureCollection("name3", TextureList()));
            
            IO::Path::List paths;
            paths.push_back(IO::Path("./some_collection1.wad"));
            paths.push_back(IO::Path("./some_collection2.wad"));
            paths.push_back(IO::Path("./some_collection3.wad"));
            
            for (size_t i = 0; i < 3; ++i)
                EXPECT_CALL(*game, doLoadTextureCollection(paths[i])).WillOnce(Return(collections[i]));
            
            textureManager.addTextureCollections(paths);
            
            ASSERT_THROW(textureManager.removeTextureCollection(IO::Path("does_not_exist")), AssetException);
            
            textureManager.removeTextureCollection(paths[1]);
            
            const TextureCollectionList& managerCollections = textureManager.collections();
            ASSERT_EQ(2u, managerCollections.size());
            ASSERT_EQ(collections[0], managerCollections[0]);
            ASSERT_EQ(collections[2], managerCollections[1]);
        }

        TEST(TextureManagerTest, reset) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            Model::MockGamePtr game = Model::MockGame::newGame();
            TextureManager textureManager;
            textureManager.reset(game);
            
            TextureCollectionList collections;
            collections.push_back(new TextureCollection("name1", TextureList()));
            collections.push_back(new TextureCollection("name2", TextureList()));
            collections.push_back(new TextureCollection("name3", TextureList()));
            
            IO::Path::List paths;
            paths.push_back(IO::Path("./some_collection1.wad"));
            paths.push_back(IO::Path("./some_collection2.wad"));
            paths.push_back(IO::Path("./some_collection3.wad"));
            
            for (size_t i = 0; i < 3; ++i)
                EXPECT_CALL(*game, doLoadTextureCollection(paths[i])).WillOnce(Return(collections[i]));
            
            textureManager.addTextureCollections(paths);
            textureManager.reset(game);
            ASSERT_TRUE(textureManager.collections().empty());
        }
    }
}
