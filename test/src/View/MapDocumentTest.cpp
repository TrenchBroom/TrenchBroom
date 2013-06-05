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

#include "IO/Path.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        TEST(MapDocumentTest, newDocumentCreatesFrame) {
            MapDocument::Ptr document = MapDocument::newMapDocument();
            document->newDocument();
            ASSERT_EQ(IO::Path(""), document->path());
            ASSERT_EQ(String(""), document->filename());
            ASSERT_TRUE(document->frame() != NULL);
        }
        
        TEST(MapDocumentTest, openDocumentCreatesFrame) {
            MapDocument::Ptr document = MapDocument::newMapDocument();
            document->openDocument(IO::Path("/test/blah/hey.map"));
            ASSERT_EQ(IO::Path("/test/blah/hey.map"), document->path());
            ASSERT_EQ(String("hey.map"), document->filename());
            ASSERT_TRUE(document->frame() != NULL);
        }
    }
}

