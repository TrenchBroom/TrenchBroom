/* Copyright (C) 2026 */
#include "ui/AcceptanceAssertions.h"

#include <QJsonArray>

#include <algorithm>
#include <cmath>
#include <ostream>
#include <ranges>
#include <variant>

namespace tb::ui
{
namespace
{
AcceptanceAssertionError error(
  const AcceptanceAssertionErrorCode code, std::string message)
{
  return {code, std::move(message)};
}

template <typename Value>
AcceptanceAssertionError queryError(const Result<Value, AcceptanceGeometryError>& result)
{
  return {
    AcceptanceAssertionErrorCode::GeometryQueryFailed,
    std::get<AcceptanceGeometryError>(result.error()).message};
}

Result<vm::vec3d, AcceptanceAssertionError> vec(const QJsonValue& value, const char* name)
{
  if (!value.isArray() || value.toArray().size() != 3)
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      std::string{name} + " must be a three-vector");
  const auto a = value.toArray();
  if (!a[0].isDouble() || !a[1].isDouble() || !a[2].isDouble())
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      std::string{name} + " must contain numbers");
  const auto result = vm::vec3d{a[0].toDouble(), a[1].toDouble(), a[2].toDouble()};
  if (
    !std::isfinite(result.x()) || !std::isfinite(result.y())
    || !std::isfinite(result.z()))
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      std::string{name} + " must be finite");
  return result;
}

Result<double, AcceptanceAssertionError> number(
  const QJsonObject& object,
  const char* name,
  const std::optional<double> fallback = std::nullopt)
{
  if (!object.contains(name))
  {
    if (fallback)
      return *fallback;
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      std::string{name} + " is required");
  }
  const auto value = object.value(name);
  if (!value.isDouble() || !std::isfinite(value.toDouble()))
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      std::string{name} + " must be finite");
  return value.toDouble();
}

Result<int, AcceptanceAssertionError> grid(
  const QJsonObject& object, const char* name, const int fallback)
{
  const auto value = number(object, name, fallback);
  if (value.is_error())
    return std::get<AcceptanceAssertionError>(value.error());
  const auto result = value.value();
  if (result != std::floor(result) || result < 1.0 || result > 65.0)
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      std::string{name} + " must be an integer in [1,65]");
  return static_cast<int>(result);
}

std::optional<AcceptanceAssertionType> assertionType(const QString& value)
{
  if (value == "boundsVisible")
    return AcceptanceAssertionType::BoundsVisible;
  if (value == "boundsNotVisible")
    return AcceptanceAssertionType::BoundsNotVisible;
  if (value == "clearSightline")
    return AcceptanceAssertionType::ClearSightline;
  if (value == "openingClearance")
    return AcceptanceAssertionType::OpeningClearance;
  if (value == "playerClearance")
    return AcceptanceAssertionType::PlayerClearance;
  if (value == "materialCoverage")
    return AcceptanceAssertionType::MaterialCoverage;
  if (value == "depthRange")
    return AcceptanceAssertionType::DepthRange;
  return std::nullopt;
}

Result<AcceptanceBounds, AcceptanceAssertionError> bounds(const QJsonValue& value)
{
  if (!value.isObject())
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration, "bounds must be an object");
  const auto object = value.toObject();
  const auto minimum = vec(object.value("min"), "bounds.min");
  const auto maximum = vec(object.value("max"), "bounds.max");
  if (minimum.is_error())
    return std::get<AcceptanceAssertionError>(minimum.error());
  if (maximum.is_error())
    return std::get<AcceptanceAssertionError>(maximum.error());
  if (
    minimum.value().x() > maximum.value().x() || minimum.value().y() > maximum.value().y()
    || minimum.value().z() > maximum.value().z())
  {
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      "bounds min must not exceed bounds max");
  }
  return AcceptanceBounds{minimum.value(), maximum.value()};
}

