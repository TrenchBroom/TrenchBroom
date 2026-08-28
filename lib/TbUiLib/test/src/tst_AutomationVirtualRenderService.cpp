/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QApplication>
#include <QFileInfo>
#include <QOpenGLContext>
#include <QTemporaryDir>

#include "gl/GlManager.h"
#include "mdl/GameConfigFixture.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "ui/AppControllerFixture.h"
#include "ui/AutomationRenderRequest.h"
#include "ui/AutomationVirtualRenderService.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/QPathUtils.h"

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

} // namespace

TEST_CASE("Automation virtual render service")
{
  auto fixture = AppControllerFixture{};
  auto& appController = fixture.appController();
  auto document = makeDocument(appController);
  auto outputDirectory = QTemporaryDir{};
  REQUIRE(outputDirectory.isValid());

  const auto* focusBefore = QApplication::focusWidget();
  const auto* contextBefore = QOpenGLContext::currentContext();
  const auto revisionBefore = document->map().modificationCount();
  auto service = AutomationVirtualRenderService{
    appController, pathFromQString(outputDirectory.path())};

  const auto first = service.capture(*document, makeRequest());
  auto depthRequest = makeRequest();
  depthRequest.outputs.depth = true;
  const auto second = service.capture(*document, depthRequest);

  INFO(first.message.toStdString());
  INFO(static_cast<int>(first.error));
  REQUIRE(first);
  INFO(second.message.toStdString());
  INFO(static_cast<int>(second.error));
  REQUIRE(second);
  CHECK(first.revision == revisionBefore);
  CHECK(second.revision == revisionBefore);
  CHECK(first.output.captureMode == automation::AutomationCaptureMode::Offscreen);
  CHECK(first.output.size.width == 64);
  CHECK(first.output.size.height == 48);
  CHECK(first.output.imagePath != second.output.imagePath);
  CHECK_FALSE(first.output.depth);
  REQUIRE(second.output.depth);
  CHECK(second.output.depth->size.width == 64);
  CHECK(second.output.depth->size.height == 48);
  CHECK(QFileInfo{pathAsQString(first.output.imagePath)}.isFile());
  CHECK(QFileInfo{pathAsQString(second.output.imagePath)}.isFile());
  CHECK(QFileInfo{pathAsQString(second.output.depth->path)}.isFile());
  CHECK(QApplication::focusWidget() == focusBefore);
  CHECK(QOpenGLContext::currentContext() == contextBefore);
  CHECK(document->map().modificationCount() == revisionBefore);
}

} // namespace tb::ui
