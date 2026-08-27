/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QTemporaryDir>

#include "ui/AcceptanceImageComparison.h"
#include "ui/CatchConfig.h"

#include <limits>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

AcceptanceComparison comparison()
{
  auto comparison = AcceptanceComparison{};
  comparison.id = "comparison";
  comparison.metrics = {{
    "pixels",
    AcceptanceMetricType::Color,
    std::nullopt,
    {{"absoluteError", 0.0}, {"relativeError", 0.0}, {"maxChangedFraction", 0.2}},
  }};
  return comparison;
}

void writeDepth(const std::filesystem::path& path, const std::vector<float>& values)
{
  auto file = QFile{QString::fromStdString(path.string())};
  REQUIRE(file.open(QIODevice::WriteOnly));
  REQUIRE(file.write("Pf\n2 2\n-1.0\n") == 12);
  for (auto y = 1; y >= 0; --y)
    for (auto x = 0; x < 2; ++x)
    {
      const auto value = values[static_cast<size_t>(y * 2 + x)];
      REQUIRE(file.write(reinterpret_cast<const char*>(&value), sizeof(value)) == 4);
    }
}

} // namespace

TEST_CASE("AcceptanceImageComparison")
{
  auto directory = QTemporaryDir{};
  REQUIRE(directory.isValid());
  const auto referencePath =
    std::filesystem::path{directory.path().toStdString()} / "reference.png";
  const auto targetPath =
    std::filesystem::path{directory.path().toStdString()} / "target.png";
  auto reference = QImage{4, 4, QImage::Format_RGBA8888};
  reference.fill(Qt::black);
  auto target = reference;
  target.setPixelColor(2, 1, Qt::white);
  REQUIRE(reference.save(QString::fromStdString(referencePath.string())));
  REQUIRE(target.save(QString::fromStdString(targetPath.string())));

  SECTION("measures changed fraction and deterministic changed-pixel bounds")
  {
    const auto result = compareAcceptanceImages(referencePath, targetPath, comparison());
    REQUIRE(result.is_success());
    REQUIRE(result.value().metrics.size() == 1u);
    const auto& metric = result.value().metrics.front();
    CHECK(metric.passed);
    CHECK(metric.diagnostic.comparedPixels == 16u);
    CHECK(metric.diagnostic.changedPixels == 1u);
    CHECK(metric.diagnostic.changedFraction == 1.0 / 16.0);
    REQUIRE(metric.diagnostic.changedBounds);
    CHECK(metric.diagnostic.changedBounds->min == vm::vec3d{2.0, 1.0, 0.0});
    CHECK(metric.diagnostic.changedBounds->max == vm::vec3d{3.0, 2.0, 0.0});
    CHECK(
      acceptanceImageComparisonReportToJson(result.value())
        .value("metrics")
        .toArray()[0]
        .toObject()
        .value("diagnostic")
        .toObject()
        .value("changedPixels")
      == 1);
  }

  SECTION("uses an optional normalized ROI and configurable tolerances")
  {
    auto outside = comparison();
    outside.metrics.front().maskId = "outside";
    outside.masks = {{"outside", 0.0, 0.0, 0.5, 0.5}};
    const auto ignored = compareAcceptanceImages(referencePath, targetPath, outside);
    REQUIRE(ignored.is_success());
    CHECK(ignored.value().metrics.front().diagnostic.comparedPixels == 4u);
    CHECK(ignored.value().metrics.front().diagnostic.changedPixels == 0u);

    auto tolerant = comparison();
    tolerant.metrics.front().configuration.insert("relativeError", 1.0);
    const auto accepted = compareAcceptanceImages(referencePath, targetPath, tolerant);
    REQUIRE(accepted.is_success());
    CHECK(accepted.value().metrics.front().passed);
    CHECK(accepted.value().metrics.front().diagnostic.changedPixels == 0u);
  }

  SECTION("uses EV6 linear depth truthfully for exact depth and silhouette metrics")
  {
    const auto depthReference =
      std::filesystem::path{directory.path().toStdString()} / "reference.pfm";
    const auto depthTarget =
      std::filesystem::path{directory.path().toStdString()} / "target.pfm";
    writeDepth(depthReference, {1.0f, 2.0f, 3.0f, 4.0f});
    writeDepth(depthTarget, {1.0f, 2.0f, 3.0f, 4.0f});
    auto depth = comparison();
    depth.metrics.front().type = AcceptanceMetricType::Depth;
    depth.metrics.front().configuration = {
      {"absoluteError", 0.0}, {"relativeError", 0.0}, {"maxChangedFraction", 0.0}};
    const auto referenceBuffers = AcceptanceImageBuffers{referencePath, depthReference};
    const auto targetBuffers = AcceptanceImageBuffers{targetPath, depthTarget};
    const auto identical =
      compareAcceptanceImages(referenceBuffers, targetBuffers, depth);
    REQUIRE(identical.is_success());
    CHECK(identical.value().metrics.front().passed);
    CHECK(identical.value().metrics.front().diagnostic.changedPixels == 0u);

    writeDepth(depthTarget, {1.0f, 2.0f, 3.0f, 4.25f});
    depth.metrics.front().configuration = {
      {"absoluteError", 0.5}, {"relativeError", 0.0}, {"maxChangedFraction", 0.0}};
    const auto tolerant = compareAcceptanceImages(referenceBuffers, targetBuffers, depth);
    REQUIRE(tolerant.is_success());
    CHECK(tolerant.value().metrics.front().passed);
    CHECK(tolerant.value().metrics.front().diagnostic.changedPixels == 0u);

    depth.masks = {{"topLeft", 0.0, 0.0, 0.5, 0.5}};
    depth.metrics.front().maskId = "topLeft";
    depth.metrics.front().configuration.insert("absoluteError", 0.0);
    const auto masked = compareAcceptanceImages(referenceBuffers, targetBuffers, depth);
    REQUIRE(masked.is_success());
    CHECK(masked.value().metrics.front().diagnostic.changedPixels == 0u);

    writeDepth(depthTarget, {1.0f, std::numeric_limits<float>::infinity(), 3.0f, 4.0f});
    auto silhouette = comparison();
    silhouette.metrics.front().type = AcceptanceMetricType::Silhouette;
    const auto coverage =
      compareAcceptanceImages(referenceBuffers, targetBuffers, silhouette);
    REQUIRE(coverage.is_success());
    CHECK(coverage.value().metrics.front().diagnostic.changedPixels == 1u);
    CHECK_FALSE(coverage.value().metrics.front().passed);

    const auto missing = compareAcceptanceImages(
      AcceptanceImageBuffers{referencePath, std::nullopt}, targetBuffers, silhouette);
    REQUIRE(missing.is_error());
    CHECK(
      std::get<AcceptanceImageComparisonError>(missing.error()).code
      == AcceptanceImageComparisonErrorCode::MissingAuxiliaryBuffer);
  }

  SECTION("combines color and depth metrics in authored order")
  {
    const auto depthReference =
      std::filesystem::path{directory.path().toStdString()} / "mixed-reference.pfm";
    const auto depthTarget =
      std::filesystem::path{directory.path().toStdString()} / "mixed-target.pfm";
    writeDepth(depthReference, {1.0f, 2.0f, 3.0f, 4.0f});
    writeDepth(depthTarget, {1.0f, 2.0f, 3.0f, 10.0f});
    auto mixed = comparison();
    mixed.metrics = {
      {"depth",
       AcceptanceMetricType::Depth,
       std::nullopt,
       {{"absoluteError", 6.0}, {"relativeError", 0.0}, {"maxChangedFraction", 0.0}}},
      {"color",
       AcceptanceMetricType::Color,
       std::nullopt,
       {{"absoluteError", 0.0}, {"relativeError", 0.0}, {"maxChangedFraction", 0.2}}},
    };
    const auto referenceBuffers = AcceptanceImageBuffers{referencePath, depthReference};
    const auto targetBuffers = AcceptanceImageBuffers{targetPath, depthTarget};
    const auto accepted = compareAcceptanceImages(referenceBuffers, targetBuffers, mixed);
    REQUIRE(accepted.is_success());
    REQUIRE(accepted.value().metrics.size() == 2u);
    CHECK(accepted.value().metrics[0].metricId == "depth");
    CHECK(accepted.value().metrics[0].passed);
    CHECK(accepted.value().metrics[1].metricId == "color");
    CHECK(accepted.value().metrics[1].passed);
    CHECK(
      acceptanceImageComparisonReportToJson(accepted.value()).value("passed") == true);

    mixed.metrics[0].configuration.insert("absoluteError", 5.0);
    const auto rejected = compareAcceptanceImages(referenceBuffers, targetBuffers, mixed);
    REQUIRE(rejected.is_success());
    CHECK_FALSE(rejected.value().metrics[0].passed);
    CHECK(
      acceptanceImageComparisonReportToJson(rejected.value()).value("passed") == false);
  }

  SECTION("rejects metrics whose auxiliary capture buffer is not implemented")
  {
    auto unsupported = comparison();
    unsupported.metrics.front().type = AcceptanceMetricType::ObjectId;
    const auto result = compareAcceptanceImages(
      AcceptanceImageBuffers{referencePath, std::nullopt},
      AcceptanceImageBuffers{targetPath, std::nullopt},
      unsupported);
    REQUIRE(result.is_error());
    CHECK(
      std::get<AcceptanceImageComparisonError>(result.error()).code
      == AcceptanceImageComparisonErrorCode::MissingAuxiliaryBuffer);
  }
}

} // namespace tb::ui