Result<AcceptanceAlignment, AcceptanceAssertionError> alignment(const QJsonValue& value)
{
  if (!value.isObject() || !value.toObject().value("type").isString())
  {
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      "context.alignment.type must be a string");
  }
  const auto object = value.toObject();
  const auto type = object.value("type").toString();
  auto result = AcceptanceAlignment{};
  if (type == "identity")
    result.type = AcceptanceAlignmentType::Identity;
  else if (type == "independent")
    result.type = AcceptanceAlignmentType::Independent;
  else if (type == "matrix")
  {
    result.type = AcceptanceAlignmentType::Matrix;
    const auto values = object.value("matrix");
    if (!values.isArray() || values.toArray().size() != 16)
    {
      return error(
        AcceptanceAssertionErrorCode::InvalidConfiguration,
        "Matrix alignment requires 16 values");
    }
    for (size_t index = 0u; index < result.matrix.size(); ++index)
    {
      const auto number = values.toArray().at(static_cast<qsizetype>(index));
      if (!number.isDouble() || !std::isfinite(number.toDouble()))
      {
        return error(
          AcceptanceAssertionErrorCode::InvalidConfiguration,
          "Matrix alignment values must be finite");
      }
      result.matrix[index] = number.toDouble();
    }
  }
  else if (type == "landmarks")
  {
    result.type = AcceptanceAlignmentType::Landmarks;
    const auto values = object.value("landmarks");
    if (!values.isArray() || values.toArray().size() < 3)
    {
      return error(
        AcceptanceAssertionErrorCode::InvalidConfiguration,
        "Landmark alignment requires at least three landmarks");
    }
    for (const auto& landmark : values.toArray())
    {
      if (!landmark.isObject())
      {
        return error(
          AcceptanceAssertionErrorCode::InvalidConfiguration,
          "Each landmark must be an object");
      }
      const auto landmarkObject = landmark.toObject();
      const auto reference = vec(landmarkObject.value("reference"), "landmark.reference");
      const auto target = vec(landmarkObject.value("target"), "landmark.target");
      if (reference.is_error())
        return std::get<AcceptanceAssertionError>(reference.error());
      if (target.is_error())
        return std::get<AcceptanceAssertionError>(target.error());
      result.landmarks.push_back({reference.value(), target.value()});
    }
  }
  else
  {
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration, "Unknown alignment type");
  }
  return result;
}

vm::vec3d point(const AcceptanceAlignment& alignment, const vm::vec3d& value)
{
  const auto& m = alignment.matrix;
  return {
    m[0] * value.x() + m[1] * value.y() + m[2] * value.z() + m[3],
    m[4] * value.x() + m[5] * value.y() + m[6] * value.z() + m[7],
    m[8] * value.x() + m[9] * value.y() + m[10] * value.z() + m[11]};
}

vm::vec3d vector(const AcceptanceAlignment& alignment, const vm::vec3d& value)
{
  const auto& m = alignment.matrix;
  return {
    m[0] * value.x() + m[1] * value.y() + m[2] * value.z(),
    m[4] * value.x() + m[5] * value.y() + m[6] * value.z(),
    m[8] * value.x() + m[9] * value.y() + m[10] * value.z()};
}

Result<vm::vec3d, AcceptanceAssertionError> transform(
  const AcceptanceAssertionContext& context, const vm::vec3d& value, const bool isPoint)
{
  if (
    context.geometrySpace == AcceptanceAssertionSpace::Reference
    || context.alignment.type == AcceptanceAlignmentType::Identity)
    return value;
  if (context.alignment.type != AcceptanceAlignmentType::Matrix)
    return error(
      AcceptanceAssertionErrorCode::UnsupportedTransformation,
      "Target-space assertions require identity or matrix alignment");
  const auto result =
    isPoint ? point(context.alignment, value) : vector(context.alignment, value);
  if (vm::is_zero(result, 0.000001) && !isPoint)
    return error(
      AcceptanceAssertionErrorCode::UnsupportedTransformation,
      "Alignment collapses assertion direction");
  return result;
}

AcceptanceAssertionResult castClear(
  const AcceptanceGeometryQuery& geometry,
  const AcceptanceStructuralRay& ray,
  const double tolerance)
{
  const auto hits = geometry.cast(ray);
  if (hits.is_error())
    return queryError(hits);
  auto ordered = hits.value();
  std::ranges::sort(ordered, {}, &AcceptanceGeometryHit::distance);
  if (!ordered.empty() && ordered.front().distance <= tolerance)
    return error(
      AcceptanceAssertionErrorCode::StartInsideSolid, "Ray starts inside solid geometry");
  const auto clear =
    ordered.empty() || ordered.front().distance >= ray.maxDistance - tolerance;
  return AcceptanceAssertionReport{
    clear, 1u, clear ? 1u : 0u, clear ? 1.0 : 0.0, 0.0, 0.0, clear ? "clear" : "blocked"};
}

