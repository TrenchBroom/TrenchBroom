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
#include "Assets/Material.h"
#include "BenchmarkUtils.h"
#include "Error.h"
#include "Exceptions.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/MapFormat.h"
#include "Model/WorldNode.h"
#include "Renderer/BrushRenderer.h"

#include "kdl/result.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <tuple>
#include <vector>

namespace TrenchBroom
{
namespace Renderer
{
static constexpr size_t NumBrushes = 64'000;
static constexpr size_t NumMaterials = 256;

/**
 * Both returned vectors need to be freed with VecUtils::clearAndDelete
 */
static std::pair<std::vector<Model::BrushNode*>, std::vector<Assets::Material*>>
makeBrushes()
{
  // make materials
  std::vector<Assets::Material*> materials;
  for (size_t i = 0; i < NumMaterials; ++i)
  {
    auto materialName = "material " + std::to_string(i);
    materials.push_back(
      new Assets::Material(std::move(materialName), Assets::Texture{64, 64}));
  }

  // make brushes, cycling through the materials for each face
  const vm::bbox3 worldBounds(4096.0);

  Model::BrushBuilder builder(Model::MapFormat::Standard, worldBounds);

  std::vector<Model::BrushNode*> result;
  size_t currentMaterialIndex = 0;
  for (size_t i = 0; i < NumBrushes; ++i)
  {
    Model::Brush brush = builder.createCube(64.0, "") | kdl::value();
    for (Model::BrushFace& face : brush.faces())
    {
      face.setMaterial(materials.at((currentMaterialIndex++) % NumMaterials));
    }
    Model::BrushNode* brushNode = new Model::BrushNode(std::move(brush));
    result.push_back(brushNode);
  }

  // ensure the brushes have their vertices cached.
  // we're not benchmarking that, so we don't
  // want it mixed into the timing

  BrushRenderer tempRenderer;
  for (auto* brushNode : result)
  {
    tempRenderer.addBrush(brushNode);
  }
  tempRenderer.validate();
  tempRenderer.clear();

  return {result, materials};
}

TEST_CASE("BrushRendererBenchmark.benchBrushRenderer")
{
  auto [brushes, materials] = makeBrushes();

  BrushRenderer r;

  timeLambda(
    [&]() {
      for (auto* brush : brushes)
      {
        r.addBrush(brush);
      }
    },
    "add " + std::to_string(brushes.size()) + " brushes to BrushRenderer");
  timeLambda(
    [&]() {
      if (!r.valid())
      {
        r.validate();
      }
    },
    "validate after adding " + std::to_string(brushes.size())
      + " brushes to BrushRenderer");

  // Tiny change: remove the last brush
  timeLambda([&]() { r.removeBrush(brushes.back()); }, "call removeBrush once");
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
          r.removeBrush(brushes[i]);
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

  kdl::vec_clear_and_delete(brushes);
  kdl::vec_clear_and_delete(materials);
}
} // namespace Renderer
} // namespace TrenchBroom
