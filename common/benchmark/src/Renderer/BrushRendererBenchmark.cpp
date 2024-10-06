/*
 Copyright (C) 2018 Eric Wasylishen

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

#include "../../test/src/Catch2.h"
#include "BenchmarkUtils.h"
#include "Renderer/BrushRenderer.h"
#include "asset/Material.h"
#include "asset/Texture.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/MapFormat.h"
#include "mdl/WorldNode.h"

#include "kdl/result.h"

#include <fmt/format.h>

#include <string>
#include <tuple>
#include <vector>

namespace tb::Renderer
{
namespace
{

constexpr size_t NumBrushes = 64'000;
constexpr size_t NumMaterials = 256;

/**
 * Both returned vectors need to be freed with VecUtils::clearAndDelete
 */
auto makeBrushes()
{
  // make materials
  auto materials = std::vector<asset::Material>{};
  for (size_t i = 0; i < NumMaterials; ++i)
  {
    auto materialName = "material " + std::to_string(i);
    auto textureResource = createTextureResource(asset::Texture{64, 64});
    materials.emplace_back(std::move(materialName), std::move(textureResource));
  }

  // make brushes, cycling through the materials for each face
  const auto worldBounds = vm::bbox3d{4096.0};

  auto builder = mdl::BrushBuilder{mdl::MapFormat::Standard, worldBounds};

  auto result = std::vector<std::unique_ptr<mdl::BrushNode>>{};
  size_t currentMaterialIndex = 0;
  for (size_t i = 0; i < NumBrushes; ++i)
  {
    auto brush = builder.createCube(64.0, "") | kdl::value();
    for (auto& face : brush.faces())
    {
      face.setMaterial(&materials.at((currentMaterialIndex++) % NumMaterials));
    }
    result.push_back(std::make_unique<mdl::BrushNode>(std::move(brush)));
  }

  // ensure the brushes have their vertices cached.
  // we're not benchmarking that, so we don't
  // want it mixed into the timing

  BrushRenderer tempRenderer;
  for (auto& brushNode : result)
  {
    tempRenderer.addBrush(brushNode.get());
  }
  tempRenderer.validate();
  tempRenderer.clear();

  return std::tuple{std::move(result), std::move(materials)};
}

} // namespace

TEST_CASE("BrushRendererBenchmark.benchBrushRenderer")
{
  auto [brushes, materials] = makeBrushes();

  BrushRenderer r;

  timeLambda(
    [&]() {
      for (const auto& brush : brushes)
      {
        r.addBrush(brush.get());
      }
    },
    fmt::format("add {} brushes to BrushRenderer", brushes.size()));
  timeLambda(
    [&]() {
      if (!r.valid())
      {
        r.validate();
      }
    },
    fmt::format("validate after adding {} brushes to BrushRenderer", brushes.size()));

  // Tiny change: remove the last brush
  timeLambda([&]() { r.removeBrush(brushes.back().get()); }, "call removeBrush once");
  timeLambda(
    [&]() {
      if (!r.valid())
      {
        r.validate();
      }
    },
    "validate after removing one brush");

  // Large change: keep every second brush
  timeLambda(
    [&]() {
      for (size_t i = 0; i < brushes.size(); ++i)
      {
        if ((i % 2) == 0)
        {
          r.removeBrush(brushes[i].get());
        }
      }
    },
    "remove every second brush");

  timeLambda(
    [&]() {
      if (!r.valid())
      {
        r.validate();
      }
    },
    "validate remaining brushes");
}

} // namespace tb::Renderer