AcceptanceAssertionResult sightline(
  const AcceptanceGeometryQuery& geometry,
  const AcceptanceAssertion& assertion,
  const AcceptanceAssertionContext& context)
{
  const auto& config = assertion.configuration;
  const auto originValue = vec(config.value("origin"), "origin");
  const auto targetValue = vec(config.value("target"), "target");
  const auto width = number(config, "corridorWidth", 0.0);
  const auto samples = grid(config, "grid", 1);
  const auto required = number(config, "minimumClearFraction", 1.0);
  if (originValue.is_error())
    return std::get<AcceptanceAssertionError>(originValue.error());
  if (targetValue.is_error())
    return std::get<AcceptanceAssertionError>(targetValue.error());
  if (width.is_error())
    return std::get<AcceptanceAssertionError>(width.error());
  if (samples.is_error())
    return std::get<AcceptanceAssertionError>(samples.error());
  if (
    required.is_error() || width.value() < 0.0 || required.value() < 0.0
    || required.value() > 1.0)
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      "Invalid corridor width or clear fraction");
  const auto origin = transform(context, originValue.value(), true);
  const auto target = transform(context, targetValue.value(), true);
  if (origin.is_error())
    return std::get<AcceptanceAssertionError>(origin.error());
  if (target.is_error())
    return std::get<AcceptanceAssertionError>(target.error());
  const auto delta = target.value() - origin.value();
  if (vm::is_zero(delta, context.tolerance))
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      "Sightline endpoints must differ");
  const auto direction = vm::normalize(delta);
  auto up =
    std::abs(direction.z()) < 0.9 ? vm::vec3d{0.0, 0.0, 1.0} : vm::vec3d{0.0, 1.0, 0.0};
  const auto right = vm::normalize(vm::cross(direction, up));
  up = vm::normalize(vm::cross(right, direction));
  auto clear = size_t{0u};
  const auto total = static_cast<size_t>(samples.value() * samples.value());
  for (int y = 0; y < samples.value(); ++y)
    for (int x = 0; x < samples.value(); ++x)
    {
      const auto scaleX =
        samples.value() == 1
          ? 0.0
          : (static_cast<double>(x) / static_cast<double>(samples.value() - 1) - 0.5)
              * width.value();
      const auto scaleY =
        samples.value() == 1
          ? 0.0
          : (static_cast<double>(y) / static_cast<double>(samples.value() - 1) - 0.5)
              * width.value();
      const auto start = origin.value() + right * scaleX + up * scaleY;
      const auto end = target.value() + right * scaleX + up * scaleY;
      const auto rayDirection = end - start;
      const auto distance = vm::length(rayDirection);
      const auto result = castClear(
        geometry, {start, vm::normalize(rayDirection), distance}, context.tolerance);
      if (result.is_error())
        return std::get<AcceptanceAssertionError>(result.error());
      clear += result.value().clearRays;
    }
  const auto coverage = static_cast<double>(clear) / static_cast<double>(total);
  return AcceptanceAssertionReport{
    coverage >= required.value(),
    total,
    clear,
    coverage,
    0.0,
    0.0,
    coverage >= required.value() ? "sightline clear" : "sightline blocked"};
}

AcceptanceAssertionResult boundsVisibility(
  const AcceptanceGeometryQuery& geometry,
  const AcceptanceAssertion& assertion,
  const AcceptanceAssertionContext& context)
{
  if (!assertion.bounds || !context.visibilityCamera)
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      "Bounds visibility needs bounds and a camera");
  const auto minimum = number(
    assertion.configuration,
    "minimumCoverage",
    assertion.type == AcceptanceAssertionType::BoundsVisible ? 0.01 : 0.0);
  if (minimum.is_error() || minimum.value() < 0.0 || minimum.value() > 1.0)
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration, "Invalid bounds coverage");
  const auto camera = transform(context, context.visibilityCamera->position, true);
  if (camera.is_error())
    return std::get<AcceptanceAssertionError>(camera.error());
  const auto& b = *assertion.bounds;
  auto points = std::vector<vm::vec3d>{};
  for (const auto x : {b.min.x(), b.max.x()})
    for (const auto y : {b.min.y(), b.max.y()})
      for (const auto z : {b.min.z(), b.max.z()})
        points.push_back({x, y, z});
  points.push_back((b.min + b.max) / 2.0);
  auto visible = size_t{0u};
  for (const auto& sample : points)
  {
    const auto end = transform(context, sample, true);
    if (end.is_error())
      return std::get<AcceptanceAssertionError>(end.error());
    const auto delta = end.value() - camera.value();
    const auto result = castClear(
      geometry,
      {camera.value(), vm::normalize(delta), vm::length(delta)},
      context.tolerance);
    if (result.is_error())
      return std::get<AcceptanceAssertionError>(result.error());
    visible += result.value().clearRays;
  }
  const auto coverage = static_cast<double>(visible) / static_cast<double>(points.size());
  const auto passed = assertion.type == AcceptanceAssertionType::BoundsVisible
                        ? coverage >= minimum.value()
                        : coverage <= minimum.value();
  return AcceptanceAssertionReport{
    passed,
    points.size(),
    visible,
    coverage,
    0.0,
    0.0,
    passed ? "bounds visibility accepted" : "bounds visibility rejected"};
}

