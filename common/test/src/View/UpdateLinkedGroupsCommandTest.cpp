/*
 Copyright (C) 2022 Kristian Duske

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

#include "View/UpdateLinkedGroupsCommand.h"

#include "Model/BrushNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "View/CurrentGroupCommand.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/MapDocumentTest.h"

#include "TestUtils.h"

#include "Catch2.h"

namespace TrenchBroom {
namespace View {

TEST_CASE_METHOD(MapDocumentTest, "UpdateLinkedGroupsCommandTest.collateWith") {
  const auto createLinkedGroup = [&]() {
    auto* brushNode = createBrushNode();
    document->addNodes({{document->parentForNodes(), {brushNode}}});
    document->selectNodes({brushNode});

    auto* groupNode = document->groupSelection("group");
    document->selectNodes({groupNode});

    auto* linkedGroupNode = document->createLinkedDuplicate();
    document->deselectAll();

    return std::make_tuple(groupNode, linkedGroupNode);
  };

  auto [groupNode1, linkedGroupNode1] = createLinkedGroup();
  auto [groupNode2, linkedGroupNode2] = createLinkedGroup();

  SECTION("Collate two UpdateLinkedGroupCommand instances") {
    auto firstCommand = UpdateLinkedGroupsCommand{{groupNode1}};
    auto secondCommand = UpdateLinkedGroupsCommand{{groupNode1, groupNode2}};

    firstCommand.performDo(static_cast<MapDocumentCommandFacade*>(document.get()));
    secondCommand.performDo(static_cast<MapDocumentCommandFacade*>(document.get()));

    CHECK(firstCommand.collateWith(secondCommand));
  }

  SECTION("Collate UpdateLinkedGroupCommand with another command") {
    auto firstCommand = UpdateLinkedGroupsCommand{{groupNode1}};
    auto secondCommand = CurrentGroupCommand{groupNode2};

    firstCommand.performDo(static_cast<MapDocumentCommandFacade*>(document.get()));
    secondCommand.performDo(static_cast<MapDocumentCommandFacade*>(document.get()));

    CHECK_FALSE(firstCommand.collateWith(secondCommand));
  }
}
} // namespace View
} // namespace TrenchBroom
