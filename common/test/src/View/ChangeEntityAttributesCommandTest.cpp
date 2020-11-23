/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Color.h"
#include "Assets/EntityDefinition.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

#include <vecmath/bbox.h>

#include <vector>

#include "Catch2.h"
#include "GTestCompat.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace View {
        class ChangeEntityAttributesCommandTest : public MapDocumentTest {
        public:
            ChangeEntityAttributesCommandTest() :
            MapDocumentTest(Model::MapFormat::Valve) {}
        };

        TEST_CASE_METHOD(ChangeEntityAttributesCommandTest, "ChangeEntityAttributesCommandTest.changeClassname") {
            // need to recreate these because document->setEntityDefinitions will delete the old ones
            m_pointEntityDef = new Assets::PointEntityDefinition("point_entity", Color(), vm::bbox3(16.0), "this is a point entity", {}, {});

            Assets::PointEntityDefinition* largeEntityDef = new Assets::PointEntityDefinition("large_entity", Color(), vm::bbox3(64.0), "this is a point entity", {}, {});
            document->setEntityDefinitions(std::vector<Assets::EntityDefinition*>{ m_pointEntityDef, largeEntityDef });

            Model::EntityNode* entityNode = new Model::EntityNode({
                {"classname", "large_entity"}
            });
            
            document->addNode(entityNode, document->parentForNodes());
            REQUIRE(entityNode->entity().definition() == largeEntityDef);
            
            document->select(entityNode);
            REQUIRE(document->selectionBounds().size() == largeEntityDef->bounds().size());
            
            document->setAttribute("classname", "point_entity");
            CHECK(entityNode->entity().definition() == m_pointEntityDef);
            CHECK(document->selectionBounds().size() == m_pointEntityDef->bounds().size());
            
            document->removeAttribute("classname");
            CHECK(entityNode->entity().definition() == nullptr);
            CHECK(document->selectionBounds().size() == Model::EntityNode::DefaultBounds.size());
            
            document->setAttribute("temp", "large_entity");
            document->renameAttribute("temp", "classname");
            CHECK(document->selectionBounds().size() == largeEntityDef->bounds().size());
            
            document->undoCommand();
            CHECK(entityNode->entity().definition() == nullptr);
            CHECK(document->selectionBounds().size() == Model::EntityNode::DefaultBounds.size());
        }
    }
}