AcceptanceAssertionResult opening(
  const AcceptanceGeometryQuery& geometry,
  const AcceptanceAssertion& assertion,
  const AcceptanceAssertionContext& context)
{
  const auto& c = assertion.configuration;
  const auto originValue = vec(c.value("origin"), "origin");
  const auto directionValue = vec(c.value("direction"), "direction");
  const auto upValue = vec(c.value("up"), "up");
  const auto width = number(c, "width");
  const auto height = number(c, "height");
  const auto depth = number(c, "depth");
  const auto samples = grid(c, "grid", 5);
  const auto requiredWidth =
    number(c, "minimumWidth", width.is_success() ? width.value() : 0.0);
  const auto requiredHeight =
    number(c, "minimumHeight", height.is_success() ? height.value() : 0.0);
  if (
    originValue.is_error() || directionValue.is_error() || upValue.is_error()
    || width.is_error() || height.is_error() || depth.is_error() || samples.is_error()
    || requiredWidth.is_error() || requiredHeight.is_error())
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      "Opening clearance configuration is incomplete");
  if (
    width.value() <= 0.0 || height.value() <= 0.0 || depth.value() <= 0.0
    || requiredWidth.value() <= 0.0 || requiredHeight.value() <= 0.0)
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      "Opening dimensions must be positive");
  const auto origin = transform(context, originValue.value(), true);
  const auto direction = transform(context, directionValue.value(), false);
  const auto upValueTransformed = transform(context, upValue.value(), false);
  if (origin.is_error() || direction.is_error() || upValueTransformed.is_error())
    return error(
      AcceptanceAssertionErrorCode::UnsupportedTransformation,
      "Opening transformation failed");
  const auto forward = vm::normalize(direction.value());
  const auto right = vm::normalize(vm::cross(forward, upValueTransformed.value()));
  const auto up = vm::normalize(vm::cross(right, forward));
  if (vm::is_zero(right, context.tolerance))
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      "Opening direction and up must not be parallel");
  const auto n = samples.value();
  const auto count = static_cast<size_t>(n);
  auto clear = std::vector<std::vector<bool>>(count, std::vector<bool>(count));
  auto clearCount = size_t{0u};
  for (int y = 0; y < n; ++y)
    for (int x = 0; x < n; ++x)
    {
      const auto px = (static_cast<double>(x) + 0.5) / static_cast<double>(n) - 0.5,
                 py = (static_cast<double>(y) + 0.5) / static_cast<double>(n) - 0.5;
      const auto start =
        origin.value() + right * (px * width.value()) + up * (py * height.value());
      const auto result =
        castClear(geometry, {start, forward, depth.value()}, context.tolerance);
      if (result.is_error())
        return std::get<AcceptanceAssertionError>(result.error());
      clear[static_cast<size_t>(y)][static_cast<size_t>(x)] = result.value().passed;
      clearCount += clear[static_cast<size_t>(y)][static_cast<size_t>(x)] ? 1u : 0u;
    }
  int bestArea = 0, bestW = 0, bestH = 0;
  auto heights = std::vector<int>(count);
  for (int y = 0; y < n; ++y)
  {
    for (int x = 0; x < n; ++x)
      heights[static_cast<size_t>(x)] =
        clear[static_cast<size_t>(y)][static_cast<size_t>(x)]
          ? heights[static_cast<size_t>(x)] + 1
          : 0;
    for (int left = 0; left < n; ++left)
    {
      int minH = heights[static_cast<size_t>(left)];
      for (int rightIndex = left; rightIndex < n; ++rightIndex)
      {
        minH = std::min(minH, heights[static_cast<size_t>(rightIndex)]);
        const int area = minH * (rightIndex - left + 1);
        if (area > bestArea)
        {
          bestArea = area;
          bestW = rightIndex - left + 1;
          bestH = minH;
        }
      }
    }
  }
  const auto measuredWidth =
    width.value() * static_cast<double>(bestW) / static_cast<double>(n);
  const auto measuredHeight =
    height.value() * static_cast<double>(bestH) / static_cast<double>(n);
  return AcceptanceAssertionReport{
    measuredWidth + context.tolerance >= requiredWidth.value()
      && measuredHeight + context.tolerance >= requiredHeight.value(),
    static_cast<size_t>(n * n),
    clearCount,
    static_cast<double>(clearCount) / static_cast<double>(n * n),
    measuredWidth,
    measuredHeight,
    "opening clearance evaluated"};
}

