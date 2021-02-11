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

#include "Assets/EntityDefinition.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/Layer.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

#include <vecmath/bbox.h>

#include <vector>

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        class SetLockStateTest : public MapDocumentTest {
        public:
            SetLockStateTest() :
            MapDocumentTest(Model::MapFormat::Valve) {}
        };

        TEST_CASE_METHOD(SetLockStateTest, "SetLockStateTest.modificationCount") {
            auto* brushNode = createBrushNode();
            auto* entityNode = new Model::EntityNode{};

            auto* entityNodeInGroup = new Model::EntityNode{};

            document->addNodes({{document->parentForNodes(), {brushNode, entityNode, entityNodeInGroup}}});
            document->deselectAll();
            document->select(entityNodeInGroup);
            
            auto* groupNode = document->groupSelection("group");
            document->deselectAll();

            auto* layerNode = new Model::LayerNode{Model::Layer{"layer"}};
            document->addNodes({{document->world(), {layerNode}}});

            const auto originalModificationCount = document->modificationCount();

            document->lock({brushNode, entityNode, groupNode});
            CHECK(document->modificationCount() == originalModificationCount);

            document->undoCommand();
            CHECK(document->modificationCount() == originalModificationCount);

            document->lock({layerNode});
            CHECK(document->modificationCount() == originalModificationCount + 1u);

            document->undoCommand();
            CHECK(document->modificationCount() == originalModificationCount);
        }
    }
}
