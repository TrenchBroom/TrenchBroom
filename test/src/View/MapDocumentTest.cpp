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
#include "Model/QuakeGame.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"

#include <wx/frame.h>

namespace TrenchBroom {
    namespace View {
        TEST(MapDocumentTest, newDocument) {
            MapDocument::Ptr document = MapDocument::newMapDocument();
            Model::Game::Ptr game = Model::QuakeGame::newGame();
            document->newDocument(game);
            
            ASSERT_EQ(IO::Path(""), document->path());
            ASSERT_EQ(String(""), document->filename());
        }
        
        TEST(MapDocumentTest, openDocument) {
            MapDocument::Ptr document = MapDocument::newMapDocument();
            Model::Game::Ptr game = Model::QuakeGame::newGame();
            document->openDocument(game, IO::Path("data/View/DocumentManager/TestDoc1.map"));
            
            ASSERT_EQ(IO::Path("data/View/DocumentManager/TestDoc1.map"), document->path());
            ASSERT_EQ(String("TestDoc1.map"), document->filename());
        }
    }
}

