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
#include "gl/Material.h"
#include "gl/OrthographicCamera.h"
#include "gl/Texture.h"
#include "gl/TextureResource.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/HitType.h"
#include "mdl/MapFormat.h"
#include "mdl/PickResult.h"
#include "ui/CatchConfig.h"
#include "ui/UvViewHelper.h"

#include "kd/result.h"

#include "vm/approx.h"

#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("UvViewHelper")
{
  auto material =
    gl::Material{"material", gl::createTextureResource(gl::Texture{64, 64})};

  const auto worldBounds = vm::bbox3d{8192.0};
  auto builder = mdl::BrushBuilder{mdl::MapFormat::Standard, worldBounds};

  auto brush = builder.createCuboid(vm::bbox3d{16.0}, "material") | kdl::value();
  const auto topFaceIndex = *brush.findFace(vm::vec3d{0, 0, 1});
  brush.face(topFaceIndex).setMaterial(&material);

  auto brushNode = std::make_unique<mdl::BrushNode>(std::move(brush));
  const auto faceHandle = mdl::BrushFaceHandle{brushNode.get(), topFaceIndex};

  auto camera = gl::OrthographicCamera{
    1.0f,
    8000.0f,
    gl::Camera::Viewport{0, 0, 1024, 768},
    vm::vec3f{0, 0, 100},
    vm::vec3f{0, 0, -1},
    vm::vec3f{0, 1, 0}};

  auto helper = UvViewHelper{camera};

  SECTION("valid, face and material before a face is set")
  {
    CHECK(!helper.valid());
    CHECK(helper.face() == nullptr);
    CHECK(helper.material() == nullptr);
  }

  SECTION("setFaceHandle")
  {
    helper.setFaceHandle(faceHandle);

    CHECK(helper.valid());
    CHECK(helper.face() == &faceHandle.face());
    CHECK(helper.material() == &material);

    SECTION("resets the camera to look along the face's normal")
    {
      CHECK(camera.direction() == vm::vec3f{0, 0, -1});
      CHECK(camera.up() == vm::vec3f{0, 1, 0});
      CHECK(camera.position() == vm::vec3f{0, 0, 16});
    }

    SECTION("resets the origin to one of the face's corners")
    {
      const auto origin = helper.origin();
      const auto isCorner =
        origin == vm::vec3d{-16, -16, 16} || origin == vm::vec3d{16, -16, 16}
        || origin == vm::vec3d{-16, 16, 16} || origin == vm::vec3d{16, 16, 16};
      CHECK(isCorner);
    }

    SECTION("setting the same handle again is a no-op")
    {
      const auto originBefore = helper.origin();
      camera.moveTo(vm::vec3f{0, 0, 200});

      helper.setFaceHandle(faceHandle);

      // the camera was not reset again, so the manual move is still in effect
      CHECK(camera.position() == vm::vec3f{0, 0, 200});
      CHECK(helper.origin() == originBefore);
    }
  }

  SECTION("subDivisions")
  {
    CHECK(helper.subDivisions() == vm::vec2i{1, 1});

    helper.setSubDivisions(vm::vec2i{2, 4});
    CHECK(helper.subDivisions() == vm::vec2i{2, 4});
  }

  SECTION("stripeSize is the texture size divided by the subdivisions")
  {
    helper.setFaceHandle(faceHandle);

    CHECK(helper.stripeSize() == vm::vec2d{64, 64});

    helper.setSubDivisions(vm::vec2i{2, 4});
    CHECK(helper.stripeSize() == vm::vec2d{32, 16});
  }

  SECTION("originInFaceCoords round-trips through setOriginInFaceCoords")
  {
    helper.setFaceHandle(faceHandle);

    helper.setOriginInFaceCoords(vm::vec2f{10, 20});
    CHECK(vm::vec2f{helper.originInFaceCoords()} == vm::vec2f{10, 20});
  }

  SECTION("computeDistanceFromUvGrid snaps to the nearest stripe corner")
  {
    helper.setFaceHandle(faceHandle);
    // stripe size is 64x64 (subDivisions defaults to {1, 1})

    CHECK(helper.computeDistanceFromUvGrid(vm::vec3d{70, 70, 0}) == vm::vec2f{-6, -6});
    CHECK(helper.computeDistanceFromUvGrid(vm::vec3d{58, 58, 0}) == vm::vec2f{6, 6});
    CHECK(helper.computeDistanceFromUvGrid(vm::vec3d{64, 64, 0}) == vm::vec2f{0, 0});
  }

  SECTION("snapDelta")
  {
    helper.setFaceHandle(faceHandle);
    camera.setZoom(1.0f);

    SECTION("snaps to the grid when close enough")
    {
      // 8.0f / zoom == 8.0f, so a distance under that snaps
      CHECK(
        helper.snapDelta(vm::vec2f{3.4f, -2.1f}, vm::vec2f{0.5f, 1.5f})
        == vm::approx{vm::vec2f{3.9f, -0.6f}});
    }

    SECTION("rounds the delta when too far from the grid")
    {
      CHECK(
        helper.snapDelta(vm::vec2f{3.4f, -2.6f}, vm::vec2f{10.0f, 10.0f})
        == vm::vec2f{3.0f, -3.0f});
    }
  }

  SECTION("pickUvGrid")
  {
    helper.setFaceHandle(faceHandle);

    const mdl::HitType::Type hitTypes[2] = {
      mdl::HitType::freeType(), mdl::HitType::freeType()};

    SECTION("no hit when the ray misses the face's plane")
    {
      // parallel to the face's boundary plane (z = 16), so it never intersects
      const auto ray = vm::ray3d{{0, 0, 100}, {1, 0, 0}};
      auto pickResult = mdl::PickResult{};
      helper.pickUvGrid(ray, hitTypes, pickResult);

      CHECK(pickResult.empty());
    }

    SECTION("hits both gridlines at a grid intersection")
    {
      // the face's UV coordinate system maps world (x, y) directly to UV space here
      // (offset 0, scale 1), and the stripe size is 64x64, so world (0, 0, 16) is
      // exactly on both the vertical and horizontal gridlines through the origin
      const auto ray = vm::ray3d{{0, 0, 100}, {0, 0, -1}};
      auto pickResult = mdl::PickResult{};
      helper.pickUvGrid(ray, hitTypes, pickResult);

      CHECK(pickResult.all().size() == 2u);
    }
  }

  SECTION("originInUvCoords")
  {
    helper.setFaceHandle(faceHandle);
    CHECK(!vm::is_nan(vm::vec2d{helper.originInUvCoords()}));
  }

  SECTION("computeOriginHandleVertices and computeScaleHandleVertices")
  {
    helper.setFaceHandle(faceHandle);

    auto x1 = vm::vec3d{}, x2 = vm::vec3d{}, y1 = vm::vec3d{}, y2 = vm::vec3d{};

    helper.computeOriginHandleVertices(x1, x2, y1, y2);
    CHECK(!vm::is_nan(x1));
    CHECK(!vm::is_nan(x2));
    CHECK(!vm::is_nan(y1));
    CHECK(!vm::is_nan(y2));
    CHECK(x1 != x2);
    CHECK(y1 != y2);

    helper.computeScaleHandleVertices(vm::vec2d{10, 20}, x1, x2, y1, y2);
    CHECK(!vm::is_nan(x1));
    CHECK(!vm::is_nan(x2));
    CHECK(!vm::is_nan(y1));
    CHECK(!vm::is_nan(y2));
  }

  SECTION("cameraViewportChanged resets the zoom once the viewport becomes valid")
  {
    // a viewport too small to compute a zoom factor from
    camera.setViewport(gl::Camera::Viewport{0, 0, 10, 10});
    helper.setFaceHandle(faceHandle);

    helper.cameraViewportChanged();
    camera.setViewport(gl::Camera::Viewport{0, 0, 1024, 768});
    helper.cameraViewportChanged();

    // no direct way to observe m_zoomValid, but this should at least not crash and
    // should have exercised the resetZoom() call inside cameraViewportChanged()
    CHECK(camera.zoom() > 0.0f);
  }
}

} // namespace tb::ui