AcceptanceAssertionResult playerClearance(
  const AcceptanceGeometryQuery& geometry,
  const AcceptanceAssertion& assertion,
  const AcceptanceAssertionContext& context)
{
  const auto& config = assertion.configuration;
  const auto startValue = vec(config.value("start"), "start");
  const auto endValue =
    config.contains("end") ? vec(config.value("end"), "end") : startValue;
  const auto radius = number(config, "radius");
  const auto height = number(config, "height");
  const auto maxStep =
    number(config, "maxStep", radius.is_success() ? radius.value() : 0.0);
  if (
    startValue.is_error() || endValue.is_error() || radius.is_error() || height.is_error()
    || maxStep.is_error())
  {
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      "Player clearance requires finite start, optional end, radius, height, and "
      "maxStep");
  }
  if (radius.value() <= 0.0 || height.value() <= 0.0 || maxStep.value() <= 0.0)
  {
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      "Player clearance radius, height, and maxStep must be positive");
  }

  const auto start = transform(context, startValue.value(), true);
  const auto end = transform(context, endValue.value(), true);
  if (start.is_error())
    return std::get<AcceptanceAssertionError>(start.error());
  if (end.is_error())
    return std::get<AcceptanceAssertionError>(end.error());

  const auto delta = end.value() - start.value();
  const auto length = vm::length(delta);
  const auto intervals = std::ceil(length / maxStep.value());
  constexpr auto MaxPlayerClearanceSamples = 4096.0;
  if (!std::isfinite(intervals) || intervals + 1.0 > MaxPlayerClearanceSamples)
  {
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      "Player clearance segment exceeds the 4096-sample limit");
  }
  const auto sampleCount = static_cast<size_t>(intervals) + 1u;
  auto clearCount = size_t{0u};
  for (size_t index = 0u; index < sampleCount; ++index)
  {
    const auto fraction = sampleCount == 1u ? 0.0
                                            : static_cast<double>(index)
                                                / static_cast<double>(sampleCount - 1u);
    const auto position = start.value() + delta * fraction;
    const auto skin =
      std::min(context.tolerance, std::min(radius.value(), height.value()) / 2.0);
    const auto bounds = vm::bbox3d{
      {position.x() - radius.value() + skin,
       position.y() - radius.value() + skin,
       position.z() + skin},
      {position.x() + radius.value() - skin,
       position.y() + radius.value() - skin,
       position.z() + height.value() - skin}};
    const auto blocked = geometry.intersects(bounds);
    if (blocked.is_error())
      return queryError(blocked);
    if (!blocked.value())
      ++clearCount;
  }

  const auto passed = clearCount == sampleCount;
  return AcceptanceAssertionReport{
    passed,
    sampleCount,
    clearCount,
    static_cast<double>(clearCount) / static_cast<double>(sampleCount),
    radius.value() * 2.0,
    height.value(),
    passed ? "player volume clear" : "player volume blocked"};
}
} // namespace

std::ostream& operator<<(std::ostream& lhs, const AcceptanceAssertionError& rhs)
{
  return lhs << rhs.message;
}

