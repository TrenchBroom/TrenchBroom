/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include "vm/vec.h"

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace tb::ui
{

/**
 * Canonical content identities used by the optional EQ exporter provenance
 * sidecar. These functions deliberately do not load the sidecar or depend on
 * editor state: they reproduce schema v1 over already-solved map geometry.
 */
struct MapProvenancePlane
{
  vm::vec3d normal;
  double distance;
};

struct MapProvenanceFace
{
  MapProvenancePlane plane;
  std::vector<vm::vec3d> polygon;
};

struct MapProvenanceAxis
{
  vm::vec3d axis;
  double offset;
  double scale;
  std::optional<double> rotation;
};

struct MapProvenancePresentation
{
  std::string material;
  std::optional<double> contents;
  std::optional<double> flags;
  std::optional<double> value;
  std::optional<std::array<double, 4>> color;
  MapProvenanceAxis u;
  MapProvenanceAxis v;
};

enum class MapProvenanceDiagnosticCode
{
  PresentationScaleUsesTrenchBroomSafeFallback,
  PresentationContainsNonFiniteValue,
  PresentationIntegerExpected,
};

struct MapProvenanceDiagnostic
{
  MapProvenanceDiagnosticCode code;
  std::string field;
};

struct MapProvenancePresentationResult
{
  std::optional<std::string> fingerprint;
  std::optional<std::string> id;
  std::optional<std::string> preimage;
  std::vector<MapProvenanceDiagnostic> diagnostics;
};

std::string canonicalMapProvenancePlane(const MapProvenancePlane& plane);
std::string canonicalMapProvenanceFacePolygon(const MapProvenanceFace& face);

std::string mapProvenanceBrushGeometryFingerprint(
  const std::vector<MapProvenancePlane>& planes);
std::string mapProvenanceBrushId(const std::vector<MapProvenancePlane>& planes);
std::string mapProvenanceFaceGeometryFingerprint(
  const std::string& brushFingerprint, const MapProvenanceFace& face);
std::string mapProvenanceFaceId(
  const std::string& brushFingerprint, const MapProvenanceFace& face);

MapProvenancePresentationResult mapProvenancePresentationFingerprint(
  const MapProvenancePresentation& presentation);

} // namespace tb::ui
