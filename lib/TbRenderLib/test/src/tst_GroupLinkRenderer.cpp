/*
 Copyright (C) 2026 Kristian Duske

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

#include "TestPreferenceStore.h"
#include "base/PreferenceManager.h"
#include "gl/Vertex.h"
#include "mdl/BrushNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_NodeVisibility.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/TestFactory.h"
#include "prefs/Preferences.h"
#include "render/GroupLinkRenderer.h"

#include "vm/vec.h"

#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

namespace
{

// creates the global PreferenceManager instance (getLinks() reads
// Preferences::LinkedGroupColor) and destroys it again when it goes out of scope, so no
// instance leaks into other test cases
struct PreferenceManagerInstance
{
  PreferenceManagerInstance()
  {
    PreferenceManager::createInstance(std::make_unique<TestPreferenceStore>(), true);
  }

  ~PreferenceManagerInstance() { PreferenceManager::destroyInstance(); }
};

} // namespace

TEST_CASE("GroupLinkRenderer")
{
  auto preferenceManager = PreferenceManagerInstance{};

  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  auto* brushNode = mdl::createBrushNode(map);
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {brushNode}}});
  mdl::selectNodes(map, {brushNode});

  auto* groupA = mdl::groupSelectedNodes(map, "group A");
  REQUIRE(groupA != nullptr);

  auto* groupB = mdl::createLinkedDuplicate(map);
  REQUIRE(groupB != nullptr);

  mdl::deselectAll(map);

  auto renderer = GroupLinkRenderer{map};

  const auto linkColor = pref(Preferences::LinkedGroupColor).to<RgbaF>().toVec();
  const auto anchorA = vm::vec3f{groupA->logicalBounds().center()};
  const auto anchorB = vm::vec3f{groupB->logicalBounds().center()};

  SECTION("getLinks")
  {
    SECTION("returns no links when nothing is selected and no group is open")
    {
      CHECK(renderer.getLinks().empty());
    }

    SECTION(
      "with exactly one linked group selected, returns a link to every other "
      "group sharing its link id, excluding itself")
    {
      mdl::selectNodes(map, {groupA});

      const auto links = renderer.getLinks();
      REQUIRE(links.size() == 2);
      CHECK(gl::getVertexComponent<0>(links[0]) == anchorA);
      CHECK(gl::getVertexComponent<1>(links[0]) == linkColor);
      CHECK(gl::getVertexComponent<0>(links[1]) == anchorB);
      CHECK(gl::getVertexComponent<1>(links[1]) == linkColor);
    }

    SECTION("excludes linked groups that are not visible")
    {
      mdl::selectNodes(map, {groupA});
      mdl::hideNodes(map, {groupB});

      CHECK(renderer.getLinks().empty());
    }

    SECTION(
      "falls back to the currently open group when the selection is not exactly "
      "one group")
    {
      mdl::openGroup(map, *groupA);

      const auto links = renderer.getLinks();
      REQUIRE(links.size() == 2);
      CHECK(gl::getVertexComponent<0>(links[0]) == anchorA);
      CHECK(gl::getVertexComponent<0>(links[1]) == anchorB);
    }

    SECTION(
      "falls back to the currently open group when more than one group is "
      "selected")
    {
      mdl::openGroup(map, *groupA);
      mdl::selectNodes(map, {groupA, groupB});

      const auto links = renderer.getLinks();
      REQUIRE(links.size() == 2);
      CHECK(gl::getVertexComponent<0>(links[0]) == anchorA);
      CHECK(gl::getVertexComponent<0>(links[1]) == anchorB);
    }
  }
}

} // namespace tb::render
