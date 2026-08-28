/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceMapGeometry.h"

#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Picking.h"
#include "mdl/PatchNode.h"
#include "mdl/PickResult.h"
#include "ui/AutomationDocumentRegistry.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"

#include <algorithm>
#include <cmath>
#include <ranges>
#include <utility>

namespace tb::ui
{
namespace
{

AcceptanceGeometryError error(std::string message)
{
  return {std::move(message)};
}

} // namespace

AcceptanceMapGeometryQuery::AcceptanceMapGeometryQuery(mdl::Map& map)
  : m_map{map}
{
}

Result<std::vector<AcceptanceGeometryHit>, AcceptanceGeometryError>
AcceptanceMapGeometryQuery::cast(const AcceptanceStructuralRay& ray) const
{
  if (
    !std::isfinite(ray.maxDistance) || ray.maxDistance <= 0.0
    || !std::isfinite(ray.origin.x()) || !std::isfinite(ray.origin.y())
    || !std::isfinite(ray.origin.z()) || !std::isfinite(ray.direction.x())
    || !std::isfinite(ray.direction.y()) || !std::isfinite(ray.direction.z())
    || vm::is_zero(ray.direction, 0.000001))
  {
    return error("Structural ray must have finite origin, direction, and positive range");
  }

  auto hits = std::vector<AcceptanceGeometryHit>{};
  const auto containing = mdl::findNodesContaining(m_map, ray.origin);
  if (std::ranges::any_of(containing, [](const auto* node) {
        return dynamic_cast<const mdl::BrushNode*>(node) != nullptr;
      }))
  {
    hits.push_back({0.0});
  }

  const auto pickRay = vm::ray3d{ray.origin, vm::normalize(ray.direction)};
  auto result = mdl::PickResult::byDistance();
  mdl::pick(m_map, pickRay, result);
  for (const auto& hit : result.all())
  {
    if (
      (hit.type() == mdl::BrushNode::BrushHitType
       || hit.type() == mdl::PatchNode::PatchHitType)
      && std::isfinite(hit.distance()) && hit.distance() >= 0.0
      && hit.distance() <= ray.maxDistance)
    {
      hits.push_back({hit.distance()});
    }
  }
  std::ranges::sort(hits, {}, &AcceptanceGeometryHit::distance);
  return hits;
}

AcceptanceMapGeometryProvider::AcceptanceMapGeometryProvider(
  AcceptanceMapResolver& resolver)
  : m_resolver{resolver}
{
}

Result<std::shared_ptr<const AcceptanceGeometryQuery>, AcceptanceGeometryError>
AcceptanceMapGeometryProvider::geometryFor(
  const AcceptanceCaptureDocumentIdentity& document)
{
  if (document.documentId.empty())
    return error("Acceptance geometry requires an explicit documentId");
  const auto map = m_resolver.resolve(document);
  if (map.is_error())
    return std::get<AcceptanceGeometryError>(map.error());
  if (map.value() == nullptr)
    return error("Acceptance geometry resolver returned no map for documentId");
  return std::make_shared<AcceptanceMapGeometryQuery>(*map.value());
}

AcceptanceAutomationMapResolver::AcceptanceAutomationMapResolver(
  const AutomationDocumentRegistry& documents, HiddenMapResolver hiddenMap)
  : m_documents{documents}
  , m_hiddenMap{std::move(hiddenMap)}
{
}

Result<mdl::Map*, AcceptanceGeometryError> AcceptanceAutomationMapResolver::resolve(
  const AcceptanceCaptureDocumentIdentity& document)
{
  if (document.documentId.empty())
    return error("Acceptance geometry requires an explicit documentId");
  auto* window = m_documents.findWindow(QString::fromStdString(document.documentId));
  if (window != nullptr)
    return &window->document().map();
  if (m_hiddenMap)
  {
    if (auto* map = m_hiddenMap(document.documentId); map != nullptr)
      return map;
  }
  return error(
    "Acceptance geometry documentId is not a live or hidden captured document");
}

} // namespace tb::ui
