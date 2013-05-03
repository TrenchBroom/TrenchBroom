/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef TrenchBroom_AddObjectsCommandTest_h
#define TrenchBroom_AddObjectsCommandTest_h

#include "TestSuite.h"
#include "Controller/AddObjectsCommand.h"
#include "Model/Entity.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Controller {
        class AddObjectsCommandTest : public TestSuite<AddObjectsCommandTest> {
        protected:
            void registerTestCases() {
                registerTestCase(&AddObjectsCommandTest::testAddEntity);
            }
        public:
            void testAddEntity() {
                Model::MapDocument document;
                document.OnCreate("", 0);
                
                Model::Entity entity(document.map().worldBounds());
                
                Controller::AddObjectsCommand* command = Controller::AddObjectsCommand::addEntity(document, entity);
                delete command;
            }
        };
    }
}

#endif
