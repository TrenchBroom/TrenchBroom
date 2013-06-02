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
#include "View/DocumentManager.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        TEST(DocumentManagerTest, SDINewDocument) {
            DocumentManager manager(true);

            MapDocumentPtr document1 = manager.newDocument();
            ASSERT_TRUE(document1 != NULL);
            
            MapDocumentPtr document2 = manager.newDocument();
            ASSERT_TRUE(document2 != NULL);
            
            const DocumentList& documents = manager.documents();
            ASSERT_EQ(1, documents.size());
            ASSERT_EQ(document1, documents[0]);
            ASSERT_EQ(document2, documents[0]);
        }
        
        TEST(DocumentManagerTest, MDINewDocument) {
            DocumentManager manager(false);
            
            MapDocumentPtr document1 = manager.newDocument();
            ASSERT_TRUE(document1 != NULL);
            
            MapDocumentPtr document2 = manager.newDocument();
            ASSERT_TRUE(document2 != NULL);
            
            const DocumentList& documents = manager.documents();
            ASSERT_EQ(2, documents.size());
            ASSERT_EQ(document1, documents[0]);
            ASSERT_EQ(document2, documents[1]);
        }
        
        TEST(DocumentManagerTest, SDIOpenDocument) {
            DocumentManager manager(true);
            const IO::Path path1("data/View/DocumentManager/TestDoc1.map");
            const IO::Path path2("data/View/DocumentManager/TestDoc2.map");
            
            MapDocumentPtr document1 = manager.openDocument(path1);
            ASSERT_TRUE(document1 != NULL);
            ASSERT_EQ(path1, document1->path());
            
            MapDocumentPtr document2 = manager.openDocument(path2);
            ASSERT_TRUE(document2 != NULL);
            ASSERT_EQ(path2, document2->path());
            
            const DocumentList& documents = manager.documents();
            ASSERT_EQ(1, documents.size());
            ASSERT_EQ(document1, documents[0]);
            ASSERT_EQ(document2, documents[0]);
        }
        
        TEST(DocumentManagertest, MDIOpenDocument) {
            DocumentManager manager(false);
            const IO::Path path1("data/View/DocumentManager/TestDoc1.map");
            const IO::Path path2("data/View/DocumentManager/TestDoc2.map");
            
            MapDocumentPtr document1 = manager.openDocument(path1);
            ASSERT_TRUE(document1 != NULL);
            ASSERT_EQ(path1, document1->path());
            
            MapDocumentPtr document2 = manager.openDocument(path2);
            ASSERT_TRUE(document2 != NULL);
            ASSERT_EQ(path2, document2->path());
            
            const DocumentList& documents = manager.documents();
            ASSERT_EQ(2, documents.size());
            ASSERT_EQ(document1, documents[0]);
            ASSERT_EQ(document2, documents[1]);
        }
        
        TEST(DocumentManagerTest, CloseDocument) {
            DocumentManager manager(false);
            MapDocumentPtr unknownDocument = MapDocument::newMapDocument();
            MapDocumentPtr knownDocument1 = manager.newDocument();
            MapDocumentPtr knownDocument2 = manager.newDocument();
            
            ASSERT_THROW(manager.closeDocument(unknownDocument), DocumentManagerException);
            ASSERT_TRUE(manager.closeDocument(knownDocument1));

            const DocumentList& documents = manager.documents();
            ASSERT_EQ(1, documents.size());
            ASSERT_EQ(knownDocument2, documents[0]);
        }
        
        TEST(DocumentManagerTest, CloseAllDocuments) {
            DocumentManager manager(false);
            
            MapDocumentPtr document1 = manager.newDocument();
            MapDocumentPtr document2 = manager.newDocument();

            manager.closeAllDocuments();
            ASSERT_TRUE(manager.documents().empty());
        }
    }
}
