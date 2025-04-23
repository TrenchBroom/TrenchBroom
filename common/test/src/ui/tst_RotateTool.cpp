/*
 Copyright (C) 2010 Kristian Duske

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

#include "mdl/BrushNode.h" // IWYU pragma: keep
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "ui/Grid.h"
#include "ui/MapDocumentTest.h"
#include "ui/RotateTool.h"

#include "Catch2.h"

namespace tb::ui
{

TEST_CASE_METHOD(MapDocumentTest, "RotateTool")
{
  auto tool = RotateTool{document};
  tool.activate();

  SECTION("resetRotationCenter")
  {
    auto entity1 = mdl::Entity{};
    entity1.setOrigin(vm::vec3d{8, 16, 32});

    auto entity2 = mdl::Entity{};
    entity2.setOrigin(vm::vec3d{16, 24, 32});

    auto* entityNode1 = new mdl::EntityNode{std::move(entity1)};
    auto* entityNode2 = new mdl::EntityNode{std::move(entity2)};
    auto* brushNode = createBrushNode();

    document->addNodes(
      {{document->parentForNodes(), {entityNode1, entityNode2, brushNode}}});

    SECTION("If nothing is selected")
    {
      tool.resetRotationCenter();
      CHECK(tool.rotationCenter() == vm::bbox3d{}.center());
    }

    SECTION("If a single entity is selected")
    {
      document->selectNodes({entityNode1});

      tool.resetRotationCenter();
      CHECK(tool.rotationCenter() == vm::vec3d{8, 16, 32});
    }

    SECTION("If multiple entities are selected")
    {
      document->selectNodes({entityNode1, entityNode2});

      tool.resetRotationCenter();
      CHECK(
        tool.rotationCenter()
        == document->grid().snap(document->selectionBounds().center()));
    }

    SECTION("If a mix of nodes is selected")
    {
      document->selectNodes({entityNode1, brushNode});

      tool.resetRotationCenter();
      CHECK(
        tool.rotationCenter()
        == document->grid().snap(document->selectionBounds().center()));
    }
  }
}


} // namespace tb::ui