Result<AcceptanceAssertion, AcceptanceAssertionError> acceptanceAssertionFromJson(
  const QJsonObject& json)
{
  const auto id = json.value("id");
  const auto typeName = json.value("type");
  const auto configuration = json.value("configuration");
  if (
    !id.isString() || id.toString().isEmpty() || !typeName.isString()
    || !configuration.isObject())
  {
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      "assertion requires non-empty id, type, and configuration object");
  }
  const auto type = assertionType(typeName.toString());
  if (!type)
  {
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration, "Unknown assertion type");
  }
  if (json.contains("maskId") && !json.value("maskId").isString())
  {
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      "assertion.maskId must be a string");
  }
  auto result = AcceptanceAssertion{
    id.toString().toStdString(),
    *type,
    json.contains("maskId") ? std::optional{json.value("maskId").toString().toStdString()}
                            : std::nullopt,
    std::nullopt,
    configuration.toObject()};
  if (json.contains("bounds"))
  {
    const auto parsed = bounds(json.value("bounds"));
    if (parsed.is_error())
      return std::get<AcceptanceAssertionError>(parsed.error());
    result.bounds = parsed.value();
  }
  return result;
}

Result<AcceptanceAssertionContext, AcceptanceAssertionError>
acceptanceAssertionContextFromJson(const QJsonObject& json)
{
  auto result = AcceptanceAssertionContext{};
  if (json.contains("alignment"))
  {
    const auto parsed = alignment(json.value("alignment"));
    if (parsed.is_error())
      return std::get<AcceptanceAssertionError>(parsed.error());
    result.alignment = parsed.value();
  }
  if (json.contains("geometrySpace"))
  {
    const auto value = json.value("geometrySpace");
    if (!value.isString())
    {
      return error(
        AcceptanceAssertionErrorCode::InvalidConfiguration,
        "context.geometrySpace must be 'reference' or 'target'");
    }
    if (value.toString() == "reference")
      result.geometrySpace = AcceptanceAssertionSpace::Reference;
    else if (value.toString() == "target")
      result.geometrySpace = AcceptanceAssertionSpace::Target;
    else
    {
      return error(
        AcceptanceAssertionErrorCode::InvalidConfiguration,
        "context.geometrySpace must be 'reference' or 'target'");
    }
  }
  if (json.contains("visibilityCamera"))
  {
    const auto value = json.value("visibilityCamera");
    if (!value.isObject())
    {
      return error(
        AcceptanceAssertionErrorCode::InvalidConfiguration,
        "context.visibilityCamera must be an object");
    }
    const auto position =
      vec(value.toObject().value("position"), "visibilityCamera.position");
    if (position.is_error())
      return std::get<AcceptanceAssertionError>(position.error());
    auto camera = AcceptanceCamera{};
    camera.position = position.value();
    result.visibilityCamera = camera;
  }
  if (json.contains("tolerance"))
  {
    const auto tolerance = json.value("tolerance");
    if (
      !tolerance.isDouble() || !std::isfinite(tolerance.toDouble())
      || tolerance.toDouble() < 0.0)
    {
      return error(
        AcceptanceAssertionErrorCode::InvalidConfiguration,
        "context.tolerance must be a non-negative finite number");
    }
    result.tolerance = tolerance.toDouble();
  }
  return result;
}

QJsonObject acceptanceAssertionReportToJson(const AcceptanceAssertionReport& report)
{
  return {
    {"passed", report.passed},
    {"totalRays", static_cast<qint64>(report.totalRays)},
    {"clearRays", static_cast<qint64>(report.clearRays)},
    {"coverage", report.coverage},
    {"measuredWidth", report.measuredWidth},
    {"measuredHeight", report.measuredHeight},
    {"message", QString::fromStdString(report.message)},
  };
}

AcceptanceAssertionEvaluator::AcceptanceAssertionEvaluator(
  const AcceptanceGeometryQuery& geometry)
  : m_geometry{geometry}
{
}
AcceptanceAssertionResult AcceptanceAssertionEvaluator::evaluate(
  const AcceptanceAssertion& assertion, const AcceptanceAssertionContext& context) const
{
  if (!std::isfinite(context.tolerance) || context.tolerance < 0.0)
    return error(
      AcceptanceAssertionErrorCode::InvalidConfiguration,
      "Tolerance must be non-negative");
  switch (assertion.type)
  {
  case AcceptanceAssertionType::ClearSightline:
    return sightline(m_geometry, assertion, context);
  case AcceptanceAssertionType::BoundsVisible:
  case AcceptanceAssertionType::BoundsNotVisible:
    return boundsVisibility(m_geometry, assertion, context);
  case AcceptanceAssertionType::OpeningClearance:
    return opening(m_geometry, assertion, context);
  case AcceptanceAssertionType::PlayerClearance:
    return playerClearance(m_geometry, assertion, context);
  default:
    return error(
      AcceptanceAssertionErrorCode::UnsupportedType,
      "Assertion type is not implemented by AV5");
  }
}
} // namespace tb::ui
