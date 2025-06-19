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

#include "TestLogger.h"
#include "TestUtils.h"
#include "io/LoadEntityModel.h"
#include "mdl/EntityModel.h"
#include "mdl/Game.h" // IWYU pragma: keep
#include "mdl/GameConfig.h"
#include "mdl/Material.h"
#include "mdl/Texture.h"
#include "mdl/TextureResource.h"
#include "render/IndexRangeMapBuilder.h"
#include "render/MaterialIndexRangeRenderer.h"

#include "vm/approx.h"
#include "vm/bbox.h"
#include "vm/intersection.h"

#include <filesystem>

#include "Catch2.h"

namespace tb::mdl
{
TEST_CASE("BSP model intersection test")
{
  auto logger = TestLogger{};
  auto [game, gameConfig] = loadGame("Quake");

  const auto path = std::filesystem::path{"cube.bsp"};
  const auto loadMaterial = [](auto) -> Material {
    throw std::runtime_error{"should not be called"};
  };

  auto model = io::loadEntityModelSync(
    game->gameFileSystem(), game->config().materialConfig, path, loadMaterial, logger);

  auto& frame = model.value().data()->frames().at(0);

  const auto box = vm::bbox3f(vm::vec3f::fill(-32), vm::vec3f::fill(32));
  CHECK(box == frame.bounds());

  // test some hitting rays
  for (int x = -45; x <= 45; x += 15)
  {
    for (int y = -45; y <= 45; y += 15)
    {
      for (int z = -45; z <= 45; z += 15)
      {
        // shoot a ray from (x, y, z) to (0, 0, 0), it will hit the box
        const auto startPoint = vm::vec3f(x, y, z);
        if (box.contains(startPoint))
        {
          continue;
        }
        const auto endPoint = vm::vec3f{0, 0, 0};
        const auto ray = vm::ray3f(startPoint, vm::normalize(endPoint - startPoint));

        const auto treeDist = frame.intersect(ray);
        const auto expected = vm::intersect_ray_bbox(ray, box);

        CHECK(expected == vm::optional_approx(treeDist));
      }
    }
  }

  // test a missing ray
  const auto missRay = vm::ray3f(vm::vec3f(0, -33, -33), vm::vec3f{0, 1, 0});
  CHECK(frame.intersect(missRay) == std::nullopt);
  CHECK(vm::intersect_ray_bbox(missRay, box) == std::nullopt);
}

static Material makeDummyMaterial(std::string name)
{
  auto textureResource = createTextureResource(Texture{1, 1});
  return Material{std::move(name), std::move(textureResource)};
}

static render::IndexRangeMapBuilder<EntityModelVertex::Type> makeDummyBuilder()
{
  auto size = render::IndexRangeMap::Size{};
  size.inc(render::PrimType::Triangles, 1);

  auto builder = render::IndexRangeMapBuilder<EntityModelVertex::Type>{3, size};
  builder.addTriangle(EntityModelVertex{}, EntityModelVertex{}, EntityModelVertex{});

  return builder;
}

TEST_CASE("EntityModelTest.buildRenderer.defaultSkinIndex")
{
  // Ensure that when rendering a model with multiple surfaces
  // and when each surface has a variable number of skins, we
  // default to a skin index of 0 if the provided index is not
  // present for the surface.

  auto modelData = EntityModelData{PitchType::Normal, Orientation::Oriented};
  auto& frame = modelData.addFrame("test", vm::bbox3f{0, 8});

  // Prepare the first surface - it will only have one skin
  auto& surface1 = modelData.addSurface("surface 1", 1);

  auto materials1 = std::vector<Material>{};
  materials1.push_back(makeDummyMaterial("skin1"));
  surface1.setSkins(std::move(materials1));

  auto builder1 = makeDummyBuilder();
  surface1.addMesh(frame, builder1.vertices(), builder1.indices());

  // The second surface will have two skins
  auto& surface2 = modelData.addSurface("surface 2", 1);

  auto materials2 = std::vector<Material>{};
  materials2.push_back(makeDummyMaterial("skin2a"));
  materials2.push_back(makeDummyMaterial("skin2b"));
  surface2.setSkins(std::move(materials2));

  auto builder2 = makeDummyBuilder();
  surface2.addMesh(frame, builder2.vertices(), builder2.indices());

  // even though the model has 2 skins, we should get a valid renderer even if we request
  // to use skin 3
  const auto renderer0 = modelData.buildRenderer(0, 0);
  const auto renderer1 = modelData.buildRenderer(1, 0);
  const auto renderer2 = modelData.buildRenderer(2, 0);

  CHECK(renderer0 != nullptr);
  CHECK(renderer1 != nullptr);
  CHECK(renderer2 != nullptr);
}
} // namespace tb::mdl
