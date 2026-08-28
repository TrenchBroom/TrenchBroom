/*
 Copyright (C) 2026

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

// This is a manual benchmark, not a correctness test: it loads real converted EverQuest
// zone maps from ~/norrath-maps and measures how expensive it is to react to the
// notifier storm a brush drag produces (one notification per mouse-move). It is tagged
// "[.]" (Catch2's "hidden" tag) so it is excluded from the default ctest run - it depends
// on map files that only exist on this machine and takes noticeably longer than the rest
// of the suite. Run it explicitly with:
//
//   ctest --test-dir <build>/lib/TbUiLib/test -R OutlinerModelBenchmark
//     (or) <build>/lib/TbUiLib/test/TbUiLibTest "[benchmark]"
//
// Numbers should be taken from a RelWithDebInfo/Release build - a Debug build has no
// optimization and live assertions, and will misrepresent the relative cost of the two
// code paths being compared here.

#include "gl/ResourceManager.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EnvironmentConfig.h"
#include "mdl/GameConfigFixture.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "mdl/Node.h"
#include "mdl/TestUtils.h"
#include "mdl/WorldNode.h"
#include "ui/AppControllerFixture.h"
#include "ui/MapDocument.h"
#include "ui/OutlinerModel.h"

#include "kd/task_manager.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace tb::ui
{
namespace
{

std::filesystem::path norrathMapsRoot()
{
  const auto* home = std::getenv("HOME");
  REQUIRE(home != nullptr);
  return std::filesystem::path{home} / "norrath-maps";
}

/**
 * Builds a GameInfo good enough to load the converted EverQuest .map files: Valve-format
 * brush geometry plus func_group entities used as layers/groups, referencing materials
 * as "<zone>/<name>" under norrathMapsRoot()/textures. Whether every material actually
 * resolves does not matter for this benchmark - only the shape of the resulting node
 * tree does, and that is determined entirely by parsing, not by material lookup.
 */
mdl::GameInfo everQuestGameInfo()
{
  auto gameConfig = mdl::GameConfig{};
  gameConfig.name = "EverQuest";
  gameConfig.fileFormats = {{.format = "Valve", .initialMap = {}}};
  gameConfig.materialConfig = {
    .root = "textures",
    .extensions = {".png"},
    .palette = {},
    .property = std::nullopt,
    .shaderSearchPath = {},
    .excludes = {},
  };
  gameConfig.forceEmptyNewMap = true;

  return mdl::detail::makeGameInfoFixture(std::move(gameConfig), norrathMapsRoot());
}

/**
 * Finds the first BrushNode anywhere in the tree - stands in for "the brush the user
 * currently has an extrude handle on".
 */
mdl::BrushNode* findFirstBrush(mdl::Node& node)
{
  if (auto* brushNode = dynamic_cast<mdl::BrushNode*>(&node))
  {
    return brushNode;
  }
  for (auto* child : node.children())
  {
    if (auto* found = findFirstBrush(*child))
    {
      return found;
    }
  }
  return nullptr;
}

size_t countBrushes(mdl::Node& node)
{
  if (dynamic_cast<mdl::BrushNode*>(&node))
  {
    return 1;
  }
  auto count = size_t{0};
  for (auto* child : node.children())
  {
    count += countBrushes(*child);
  }
  return count;
}

/**
 * Collects up to `limit` brush nodes scattered across the tree (every node visited, not
 * just the first), used to measure indexForNode()'s std::find-over-siblings cost across
 * a representative sample rather than a single always-first-child node.
 */
void collectBrushes(mdl::Node& node, std::vector<mdl::BrushNode*>& out, const size_t limit)
{
  if (out.size() >= limit)
  {
    return;
  }
  if (auto* brushNode = dynamic_cast<mdl::BrushNode*>(&node))
  {
    out.push_back(brushNode);
    return;
  }
  for (auto* child : node.children())
  {
    collectBrushes(*child, out, limit);
    if (out.size() >= limit)
    {
      return;
    }
  }
}

/**
 * Reproduces the algorithm that used to run on every documentDidChangeNotifier firing
 * (removed from OutlinerModel in this change): walk the entire tree and emit dataChanged
 * for every row, recursively (this is OutlinerModel::emitDataChangedRecursive, deleted
 * from the production model by this change). It is rebuilt here, against the model's
 * public API, purely to measure what that handler cost.
 */
void oldFullTreeRefresh(OutlinerModel& model, const QModelIndex& parent = {})
{
  const auto rc = model.rowCount(parent);
  if (rc == 0)
  {
    return;
  }

  emit model.dataChanged(
    model.index(0, 0, parent), model.index(rc - 1, OutlinerModel::ColumnCount - 1, parent));

  for (int row = 0; row < rc; ++row)
  {
    oldFullTreeRefresh(model, model.index(row, 0, parent));
  }
}

struct LoadedBenchmarkMap
{
  // Everything is heap-allocated and kept behind pointers/unique_ptrs that are only ever
  // moved as pointers, never as the pointed-to objects: MapDocument's own move
  // constructor needs kdl::task_manager and gl::ResourceManager to be complete, which
  // would otherwise force a lot of unrelated headers into this translation unit.
  std::unique_ptr<kdl::task_manager> taskManager;
  std::unique_ptr<gl::ResourceManager> resourceManager;
  std::unique_ptr<MapDocument> document;
  mdl::BrushNode* brush = nullptr;
};

