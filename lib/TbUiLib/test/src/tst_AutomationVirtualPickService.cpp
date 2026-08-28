/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QApplication>

#include "gl/OrthographicCamera.h"
#include "gl/PerspectiveCamera.h"
#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/TestFactory.h"
#include "ui/AutomationRenderRequest.h"
#include "ui/AutomationVirtualPickService.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/MapViewContext.h"

#include <type_traits>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

automation::AutomationRenderRequest makeRequest(
  const automation::AutomationProjection projection =
    automation::AutomationProjection::Perspective)
{
  auto request = automation::AutomationRenderRequest{
    .camera =
      {
        .projection = projection,
        .position = {-128.0, 0.0, 0.0},
        .direction = {1.0, 0.0, 0.0},
        .up = {0.0, 0.0, 1.0},
        .nearPlane = 1.0,
        .farPlane = 65536.0,
      },
    .size = {64, 48},
  };
  if (projection == automation::AutomationProjection::Perspective)
  {
    request.camera.verticalFov = 90.0;
  }
  else
  {
    request.camera.zoom = 1.0;
  }
  return request;
}

template <typename Camera>
MapViewPickResult interactiveEquivalent(
  mdl::Map& map,
  Camera& camera,
  const automation::AutomationRenderRequest& request,
  const float x,
  const float y)
{
  camera.setNearPlane(static_cast<float>(request.camera.nearPlane));
  camera.setFarPlane(static_cast<float>(request.camera.farPlane));
  camera.setViewport({0, 0, request.size.width, request.size.height});
  camera.moveTo(vm::vec3f{request.camera.position});
  camera.setDirection(vm::vec3f{request.camera.direction}, vm::vec3f{request.camera.up});
  if constexpr (std::is_same_v<Camera, gl::PerspectiveCamera>)
  {
    camera.setFov(static_cast<float>(*request.camera.verticalFov));
  }
  else
  {
    camera.setZoom(static_cast<float>(*request.camera.zoom));
  }
  return pickMapView(map, camera, x, y);
}

} // namespace

TEST_CASE("Automation virtual pick service")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();
  auto* brushNode = mdl::createBrushNode(map);
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {brushNode}}});
  const auto* focusBefore = QApplication::focusWidget();
  const auto revisionBefore = map.modificationCount();
  const auto picker = AutomationVirtualPickService{};

  SECTION("uses the same perspective ray and semantic hit as the capture camera")
  {
    const auto request = makeRequest();
    const auto result = picker.pick(document, request, 32.0, 24.0);
    auto camera = gl::PerspectiveCamera{};
    const auto expected = interactiveEquivalent(map, camera, request, 32.0f, 24.0f);

    REQUIRE(result);
    CHECK(result.revision == revisionBefore);
    CHECK(result.pick.ray.origin == expected.ray.origin);
    CHECK(result.pick.ray.direction == expected.ray.direction);
    REQUIRE_FALSE(result.pick.hits.empty());
    REQUIRE(result.pick.hits.front().node);
    REQUIRE(result.pick.hits.front().faceIndex);
    CHECK(result.pick.hits.front().node->name == brushNode->name());
    CHECK(QApplication::focusWidget() == focusBefore);
    CHECK(map.modificationCount() == revisionBefore);
  }

  SECTION("supports orthographic camera rays and hit ordering")
  {
    const auto request = makeRequest(automation::AutomationProjection::Orthographic);
    const auto result = picker.pick(document, request, 32.0, 24.0);
    auto camera = gl::OrthographicCamera{};
    const auto expected = interactiveEquivalent(map, camera, request, 32.0f, 24.0f);

    REQUIRE(result);
    CHECK(result.pick.ray.origin == expected.ray.origin);
    CHECK(result.pick.ray.direction == expected.ray.direction);
    REQUIRE_FALSE(result.pick.hits.empty());
  }

  SECTION("rejects pixels outside the requested image")
  {
    const auto request = makeRequest();
    const auto result = picker.pick(document, request, 64.0, 24.0);

    CHECK_FALSE(result);
    CHECK(result.error == AutomationVirtualPickError::InvalidPixel);
    CHECK(result.revision == revisionBefore);
  }
}

} // namespace tb::ui
