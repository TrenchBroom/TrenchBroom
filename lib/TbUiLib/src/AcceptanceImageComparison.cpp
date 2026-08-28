/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceImageComparison.h"

#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QtEndian>

#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>
#include <ostream>
#include <ranges>
#include <string_view>
#include <utility>
#include <variant>

namespace tb::ui
{
namespace
{

AcceptanceImageComparisonError error(
  const AcceptanceImageComparisonErrorCode code, std::string message)
{
  return {code, std::move(message)};
}

struct DifferenceSettings
{
  double absoluteError = 0.0;
  double relativeError = 0.0;
  double maxChangedFraction = 0.0;
};

Result<DifferenceSettings, AcceptanceImageComparisonError> settingsFor(
  const AcceptanceMetric& metric)
{
  auto settings = DifferenceSettings{};
  for (const auto& [key, destination] : {
         std::pair{"absoluteError", &settings.absoluteError},
         std::pair{"relativeError", &settings.relativeError},
         std::pair{"maxChangedFraction", &settings.maxChangedFraction},
       })
  {
    if (!metric.configuration.contains(key))
      continue;
    const auto value = metric.configuration.value(key);
    const auto unitBounded = key != std::string_view{"absoluteError"}
                             || metric.type == AcceptanceMetricType::Color;
    if (
      !value.isDouble() || !std::isfinite(value.toDouble()) || value.toDouble() < 0.0
      || (unitBounded && value.toDouble() > 1.0))
    {
      return error(
        AcceptanceImageComparisonErrorCode::InvalidConfiguration,
        "Image metric '" + metric.id + "' has an invalid " + key);
    }
    *destination = value.toDouble();
  }
  return settings;
}

std::optional<AcceptanceMask> maskFor(
  const AcceptanceComparison& comparison, const AcceptanceMetric& metric)
{
  if (!metric.maskId)
    return std::nullopt;
  const auto found =
    std::ranges::find(comparison.masks, *metric.maskId, &AcceptanceMask::id);
  return found == comparison.masks.end() ? std::nullopt
                                         : std::optional<AcceptanceMask>{*found};
}

QString metricTypeName(const AcceptanceMetricType type)
{
  switch (type)
  {
  case AcceptanceMetricType::Color:
    return "color";
  case AcceptanceMetricType::Depth:
    return "depth";
  case AcceptanceMetricType::Silhouette:
    return "silhouette";
  case AcceptanceMetricType::EdgeMap:
    return "edgeMap";
  case AcceptanceMetricType::MaterialId:
    return "materialId";
  case AcceptanceMetricType::ObjectId:
    return "objectId";
  }
  return {};
}

struct DepthImage
{
  int width;
  int height;
  std::vector<float> values;
};

Result<DepthImage, AcceptanceImageComparisonError> loadDepth(
  const std::optional<std::filesystem::path>& path)
{
  if (!path)
    return error(
      AcceptanceImageComparisonErrorCode::MissingAuxiliaryBuffer, "Depth buffer missing");
  auto file = QFile{QString::fromStdString(path->string())};
  if (!file.open(QIODevice::ReadOnly))
    return error(
      AcceptanceImageComparisonErrorCode::ImageLoadFailed, "Could not read depth buffer");
  const auto data = file.readAll();
  const auto first = data.indexOf('\n');
  const auto second = first < 0 ? -1 : data.indexOf('\n', first + 1);
  const auto third = second < 0 ? -1 : data.indexOf('\n', second + 1);
  if (
    first < 0 || second < 0 || third < 0 || data.left(first) != "Pf"
    || data.mid(second + 1, third - second - 1).trimmed() != "-1.0")
    return error(
      AcceptanceImageComparisonErrorCode::ImageLoadFailed, "Invalid EV6 depth PFM");
  const auto dimensions = data.mid(first + 1, second - first - 1).split(' ');
  if (dimensions.size() != 2)
    return error(
      AcceptanceImageComparisonErrorCode::ImageLoadFailed,
      "Invalid EV6 depth dimensions");
  const auto width = dimensions[0].toInt();
  const auto height = dimensions[1].toInt();
  const auto bytes =
    static_cast<qint64>(width) * height * static_cast<qint64>(sizeof(float));
  if (width <= 0 || height <= 0 || data.size() - third - 1 != bytes)
    return error(
      AcceptanceImageComparisonErrorCode::ImageLoadFailed, "Invalid EV6 depth data size");
  auto result =
    DepthImage{width, height, std::vector<float>(static_cast<size_t>(width * height))};
  const auto* samples = data.constData() + third + 1;
  for (auto y = 0; y < height; ++y)
    for (auto x = 0; x < width; ++x)
    {
      const auto bits = qFromLittleEndian<quint32>(
        samples + (y * width + x) * static_cast<int>(sizeof(float)));
      result.values[static_cast<size_t>((height - 1 - y) * width + x)] =
        std::bit_cast<float>(bits);
    }
  return result;
}

} // namespace

std::ostream& operator<<(std::ostream& lhs, const AcceptanceImageComparisonError& rhs)
{
  return lhs << rhs.message;
}

AcceptanceImageComparisonResult compareAcceptanceImages(
  const std::filesystem::path& referencePath,
  const std::filesystem::path& targetPath,
  const AcceptanceComparison& comparison)
{
  if (comparison.metrics.empty())
    return AcceptanceImageComparisonReport{referencePath, targetPath, {}};

  const auto reference =
    QImage{QString::fromStdString(referencePath.string())}.convertToFormat(
      QImage::Format_RGBA8888);
  const auto target = QImage{QString::fromStdString(targetPath.string())}.convertToFormat(
    QImage::Format_RGBA8888);
  if (reference.isNull() || target.isNull())
  {
    return error(
      AcceptanceImageComparisonErrorCode::ImageLoadFailed,
      "Could not load captured image for comparison");
  }
  if (reference.size() != target.size())
  {
    return error(
      AcceptanceImageComparisonErrorCode::ImageSizeMismatch,
      "Captured images must have the same dimensions");
  }

  auto result = AcceptanceImageComparisonReport{referencePath, targetPath, {}};
  for (const auto& metric : comparison.metrics)
  {
    if (metric.type != AcceptanceMetricType::Color)
    {
      return error(
        AcceptanceImageComparisonErrorCode::MissingAuxiliaryBuffer,
        "Semantic metric requires its corresponding auxiliary capture buffer");
    }
    const auto settings = settingsFor(metric);
    if (settings.is_error())
      return std::get<AcceptanceImageComparisonError>(settings.error());
    const auto mask = maskFor(comparison, metric);
    if (metric.maskId && !mask)
    {
      return error(
        AcceptanceImageComparisonErrorCode::InvalidConfiguration,
        "Image metric references a missing mask");
    }
    const auto minX =
      mask ? static_cast<int>(std::floor(mask->x * reference.width())) : 0;
    const auto minY =
      mask ? static_cast<int>(std::floor(mask->y * reference.height())) : 0;
    const auto maxX =
      mask ? static_cast<int>(std::ceil((mask->x + mask->width) * reference.width()))
           : reference.width();
    const auto maxY =
      mask ? static_cast<int>(std::ceil((mask->y + mask->height) * reference.height()))
           : reference.height();
    if (minX >= maxX || minY >= maxY)
    {
      return error(
        AcceptanceImageComparisonErrorCode::EmptyRegion,
        "Image metric mask does not cover a pixel");
    }

    auto diagnostic = AcceptanceImageDifferenceDiagnostic{};
    auto changedMinX = std::numeric_limits<int>::max();
    auto changedMinY = std::numeric_limits<int>::max();
    auto changedMaxX = std::numeric_limits<int>::min();
    auto changedMaxY = std::numeric_limits<int>::min();
    for (auto y = minY; y < maxY; ++y)
    {
      const auto* referenceLine = reference.constScanLine(y);
      const auto* targetLine = target.constScanLine(y);
      for (auto x = minX; x < maxX; ++x)
      {
        const auto* referencePixel = referenceLine + x * 4;
        const auto* targetPixel = targetLine + x * 4;
        auto absoluteError = 0.0;
        auto relativeError = 0.0;
        for (auto channel = 0; channel < 4; ++channel)
        {
          const auto referenceValue =
            static_cast<double>(referencePixel[channel]) / 255.0;
          const auto targetValue = static_cast<double>(targetPixel[channel]) / 255.0;
          const auto difference = std::abs(referenceValue - targetValue);
          absoluteError = std::max(absoluteError, difference);
          relativeError = std::max(
            relativeError,
            difference / std::max({referenceValue, targetValue, 1.0 / 255.0}));
        }
        ++diagnostic.comparedPixels;
        diagnostic.maximumAbsoluteError =
          std::max(diagnostic.maximumAbsoluteError, absoluteError);
        diagnostic.maximumRelativeError =
          std::max(diagnostic.maximumRelativeError, relativeError);
        if (
          absoluteError > settings.value().absoluteError
          && relativeError > settings.value().relativeError)
        {
          ++diagnostic.changedPixels;
          changedMinX = std::min(changedMinX, x);
          changedMinY = std::min(changedMinY, y);
          changedMaxX = std::max(changedMaxX, x + 1);
          changedMaxY = std::max(changedMaxY, y + 1);
        }
      }
    }
    diagnostic.changedFraction = static_cast<double>(diagnostic.changedPixels)
                                 / static_cast<double>(diagnostic.comparedPixels);
    if (diagnostic.changedPixels > 0u)
    {
      diagnostic.changedBounds = AcceptanceBounds{
        {static_cast<double>(changedMinX), static_cast<double>(changedMinY), 0.0},
        {static_cast<double>(changedMaxX), static_cast<double>(changedMaxY), 0.0}};
    }
    result.metrics.push_back(
      {metric.id,
       metric.type,
       diagnostic.changedFraction <= settings.value().maxChangedFraction,
       diagnostic});
  }
  return result;
}

AcceptanceImageComparisonResult compareAcceptanceImages(
  const AcceptanceImageBuffers& referenceBuffers,
  const AcceptanceImageBuffers& targetBuffers,
  const AcceptanceComparison& comparison)
{
  if (std::ranges::any_of(comparison.metrics, [](const auto& metric) {
        return metric.type == AcceptanceMetricType::EdgeMap
               || metric.type == AcceptanceMetricType::MaterialId
               || metric.type == AcceptanceMetricType::ObjectId;
      }))
    return error(
      AcceptanceImageComparisonErrorCode::MissingAuxiliaryBuffer,
      "The requested structural metric has no capture buffer");

  auto report = AcceptanceImageComparisonReport{
    referenceBuffers.colorPath, targetBuffers.colorPath, {}};
  auto colorReports = std::vector<AcceptanceImageMetricReport>{};
  auto colorComparison = comparison;
  std::erase_if(colorComparison.metrics, [](const auto& metric) {
    return metric.type != AcceptanceMetricType::Color;
  });
  if (!colorComparison.metrics.empty())
  {
    const auto color = compareAcceptanceImages(
      referenceBuffers.colorPath, targetBuffers.colorPath, colorComparison);
    if (color.is_error())
      return std::get<AcceptanceImageComparisonError>(color.error());
    colorReports = color.value().metrics;
  }

  auto semanticReports = std::vector<AcceptanceImageMetricReport>{};
  auto semanticComparison = comparison;
  std::erase_if(semanticComparison.metrics, [](const auto& metric) {
    return metric.type != AcceptanceMetricType::Depth
           && metric.type != AcceptanceMetricType::Silhouette;
  });
  if (!semanticComparison.metrics.empty())
  {
    const auto reference = loadDepth(referenceBuffers.depthPath);
    const auto target = loadDepth(targetBuffers.depthPath);
    if (reference.is_error())
      return std::get<AcceptanceImageComparisonError>(reference.error());
    if (target.is_error())
      return std::get<AcceptanceImageComparisonError>(target.error());
    if (
      reference.value().width != target.value().width
      || reference.value().height != target.value().height)
      return error(
        AcceptanceImageComparisonErrorCode::ImageSizeMismatch,
        "Depth buffers must have the same dimensions");
    for (const auto& metric : semanticComparison.metrics)
    {
      const auto settings = settingsFor(metric);
      if (settings.is_error())
        return std::get<AcceptanceImageComparisonError>(settings.error());
      const auto mask = maskFor(semanticComparison, metric);
      if (metric.maskId && !mask)
        return error(
          AcceptanceImageComparisonErrorCode::InvalidConfiguration,
          "Image metric references a missing mask");
      const auto minX =
        mask ? static_cast<int>(std::floor(mask->x * reference.value().width)) : 0;
      const auto minY =
        mask ? static_cast<int>(std::floor(mask->y * reference.value().height)) : 0;
      const auto maxX =
        mask
          ? static_cast<int>(std::ceil((mask->x + mask->width) * reference.value().width))
          : reference.value().width;
      const auto maxY = mask ? static_cast<int>(std::ceil(
                                 (mask->y + mask->height) * reference.value().height))
                             : reference.value().height;
      if (minX >= maxX || minY >= maxY)
        return error(
          AcceptanceImageComparisonErrorCode::EmptyRegion,
          "Image metric mask does not cover a pixel");
      auto diagnostic = AcceptanceImageDifferenceDiagnostic{};
      auto changedMinX = std::numeric_limits<int>::max();
      auto changedMinY = std::numeric_limits<int>::max();
      auto changedMaxX = std::numeric_limits<int>::min();
      auto changedMaxY = std::numeric_limits<int>::min();
      for (auto y = minY; y < maxY; ++y)
        for (auto x = minX; x < maxX; ++x)
        {
          const auto a = reference.value()
                           .values[static_cast<size_t>(y * reference.value().width + x)];
          const auto b =
            target.value().values[static_cast<size_t>(y * target.value().width + x)];
          const auto ah = std::isfinite(a), bh = std::isfinite(b);
          const auto absolute =
            ah && bh ? std::abs(static_cast<double>(a) - b)
                     : (ah == bh ? 0.0 : std::numeric_limits<double>::infinity());
          const auto relative = ah && bh ? absolute
                                             / std::max(
                                               {std::abs(static_cast<double>(a)),
                                                std::abs(static_cast<double>(b)),
                                                1.0})
                                         : absolute;
          ++diagnostic.comparedPixels;
          // Coverage changes are reported through changedPixels. Keeping these finite
          // makes the JSON diagnostic deterministic for depth's +infinity no-hit.
          if (std::isfinite(absolute))
          {
            diagnostic.maximumAbsoluteError =
              std::max(diagnostic.maximumAbsoluteError, absolute);
            diagnostic.maximumRelativeError =
              std::max(diagnostic.maximumRelativeError, relative);
          }
          const auto changed = metric.type == AcceptanceMetricType::Silhouette
                                 ? ah != bh
                                 : (absolute > settings.value().absoluteError
                                    && relative > settings.value().relativeError);
          if (changed)
          {
            ++diagnostic.changedPixels;
            changedMinX = std::min(changedMinX, x);
            changedMinY = std::min(changedMinY, y);
            changedMaxX = std::max(changedMaxX, x + 1);
            changedMaxY = std::max(changedMaxY, y + 1);
          }
        }
      diagnostic.changedFraction =
        static_cast<double>(diagnostic.changedPixels) / diagnostic.comparedPixels;
      if (diagnostic.changedPixels > 0u)
      {
        diagnostic.changedBounds = AcceptanceBounds{
          {static_cast<double>(changedMinX), static_cast<double>(changedMinY), 0.0},
          {static_cast<double>(changedMaxX), static_cast<double>(changedMaxY), 0.0}};
      }
      semanticReports.push_back(
        {metric.id,
         metric.type,
         diagnostic.changedFraction <= settings.value().maxChangedFraction,
         diagnostic});
    }
  }

  auto colorIndex = size_t{0u};
  auto semanticIndex = size_t{0u};
  report.metrics.reserve(comparison.metrics.size());
  for (const auto& metric : comparison.metrics)
  {
    if (metric.type == AcceptanceMetricType::Color)
      report.metrics.push_back(colorReports.at(colorIndex++));
    else
      report.metrics.push_back(semanticReports.at(semanticIndex++));
  }
  return report;
}

QJsonObject acceptanceImageComparisonReportToJson(
  const AcceptanceImageComparisonReport& report)
{
  auto metrics = QJsonArray{};
  auto passed = true;
  for (const auto& metric : report.metrics)
  {
    passed = passed && metric.passed;
    auto diagnostic = QJsonObject{
      {"comparedPixels", static_cast<qint64>(metric.diagnostic.comparedPixels)},
      {"changedPixels", static_cast<qint64>(metric.diagnostic.changedPixels)},
      {"changedFraction", metric.diagnostic.changedFraction},
      {"maximumAbsoluteError", metric.diagnostic.maximumAbsoluteError},
      {"maximumRelativeError", metric.diagnostic.maximumRelativeError}};
    if (metric.diagnostic.changedBounds)
    {
      diagnostic.insert(
        "changedBounds",
        QJsonObject{
          {"min",
           QJsonArray{
             metric.diagnostic.changedBounds->min.x(),
             metric.diagnostic.changedBounds->min.y(),
             metric.diagnostic.changedBounds->min.z()}},
          {"max",
           QJsonArray{
             metric.diagnostic.changedBounds->max.x(),
             metric.diagnostic.changedBounds->max.y(),
             metric.diagnostic.changedBounds->max.z()}}});
    }
    metrics.push_back(
      QJsonObject{
        {"id", QString::fromStdString(metric.metricId)},
        {"type", metricTypeName(metric.metricType)},
        {"passed", metric.passed},
        {"diagnostic", diagnostic}});
  }
  return {
    {"referencePath", QString::fromStdString(report.referencePath.generic_string())},
    {"targetPath", QString::fromStdString(report.targetPath.generic_string())},
    {"passed", passed},
    {"metrics", metrics}};
}

} // namespace tb::ui
