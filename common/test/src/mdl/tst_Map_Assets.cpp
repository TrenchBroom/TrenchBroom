/*
 Copyright (C) 2025 Kristian Duske

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

#include "MapFixture.h"
#include "TestUtils.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/WorldNode.h"

#include "kdl/vector_utils.h"

#include <ranges>

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("Map_Assets")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();

  SECTION("reloadMaterialCollections")
  {
    fixture.load(
      "fixture/test/ui/MapDocumentTest/reloadMaterialCollectionsQ2.map",
      {.mapFormat = MapFormat::Quake2, .game = LoadGameFixture{"Quake2"}});

    const auto faces = map.world()->defaultLayer()->children()
                       | std::views::transform([&](const auto* node) {
                           const auto* brushNode =
                             dynamic_cast<const mdl::BrushNode*>(node);
                           REQUIRE(brushNode);
                           return &brushNode->brush().faces().front();
                         })
                       | kdl::to_vector;

    REQUIRE(faces.size() == 4);
    REQUIRE(
      kdl::vec_transform(
        faces, [](const auto* face) { return face->attributes().materialName(); })
      == std::vector<std::string>{
        "b_pv_v1a1", "e1m1/b_pv_v1a2", "e1m1/f1/b_rc_v4", "lavatest"});

    REQUIRE(
      kdl::none_of(faces, [](const auto* face) { return face->material() == nullptr; }));

    CHECK_NOTHROW(map.reloadMaterialCollections());

    REQUIRE(
      kdl::none_of(faces, [](const auto* face) { return face->material() == nullptr; }));
  }
}

} // namespace tb::mdl
