/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/MapProvenance.h"

#include <QByteArrayView>
#include <QCryptographicHash>
#include <QString>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numbers>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace tb::ui
{
namespace
{

constexpr auto NormalQuantization = 1'000'000.0;
constexpr auto DistanceQuantization = 10'000.0;
constexpr auto VertexQuantization = 10'000.0;
constexpr auto EffectiveAxisQuantization = 1'000'000.0;
constexpr auto OffsetQuantization = 10'000.0;
constexpr auto ValueQuantization = 1'000'000.0;
constexpr auto ColorQuantization = 1'000'000.0;
constexpr auto MaxExactInteger = 9'007'199'254'740'990.0;
constexpr auto SafeScale = 0.001f;

using QuantizedPoint = std::array<int64_t, 3>;

bool finite(const vm::vec3d& value)
{
  return std::isfinite(value.x()) && std::isfinite(value.y()) && std::isfinite(value.z());
}

int64_t quantize(const double value, const double scale)
{
  if (!std::isfinite(value))
    throw std::invalid_argument{"Cannot quantize a non-finite value"};
  const auto scaled = value * scale;
  if (!std::isfinite(scaled) || std::abs(scaled) > MaxExactInteger)
    throw std::invalid_argument{"Quantized value is outside the exact integer range"};
  return static_cast<int64_t>(
    scaled < 0.0 ? -std::floor(-scaled + 0.5) : std::floor(scaled + 0.5));
}

std::string decimal(const int64_t value)
{
  return std::to_string(value == 0 ? 0 : value);
}

std::array<double, 4> normalizedPlane(const MapProvenancePlane& plane)
{
  if (!finite(plane.normal) || !std::isfinite(plane.distance))
    throw std::invalid_argument{"Plane contains a non-finite value"};
  const auto length = std::hypot(plane.normal.x(), plane.normal.y(), plane.normal.z());
  if (!(length > 0.0) || !std::isfinite(length))
    throw std::invalid_argument{"Plane normal is degenerate"};
  return {
    plane.normal.x() / length,
    plane.normal.y() / length,
    plane.normal.z() / length,
    plane.distance / length};
}

std::string sha256(const std::string_view preimage)
{
  auto hash = QCryptographicHash{QCryptographicHash::Sha256};
  hash.addData(QByteArrayView{preimage.data(), static_cast<qsizetype>(preimage.size())});
  return "sha256:" + hash.result().toHex().toStdString();
}

bool samePoint(const QuantizedPoint& lhs, const QuantizedPoint& rhs)
{
  return lhs == rhs;
}

std::array<double, 3> polygonNormal(const std::vector<QuantizedPoint>& points)
{
  auto normal = std::array<double, 3>{0.0, 0.0, 0.0};
  for (size_t i = 0u; i < points.size(); ++i)
  {
    const auto& a = points[i];
    const auto& b = points[(i + 1u) % points.size()];
    normal[0] += static_cast<double>(a[1] - b[1]) * static_cast<double>(a[2] + b[2]);
    normal[1] += static_cast<double>(a[2] - b[2]) * static_cast<double>(a[0] + b[0]);
    normal[2] += static_cast<double>(a[0] - b[0]) * static_cast<double>(a[1] + b[1]);
  }
  return normal;
}

std::string quantizedPointToString(const QuantizedPoint& point)
{
  return decimal(point[0]) + "," + decimal(point[1]) + "," + decimal(point[2]);
}

std::string optionalInteger(
  const std::optional<double>& value,
  const std::string_view field,
  std::vector<MapProvenanceDiagnostic>& diagnostics)
{
  if (!value)
    return "absent";
  if (!std::isfinite(*value))
  {
    diagnostics.push_back(
      {MapProvenanceDiagnosticCode::PresentationContainsNonFiniteValue,
       std::string{field}});
    return {};
  }
  if (std::trunc(*value) != *value)
  {
    diagnostics.push_back(
      {MapProvenanceDiagnosticCode::PresentationIntegerExpected, std::string{field}});
    return {};
  }
  if (
    *value < static_cast<double>(std::numeric_limits<int64_t>::min())
    || *value > static_cast<double>(std::numeric_limits<int64_t>::max()))
  {
    diagnostics.push_back(
      {MapProvenanceDiagnosticCode::PresentationIntegerExpected, std::string{field}});
    return {};
  }
  return "i:" + decimal(static_cast<int64_t>(*value));
}

std::string optionalQuantized(
  const std::optional<double>& value,
  const double scale,
  const std::string_view field,
  std::vector<MapProvenanceDiagnostic>& diagnostics)
{
  if (!value)
    return "absent";
  if (!std::isfinite(*value))
  {
    diagnostics.push_back(
      {MapProvenanceDiagnosticCode::PresentationContainsNonFiniteValue,
       std::string{field}});
    return {};
  }
  return "q:" + decimal(quantize(*value, scale));
}

std::optional<std::string> canonicalAxis(
  const MapProvenanceAxis& input,
  const char name,
  std::vector<MapProvenanceDiagnostic>& diagnostics)
{
  if (
    !finite(input.axis) || !std::isfinite(input.offset) || !std::isfinite(input.scale)
    || (input.rotation && !std::isfinite(*input.rotation)))
  {
    diagnostics.push_back(
      {MapProvenanceDiagnosticCode::PresentationContainsNonFiniteValue,
       std::string(1u, name)});
    return std::nullopt;
  }

  const auto scale32 = static_cast<float>(input.scale);
  if (!std::isfinite(scale32) || std::abs(scale32) <= SafeScale)
  {
    diagnostics.push_back(
      {MapProvenanceDiagnosticCode::PresentationScaleUsesTrenchBroomSafeFallback,
       std::string(1u, name) + ".scale"});
    return std::nullopt;
  }

  const auto effective = vm::vec3d{
    input.axis.x() / static_cast<double>(scale32),
    input.axis.y() / static_cast<double>(scale32),
    input.axis.z() / static_cast<double>(scale32)};
  if (!finite(effective))
  {
    diagnostics.push_back(
      {MapProvenanceDiagnosticCode::PresentationContainsNonFiniteValue,
       std::string(1u, name) + ".effectiveAxis"});
    return std::nullopt;
  }

  return std::string(1u, name)
         + ":q:" + decimal(quantize(effective.x(), EffectiveAxisQuantization)) + ","
         + decimal(quantize(effective.y(), EffectiveAxisQuantization)) + ","
         + decimal(quantize(effective.z(), EffectiveAxisQuantization)) + ","
         + decimal(quantize(input.offset, OffsetQuantization)) + "\n";
}

} // namespace

std::string canonicalMapProvenancePlane(const MapProvenancePlane& plane)
{
  const auto normalized = normalizedPlane(plane);
  return "n:" + decimal(quantize(normalized[0], NormalQuantization)) + ","
         + decimal(quantize(normalized[1], NormalQuantization)) + ","
         + decimal(quantize(normalized[2], NormalQuantization))
         + ";d:" + decimal(quantize(normalized[3], DistanceQuantization));
}

std::string canonicalMapProvenanceFacePolygon(const MapProvenanceFace& face)
{
  const auto normal = normalizedPlane(face.plane);
  auto points = std::vector<QuantizedPoint>{};
  for (const auto& point : face.polygon)
  {
    if (!finite(point))
      throw std::invalid_argument{"Polygon contains a non-finite value"};
    const auto quantized = QuantizedPoint{
      quantize(point.x(), VertexQuantization),
      quantize(point.y(), VertexQuantization),
      quantize(point.z(), VertexQuantization)};
    if (points.empty() || !samePoint(points.back(), quantized))
      points.push_back(quantized);
  }
  if (points.size() > 1u && samePoint(points.front(), points.back()))
    points.pop_back();
  if (points.size() < 3u)
    throw std::invalid_argument{
      "Polygon has fewer than three distinct adjacent vertices"};

  const auto polygon = polygonNormal(points);
  const auto winding =
    polygon[0] * normal[0] + polygon[1] * normal[1] + polygon[2] * normal[2];
  if (winding == 0.0)
    throw std::invalid_argument{"Polygon winding is degenerate"};
  if (winding < 0.0)
    std::ranges::reverse(points);

  const auto first = std::ranges::min_element(points);
  std::rotate(points.begin(), first, points.end());

  auto result = std::string{};
  for (const auto& point : points)
  {
    if (!result.empty())
      result += "|";
    result += quantizedPointToString(point);
  }
  return result;
}

std::string mapProvenanceBrushGeometryFingerprint(
  const std::vector<MapProvenancePlane>& planes)
{
  if (planes.empty())
    throw std::invalid_argument{"Brush has no planes"};
  auto canonical = std::set<std::string>{};
  for (const auto& plane : planes)
    canonical.insert(canonicalMapProvenancePlane(plane));
  auto joined = std::string{};
  for (const auto& plane : canonical)
  {
    if (!joined.empty())
      joined += "|";
    joined += plane;
  }
  return sha256("eqexport:brush-geometry:v1\nplanes:" + joined + "\n");
}

std::string mapProvenanceBrushId(const std::vector<MapProvenancePlane>& planes)
{
  return "brush:v1:" + mapProvenanceBrushGeometryFingerprint(planes);
}

std::string mapProvenanceFaceGeometryFingerprint(
  const std::string& brushFingerprint, const MapProvenanceFace& face)
{
  return sha256(
    "eqexport:face-geometry:v1\nbrush:" + brushFingerprint
    + "\nplane:" + canonicalMapProvenancePlane(face.plane)
    + "\npolygon:" + canonicalMapProvenanceFacePolygon(face) + "\n");
}

std::string mapProvenanceFaceId(
  const std::string& brushFingerprint, const MapProvenanceFace& face)
{
  return "face:v1:" + mapProvenanceFaceGeometryFingerprint(brushFingerprint, face);
}

MapProvenancePresentationResult mapProvenancePresentationFingerprint(
  const MapProvenancePresentation& presentation)
{
  auto diagnostics = std::vector<MapProvenanceDiagnostic>{};
  const auto contents = optionalInteger(presentation.contents, "contents", diagnostics);
  const auto flags = optionalInteger(presentation.flags, "flags", diagnostics);
  const auto value =
    optionalQuantized(presentation.value, ValueQuantization, "value", diagnostics);
  const auto u = canonicalAxis(presentation.u, 'u', diagnostics);
  const auto v = canonicalAxis(presentation.v, 'v', diagnostics);

  auto color = std::string{"absent"};
  if (presentation.color)
  {
    if (!std::ranges::all_of(
          *presentation.color, [](const auto channel) { return std::isfinite(channel); }))
      diagnostics.push_back(
        {MapProvenanceDiagnosticCode::PresentationContainsNonFiniteValue, "color"});
    else
    {
      color = "q:";
      for (size_t i = 0u; i < presentation.color->size(); ++i)
      {
        if (i > 0u)
          color += ",";
        color += decimal(quantize((*presentation.color)[i], ColorQuantization));
      }
    }
  }

  if (!diagnostics.empty() || !u || !v)
    return {.diagnostics = std::move(diagnostics)};

  const auto material =
    QString::fromUtf8(
      presentation.material.data(), static_cast<qsizetype>(presentation.material.size()))
      .normalized(QString::NormalizationForm_C)
      .toUtf8();
  const auto preimage = std::string{"eqexport:presentation:v1\n"} + "material:"
                        + std::to_string(material.size()) + ":" + material.toStdString()
                        + "\ncontents:" + contents + "\nflags:" + flags
                        + "\nvalue:" + value + "\ncolor:" + color + "\n" + *u + *v;
  const auto fingerprint = sha256(preimage);
  return {
    .fingerprint = fingerprint,
    .id = "presentation:v1:" + fingerprint,
    .preimage = preimage,
    .diagnostics = std::move(diagnostics)};
}

} // namespace tb::ui
