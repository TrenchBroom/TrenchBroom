/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceSolidSpace.h"

#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Picking.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <ranges>
#include <utility>

namespace tb::ui
{
namespace
{

AcceptanceSolidSpaceError error(std::string message)
{
  return {std::move(message)};
}

bool finite(const vm::vec3d& point)
{
  return std::isfinite(point.x()) && std::isfinite(point.y()) && std::isfinite(point.z());
}

Result<size_t, AcceptanceSolidSpaceError> cellsAlongAxis(
  const double min, const double max, const double cellSize)
{
  const auto count = std::ceil((max - min) / cellSize);
  if (
    !std::isfinite(count) || count < 1.0
    || count > static_cast<double>(std::numeric_limits<size_t>::max()))
    return error("Solid-space sampling resolution is out of range");
  return static_cast<size_t>(count);
}

Result<size_t, AcceptanceSolidSpaceError> checkedCellCount(
  const size_t x, const size_t y, const size_t z, const size_t maxSamples)
{
  if (x > maxSamples / y || x * y > maxSamples / z)
    return error("Solid-space sampling exceeds maxSamples");
  return x * y * z;
}

vm::bbox3d cellBounds(
  const vm::bbox3d& bounds,
  const double cellSize,
  const size_t x,
  const size_t y,
  const size_t z)
{
  const auto min = vm::vec3d{
    bounds.min.x() + static_cast<double>(x) * cellSize,
    bounds.min.y() + static_cast<double>(y) * cellSize,
    bounds.min.z() + static_cast<double>(z) * cellSize};
  const auto max = vm::vec3d{
    std::min(min.x() + cellSize, bounds.max.x()),
    std::min(min.y() + cellSize, bounds.max.y()),
    std::min(min.z() + cellSize, bounds.max.z())};
  return {min, max};
}

void addDiscrepancy(
  AcceptanceSolidSpaceDiscrepancy& discrepancy, const vm::bbox3d& bounds)
{
  ++discrepancy.cellCount;
  discrepancy.bounds =
    discrepancy.bounds ? vm::merge(*discrepancy.bounds, bounds) : bounds;
  discrepancy.cells.push_back(bounds);
}

} // namespace

AcceptanceMapSolidSpaceQuery::AcceptanceMapSolidSpaceQuery(
  mdl::Map& map, std::shared_ptr<const void> keepAlive)
  : m_map{map}
  , m_keepAlive{std::move(keepAlive)}
{
}

Result<bool, AcceptanceSolidSpaceError> AcceptanceMapSolidSpaceQuery::isSolid(
  const vm::vec3d& point) const
{
  if (!finite(point))
    return error("Solid-space sample point must be finite");
  const auto containing = mdl::findNodesContaining(m_map, point);
  return std::ranges::any_of(containing, [](const auto* node) {
    return dynamic_cast<const mdl::BrushNode*>(node) != nullptr;
  });
}

Result<AcceptanceSolidSpaceComparisonReport, AcceptanceSolidSpaceError>
AcceptanceSolidSpaceComparison::compare(
  const AcceptanceSolidSpaceQuery& reference,
  const AcceptanceSolidSpaceQuery& candidate,
  const AcceptanceSolidSpaceComparisonOptions& options) const
{
  if (
    !finite(options.bounds.min) || !finite(options.bounds.max)
    || !options.bounds.is_valid() || options.bounds.is_empty())
    return error("Solid-space comparison requires finite, nonempty bounds");
  if (!std::isfinite(options.cellSize) || options.cellSize <= 0.0)
    return error("Solid-space comparison requires a finite positive cellSize");
  if (options.maxSamples == 0u)
    return error("Solid-space comparison requires maxSamples greater than zero");

  const auto xCells =
    cellsAlongAxis(options.bounds.min.x(), options.bounds.max.x(), options.cellSize);
  const auto yCells =
    cellsAlongAxis(options.bounds.min.y(), options.bounds.max.y(), options.cellSize);
  const auto zCells =
    cellsAlongAxis(options.bounds.min.z(), options.bounds.max.z(), options.cellSize);
  if (xCells.is_error())
    return std::get<AcceptanceSolidSpaceError>(xCells.error());
  if (yCells.is_error())
    return std::get<AcceptanceSolidSpaceError>(yCells.error());
  if (zCells.is_error())
    return std::get<AcceptanceSolidSpaceError>(zCells.error());
  const auto totalCells =
    checkedCellCount(xCells.value(), yCells.value(), zCells.value(), options.maxSamples);
  if (totalCells.is_error())
    return std::get<AcceptanceSolidSpaceError>(totalCells.error());

  auto report = AcceptanceSolidSpaceComparisonReport{};
  report.totalCells = totalCells.value();
  for (size_t z = 0u; z < zCells.value(); ++z)
  {
    for (size_t y = 0u; y < yCells.value(); ++y)
    {
      for (size_t x = 0u; x < xCells.value(); ++x)
      {
        if (options.cancelled && options.cancelled())
        {
          report.status = AcceptanceSolidSpaceComparisonStatus::Cancelled;
          return report;
        }
        const auto bounds = cellBounds(options.bounds, options.cellSize, x, y, z);
        const auto sample = (bounds.min + bounds.max) / 2.0;
        const auto referenceSolid = reference.isSolid(sample);
        if (referenceSolid.is_error())
          return std::get<AcceptanceSolidSpaceError>(referenceSolid.error());
        const auto candidateSolid = candidate.isSolid(sample);
        if (candidateSolid.is_error())
          return std::get<AcceptanceSolidSpaceError>(candidateSolid.error());

        ++report.sampledCells;
        if (!referenceSolid.value() && candidateSolid.value())
          addDiscrepancy(report.newlySolid, bounds);
        else if (referenceSolid.value() && !candidateSolid.value())
          addDiscrepancy(report.newlyEmpty, bounds);
      }
    }
  }
  return report;
}

} // namespace tb::ui
