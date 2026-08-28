/* Copyright (C) 2026 */
#pragma once

#include "ui/AcceptanceView.h"

#include "vm/bbox.h"

#include <optional>
#include <string>

namespace tb::ui
{

struct AcceptanceStructuralRay
{
  vm::vec3d origin;
  vm::vec3d direction;
  double maxDistance = 0.0;
};

struct AcceptanceGeometryHit
{
  double distance = 0.0;
};

struct AcceptanceGeometryError
{
  std::string message;
};

class AcceptanceGeometryQuery
{
public:
  virtual ~AcceptanceGeometryQuery() = default;
  virtual Result<std::vector<AcceptanceGeometryHit>, AcceptanceGeometryError> cast(
    const AcceptanceStructuralRay& ray) const = 0;

  /** Tests an explicit volume without consulting selection, a view, or GUI focus. */
  virtual Result<bool, AcceptanceGeometryError> intersects(const vm::bbox3d&) const
  {
    return AcceptanceGeometryError{"Structural volume intersection is not available"};
  }

  /**
   * Whether the query can be evaluated on a worker thread. Queries over a live editor
   * map must return false so that the map cannot race a UI-thread mutation.
   */
  virtual bool isThreadSafe() const { return true; }
};

enum class AcceptanceAssertionSpace
{
  Reference,
  Target,
};

struct AcceptanceAssertionContext
{
  AcceptanceAlignment alignment;
  AcceptanceAssertionSpace geometrySpace = AcceptanceAssertionSpace::Reference;
  std::optional<AcceptanceCamera> visibilityCamera;
  double tolerance = 0.01;
};

enum class AcceptanceAssertionErrorCode
{
  UnsupportedType,
  InvalidConfiguration,
  GeometryQueryFailed,
  StartInsideSolid,
  UnsupportedTransformation,
};

struct AcceptanceAssertionError
{
  AcceptanceAssertionErrorCode code = AcceptanceAssertionErrorCode::InvalidConfiguration;
  std::string message;
};

std::ostream& operator<<(std::ostream& lhs, const AcceptanceAssertionError& rhs);

struct AcceptanceAssertionReport
{
  bool passed = false;
  size_t totalRays = 0u;
  size_t clearRays = 0u;
  double coverage = 0.0;
  double measuredWidth = 0.0;
  double measuredHeight = 0.0;
  std::string message;
};

using AcceptanceAssertionResult =
  Result<AcceptanceAssertionReport, AcceptanceAssertionError>;

/** Parses one non-persistent assertion payload used by direct automation. */
Result<AcceptanceAssertion, AcceptanceAssertionError> acceptanceAssertionFromJson(
  const QJsonObject& json);

/** Parses the renderer-independent context used by direct automation assertions. */
Result<AcceptanceAssertionContext, AcceptanceAssertionError>
acceptanceAssertionContextFromJson(const QJsonObject& json);

QJsonObject acceptanceAssertionReportToJson(const AcceptanceAssertionReport& report);

/** Evaluates AV5 CPU-only structural assertions without a renderer or a map view. */
class AcceptanceAssertionEvaluator
{
public:
  explicit AcceptanceAssertionEvaluator(const AcceptanceGeometryQuery& geometry);

  AcceptanceAssertionResult evaluate(
    const AcceptanceAssertion& assertion,
    const AcceptanceAssertionContext& context = {}) const;

private:
  const AcceptanceGeometryQuery& m_geometry;
};

} // namespace tb::ui
