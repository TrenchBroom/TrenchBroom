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

#include "TestEnvironment.h"
#include "base/Logger.h"
#include "fs/DiskFileSystem.h"
#include "fs/DiskIO.h"
#include "fs/Reader.h"
#include "mdl/CatchConfig.h"
#include "mdl/EntityModel.h"
#include "mdl/LoadSpriteModel.h"
#include "mdl/Palette.h"

#include "kd/result.h"

#include "vm/approx.h"
#include "vm/bbox.h"
#include "vm/ray.h"
#include "vm/vec.h"

#include <optional>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{

TEST_CASE("loadSpriteModel")
{
  auto logger = NullLogger{};

  const auto palettePath = "test/mdl/LoadSpriteModel/palette.lmp";
  auto diskFs = fs::DiskFileSystem{getFixtureRoot()};
  auto paletteFile = diskFs.openFile(palettePath) | kdl::value();
  const auto palette = mdl::loadPalette(*paletteFile, palettePath) | kdl::value();

  // Each fixture is a single 512x512 frame centered on the origin (xOffset=-256,
  // yOffset=256), differing only in their orientation type.
  const auto x1 = -256.0f;
  const auto x2 = 256.0f;
  const auto y1 = -256.0f;
  const auto y2 = 256.0f;

  const auto loadFixture = [&](const std::string& name) {
    const auto sprPath = getFixtureRoot() / "test/mdl/LoadSpriteModel" / (name + ".spr");
    auto sprFile = fs::Disk::openFile(sprPath) | kdl::value();
    auto reader = sprFile->reader().buffer();
    return loadSpriteModel(name, reader, palette, logger);
  };

  SECTION("billboard orientations")
  {
    const auto [fixtureName, expectedOrientation] =
      GENERATE(table<std::string, Orientation>({
        {"ViewPlaneParallelUpright", Orientation::ViewPlaneParallelUpright},
        {"FacingUpright", Orientation::FacingUpright},
        {"ViewPlaneParallel", Orientation::ViewPlaneParallel},
        {"ViewPlaneParallelOriented", Orientation::ViewPlaneParallelOriented},
      }));

    loadFixture(fixtureName) | kdl::transform([&](const auto& modelData) {
      CHECK(modelData.pitchType() == PitchType::Normal);
      CHECK(modelData.orientation() == expectedOrientation);
      CHECK(modelData.surfaceCount() == 1u);
      CHECK(modelData.frameCount() == 1u);

      const auto& surface = modelData.surfaces().front();
      CHECK(surface.skinCount() == 1u);
      CHECK(surface.frameCount() == 1u);

      // The billboarded orientation types are rendered entirely in the vertex shader
      // using the mesh's x (horizontal) and y (vertical) components, with z unused, so
      // the mesh lies flat in the local z=0 plane.
      const auto& frame = modelData.frames().front();
      CHECK(frame.bounds() == vm::bbox3f{{x1, x1, y1}, {x2, x2, y2}});

      // a ray fired straight down the z axis, through the middle of the sprite, hits it
      const auto hit = frame.intersect(vm::ray3f{{0, 0, 1000}, {0, 0, -1}});
      CHECK(std::optional{1000.0f} == vm::optional_approx(hit));

      // a ray fired through a point outside the sprite's horizontal extent misses
      CHECK(frame.intersect(vm::ray3f{{300, 0, 1000}, {0, 0, -1}}) == std::nullopt);

      // regression check: the mesh must not be perpendicular to the x axis, i.e. it must
      // not use the same local axis convention as Orientation::Oriented
      CHECK(frame.intersect(vm::ray3f{{-1000, 0, 0}, {1, 0, 0}}) == std::nullopt);
    }) | kdl::transform_error([](const auto& e) { FAIL(e); });
  }

  SECTION("Oriented")
  {
    loadFixture("Oriented") | kdl::transform([&](const auto& modelData) {
      CHECK(modelData.pitchType() == PitchType::Normal);
      CHECK(modelData.orientation() == Orientation::Oriented);
      CHECK(modelData.surfaceCount() == 1u);
      CHECK(modelData.frameCount() == 1u);

      const auto& surface = modelData.surfaces().front();
      CHECK(surface.skinCount() == 1u);
      CHECK(surface.frameCount() == 1u);

      // Oriented sprites are rotated like a regular model, using the same local axis
      // convention as Quake's AngleVectors(): local +x is forward (unused, since the
      // sprite is flat), local +y is left (so local -y is right), and local +z is up. So
      // the mesh must lie in the local x=0 plane.
      const auto& frame = modelData.frames().front();
      CHECK(frame.bounds() == vm::bbox3f{{0, -x2, y1}, {0, -x1, y2}});

      // a ray fired straight down the x axis, through the middle of the sprite,
      // hits it
      const auto hit = frame.intersect(vm::ray3f{{-1000, 0, 0}, {1, 0, 0}});
      CHECK(std::optional{1000.0f} == vm::optional_approx(hit));

      // a ray fired through a point outside the sprite's extent misses
      CHECK(frame.intersect(vm::ray3f{{-1000, 300, 0}, {1, 0, 0}}) == std::nullopt);

      // regression check: the mesh must not be billboarded in the local z=0 plane
      CHECK(frame.intersect(vm::ray3f{{0, 0, 1000}, {0, 0, -1}}) == std::nullopt);
    }) | kdl::transform_error([](const auto& e) { FAIL(e); });
  }
}

} // namespace tb::mdl
