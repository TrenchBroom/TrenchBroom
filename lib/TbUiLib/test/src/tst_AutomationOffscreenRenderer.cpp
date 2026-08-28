/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include "gl/GlManager.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/GameConfigFixture.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/WorldNode.h"
#include "ui/AppControllerFixture.h"
#include "ui/AutomationOffscreenRenderer.h"
#include "ui/AutomationRenderRequest.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"

#include <cmath>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

std::unique_ptr<MapDocument> makeDocument(AppController& appController)
{
  return MapDocument::createDocument(
           appController.environmentConfig(),
           mdl::QuakeGameInfo,
           mdl::MapFormat::Valve,
           vm::bbox3d{8192.0},
           appController.taskManager(),
           appController.glManager().resourceManager())
         | kdl::value();
}

automation::AutomationRenderRequest makeRequest()
{
  return {
    .camera =
      {
        .projection = automation::AutomationProjection::Perspective,
        .position = {0.0, -128.0, 64.0},
        .direction = {0.0, 1.0, 0.0},
        .up = {0.0, 0.0, 1.0},
        .verticalFov = 90.0,
        .nearPlane = 1.0,
        .farPlane = 65536.0,
      },
    .size = {64, 48},
  };
}

mdl::BrushNode* addCuboid(mdl::Map& map, const vm::bbox3d& bounds)
{
  const auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
  const auto brush = builder.createCuboid(bounds, "automation/test") | kdl::value();
  auto* node = new mdl::BrushNode{std::move(brush)};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {node}}});
  return node;
}

} // namespace

TEST_CASE("Automation offscreen renderer")
{
  auto fixture = AppControllerFixture{};
  auto& appController = fixture.appController();
  auto document = makeDocument(appController);

  auto surface = QOffscreenSurface{};
  surface.setFormat(QSurfaceFormat::defaultFormat());
  surface.create();

  auto context = QOpenGLContext{};
  context.setFormat(surface.format());
  context.setShareContext(QOpenGLContext::globalShareContext());
  context.create();
  REQUIRE(surface.isValid());
  REQUIRE(context.isValid());

  const auto* focusBefore = QApplication::focusWidget();
  const auto revisionBefore = document->map().modificationCount();
  auto renderer =
    AutomationOffscreenRenderer{context, surface, appController.glManager()};

  const auto result = renderer.capture(*document, makeRequest());

  INFO(result.message.toStdString());
  INFO(static_cast<int>(result.error));
  REQUIRE(result);
  CHECK(result.revision == revisionBefore);
  CHECK(result.image.size() == QSize{64, 48});
  CHECK_FALSE(result.image.isNull());
  CHECK(QApplication::focusWidget() == focusBefore);
  CHECK(document->map().modificationCount() == revisionBefore);
}

TEST_CASE("Automation offscreen renderer depth output")
{
  auto fixture = AppControllerFixture{};
  auto& appController = fixture.appController();
  auto document = makeDocument(appController);
  addCuboid(document->map(), {{10.0, -2.0, -2.0}, {20.0, 2.0, 2.0}});

  auto surface = QOffscreenSurface{};
  surface.setFormat(QSurfaceFormat::defaultFormat());
  surface.create();
  auto context = QOpenGLContext{};
  context.setFormat(surface.format());
  context.setShareContext(QOpenGLContext::globalShareContext());
  context.create();
  REQUIRE(surface.isValid());
  REQUIRE(context.isValid());

  auto request = makeRequest();
  request.camera.position = {0.0, 0.0, 0.0};
  request.camera.direction = {1.0, 0.0, 0.0};
  request.camera.nearPlane = 1.0;
  request.camera.farPlane = 100.0;
  request.outputs.depth = true;
  auto renderer =
    AutomationOffscreenRenderer{context, surface, appController.glManager()};
  const auto result = renderer.capture(*document, request);

  INFO(result.message.toStdString());
  REQUIRE(result);
  CHECK_FALSE(result.image.isNull());
  REQUIRE(result.depth);
  REQUIRE(result.depth->values.size() == 64u * 48u);
  const auto center = result.depth->values[24u * 64u + 32u];
  CHECK(center == Catch::Approx(10.0f).margin(0.02f));
  CHECK(std::isinf(result.depth->values[0]));
}

TEST_CASE("Automation offscreen renderer shows selected changes without highlighting")
{
  auto fixture = AppControllerFixture{};
  auto& appController = fixture.appController();
  auto document = makeDocument(appController);

  auto surface = QOffscreenSurface{};
  surface.setFormat(QSurfaceFormat::defaultFormat());
  surface.create();
  auto context = QOpenGLContext{};
  context.setFormat(surface.format());
  context.setShareContext(QOpenGLContext::globalShareContext());
  context.create();
  REQUIRE(surface.isValid());
  REQUIRE(context.isValid());

  auto renderer =
    AutomationOffscreenRenderer{context, surface, appController.glManager()};
  const auto request = makeRequest();
  const auto before = renderer.capture(*document, request);
  REQUIRE(before);

  auto& map = document->map();
  auto* cuboid = addCuboid(map, {{-24.0, -8.0, 40.0}, {24.0, 8.0, 88.0}});
  mdl::selectNodes(map, {cuboid});
  const auto changedRevision = document->map().modificationCount();
  const auto after = renderer.capture(*document, request);

  INFO(after.message.toStdString());
  REQUIRE(after);
  CHECK(after.revision == changedRevision);
  CHECK(after.image != before.image);
  CHECK(cuboid->selected());
  CHECK(map.modificationCount() == changedRevision);
}

} // namespace tb::ui
