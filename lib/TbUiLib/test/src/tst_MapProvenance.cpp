/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

#include "ui/CatchConfig.h"
#include "ui/MapProvenance.h"

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

QJsonObject fixture()
{
  const auto path = QTest::qFindTestData("fixture/test/map-provenance-v1.json");
  REQUIRE_FALSE(path.isEmpty());
  auto file = QFile{path};
  REQUIRE(file.open(QIODevice::ReadOnly));
  const auto document = QJsonDocument::fromJson(file.readAll());
  REQUIRE(document.isObject());
  return document.object();
}

vm::vec3d vector(const QJsonArray& value)
{
  REQUIRE(value.size() == 3);
  return {value[0].toDouble(), value[1].toDouble(), value[2].toDouble()};
}

MapProvenancePlane plane(const QJsonObject& value)
{
  return {vector(value.value("normal").toArray()), value.value("distance").toDouble()};
}

std::vector<MapProvenancePlane> planes(const QJsonArray& value)
{
  auto result = std::vector<MapProvenancePlane>{};
  result.reserve(static_cast<size_t>(value.size()));
  for (const auto& item : value)
    result.push_back(plane(item.toObject()));
  return result;
}

MapProvenanceFace face(const QJsonObject& value)
{
  auto result = MapProvenanceFace{plane(value.value("plane").toObject()), {}};
  for (const auto& point : value.value("polygon").toArray())
    result.polygon.push_back(vector(point.toArray()));
  return result;
}

MapProvenanceAxis axis(const QJsonObject& value)
{
  const auto rotation = value.value("rotation");
  return {
    vector(value.value("axis").toArray()),
    value.value("offset").toDouble(),
    value.value("scale").toDouble(),
    rotation.isUndefined() ? std::nullopt : std::optional{rotation.toDouble()}};
}

MapProvenancePresentation presentation(const QJsonObject& value)
{
  const auto optionalDouble = [&value](const char* key) -> std::optional<double> {
    const auto item = value.value(key);
    return item.isUndefined() ? std::nullopt : std::optional{item.toDouble()};
  };
  auto color = std::optional<std::array<double, 4>>{};
  if (const auto colorValue = value.value("color"); !colorValue.isUndefined())
  {
    const auto array = colorValue.toArray();
    REQUIRE(array.size() == 4);
    color = std::array<double, 4>{
      array[0].toDouble(), array[1].toDouble(), array[2].toDouble(), array[3].toDouble()};
  }
  return {
    value.value("material").toString().toUtf8().toStdString(),
    optionalDouble("contents"),
    optionalDouble("flags"),
    optionalDouble("value"),
    color,
    axis(value.value("u").toObject()),
    axis(value.value("v").toObject())};
}

bool hasDiagnostic(
  const MapProvenancePresentationResult& result,
  const MapProvenanceDiagnosticCode code,
  const std::string_view field)
{
  return std::ranges::any_of(result.diagnostics, [=](const auto& diagnostic) {
    return diagnostic.code == code && diagnostic.field == field;
  });
}

} // namespace

