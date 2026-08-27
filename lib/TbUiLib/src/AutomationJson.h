/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include "mdl/Map_Geometry.h"
#include "mdl/Node.h"
#include "ui/LocalJsonRpcServer.h"

#include "vm/vec.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace tb::mdl
{
class BrushNode;
class WorldNode;
} // namespace tb::mdl

namespace tb::ui
{
struct MapViewContext;
struct MapViewPickResult;

namespace automation
{

struct NodeQuery
{
  QString pattern;
  std::vector<QString> types;
  std::vector<QString> materials;
  std::vector<QString> classnames;
  std::optional<vm::bbox3d> bounds;
  std::optional<mdl::NodePath> ancestorPath;
  int offset = 0;
  int limit = 200;
  bool aggregate = false;
  bool pathsOnly = false;
};

struct NodeQueryResult
{
  QJsonArray nodes;
  QJsonObject aggregate;
  int total = 0;
  bool truncated = false;
  std::optional<int> nextOffset;
};

/**
 * A read-only selector for faces that lie in an axis-aligned plane. The optional
 * material filter is intentionally exact: callers can use it to distinguish a
 * water surface from the matching floor or volume face at the same height.
 */
struct FootprintFaceSelector
{
  vm::axis::type axis;
  double coordinate;
  std::optional<std::string> material;
};

struct FootprintSource
{
  mdl::NodePath path;
  size_t faceIndex;
};

struct BrushFootprint
{
  std::vector<vm::vec3d> vertices;
  vm::bbox3d bounds;
  std::vector<FootprintSource> sources;
};

JsonRpcResponse invalidParams(const QString& message);
JsonRpcResponse revisionConflict(size_t actualRevision);

QJsonObject contextToJson(const MapViewContext& context);
QJsonObject pickToJson(const MapViewPickResult& pick);
QJsonObject brushToJson(const mdl::BrushNode& brush, const mdl::WorldNode& world);
/**
 * Extracts canonical, convex face polygons from explicit brushes. Identical polygons
 * are coalesced while retaining every source face, which makes overlapping generated
 * geometry visible without making clients deduplicate it themselves.
 */
std::vector<BrushFootprint> extractFootprints(
  const mdl::WorldNode& world,
  const std::vector<const mdl::BrushNode*>& brushes,
  const FootprintFaceSelector& selector);
QJsonArray footprintsToJson(const std::vector<BrushFootprint>& footprints);
QJsonArray queryNodes(const mdl::WorldNode& world, const QString& pattern, int limit);
NodeQueryResult queryNodes(const mdl::WorldNode& world, const NodeQuery& query);

std::optional<mdl::NodePath> nodePathFromJson(const QJsonValue& value);
/**
 * Decodes a non-negative JSON integer that fits in size_t. JSON represents all numbers
 * as doubles, so rejecting fractional and out-of-range values is essential before a
 * node path, face index, or revision number is used as an integer.
 */
std::optional<size_t> sizeFromJson(const QJsonValue& value);
std::optional<vm::vec3d> vec3FromJson(const QJsonValue& value);

/**
 * Parses the public JSON form of a Z-axis planar profile. Keeping this conversion
 * separate from the RPC dispatcher makes its grid snapping and role/material defaults
 * directly testable without a live map window.
 */
std::optional<mdl::PlanarProfileSpec> planarProfileSpecFromJson(
  const QJsonObject& params);

/** Parses a grid-snapped profile extrusion authored in one of the principal planes. */
std::optional<mdl::ProfileExtrusionSpec> profileExtrusionSpecFromJson(
  const QJsonObject& params);

} // namespace automation
} // namespace tb::ui
