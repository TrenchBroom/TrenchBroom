
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

#include "gl/Camera.h"
#include "gl/PerspectiveCamera.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Hit.h"
#include "mdl/LayerNode.h"
#include "mdl/Node.h"
#include "mdl/NodeHandleManager.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kd/overload.h"
#include "kd/reflection_impl.h"

#include "vm/bbox.h"
#include "vm/ray.h"
#include "vm/segment.h"
#include "vm/segment_io.h" // IWYU pragma: keep
#include "vm/vec.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb
{
namespace gl
{
class Camera;
}

namespace mdl
{
namespace
{
const auto TestVertexHitType = HitType::freeType();
const auto TestEdgeHitType = HitType::freeType();

struct TestVertexHandle
{
  vm::vec3d position;

  static std::vector<TestVertexHandle> getHandles(const Node& node)
  {
    auto result = std::vector<TestVertexHandle>{};

    node.accept(kdl::overload(
      [](const WorldNode&) {},
      [](const LayerNode&) {},
      [](const GroupNode&) {},
      [](const EntityNode&) {},
      [&](const BrushNode& brushNode) {
        for (const auto* vertex : brushNode.brush().vertices())
        {
          result.emplace_back(vertex->position());
        }
      },
      [](const PatchNode&) {}));

    return result;
  }

  static double distance(const TestVertexHandle& lhs, const TestVertexHandle& rhs)
  {
    return vm::distance(lhs.position, rhs.position);
  }

  std::optional<Hit> pick(
    const vm::ray3d& pickRay, const gl::Camera& camera, const double handleRadius) const
  {
    return camera.pickPointHandle(pickRay, position, handleRadius)
           | kdl::optional_transform([&](const auto distance) {
               return Hit{
                 TestVertexHitType,
                 distance,
                 vm::point_at_distance(pickRay, distance),
                 *this};
             });
  }

  kdl_reflect_inline(TestVertexHandle, position);
};

struct TestEdgeHandle
{
  vm::segment3d position;

  static std::vector<TestEdgeHandle> getHandles(const Node& node)
  {
    auto result = std::vector<TestEdgeHandle>{};

    node.accept(kdl::overload(
      [](const WorldNode&) {},
      [](const LayerNode&) {},
      [](const GroupNode&) {},
      [](const EntityNode&) {},
      [&](const BrushNode& brushNode) {
        for (const auto* edge : brushNode.brush().edges())
        {
          result.emplace_back(edge->segment());
        }
      },
      [](const PatchNode&) {}));

    return result;
  }

  static double distance(const TestEdgeHandle& lhs, const TestEdgeHandle& rhs)
  {
    return vm::max(
      vm::distance(lhs.position.start(), rhs.position.start()),
      vm::distance(lhs.position.end(), rhs.position.end()));
  }

  std::optional<Hit> pick(
    const vm::ray3d& pickRay, const gl::Camera& camera, double handleRadius) const
  {
    const auto handlePosition = position.center();
    return camera.pickPointHandle(pickRay, handlePosition, handleRadius)
           | kdl::optional_transform([&](const auto distance) {
               return Hit{
                 TestEdgeHitType,
                 distance,
                 vm::point_at_distance(pickRay, distance),
                 *this};
             });
  }

  kdl_reflect_inline(TestEdgeHandle, position);
};

auto translateBrush(const vm::bbox3d& worldBounds, Brush brush, const vm::vec3d& delta)
{
  brush.transform(worldBounds, vm::translation_matrix(delta), false).ignore();
  return brush;
}

} // namespace

TEST_CASE("NodeHandleManager")
{
  using namespace Catch::Matchers;

  // use a large clumping distance to avoid floating point inaccuracies
  auto manager = NodeHandleManager{3.0};
  manager.registerHandleType<TestVertexHandle>();
  manager.registerHandleType<TestEdgeHandle>();

  const auto worldBounds = vm::bbox3d{8192.0};
  auto brushBuilder = BrushBuilder{MapFormat::Quake3_Legacy, worldBounds};
  auto brushNode = BrushNode{brushBuilder.createCube(32.0, "material").value()};

  auto sharedBrushNode = BrushNode{translateBrush(
    worldBounds, brushBuilder.createCube(32.0, "material").value(), {32, 32, 0})};

  auto closeBrushNode = BrushNode{translateBrush(
    worldBounds, brushBuilder.createCube(32.0, "material").value(), {0, 0, 2})};

  const auto closeVertex = vm::vec3d{+16, +16, +18};
  const auto closeVertexWithNoise = closeVertex + vm::vec3d{0.001, 0.0, 0.0};

  const auto addNodeHandles = [&](const auto& node) {
    manager.addHandles<TestVertexHandle>(node);
    manager.addHandles<TestEdgeHandle>(node);
  };
  const auto removeNodeHandles = [&](const auto& node) {
    manager.removeHandles<TestVertexHandle>(node);
    manager.removeHandles<TestEdgeHandle>(node);
  };

  addNodeHandles(brushNode);

  SECTION("handleCount")
  {
    SECTION("no shared handles")
    {
      CHECK(manager.handleCount<TestVertexHandle>() == 8u);
      CHECK(manager.handleCount<TestEdgeHandle>() == 12u);
    }

    SECTION("shared handles")
    {
      addNodeHandles(sharedBrushNode);

      CHECK(manager.handleCount<TestVertexHandle>() == 14u);
      CHECK(manager.handleCount<TestEdgeHandle>() == 23u);
    }
  }

  SECTION("selectedCount")
  {
    SECTION("no shared handles")
    {
      CHECK(manager.selectedHandleCount<TestVertexHandle>() == 0u);
      CHECK(manager.selectedHandleCount<TestEdgeHandle>() == 0u);

      manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
      CHECK(manager.selectedHandleCount<TestVertexHandle>() == 1u);
      CHECK(manager.selectedHandleCount<TestEdgeHandle>() == 0u);

      manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
      CHECK(manager.selectedHandleCount<TestVertexHandle>() == 1u);
      CHECK(manager.selectedHandleCount<TestEdgeHandle>() == 0u);
    }

    SECTION("mixed handle types")
    {
      manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
      REQUIRE(manager.selectedHandleCount<TestVertexHandle>() == 1u);
      REQUIRE(manager.selectedHandleCount<TestEdgeHandle>() == 0u);

      manager.selectHandle(TestEdgeHandle{{{-16, -16, -16}, {+16, -16, -16}}});
      CHECK(manager.selectedHandleCount<TestVertexHandle>() == 1u);
      CHECK(manager.selectedHandleCount<TestEdgeHandle>() == 1u);
    }

    SECTION("shared handles")
    {
      addNodeHandles(sharedBrushNode);

      manager.selectHandle(TestVertexHandle{{+16, +16, +16}});
      CHECK(manager.selectedHandleCount<TestVertexHandle>() == 1u);
    }
  }

  SECTION("anyHandleSelected")
  {
    CHECK(!manager.anyHandleSelected<TestVertexHandle>());

    manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
    CHECK(manager.anyHandleSelected<TestVertexHandle>());
  }

  SECTION("allHandlesSelected")
  {
    CHECK(!manager.allHandlesSelected<TestVertexHandle>());

    manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
    CHECK(!manager.allHandlesSelected<TestVertexHandle>());

    manager.selectHandles<TestVertexHandle>(std::vector<TestVertexHandle>{
      TestVertexHandle{{-16, -16, +16}},
      TestVertexHandle{{-16, +16, -16}},
      TestVertexHandle{{-16, +16, +16}},
      TestVertexHandle{{+16, -16, -16}},
      TestVertexHandle{{+16, -16, +16}},
      TestVertexHandle{{+16, +16, -16}},
      TestVertexHandle{{+16, +16, +16}},
    });
    CHECK(manager.allHandlesSelected<TestVertexHandle>());
  }

  SECTION("containsHandle")
  {
    CHECK(manager.containsHandle(TestVertexHandle{{-16, -16, -16}}));
    CHECK(manager.containsHandle(TestEdgeHandle{{{-16, -16, -16}, {+16, -16, -16}}}));

    CHECK(!manager.containsHandle(TestVertexHandle{{-32, -16, -16}}));
  }

  SECTION("isHandleSelected")
  {
    CHECK(!manager.isHandleSelected(TestVertexHandle{{-16, -16, -16}}));

    manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
    CHECK(manager.isHandleSelected(TestVertexHandle{{-16, -16, -16}}));
    CHECK(!manager.isHandleSelected(TestVertexHandle{{999, 999, 999}}));
  }

  SECTION("allHandles")
  {
    SECTION("no shared handles")
    {
      CHECK_THAT(
        manager.allHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{-16, -16, -16}},
          TestVertexHandle{{-16, -16, +16}},
          TestVertexHandle{{-16, +16, -16}},
          TestVertexHandle{{-16, +16, +16}},
          TestVertexHandle{{+16, -16, -16}},
          TestVertexHandle{{+16, -16, +16}},
          TestVertexHandle{{+16, +16, -16}},
          TestVertexHandle{{+16, +16, +16}},
        }));
    }

    SECTION("shared handles")
    {
      addNodeHandles(sharedBrushNode);

      CHECK_THAT(
        manager.allHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{-16, -16, -16}},
          TestVertexHandle{{-16, -16, +16}},
          TestVertexHandle{{-16, +16, -16}},
          TestVertexHandle{{-16, +16, +16}},
          TestVertexHandle{{+16, -16, -16}},
          TestVertexHandle{{+16, -16, +16}},
          TestVertexHandle{{+16, +16, -16}},
          TestVertexHandle{{+16, +16, +16}},
          TestVertexHandle{{+16, +48, -16}},
          TestVertexHandle{{+16, +48, +16}},
          TestVertexHandle{{+48, +16, -16}},
          TestVertexHandle{{+48, +16, +16}},
          TestVertexHandle{{+48, +48, -16}},
          TestVertexHandle{{+48, +48, +16}},
        }));
    }
  }

  SECTION("selectedHandles")
  {
    SECTION("no shared handles")
    {
      manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
      manager.selectHandle(TestVertexHandle{{+16, +16, +16}});

      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{-16, -16, -16}},
          TestVertexHandle{{+16, +16, +16}},
        }));
    }

    SECTION("shared handles")
    {
      addNodeHandles(sharedBrushNode);

      manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
      manager.selectHandle(TestVertexHandle{{+16, +16, +16}});

      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{-16, -16, -16}},
          TestVertexHandle{{+16, +16, +16}},
        }));
    }
  }

  SECTION("unselectedHandles")
  {
    SECTION("no shared handles")
    {
      manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
      manager.selectHandle(TestVertexHandle{{+16, +16, +16}});

      CHECK_THAT(
        manager.unselectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{-16, -16, +16}},
          TestVertexHandle{{-16, +16, -16}},
          TestVertexHandle{{-16, +16, +16}},
          TestVertexHandle{{+16, -16, -16}},
          TestVertexHandle{{+16, -16, +16}},
          TestVertexHandle{{+16, +16, -16}},
        }));
    }

    SECTION("shared handles")
    {
      addNodeHandles(sharedBrushNode);

      manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
      manager.selectHandle(TestVertexHandle{{+16, +16, +16}});

      CHECK_THAT(
        manager.unselectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{-16, -16, +16}},
          TestVertexHandle{{-16, +16, -16}},
          TestVertexHandle{{-16, +16, +16}},
          TestVertexHandle{{+16, -16, -16}},
          TestVertexHandle{{+16, -16, +16}},
          TestVertexHandle{{+16, +16, -16}},
          TestVertexHandle{{+16, +48, -16}},
          TestVertexHandle{{+16, +48, +16}},
          TestVertexHandle{{+48, +16, -16}},
          TestVertexHandle{{+48, +16, +16}},
          TestVertexHandle{{+48, +48, -16}},
          TestVertexHandle{{+48, +48, +16}},
        }));
    }
  }

  SECTION("addHandles")
  {
    SECTION("shared handles keep selection")
    {
      manager.selectHandle(TestVertexHandle{{+16, +16, +16}});
      REQUIRE_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{+16, +16, +16}},
        }));

      addNodeHandles(sharedBrushNode);
      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{+16, +16, +16}},
        }));
    }

    SECTION("close handles are selected")
    {
      manager.selectHandle(TestVertexHandle{{+16, +16, +16}});
      REQUIRE_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{+16, +16, +16}},
        }));

      addNodeHandles(closeBrushNode);
      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{+16, +16, +16}},
          TestVertexHandle{closeVertex},
        }));
    }

    SECTION("non-exact handles are selected as well")
    {
      addNodeHandles(closeBrushNode);

      manager.selectHandle(TestVertexHandle{closeVertexWithNoise});

      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{+16, +16, +16}},
          TestVertexHandle{closeVertex},
        }));
    }
  }

  SECTION("removeHandles")
  {
    SECTION("no shared handles")
    {
      REQUIRE_THAT(
        manager.allHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{-16, -16, -16}},
          TestVertexHandle{{-16, -16, +16}},
          TestVertexHandle{{-16, +16, -16}},
          TestVertexHandle{{-16, +16, +16}},
          TestVertexHandle{{+16, -16, -16}},
          TestVertexHandle{{+16, -16, +16}},
          TestVertexHandle{{+16, +16, -16}},
          TestVertexHandle{{+16, +16, +16}},
        }));

      manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
      manager.selectHandle(TestVertexHandle{{+16, +16, +16}});
      REQUIRE_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{-16, -16, -16}},
          TestVertexHandle{{+16, +16, +16}},
        }));

      removeNodeHandles(brushNode);
      CHECK_THAT(
        manager.allHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{}));

      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{}));
    }

    SECTION("shared handles")
    {
      addNodeHandles(sharedBrushNode);

      SECTION("keeps shared handles")
      {
        REQUIRE_THAT(
          manager.allHandles<TestVertexHandle>(),
          UnorderedRangeEquals(std::vector<TestVertexHandle>{
            TestVertexHandle{{-16, -16, -16}},
            TestVertexHandle{{-16, -16, +16}},
            TestVertexHandle{{-16, +16, -16}},
            TestVertexHandle{{-16, +16, +16}},
            TestVertexHandle{{+16, -16, -16}},
            TestVertexHandle{{+16, -16, +16}},
            TestVertexHandle{{+16, +16, -16}},
            TestVertexHandle{{+16, +16, +16}},
            TestVertexHandle{{+16, +48, -16}},
            TestVertexHandle{{+16, +48, +16}},
            TestVertexHandle{{+48, +16, -16}},
            TestVertexHandle{{+48, +16, +16}},
            TestVertexHandle{{+48, +48, -16}},
            TestVertexHandle{{+48, +48, +16}},
          }));

        removeNodeHandles(brushNode);
        CHECK_THAT(
          manager.allHandles<TestVertexHandle>(),
          UnorderedRangeEquals(std::vector<TestVertexHandle>{
            TestVertexHandle{{+16, +16, -16}},
            TestVertexHandle{{+16, +16, +16}},
            TestVertexHandle{{+16, +48, -16}},
            TestVertexHandle{{+16, +48, +16}},
            TestVertexHandle{{+48, +16, -16}},
            TestVertexHandle{{+48, +16, +16}},
            TestVertexHandle{{+48, +48, -16}},
            TestVertexHandle{{+48, +48, +16}},
          }));
      }

      SECTION("updates selection")
      {
        manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
        manager.selectHandle(TestVertexHandle{{+16, +16, +16}});
        REQUIRE_THAT(
          manager.selectedHandles<TestVertexHandle>(),
          UnorderedRangeEquals(std::vector<TestVertexHandle>{
            TestVertexHandle{{-16, -16, -16}},
            TestVertexHandle{{+16, +16, +16}},
          }));

        removeNodeHandles(brushNode);
        CHECK_THAT(
          manager.selectedHandles<TestVertexHandle>(),
          UnorderedRangeEquals(std::vector<TestVertexHandle>{
            TestVertexHandle{{+16, +16, +16}},
          }));
      }

      SECTION("removes selected edge handle clump")
      {
        const auto sharedEdge =
          TestEdgeHandle{{{+16.0, +16.0, -16.0}, {+16.0, +16.0, +16.0}}};
        manager.selectHandle(sharedEdge);
        REQUIRE(manager.selectedHandleCount<TestEdgeHandle>() == 1u);

        removeNodeHandles(brushNode);
        // The edge is shared, so it should still exist and remain selected
        CHECK(manager.selectedHandleCount<TestEdgeHandle>() == 1u);
        CHECK(manager.containsHandle(sharedEdge));
      }
    }
  }

  SECTION("clump merging")
  {
    // With clumpDistance=3 (used by manager):
    //   V1 = {+16, +16, +16} (vertex of brushNode)
    //   V3 = {+16, +16, +20}: distance(V1,V3)=4 > 3 → separate clump
    //   V2 = {+16, +16, +18}: distance(V2,V1)=2 < 3 and distance(V2,V3)=2 < 3 → bridge
    auto bridgeFarNode = BrushNode{translateBrush(
      worldBounds, brushBuilder.createCube(32.0, "material").value(), {0, 0, 4})};
    auto bridgeNearNode = BrushNode{translateBrush(
      worldBounds, brushBuilder.createCube(32.0, "material").value(), {0, 0, 2})};

    SECTION("adding bridge handle merges two selected clumps into one")
    {
      addNodeHandles(bridgeFarNode);

      // V1 and V3 are too far apart to clump; select them as separate clumps
      manager.selectHandle(TestVertexHandle{{+16.0, +16.0, +16.0}});
      manager.selectHandle(TestVertexHandle{{+16.0, +16.0, +20.0}});
      REQUIRE(manager.selectedHandleCount<TestVertexHandle>() == 2u);

      // Adding V2 bridges V1 and V3: all three merge into one selected clump
      addNodeHandles(bridgeNearNode);

      CHECK(manager.selectedHandleCount<TestVertexHandle>() == 1u);
      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{+16.0, +16.0, +16.0}},
          TestVertexHandle{{+16.0, +16.0, +18.0}},
          TestVertexHandle{{+16.0, +16.0, +20.0}},
        }));
    }

    SECTION("removing bridge handle splits clump and re-selects both parts")
    {
      addNodeHandles(bridgeFarNode);
      addNodeHandles(bridgeNearNode);

      // V1, V2, V3 are now in one merged clump; select it via V1
      manager.selectHandle(TestVertexHandle{{+16.0, +16.0, +16.0}});
      REQUIRE(manager.selectedHandleCount<TestVertexHandle>() == 1u);
      REQUIRE_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{+16.0, +16.0, +16.0}},
          TestVertexHandle{{+16.0, +16.0, +18.0}},
          TestVertexHandle{{+16.0, +16.0, +20.0}},
        }));

      // Removing the bridge (V2) leaves V1 and V3 in separate clumps; since
      // the original merged clump was selected, both new clumps should be selected
      removeNodeHandles(bridgeNearNode);

      CHECK(manager.selectedHandleCount<TestVertexHandle>() == 2u);
      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{+16.0, +16.0, +16.0}},
          TestVertexHandle{{+16.0, +16.0, +20.0}},
        }));
    }
  }

  SECTION("selectHandle")
  {
    SECTION("handles are selected")
    {
      manager.selectHandle(TestVertexHandle{{+16, +16, +16}});

      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{+16, +16, +16}},
        }));
    }

    SECTION("close handles are selected")
    {
      addNodeHandles(closeBrushNode);
      manager.selectHandle(TestVertexHandle{{+16, +16, +16}});

      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{+16, +16, +16}},
          TestVertexHandle{closeVertex},
        }));
    }
  }

  SECTION("deselectHandle")
  {
    SECTION("handles are deselected")
    {
      manager.selectHandle(TestVertexHandle{{+16, +16, +16}});
      REQUIRE_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{+16, +16, +16}},
        }));

      manager.deselectHandle(TestVertexHandle{{+16, +16, +16}});
      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{}));
    }

    SECTION("close handles are deselected as well")
    {
      addNodeHandles(closeBrushNode);
      manager.selectHandle(TestVertexHandle{{+16, +16, +16}});
      REQUIRE_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{+16, +16, +16}},
          TestVertexHandle{closeVertex},
        }));

      manager.deselectHandle(TestVertexHandle{closeVertex});
      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{}));
    }
  }

  SECTION("toggleHandle")
  {
    SECTION("handles are toggled")
    {
      manager.toggleHandle(TestVertexHandle{{+16, +16, +16}});
      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{+16, +16, +16}},
        }));

      manager.toggleHandle(TestVertexHandle{{+16, +16, +16}});
      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{}));
    }

    SECTION("close handles are toggled as well")
    {
      addNodeHandles(closeBrushNode);
      manager.toggleHandle(TestVertexHandle{{+16, +16, +16}});
      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          TestVertexHandle{{+16, +16, +16}},
          TestVertexHandle{closeVertex},
        }));

      manager.toggleHandle(TestVertexHandle{closeVertex});
      CHECK_THAT(
        manager.selectedHandles<TestVertexHandle>(),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{}));
    }
  }

  SECTION("selectHandles")
  {
    const auto handles = std::vector<TestVertexHandle>{
      TestVertexHandle{{-16, -16, -16}},
      TestVertexHandle{{+16, +16, +16}},
    };

    manager.selectHandles<TestVertexHandle>(handles);
    CHECK(manager.isHandleSelected(TestVertexHandle{{-16, -16, -16}}));
    CHECK(manager.isHandleSelected(TestVertexHandle{{+16, +16, +16}}));
  }

  SECTION("deselectHandles")
  {
    manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
    manager.selectHandle(TestVertexHandle{{+16, +16, +16}});
    REQUIRE(manager.selectedHandleCount<TestVertexHandle>() == 2u);

    const auto handles = std::vector<TestVertexHandle>{
      TestVertexHandle{{-16, -16, -16}},
      TestVertexHandle{{+16, +16, +16}},
    };

    manager.deselectHandles<TestVertexHandle>(handles);
    CHECK(manager.selectedHandleCount<TestVertexHandle>() == 0u);
  }

  SECTION("toggleHandles")
  {
    const auto handles = std::vector<TestVertexHandle>{
      TestVertexHandle{{-16, -16, -16}},
      TestVertexHandle{{+16, +16, +16}},
    };

    manager.toggleHandles<TestVertexHandle>(handles);
    CHECK(manager.isHandleSelected(TestVertexHandle{{-16, -16, -16}}));
    CHECK(manager.isHandleSelected(TestVertexHandle{{+16, +16, +16}}));

    manager.toggleHandles<TestVertexHandle>(handles);
    CHECK(!manager.isHandleSelected(TestVertexHandle{{-16, -16, -16}}));
    CHECK(!manager.isHandleSelected(TestVertexHandle{{+16, +16, +16}}));
  }

  SECTION("deselectAllHandles")
  {
    manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
    manager.selectHandle(TestVertexHandle{{+16, +16, +16}});
    REQUIRE(manager.selectedHandleCount<TestVertexHandle>() == 2u);

    manager.deselectAllHandles<TestVertexHandle>();
    CHECK(manager.selectedHandleCount<TestVertexHandle>() == 0u);
  }

  SECTION("clear")
  {
    manager.selectHandle(TestVertexHandle{{-16, -16, -16}});
    REQUIRE(manager.handleCount<TestVertexHandle>() == 8u);

    manager.clear<TestVertexHandle>();
    CHECK(manager.handleCount<TestVertexHandle>() == 0u);
    CHECK(manager.selectedHandleCount<TestVertexHandle>() == 0u);
    CHECK(manager.allHandlesSelected<TestVertexHandle>());
  }

  SECTION("pick")
  {
    // Camera at (-200, -16, -16) looking in +X; a pick ray along +X passes directly
    // through the vertex handle at (-16, -16, -16).
    const auto camera = gl::PerspectiveCamera{
      90.0f,
      1.0f,
      8192.0f,
      gl::Camera::Viewport{0, 0, 800, 600},
      vm::vec3f{-200.0f, -16.0f, -16.0f},
      vm::vec3f{1.0f, 0.0f, 0.0f},
      vm::vec3f{0.0f, 0.0f, 1.0f}};
    const auto pickRay =
      vm::ray3d{vm::vec3d{-200.0, -16.0, -16.0}, vm::vec3d{1.0, 0.0, 0.0}};
    const auto handleRadius = 3.0;

    SECTION("ray hitting a vertex handle produces a hit")
    {
      auto pickResult = PickResult{};
      manager.pick<TestVertexHandle>(pickResult, pickRay, camera, handleRadius);

      CHECK_THAT(
        pickResult.all() | std::views::transform([](const auto& hit) {
          return hit.template target<TestVertexHandle>();
        }),
        UnorderedRangeEquals(std::vector<TestVertexHandle>{
          {{+16.0, -16.0, -16.0}},
          {{-16.0, -16.0, -16.0}},
        }));
    }

    SECTION("ray missing all handles produces no hits")
    {
      const auto missingRay =
        vm::ray3d{vm::vec3d{-200.0, 100.0, 100.0}, vm::vec3d{1.0, 0.0, 0.0}};

      auto pickResult = PickResult{};
      manager.pick<TestVertexHandle>(pickResult, missingRay, camera, handleRadius);

      CHECK(pickResult.empty());
    }

    SECTION("only handles of the requested type are returned")
    {
      auto pickResult = PickResult{};
      manager.pick<TestEdgeHandle>(pickResult, pickRay, camera, handleRadius);

      CHECK_THAT(
        pickResult.all() | std::views::transform([](const auto& hit) {
          return hit.template target<TestEdgeHandle>();
        }),
        UnorderedRangeEquals(std::vector<TestEdgeHandle>{
          {{{+16.0, -16.0, -16.0}, {-16.0, -16.0, -16.0}}},
        }));
    }
  }
}

} // namespace mdl
} // namespace tb
