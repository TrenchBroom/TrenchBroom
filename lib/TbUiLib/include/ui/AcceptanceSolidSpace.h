/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include "base/Result.h"

#include "vm/bbox.h"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace tb::mdl
{
class Map;
}

namespace tb::ui
{

struct AcceptanceSolidSpaceError
{
  std::string message;
};

/**
 * Answers whether a sample point occupies space according to one explicit occupancy
 * model. Implementations must not treat brush decomposition as meaningful: overlapping
 * brushes still produce one occupied result.
 */
class AcceptanceSolidSpaceQuery
{
public:
  virtual ~AcceptanceSolidSpaceQuery() = default;
  virtual Result<bool, AcceptanceSolidSpaceError> isSolid(
    const vm::vec3d& point) const = 0;
};

/** The concrete document instance used to answer one comparison side. */
struct AcceptanceSolidSpaceDocument
{
  std::shared_ptr<const AcceptanceSolidSpaceQuery> query;
  std::string documentId;
  size_t revision = 0u;
};

/** Resolves an explicit map path to a stable, non-owning occupancy query. */
class AcceptanceSolidSpaceProvider
{
public:
  virtual ~AcceptanceSolidSpaceProvider() = default;
  virtual Result<AcceptanceSolidSpaceDocument, AcceptanceSolidSpaceError> queryFor(
    const std::filesystem::path& path) = 0;
};

/** V1 brush-volume occupancy query over a single explicitly supplied map. */
class AcceptanceMapSolidSpaceQuery : public AcceptanceSolidSpaceQuery
{
public:
  explicit AcceptanceMapSolidSpaceQuery(
    mdl::Map& map, std::shared_ptr<const void> keepAlive = {});

  Result<bool, AcceptanceSolidSpaceError> isSolid(const vm::vec3d& point) const override;

private:
  mdl::Map& m_map;
  std::shared_ptr<const void> m_keepAlive;
};

enum class AcceptanceSolidSpaceDifference
{
  NewlySolid,
  NewlyEmpty,
};

struct AcceptanceSolidSpaceDiscrepancy
{
  AcceptanceSolidSpaceDifference difference = AcceptanceSolidSpaceDifference::NewlySolid;
  size_t cellCount = 0u;
  std::optional<vm::bbox3d> bounds;
  std::vector<vm::bbox3d> cells;
};

enum class AcceptanceSolidSpaceComparisonStatus
{
  Complete,
  Cancelled,
};

/**
 * Samples cell centres in an explicit half-open region [min, max]. A partially-sized
 * cell is used at the high edge of each axis, so reported bounds never exceed region.
 */
struct AcceptanceSolidSpaceComparisonOptions
{
  vm::bbox3d bounds;
  double cellSize = 0.0;
  size_t maxSamples = 1000000u;
  std::function<bool()> cancelled;
};

struct AcceptanceSolidSpaceComparisonReport
{
  AcceptanceSolidSpaceComparisonStatus status =
    AcceptanceSolidSpaceComparisonStatus::Complete;
  size_t totalCells = 0u;
  size_t sampledCells = 0u;
  AcceptanceSolidSpaceDiscrepancy newlySolid{
    AcceptanceSolidSpaceDifference::NewlySolid, 0u, std::nullopt, {}};
  AcceptanceSolidSpaceDiscrepancy newlyEmpty{
    AcceptanceSolidSpaceDifference::NewlyEmpty, 0u, std::nullopt, {}};
};

/**
 * Compares occupancy rather than brush identity, making results independent of the
 * source maps' brush decomposition. The configured sample cap is validated before any
 * geometry query; cancellation is observed between cells.
 */
class AcceptanceSolidSpaceComparison
{
public:
  Result<AcceptanceSolidSpaceComparisonReport, AcceptanceSolidSpaceError> compare(
    const AcceptanceSolidSpaceQuery& reference,
    const AcceptanceSolidSpaceQuery& candidate,
    const AcceptanceSolidSpaceComparisonOptions& options) const;
};

} // namespace tb::ui