TEST_CASE("MapProvenance")
{
  const auto source = fixture();
  const auto geometry = source.value("geometry").toObject();
  const auto geometryPlanes = planes(geometry.value("planes").toArray());
  const auto geometryFace = face(geometry.value("face").toObject());
  const auto geometryExpected = geometry.value("expected").toObject();
  const auto presentationSource = source.value("presentation").toObject();
  const auto basePresentation = presentation(presentationSource.value("base").toObject());
  const auto presentationExpected = presentationSource.value("expected").toObject();

  SECTION("reproduces the normative geometry and presentation fixture byte for byte")
  {
    const auto brush = mapProvenanceBrushGeometryFingerprint(geometryPlanes);
    const auto actualFace = mapProvenanceFaceGeometryFingerprint(brush, geometryFace);
    const auto actualPresentation =
      mapProvenancePresentationFingerprint(basePresentation);

    CHECK(brush == geometryExpected.value("brushFingerprint").toString().toStdString());
    CHECK(
      mapProvenanceBrushId(geometryPlanes)
      == geometryExpected.value("brushId").toString().toStdString());
    CHECK(
      actualFace == geometryExpected.value("faceFingerprint").toString().toStdString());
    CHECK(
      mapProvenanceFaceId(brush, geometryFace)
      == geometryExpected.value("faceId").toString().toStdString());
    CHECK(
      canonicalMapProvenanceFacePolygon(geometryFace)
      == geometryExpected.value("canonicalFacePolygon").toString().toStdString());
    CHECK(
      actualPresentation.fingerprint
      == presentationExpected.value("fingerprint").toString().toStdString());
    CHECK(
      actualPresentation.id == presentationExpected.value("id").toString().toStdString());
    CHECK(
      actualPresentation.preimage
      == presentationExpected.value("preimage").toString().toStdString());
    CHECK(actualPresentation.diagnostics.empty());
  }

  SECTION("keeps plane order and duplicate geometry out of the identity")
  {
    auto reversed = geometryPlanes;
    std::ranges::reverse(reversed);
    CHECK(
      mapProvenanceBrushGeometryFingerprint(reversed)
      == mapProvenanceBrushGeometryFingerprint(geometryPlanes));
    CHECK(mapProvenanceBrushId(geometryPlanes) == mapProvenanceBrushId(geometryPlanes));
  }

  SECTION("makes cyclic face polygons equivalent but preserves outward orientation")
  {
    const auto brush = mapProvenanceBrushGeometryFingerprint(geometryPlanes);
    auto cyclic = geometryFace;
    std::rotate(cyclic.polygon.begin(), cyclic.polygon.begin() + 2, cyclic.polygon.end());
    auto reversedOutward = geometryFace;
    reversedOutward.plane.normal = {0.0, 0.0, -4.0};
    reversedOutward.plane.distance = -4.0;
    CHECK(
      mapProvenanceFaceGeometryFingerprint(brush, cyclic)
      == mapProvenanceFaceGeometryFingerprint(brush, geometryFace));
    CHECK(
      mapProvenanceFaceGeometryFingerprint(brush, reversedOutward)
      != mapProvenanceFaceGeometryFingerprint(brush, geometryFace));
  }

  SECTION("uses the effective affine UV map, not raw Valve representation")
  {
    auto changed = basePresentation;
    changed.material = "textures/Stone_Path_repainted";
    changed.flags = 9.0;
    const auto equivalent =
      presentation(presentationSource.value("equivalentEffectiveAffine").toObject());
    const auto original = mapProvenancePresentationFingerprint(basePresentation);
    CHECK(
      mapProvenancePresentationFingerprint(changed).fingerprint != original.fingerprint);
    CHECK(
      mapProvenancePresentationFingerprint(equivalent).fingerprint
      == original.fingerprint);
  }

  SECTION("normalizes material NFC but preserves case and absence")
  {
    auto decomposed = basePresentation;
    decomposed.material = "textures/cafe\xCC\x81/Stone_Path";
    decomposed.contents.reset();
    decomposed.flags.reset();
    decomposed.value.reset();
    decomposed.color.reset();
    auto composed = decomposed;
    composed.material = "textures/caf\xC3\xA9/Stone_Path";
    auto zero = composed;
    zero.contents = 0.0;
    zero.flags = 0.0;
    zero.value = 0.0;
    zero.color = std::array<double, 4>{0.0, 0.0, 0.0, 0.0};
    auto changedCase = composed;
    changedCase.material = "Textures/caf\xC3\xA9/Stone_Path";
    CHECK(
      mapProvenancePresentationFingerprint(decomposed).fingerprint
      == mapProvenancePresentationFingerprint(composed).fingerprint);
    const auto decomposedResult = mapProvenancePresentationFingerprint(decomposed);
    REQUIRE(decomposedResult.preimage.has_value());
    CHECK(
      decomposedResult.preimage->find("material:25:textures/caf\xC3\xA9/Stone_Path")
      != std::string::npos);
    CHECK(
      mapProvenancePresentationFingerprint(zero).fingerprint
      != mapProvenancePresentationFingerprint(composed).fingerprint);
    CHECK(
      mapProvenancePresentationFingerprint(changedCase).fingerprint
      != mapProvenancePresentationFingerprint(composed).fingerprint);
  }

  SECTION("uses TrenchBroom's inclusive binary32 safe-scale boundary")
  {
    for (const auto& boundary : source.value("scaleBoundaries").toArray())
    {
      auto candidate = basePresentation;
      candidate.u.scale = boundary.toObject().value("input").toDouble();
      const auto result = mapProvenancePresentationFingerprint(candidate);
      const auto accepted = boundary.toObject().value("accepted").toBool();
      CHECK((result.fingerprint.has_value() == accepted));
      if (!accepted)
        CHECK(hasDiagnostic(
          result,
          MapProvenanceDiagnosticCode::PresentationScaleUsesTrenchBroomSafeFallback,
          "u.scale"));
    }
  }

  SECTION("rejects malformed presentation metadata")
  {
    auto nonFinite = basePresentation;
    nonFinite.v.offset = std::numeric_limits<double>::quiet_NaN();
    CHECK(hasDiagnostic(
      mapProvenancePresentationFingerprint(nonFinite),
      MapProvenanceDiagnosticCode::PresentationContainsNonFiniteValue,
      "v"));

    auto nonIntegral = basePresentation;
    nonIntegral.flags = 0.5;
    CHECK(hasDiagnostic(
      mapProvenancePresentationFingerprint(nonIntegral),
      MapProvenanceDiagnosticCode::PresentationIntegerExpected,
      "flags"));
  }
}

} // namespace tb::ui