std::optional<LoadedBenchmarkMap> loadBenchmarkMap(const std::string& zoneFileName)
{
  const auto path = norrathMapsRoot() / "maps" / zoneFileName;
  if (!std::filesystem::exists(path))
  {
    WARN(
      "Skipping benchmark: " << path.string()
                              << " not found (expected real EverQuest maps under "
                                 "~/norrath-maps/maps)");
    return std::nullopt;
  }

  auto result = LoadedBenchmarkMap{};
  result.taskManager = createTestTaskManager();
  result.resourceManager = std::make_unique<gl::ResourceManager>();

  // Deliberately NOT MapDocumentFixture::load(): that convenience wrapper hardcodes
  // vm::bbox3d{8192.0} as the world bounds regardless of what's being loaded. These
  // converted EverQuest zones have geometry tens to hundreds of thousands of units out
  // (see the "Raise hard world bounds" commit this branch already carries), so with the
  // old +/-8192 bounds almost everything gets rejected as "out of world bounds" and
  // silently dropped during parsing - which would make this benchmark run against a tiny
  // fraction of the real node count. Loading with MapDocument::DefaultWorldBounds
  // (+/-524288) directly is what the real application now does for these maps.
  result.document = MapDocument::loadDocument(
                       mdl::EnvironmentConfig{},
                       everQuestGameInfo(),
                       mdl::MapFormat::Valve,
                       MapDocument::DefaultWorldBounds,
                       path,
                       *result.taskManager,
                       *result.resourceManager)
                       .value();

  result.brush = findFirstBrush(result.document->map().worldNode());
  REQUIRE(result.brush != nullptr);

  return result;
}

template <typename F>
double timeMillis(F&& f)
{
  const auto start = std::chrono::steady_clock::now();
  f();
  const auto end = std::chrono::steady_clock::now();
  return std::chrono::duration<double, std::milli>(end - start).count();
}

void runBenchmark(const std::string& zoneFileName, const int simulatedMouseMoves)
{
  auto loaded = loadBenchmarkMap(zoneFileName);
  if (!loaded)
  {
    return;
  }

  auto& document = *loaded->document;
  auto model = OutlinerModel{document};

  std::cout << "[OutlinerModelBenchmark] " << zoneFileName
            << ": loaded brush count = " << countBrushes(document.map().worldNode())
            << '\n';

  const auto changedNodes =
    std::vector<mdl::Node*>{static_cast<mdl::Node*>(loaded->brush)};

  // "Before": what every documentDidChangeNotifier used to cost, reproduced against the
  // model's public API (see oldFullTreeRefresh doc comment above).
  const auto beforeMs = timeMillis([&] {
    for (auto i = 0; i < simulatedMouseMoves; ++i)
    {
      oldFullTreeRefresh(model);
    }
  });

  // "After": what actually runs now - nodesDidChangeNotifier carries the changed nodes,
  // so only their rows are touched.
  const auto afterMs = timeMillis([&] {
    for (auto i = 0; i < simulatedMouseMoves; ++i)
    {
      document.nodesDidChangeNotifier(changedNodes);
    }
  });

  // Separately: is indexForNode()'s std::find-over-siblings (used by both the old and
  // new handlers, and by selection sync) actually a cost worth caching around? Sample
  // indexForNode() over many brushes scattered across the tree.
  auto sampleBrushes = std::vector<mdl::BrushNode*>{};
  collectBrushes(document.map().worldNode(), sampleBrushes, 5000);

  auto sampleCount = 0;
  const auto indexForNodeMs = timeMillis([&] {
    for (auto* brushNode : sampleBrushes)
    {
      const auto idx = model.indexForNode(brushNode);
      sampleCount += idx.isValid() ? 1 : 0;
    }
  });

  std::cout << "[OutlinerModelBenchmark] " << zoneFileName << ": "
            << simulatedMouseMoves << " simulated mouse-moves\n"
            << "  before (full-tree walk per notification):   " << beforeMs << " ms ("
            << (beforeMs / simulatedMouseMoves) << " ms/move)\n"
            << "  after  (targeted refreshDataForNodes):       " << afterMs << " ms ("
            << (afterMs / simulatedMouseMoves) << " ms/move)\n"
            << "  speedup: " << (beforeMs / std::max(afterMs, 0.001)) << "x\n"
            << "  indexForNode() over " << sampleBrushes.size() << " brushes: "
            << indexForNodeMs << " ms total ("
            << (indexForNodeMs * 1000.0 / static_cast<double>(sampleBrushes.size()))
            << " us/call, " << sampleCount << " valid)\n";
}

} // namespace

TEST_CASE("OutlinerModelBenchmark", "[.][benchmark]")
{
  auto appControllerFixture = AppControllerFixture{};

  // 8,505 brushes.
  runBenchmark("befallen.map", 200);

  // 75,251 brushes - the useful extreme.
  runBenchmark("firiona.map", 200);
}

} // namespace tb::ui
