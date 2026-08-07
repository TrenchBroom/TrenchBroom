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
#include "mdl/Entity.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityLinkManager.h"
#include "mdl/EntityNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_NodeVisibility.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PropertyDefinition.h"
#include "mdl/TestFactory.h"
#include "prefs/Preferences.h"
#include "render/EntityLinkRenderer.h"

#include "vm/vec.h"

#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

namespace
{
using LineVertex = LinkRenderer::LineVertex;

// creates the global PreferenceManager instance (getLinks() reads
// Preferences::EntityLinkMode) and destroys it again when it goes out of scope, so no
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

TEST_CASE("EntityLinkRenderer")
{
  using namespace mdl::EntityPropertyKeys;

  auto preferenceManager = PreferenceManagerInstance{};

  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  constexpr auto sourceClassname = "source_definition";
  constexpr auto targetClassname = "target_definition";

  map.entityDefinitionManager().setDefinitions(
    {{sourceClassname,
      {},
      {},
      {
        {Target, mdl::PropertyValueTypes::LinkSource{}, {}, {}},
      }},
     {targetClassname,
      {},
      {},
      {
        {Targetname, mdl::PropertyValueTypes::LinkTarget{}, {}, {}},
      }}});

  auto* sourceNode = new mdl::EntityNode{mdl::Entity{{
    {Classname, sourceClassname},
    {Target, "link"},
    {Origin, "0 0 0"},
  }}};
  auto* targetNode = new mdl::EntityNode{mdl::Entity{{
    {Classname, targetClassname},
    {Targetname, "link"},
    {Origin, "64 0 0"},
  }}};

  mdl::addNodes(map, {{&mdl::parentForNodes(map), {sourceNode, targetNode}}});
  REQUIRE(map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));

  const auto defaultColor = Color{RgbaF{0, 1, 0, 1}};
  const auto selectedColor = Color{RgbaF{1, 0, 0, 1}};

  auto renderer = EntityLinkRenderer{map};
  renderer.setDefaultColor(defaultColor);
  renderer.setSelectedColor(selectedColor);

  const auto sourceAnchor = vm::vec3f{sourceNode->linkSourceAnchor()};
  const auto targetAnchor = vm::vec3f{targetNode->linkTargetAnchor()};

  SECTION("getLinks")
  {
    SECTION(
      "EntityLinkModeAll returns the link with the default color when neither "
      "endpoint is selected")
    {
      setPref(Preferences::EntityLinkMode, std::string{Preferences::EntityLinkModeAll});

      const auto links = renderer.getLinks();
      REQUIRE(links.size() == 2);
      CHECK(gl::getVertexComponent<0>(links[0]) == sourceAnchor);
      CHECK(gl::getVertexComponent<1>(links[0]) == defaultColor.to<RgbaF>().toVec());
      CHECK(gl::getVertexComponent<0>(links[1]) == targetAnchor);
      CHECK(gl::getVertexComponent<1>(links[1]) == defaultColor.to<RgbaF>().toVec());
    }

    SECTION(
      "both endpoints are colored with the selected color when either endpoint is "
      "selected")
    {
      setPref(Preferences::EntityLinkMode, std::string{Preferences::EntityLinkModeAll});
      mdl::selectNodes(map, {sourceNode});

      const auto links = renderer.getLinks();
      REQUIRE(links.size() == 2);
      CHECK(gl::getVertexComponent<1>(links[0]) == selectedColor.to<RgbaF>().toVec());
      CHECK(gl::getVertexComponent<1>(links[1]) == selectedColor.to<RgbaF>().toVec());
    }

    SECTION("EntityLinkModeAll excludes links to an invisible target")
    {
      setPref(Preferences::EntityLinkMode, std::string{Preferences::EntityLinkModeAll});
      mdl::hideNodes(map, {targetNode});

      CHECK(renderer.getLinks().empty());
    }

    SECTION("EntityLinkModeDirect only includes links touching a selected entity")
    {
      setPref(
        Preferences::EntityLinkMode, std::string{Preferences::EntityLinkModeDirect});

      CHECK(renderer.getLinks().empty());

      mdl::selectNodes(map, {sourceNode});
      CHECK(renderer.getLinks().size() == 2);
    }

    SECTION("EntityLinkModeTransitive follows the chain from a selected entity")
    {
      setPref(
        Preferences::EntityLinkMode, std::string{Preferences::EntityLinkModeTransitive});

      CHECK(renderer.getLinks().empty());

      mdl::selectNodes(map, {sourceNode});
      const auto links = renderer.getLinks();

      REQUIRE(links.size() == 2);
      CHECK(gl::getVertexComponent<0>(links[0]) == sourceAnchor);
      CHECK(gl::getVertexComponent<0>(links[1]) == targetAnchor);
    }

    SECTION("an unrecognized EntityLinkMode returns no links")
    {
      setPref(Preferences::EntityLinkMode, std::string{"nonsense"});
      CHECK(renderer.getLinks().empty());
    }
  }
}

} // namespace tb::render
