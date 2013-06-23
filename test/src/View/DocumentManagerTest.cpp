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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "Exceptions.h"
#include "IO/Path.h"
#include "Model/QuakeGame.h"
#include "View/DocumentManager.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        TEST(DocumentManagerTest, SDINewDocument) {
            DocumentManager manager(true);
            Model::Game::Ptr game = Model::QuakeGame::newGame();

            MapDocument::Ptr document1 = manager.newDocument(game);
            ASSERT_TRUE(document1 != NULL);
            
            MapDocument::Ptr document2 = manager.newDocument(game);
            ASSERT_TRUE(document2 != NULL);
            
            const DocumentList& documents = manager.documents();
            ASSERT_EQ(1u, documents.size());
            ASSERT_EQ(document1, documents[0]);
            ASSERT_EQ(document2, documents[0]);
        }
        
        TEST(DocumentManagerTest, MDINewDocument) {
            DocumentManager manager(false);
            Model::Game::Ptr game = Model::QuakeGame::newGame();
            
            MapDocument::Ptr document1 = manager.newDocument(game);
            ASSERT_TRUE(document1 != NULL);
            
            MapDocument::Ptr document2 = manager.newDocument(game);
            ASSERT_TRUE(document2 != NULL);
            
            const DocumentList& documents = manager.documents();
            ASSERT_EQ(2u, documents.size());
            ASSERT_EQ(document1, documents[0]);
            ASSERT_EQ(document2, documents[1]);
        }
        
        TEST(DocumentManagerTest, SDIOpenDocument) {
            DocumentManager manager(true);
            Model::Game::Ptr game = Model::QuakeGame::newGame();

            const IO::Path path1("data/View/DocumentManager/TestDoc1.map");
            const IO::Path path2("data/View/DocumentManager/TestDoc2.map");
            
            MapDocument::Ptr document1 = manager.openDocument(game, path1);
            ASSERT_TRUE(document1 != NULL);
            ASSERT_EQ(path1, document1->path());
            
            MapDocument::Ptr document2 = manager.openDocument(game, path2);
            ASSERT_TRUE(document2 != NULL);
            ASSERT_EQ(path2, document2->path());
            
            const DocumentList& documents = manager.documents();
            ASSERT_EQ(1u, documents.size());
            ASSERT_EQ(document1, documents[0]);
            ASSERT_EQ(document2, documents[0]);
        }
        
        TEST(DocumentManagertest, MDIOpenDocument) {
            DocumentManager manager(false);
            Model::Game::Ptr game = Model::QuakeGame::newGame();

            const IO::Path path1("data/View/DocumentManager/TestDoc1.map");
            const IO::Path path2("data/View/DocumentManager/TestDoc2.map");
            
            MapDocument::Ptr document1 = manager.openDocument(game, path1);
            ASSERT_TRUE(document1 != NULL);
            ASSERT_EQ(path1, document1->path());
            
            MapDocument::Ptr document2 = manager.openDocument(game, path2);
            ASSERT_TRUE(document2 != NULL);
            ASSERT_EQ(path2, document2->path());
            
            const DocumentList& documents = manager.documents();
            ASSERT_EQ(2u, documents.size());
            ASSERT_EQ(document1, documents[0]);
            ASSERT_EQ(document2, documents[1]);
        }
        
        TEST(DocumentManagerTest, CloseDocument) {
            DocumentManager manager(false);
            Model::Game::Ptr game = Model::QuakeGame::newGame();

            MapDocument::Ptr unknownDocument = MapDocument::newMapDocument();
            MapDocument::Ptr knownDocument1 = manager.newDocument(game);
            MapDocument::Ptr knownDocument2 = manager.newDocument(game);
            
            ASSERT_THROW(manager.closeDocument(unknownDocument), DocumentManagerException);
            ASSERT_TRUE(manager.closeDocument(knownDocument1));

            const DocumentList& documents = manager.documents();
            ASSERT_EQ(1u, documents.size());
            ASSERT_EQ(knownDocument2, documents[0]);
        }
        
        TEST(DocumentManagerTest, CloseAllDocuments) {
            DocumentManager manager(false);
            Model::Game::Ptr game = Model::QuakeGame::newGame();
            
            MapDocument::Ptr document1 = manager.newDocument(game);
            MapDocument::Ptr document2 = manager.newDocument(game);

            manager.closeAllDocuments();
            ASSERT_TRUE(manager.documents().empty());
        }
    }
}
