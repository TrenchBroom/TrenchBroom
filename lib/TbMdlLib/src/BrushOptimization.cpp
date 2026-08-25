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

#include "mdl/BrushOptimization.h"

#include "mdl/Brush.h"

#include "vm/vec.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <tuple>
#include <vector>

namespace tb::mdl
{
namespace
{

constexpr size_t MaxCellCount = 1'000'000u;
constexpr size_t MaxCandidateCount = 12u;
constexpr size_t MaxExactCellCount = 60u;
constexpr size_t MaxExactBoxCount = 8'192u;
constexpr size_t MaxExactSearchNodeCount = 250'000u;

using Index3 = std::array<size_t, 3u>;
using Coordinates = std::array<std::vector<double>, 3u>;

bool boundsLess(const vm::bbox3d& lhs, const vm::bbox3d& rhs)
{
  return std::tuple{
           lhs.min[0], lhs.min[1], lhs.min[2], lhs.max[0], lhs.max[1], lhs.max[2]}
         < std::tuple{
           rhs.min[0], rhs.min[1], rhs.min[2], rhs.max[0], rhs.max[1], rhs.max[2]};
}

std::vector<vm::bbox3d> canonicalize(std::vector<vm::bbox3d> bounds)
{
  std::ranges::sort(bounds, boundsLess);
  return bounds;
}

bool boundsEqual(const std::vector<vm::bbox3d>& lhs, const std::vector<vm::bbox3d>& rhs)
{
  return lhs.size() == rhs.size()
         && std::ranges::equal(
           lhs, rhs, [](const auto& l, const auto& r) { return l == r; });
}

size_t cellIndex(const Index3& index, const Index3& size)
{
  return (index[0] * size[1] + index[1]) * size[2] + index[2];
}

size_t coordinateIndex(const std::vector<double>& coordinates, const double value)
{
  return size_t(std::ranges::lower_bound(coordinates, value) - coordinates.begin());
}

bool forEachCell(const Index3& min, const Index3& max, const auto& callback)
{
  for (auto x = min[0]; x < max[0]; ++x)
  {
    for (auto y = min[1]; y < max[1]; ++y)
    {
      for (auto z = min[2]; z < max[2]; ++z)
      {
        if (!callback(Index3{x, y, z}))
        {
          return false;
        }
      }
    }
  }
  return true;
}

bool canExpand(
  const Index3& min,
  const Index3& max,
  const size_t axis,
  const bool reverse,
  const Index3& size,
  const std::vector<bool>& occupied,
  const std::vector<bool>& claimed)
{
  if ((!reverse && max[axis] == size[axis]) || (reverse && min[axis] == 0u))
  {
    return false;
  }

  auto layerMin = min;
  auto layerMax = max;
  if (reverse)
  {
    layerMin[axis] -= 1u;
    layerMax[axis] = min[axis];
  }
  else
  {
    layerMin[axis] = max[axis];
    layerMax[axis] += 1u;
  }

  return forEachCell(layerMin, layerMax, [&](const auto& index) {
    const auto flatIndex = cellIndex(index, size);
    return occupied[flatIndex] && !claimed[flatIndex];
  });
}

std::vector<vm::bbox3d> decompose(
  const Coordinates& coordinates,
  const Index3& size,
  const std::vector<bool>& occupied,
  const Index3& axisOrder,
  const Index3& reverse)
{
  auto claimed = std::vector<bool>(occupied.size(), false);
  auto result = std::vector<vm::bbox3d>{};

  for (size_t px = 0u; px < size[0]; ++px)
  {
    for (size_t py = 0u; py < size[1]; ++py)
    {
      for (size_t pz = 0u; pz < size[2]; ++pz)
      {
        const auto position = Index3{px, py, pz};
        auto index = Index3{};
        for (size_t axis = 0u; axis < 3u; ++axis)
        {
          index[axis] =
            reverse[axis] != 0u ? size[axis] - position[axis] - 1u : position[axis];
        }

        const auto flatIndex = cellIndex(index, size);
        if (!occupied[flatIndex] || claimed[flatIndex])
        {
          continue;
        }

        auto min = index;
        auto max = Index3{index[0] + 1u, index[1] + 1u, index[2] + 1u};
        for (const auto axis : axisOrder)
        {
          while (canExpand(min, max, axis, reverse[axis] != 0u, size, occupied, claimed))
          {
            if (reverse[axis] != 0u)
            {
              min[axis] -= 1u;
            }
            else
            {
              max[axis] += 1u;
            }
          }
        }

        forEachCell(min, max, [&](const auto& cell) {
          claimed[cellIndex(cell, size)] = true;
          return true;
        });

        result.emplace_back(
          vm::vec3d{
            coordinates[0][min[0]], coordinates[1][min[1]], coordinates[2][min[2]]},
          vm::vec3d{
            coordinates[0][max[0]], coordinates[1][max[1]], coordinates[2][max[2]]});
      }
    }
  }

  return canonicalize(std::move(result));
}

double surfaceArea(const std::vector<vm::bbox3d>& bounds)
{
  return std::accumulate(
    bounds.begin(), bounds.end(), 0.0, [](const auto result, const auto& box) {
      const auto size = box.size();
      return result + 2.0 * (size[0] * size[1] + size[0] * size[2] + size[1] * size[2]);
    });
}

double exteriorArea(
  const Coordinates& coordinates, const Index3& size, const std::vector<bool>& occupied)
{
  auto result = 0.0;
  forEachCell(Index3{0u, 0u, 0u}, size, [&](const auto& index) {
    if (!occupied[cellIndex(index, size)])
    {
      return true;
    }

    const auto cellSize = vm::vec3d{
      coordinates[0][index[0] + 1u] - coordinates[0][index[0]],
      coordinates[1][index[1] + 1u] - coordinates[1][index[1]],
      coordinates[2][index[2] + 1u] - coordinates[2][index[2]]};
    for (size_t axis = 0u; axis < 3u; ++axis)
    {
      const auto faceArea = cellSize[(axis + 1u) % 3u] * cellSize[(axis + 2u) % 3u];

      auto neighbor = index;
      if (index[axis] == 0u)
      {
        result += faceArea;
      }
      else
      {
        neighbor[axis] -= 1u;
        if (!occupied[cellIndex(neighbor, size)])
        {
          result += faceArea;
        }
      }

      neighbor = index;
      if (index[axis] + 1u == size[axis])
      {
        result += faceArea;
      }
      else
      {
        neighbor[axis] += 1u;
        if (!occupied[cellIndex(neighbor, size)])
        {
          result += faceArea;
        }
      }
    }
    return true;
  });
  return result;
}

double sliverPenalty(const std::vector<vm::bbox3d>& bounds)
{
  return std::accumulate(
    bounds.begin(), bounds.end(), 0.0, [](const auto result, const auto& box) {
      const auto size = box.size();
      const auto minSize = std::min({size[0], size[1], size[2]});
      const auto maxSize = std::max({size[0], size[1], size[2]});
      return result + maxSize / minSize;
    });
}

struct ExactBox
{
  vm::bbox3d bounds;
  uint64_t cellMask;
  size_t cellCount;
};

/**
 * Enumerates every cuboid that contains only occupied cells. This is intentionally
 * limited to small cell grids so that optimization remains interactive.
 */
std::vector<ExactBox> createExactBoxes(
  const Coordinates& coordinates, const Index3& size, const std::vector<bool>& occupied)
{
  const auto occupiedCount = size_t(std::ranges::count(occupied, true));
  if (occupiedCount == 0u || occupiedCount > MaxExactCellCount)
  {
    return {};
  }

  auto occupiedBits = std::vector<size_t>(occupied.size(), occupiedCount);
  auto nextBit = size_t{0u};
  for (size_t i = 0u; i < occupied.size(); ++i)
  {
    if (occupied[i])
    {
      occupiedBits[i] = nextBit++;
    }
  }

  auto result = std::vector<ExactBox>{};
  for (size_t x0 = 0u; x0 < size[0]; ++x0)
  {
    for (size_t y0 = 0u; y0 < size[1]; ++y0)
    {
      for (size_t z0 = 0u; z0 < size[2]; ++z0)
      {
        const auto min = Index3{x0, y0, z0};
        if (!occupied[cellIndex(min, size)])
        {
          continue;
        }

        for (size_t x1 = x0 + 1u; x1 <= size[0]; ++x1)
        {
          for (size_t y1 = y0 + 1u; y1 <= size[1]; ++y1)
          {
            for (size_t z1 = z0 + 1u; z1 <= size[2]; ++z1)
            {
              const auto max = Index3{x1, y1, z1};
              auto cellMask = uint64_t{0u};
              const auto valid = forEachCell(min, max, [&](const auto& index) {
                const auto flatIndex = cellIndex(index, size);
                if (!occupied[flatIndex])
                {
                  return false;
                }
                cellMask |= uint64_t{1u} << occupiedBits[flatIndex];
                return true;
              });
              if (!valid)
              {
                continue;
              }

              result.push_back(
                {vm::bbox3d{
                   vm::vec3d{coordinates[0][x0], coordinates[1][y0], coordinates[2][z0]},
                   vm::vec3d{coordinates[0][x1], coordinates[1][y1], coordinates[2][z1]}},
                 cellMask,
                 size_t(std::popcount(cellMask))});
              if (result.size() > MaxExactBoxCount)
              {
                return {};
              }
            }
          }
        }
      }
    }
  }
  return result;
}

/**
 * Finds minimum-cardinality exact covers of the occupied cells using the enumerated
 * cuboids. The search has a node budget; the greedy candidates remain available if the
 * budget is exhausted before proving a better cover.
 */
std::vector<std::vector<vm::bbox3d>> findExactDecompositions(
  const std::vector<ExactBox>& boxes,
  const size_t occupiedCellCount,
  const size_t upperBound)
{
  if (boxes.empty() || occupiedCellCount == 0u || occupiedCellCount > 63u)
  {
    return {};
  }

  const auto fullMask = (uint64_t{1u} << occupiedCellCount) - 1u;
  auto boxesByCell = std::vector<std::vector<size_t>>(occupiedCellCount);
  for (size_t boxIndex = 0u; boxIndex < boxes.size(); ++boxIndex)
  {
    auto mask = boxes[boxIndex].cellMask;
    while (mask != 0u)
    {
      const auto bit = size_t(std::countr_zero(mask));
      boxesByCell[bit].push_back(boxIndex);
      mask &= mask - 1u;
    }
  }
  for (auto& boxIndices : boxesByCell)
  {
    std::ranges::sort(boxIndices, [&](const auto lhs, const auto rhs) {
      return boxes[lhs].cellCount > boxes[rhs].cellCount;
    });
  }

  auto result = std::vector<std::vector<vm::bbox3d>>{};
  auto selectedBoxes = std::vector<size_t>{};
  auto bestCount = upperBound;
  auto visitedNodeCount = size_t{0u};

  const auto search = [&](auto&& thisLambda, const uint64_t coveredMask) -> void {
    if (++visitedNodeCount > MaxExactSearchNodeCount)
    {
      return;
    }

    const auto uncoveredMask = fullMask & ~coveredMask;
    if (uncoveredMask == 0u)
    {
      if (selectedBoxes.size() < bestCount)
      {
        bestCount = selectedBoxes.size();
        result.clear();
      }
      if (result.size() < MaxCandidateCount)
      {
        auto solution = std::vector<vm::bbox3d>{};
        solution.reserve(selectedBoxes.size());
        for (const auto boxIndex : selectedBoxes)
        {
          solution.push_back(boxes[boxIndex].bounds);
        }
        result.push_back(canonicalize(std::move(solution)));
      }
      return;
    }

    if (selectedBoxes.size() >= bestCount)
    {
      return;
    }

    auto chosenBit = size_t{0u};
    auto chosenCandidateCount = std::numeric_limits<size_t>::max();
    auto remainingBits = uncoveredMask;
    while (remainingBits != 0u)
    {
      const auto bit = size_t(std::countr_zero(remainingBits));
      const auto candidateCount =
        size_t(std::ranges::count_if(boxesByCell[bit], [&](const auto boxIndex) {
          return (boxes[boxIndex].cellMask & coveredMask) == 0u;
        }));
      if (candidateCount < chosenCandidateCount)
      {
        chosenBit = bit;
        chosenCandidateCount = candidateCount;
      }
      remainingBits &= remainingBits - 1u;
    }

    for (const auto boxIndex : boxesByCell[chosenBit])
    {
      const auto& box = boxes[boxIndex];
      if ((box.cellMask & coveredMask) != 0u)
      {
        continue;
      }

      selectedBoxes.push_back(boxIndex);
      thisLambda(thisLambda, coveredMask | box.cellMask);
      selectedBoxes.pop_back();
    }
  };
  search(search, 0u);
  return result;
}

} // namespace

bool isAxisAlignedCuboid(const Brush& brush)
{
  if (brush.faceCount() != 6u)
  {
    return false;
  }

  for (size_t axis = 0u; axis < 3u; ++axis)
  {
    const auto normal = vm::vec3d::axis(vm::axis::type(axis));
    if (!brush.findFace(normal) || !brush.findFace(-normal))
    {
      return false;
    }
  }
  return true;
}

std::vector<BrushOptimizationCandidate> createBrushOptimizationCandidates(
  const std::vector<vm::bbox3d>& inputBounds)
{
  if (inputBounds.empty())
  {
    return {};
  }

  auto coordinates = Coordinates{};
  for (const auto& bounds : inputBounds)
  {
    for (size_t axis = 0u; axis < 3u; ++axis)
    {
      if (bounds.min[axis] >= bounds.max[axis])
      {
        return {};
      }
      coordinates[axis].push_back(bounds.min[axis]);
      coordinates[axis].push_back(bounds.max[axis]);
    }
  }

  auto size = Index3{};
  auto cellCount = size_t{1u};
  for (size_t axis = 0u; axis < 3u; ++axis)
  {
    std::ranges::sort(coordinates[axis]);
    const auto [firstDuplicate, end] = std::ranges::unique(coordinates[axis]);
    coordinates[axis].erase(firstDuplicate, end);
    size[axis] = coordinates[axis].size() - 1u;

    if (size[axis] > MaxCellCount / cellCount)
    {
      return {};
    }
    cellCount *= size[axis];
  }

  auto occupied = std::vector<bool>(cellCount, false);
  for (const auto& bounds : inputBounds)
  {
    const auto min = Index3{
      coordinateIndex(coordinates[0], bounds.min[0]),
      coordinateIndex(coordinates[1], bounds.min[1]),
      coordinateIndex(coordinates[2], bounds.min[2])};
    const auto max = Index3{
      coordinateIndex(coordinates[0], bounds.max[0]),
      coordinateIndex(coordinates[1], bounds.max[1]),
      coordinateIndex(coordinates[2], bounds.max[2])};
    forEachCell(min, max, [&](const auto& index) {
      occupied[cellIndex(index, size)] = true;
      return true;
    });
  }

  const auto original = canonicalize(inputBounds);
  constexpr auto AxisOrders = std::array{
    Index3{0u, 1u, 2u},
    Index3{0u, 2u, 1u},
    Index3{1u, 0u, 2u},
    Index3{1u, 2u, 0u},
    Index3{2u, 0u, 1u},
    Index3{2u, 1u, 0u}};

  const auto unionExteriorArea = exteriorArea(coordinates, size, occupied);
  auto candidates = std::vector<BrushOptimizationCandidate>{};
  const auto addCandidate = [&](std::vector<vm::bbox3d> candidateBounds) {
    if (
      candidateBounds.size() > inputBounds.size()
      || boundsEqual(candidateBounds, original)
      || std::ranges::any_of(candidates, [&](const auto& candidate) {
           return boundsEqual(candidate.bounds, candidateBounds);
         }))
    {
      return;
    }

    const auto internalFaceArea =
      std::max(0.0, (surfaceArea(candidateBounds) - unionExteriorArea) / 2.0);
    candidates.push_back({std::move(candidateBounds), internalFaceArea});
  };

  for (const auto& axisOrder : AxisOrders)
  {
    for (size_t directionMask = 0u; directionMask < 8u; ++directionMask)
    {
      const auto reverse =
        Index3{directionMask & 1u, directionMask & 2u, directionMask & 4u};
      addCandidate(decompose(coordinates, size, occupied, axisOrder, reverse));
    }
  }

  const auto occupiedCellCount = size_t(std::ranges::count(occupied, true));
  const auto upperBound = std::accumulate(
    candidates.begin(),
    candidates.end(),
    inputBounds.size(),
    [](const auto result, const auto& candidate) {
      return std::min(result, candidate.bounds.size());
    });
  const auto exactBoxes = createExactBoxes(coordinates, size, occupied);
  for (auto& exactBounds :
       findExactDecompositions(exactBoxes, occupiedCellCount, upperBound))
  {
    addCandidate(std::move(exactBounds));
  }

  std::ranges::sort(candidates, [](const auto& lhs, const auto& rhs) {
    return std::tuple{lhs.bounds.size(), lhs.internalFaceArea, sliverPenalty(lhs.bounds)}
           < std::tuple{
             rhs.bounds.size(), rhs.internalFaceArea, sliverPenalty(rhs.bounds)};
  });
  if (candidates.size() > MaxCandidateCount)
  {
    candidates.resize(MaxCandidateCount);
  }
  return candidates;
}

} // namespace tb::mdl
