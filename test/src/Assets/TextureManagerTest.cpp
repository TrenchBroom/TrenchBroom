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

#include "TestUtils.h"
#include "Exceptions.h"
#include "Color.h"
#include "Assets/AssetTypes.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureCollectionSpec.h"
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
            
            const TextureCollectionSpec spec("somename.wad", IO::Path("./_does_not_exist.wad"));
            EXPECT_CALL(*game, doLoadTextureCollection(spec)).WillOnce(Throw(FileSystemException("")));
            
            ASSERT_FALSE(textureManager.addExternalTextureCollection(spec));
            const TextureCollectionList& collections = textureManager.collections();
            ASSERT_EQ(1u, collections.size());
            ASSERT_FALSE(collections.front()->loaded());
        }
        
        TEST(TextureCollectionTest, addExistingTextureCollection) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            Model::MockGamePtr game = Model::MockGame::newGame();
            TextureManager textureManager;
            textureManager.reset(game);

            TextureCollection* collection = new TextureCollection("somename.wad", TextureList());
            
            const TextureCollectionSpec spec("somename.wad", IO::Path("./does_exist.wad"));
            EXPECT_CALL(*game, doLoadTextureCollection(spec)).WillOnce(Return(collection));
            
            ASSERT_TRUE(textureManager.addExternalTextureCollection(spec));
            const TextureCollectionList& collections = textureManager.collections();
            ASSERT_EQ(1u, collections.size());
            ASSERT_EQ(collection, collections.front());
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
            
            typedef std::vector<TextureCollectionSpec> SpecList;
            SpecList specs;
            specs.push_back(TextureCollectionSpec("name1", IO::Path("./coll1.wad")));
            specs.push_back(TextureCollectionSpec("name2", IO::Path("./coll2.wad")));
            specs.push_back(TextureCollectionSpec("name3", IO::Path("./coll3.wad")));
            
            for (size_t i = 0; i < 3; ++i)
                EXPECT_CALL(*game, doLoadTextureCollection(specs[i])).WillOnce(Return(collections[i]));
            for (size_t i = 0; i < 3; ++i)
                textureManager.addExternalTextureCollection(specs[i]);
                
            ASSERT_THROW(textureManager.removeExternalTextureCollection("does_not_exist"), AssetException);
            
            textureManager.removeExternalTextureCollection(specs[1].name());
            
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
            
            typedef std::vector<TextureCollectionSpec> SpecList;
            SpecList specs;
            specs.push_back(TextureCollectionSpec("name1", IO::Path("./coll1.wad")));
            specs.push_back(TextureCollectionSpec("name2", IO::Path("./coll2.wad")));
            specs.push_back(TextureCollectionSpec("name3", IO::Path("./coll3.wad")));
            
            for (size_t i = 0; i < 3; ++i)
                EXPECT_CALL(*game, doLoadTextureCollection(specs[i])).WillOnce(Return(collections[i]));
            for (size_t i = 0; i < 3; ++i)
                textureManager.addExternalTextureCollection(specs[i]);
            
            textureManager.reset(game);
            ASSERT_TRUE(textureManager.collections().empty());
        }
        
        TEST(TextureManagerTest, texture) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            TextureList textures1;
            textures1.push_back(new Texture("t1",  64,  64, Color(), TextureBuffer( 64* 64*3)));
            textures1.push_back(new Texture("t2", 128, 128, Color(), TextureBuffer(128*128*3)));
            TextureCollection* collection1 = new TextureCollection("c1", textures1);
            const TextureCollectionSpec spec1("name1", IO::Path("asdf"));
            
            TextureList textures2;
            textures2.push_back(new Texture("t2",  32,  32, Color(), TextureBuffer( 32* 32*3)));
            textures2.push_back(new Texture("t3", 128, 128, Color(), TextureBuffer(128*128*3)));
            TextureCollection* collection2 = new TextureCollection("c2", textures2);
            const TextureCollectionSpec spec2("name2", IO::Path("fsda"));
            
            Model::MockGamePtr game = Model::MockGame::newGame();
            TextureManager textureManager;
            textureManager.reset(game);
            
            EXPECT_CALL(*game, doLoadTextureCollection(spec1)).WillOnce(Return(collection1));
            EXPECT_CALL(*game, doLoadTextureCollection(spec2)).WillOnce(Return(collection2));
            
            textureManager.addExternalTextureCollection(spec1);
            
            ASSERT_TRUE(textureManager.texture("t1") == textures1[0]);
            ASSERT_TRUE(textureManager.texture("t2") == textures1[1]);
            
            textureManager.addExternalTextureCollection(spec2);
            ASSERT_TRUE(textureManager.texture("t1") == textures1[0]);
            ASSERT_TRUE(textureManager.texture("t2") == textures2[0]);
            ASSERT_TRUE(textureManager.texture("t3") == textures2[1]);
            
            ASSERT_FALSE(textures1[0]->overridden());
            ASSERT_TRUE( textures1[1]->overridden());
            ASSERT_FALSE(textures2[0]->overridden());
            ASSERT_FALSE(textures2[1]->overridden());
        }
    }
}
