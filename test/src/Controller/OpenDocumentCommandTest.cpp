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

#include "Controller/OpenDocumentCommand.h"
#include "IO/Path.h"
#include "Model/Entity.h"
#include "Model/Map.h"
#include "Model/QuakeGame.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        TEST(OpenDocumentCommandTest, openDocumentInEmptyDocument) {
            View::MapDocument::Ptr doc = View::MapDocument::newMapDocument();
            Model::Game::Ptr game = Model::QuakeGame::newGame();
            const IO::Path path("data/Controller/OpenDocumentCommandTest/Cube.map");
            
            Command::Ptr command = Command::Ptr(new OpenDocumentCommand(doc, game, path));
            ASSERT_FALSE(command->undoable());
            ASSERT_TRUE(command->performDo());
            ASSERT_EQ(path, doc->path());
            ASSERT_FALSE(doc->modified());

            Model::Map::Ptr map = doc->map();
            ASSERT_EQ(1u, map->entities().size());
            
            Model::Entity::Ptr entity = map->worldspawn();
            ASSERT_TRUE(entity != NULL);
            ASSERT_EQ(1u, entity->brushes().size());
        }

        TEST(OpenDocumentCommandTest, openDocumentInExistingDocument) {
            View::MapDocument::Ptr doc = View::MapDocument::newMapDocument();
            Model::Game::Ptr game = Model::QuakeGame::newGame();
            const IO::Path path("data/Controller/OpenDocumentCommandTest/2Cubes.map");

            doc->openDocument(game, IO::Path("data/Controller/OpenDocumentCommandTest/Cube.map"));
            
            Command::Ptr command = Command::Ptr(new OpenDocumentCommand(doc, game, path));
            ASSERT_FALSE(command->undoable());
            ASSERT_TRUE(command->performDo());
            ASSERT_EQ(path, doc->path());
            ASSERT_FALSE(doc->modified());
            
            Model::Map::Ptr map = doc->map();
            ASSERT_EQ(1u, map->entities().size());
            
            Model::Entity::Ptr entity = map->worldspawn();
            ASSERT_TRUE(entity != NULL);
            ASSERT_EQ(2u, entity->brushes().size());
        }
    }
}
