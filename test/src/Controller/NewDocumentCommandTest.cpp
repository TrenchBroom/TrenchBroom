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

#include "Controller/NewDocumentCommand.h"
#include "IO/Path.h"
#include "Model/Map.h"
#include "Model/QuakeGame.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        TEST(NewDocumentCommandTest, newDocumentInEmptyDocument) {
            View::MapDocument::Ptr doc = View::MapDocument::newMapDocument();
            Model::Game::Ptr game = Model::QuakeGame::newGame();
            
            Command::Ptr command = Command::Ptr(new NewDocumentCommand(doc, game));
            ASSERT_FALSE(command->undoable());
            ASSERT_TRUE(command->performDo());
            ASSERT_EQ(IO::Path("unnamed.map"), doc->path());
            ASSERT_FALSE(doc->modified());
        }

        TEST(NewDocumentCommandTest, newDocumentInExistingDocument) {
            View::MapDocument::Ptr doc = View::MapDocument::newMapDocument();
            Model::Game::Ptr game = Model::QuakeGame::newGame();
            doc->openDocument(game, IO::Path("data/Controller/NewDocumentCommandTest/Cube.map"));
            
            Command::Ptr command = Command::Ptr(new NewDocumentCommand(doc, game));
            ASSERT_FALSE(command->undoable());
            ASSERT_TRUE(command->performDo());
            ASSERT_EQ(IO::Path("unnamed.map"), doc->path());
            ASSERT_FALSE(doc->modified());
            
            Model::Map::Ptr map = doc->map();
            ASSERT_TRUE(map->entities().empty());
            ASSERT_TRUE(map->worldspawn() == NULL);
        }
    }
}
