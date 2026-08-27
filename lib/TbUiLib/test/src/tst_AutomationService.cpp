/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocalSocket>
#include <QTest>

#include "mdl/BrushNode.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GameConfigFixture.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h"
#include "ui/AppControllerFixture.h"
#include "ui/AutomationService.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/MapViewBase.h"
#include "ui/MapWindow.h"
#include "ui/MapWindowManager.h"

#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

std::unique_ptr<QLocalSocket> connectClient(const QString& serverName)
{
  auto socket = std::make_unique<QLocalSocket>();
  socket->connectToServer(serverName);
  REQUIRE(socket->waitForConnected(1000));
  return socket;
}

QJsonObject sendRequest(QLocalSocket& socket, const QString& method, QJsonObject params)
{
  const auto request = QJsonDocument{QJsonObject{
                                       {"jsonrpc", "2.0"},
                                       {"method", method},
                                       {"params", std::move(params)},
                                       {"id", 1},
                                     }}
                         .toJson(QJsonDocument::Compact)
                       + '\n';
  REQUIRE(socket.write(request) == request.size());
  REQUIRE(socket.waitForBytesWritten(1000));

  auto timeout = QElapsedTimer{};
  timeout.start();
  while (!socket.canReadLine() && timeout.elapsed() < 1000)
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    QTest::qWait(1);
  }
  REQUIRE(socket.canReadLine());

  auto parseError = QJsonParseError{};
  const auto response =
    QJsonDocument::fromJson(socket.readLine().trimmed(), &parseError).object();
  REQUIRE(parseError.error == QJsonParseError::NoError);
  return response;
}

QJsonObject entityCreateParams(
  const size_t revision, const QString& classname, const QString& entityType)
{
  return QJsonObject{
    {"classname", classname},
    {"entityType", entityType},
    {"expectedRevision", static_cast<qint64>(revision)},
  };
}

} // namespace

TEST_CASE("AutomationService entity creation")
{
  auto fixture = AppControllerFixture{};
  auto& appController = fixture.appController();
  REQUIRE(appController.mapWindowManager()
            .createDocument(
              mdl::QuakeGameInfo, mdl::MapFormat::Valve, vm::bbox3d{8192.0}, false)
            .is_success());
  auto& map = appController.mapWindowManager().topMapWindow()->document().map();
  map.entityDefinitionManager().setDefinitions({
    mdl::EntityDefinition{
      "automation_point",
      {},
      "",
      {},
      mdl::PointEntityDefinition{vm::bbox3d{16.0}, {}, {}},
    },
    mdl::EntityDefinition{"automation_brush", {}, "", {}},
  });

  auto* selectedBrush = mdl::createBrushNode(map, "automation/material");
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {selectedBrush}}});
  mdl::selectNodes(map, {selectedBrush});
  map.setCurrentMaterialName("automation/material");

  auto service = AutomationService{appController};
  REQUIRE(service.isListening());
  auto client = connectClient(service.serverName());

  SECTION("creates an empty brush entity without changing editor state")
  {
    auto params =
      entityCreateParams(map.modificationCount(), "automation_brush", "brush");
    params.insert("properties", QJsonObject{{"targetname", "water_volume"}});
    const auto response = sendRequest(*client, "nodes.entity.create", std::move(params));

    REQUIRE(response.contains("result"));
    const auto result = response.value("result").toObject();
    const auto path = result.value("path").toArray();
    REQUIRE(path.size() == 2);
    auto* entity = dynamic_cast<mdl::EntityNode*>(map.worldNode().resolvePath(
      mdl::NodePath{
        {static_cast<size_t>(path[0].toInteger()),
         static_cast<size_t>(path[1].toInteger())}}));
    REQUIRE(entity != nullptr);
    CHECK(entity->children().empty());
    CHECK_FALSE(entity->entity().pointEntity());
    CHECK(entity->entity().hasProperty("classname", "automation_brush"));
    CHECK(entity->entity().hasProperty("targetname", "water_volume"));
    CHECK(map.selection().nodes == std::vector<mdl::Node*>{selectedBrush});
    CHECK(map.currentMaterialName() == "automation/material");

    const auto createBrushesResponse = sendRequest(
      *client,
      "geometry.createBrushes",
      QJsonObject{
        {"expectedRevision", result.value("revision")},
        {"parentPath", path},
        {"brushes",
         QJsonArray{QJsonObject{
           {"material", "automation/material"},
           {"points",
            QJsonArray{
              QJsonArray{0, 0, 0},
              QJsonArray{64, 0, 0},
              QJsonArray{64, 64, 0},
              QJsonArray{0, 64, 0},
              QJsonArray{0, 0, 64},
              QJsonArray{64, 0, 64},
              QJsonArray{64, 64, 64},
              QJsonArray{0, 64, 64},
            }},
         }}},
      });
    REQUIRE(createBrushesResponse.contains("result"));
    CHECK(entity->children().size() == 1u);
  }

  SECTION("creates a point entity with explicit properties")
  {
    auto params =
      entityCreateParams(map.modificationCount(), "automation_point", "point");
    params.insert("properties", QJsonObject{{"targetname", "marker"}});
    const auto response = sendRequest(*client, "nodes.entity.create", std::move(params));

    REQUIRE(response.contains("result"));
    const auto result = response.value("result").toObject();
    const auto path = result.value("path").toArray();
    auto* entity = dynamic_cast<mdl::EntityNode*>(map.worldNode().resolvePath(
      mdl::NodePath{
        {static_cast<size_t>(path[0].toInteger()),
         static_cast<size_t>(path[1].toInteger())}}));
    REQUIRE(entity != nullptr);
    CHECK(entity->entity().pointEntity());
    CHECK(entity->entity().hasProperty("classname", "automation_point"));
    CHECK(entity->entity().hasProperty("targetname", "marker"));
    CHECK(map.selection().nodes == std::vector<mdl::Node*>{selectedBrush});
    CHECK(map.currentMaterialName() == "automation/material");
  }

  SECTION("rejects invalid requests before mutation")
  {
    const auto initialRevision = map.modificationCount();
    auto wrongType = entityCreateParams(initialRevision, "automation_point", "brush");
    CHECK(sendRequest(*client, "nodes.entity.create", std::move(wrongType))
            .contains("error"));
    CHECK(map.modificationCount() == initialRevision);

    auto invalidProperties =
      entityCreateParams(initialRevision, "automation_point", "point");
    invalidProperties.insert("properties", QJsonObject{{"health", 100}});
    CHECK(sendRequest(*client, "nodes.entity.create", std::move(invalidProperties))
            .contains("error"));
    CHECK(map.modificationCount() == initialRevision);

    auto stale = entityCreateParams(initialRevision - 1u, "automation_point", "point");
    const auto staleResponse =
      sendRequest(*client, "nodes.entity.create", std::move(stale));
    REQUIRE(staleResponse.contains("error"));
    CHECK(staleResponse.value("error").toObject().value("code") == -32001);
    CHECK(map.modificationCount() == initialRevision);
  }

  client.reset();
  const auto savedMapPath =
    std::filesystem::temp_directory_path()
    / ("trenchbroom-automation-test-"
       + std::to_string(QCoreApplication::applicationPid()) + ".map");
  REQUIRE(map.saveAs(savedMapPath).is_success());
  appController.mapWindowManager().topMapWindow()->close();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  std::filesystem::remove(savedMapPath);
}

TEST_CASE("AutomationService profile extrusion")
{
  auto fixture = AppControllerFixture{};
  auto& appController = fixture.appController();
  REQUIRE(appController.mapWindowManager()
            .createDocument(
              mdl::QuakeGameInfo, mdl::MapFormat::Valve, vm::bbox3d{8192.0}, false)
            .is_success());
  auto& map = appController.mapWindowManager().topMapWindow()->document().map();
  auto service = AutomationService{appController};
  auto client = connectClient(service.serverName());

  const auto profile = QJsonObject{
    {"plane", "xz"},
    {"gridSize", 16},
    {"profile", QJsonArray{QJsonArray{0, 0}, QJsonArray{128, 0}, QJsonArray{64, 96}}},
    {"interval", QJsonArray{-32, 32}},
    {"material", "automation/material"},
    {"role", "gable"},
  };

  SECTION("previews exact points without changing the document")
  {
    const auto response =
      sendRequest(*client, "geometry.extrudeProfile.preview", profile);
    REQUIRE(response.contains("result"));
    const auto result = response.value("result").toObject();
    CHECK(result.value("plane") == "xz");
    CHECK(result.value("brushCount") == 1);
    const auto points =
      result.value("brushes").toArray()[0].toObject().value("points").toArray();
    REQUIRE(points.size() == 6);
    CHECK(points[0].toArray() == QJsonArray{0, -32, 0});
    CHECK(points[5].toArray() == QJsonArray{64, 32, 96});
    CHECK(map.modificationCount() == 0u);
  }

  SECTION("applies atomically and rejects a stale revision")
  {
    auto apply = profile;
    apply.insert("expectedRevision", static_cast<qint64>(map.modificationCount()));
    const auto response = sendRequest(*client, "geometry.extrudeProfile.apply", apply);
    REQUIRE(response.contains("result"));
    const auto result = response.value("result").toObject();
    CHECK(result.value("operation") == "extrudeProfile");
    CHECK(result.value("count") == 1);
    CHECK(map.worldNode().childCount() == 1u);

    const auto staleResponse =
      sendRequest(*client, "geometry.extrudeProfile.apply", apply);
    REQUIRE(staleResponse.contains("error"));
    CHECK(staleResponse.value("error").toObject().value("code") == -32001);
    CHECK(map.worldNode().childCount() == 1u);
  }

  client.reset();
  const auto savedMapPath =
    std::filesystem::temp_directory_path()
    / ("trenchbroom-profile-extrusion-test-"
       + std::to_string(QCoreApplication::applicationPid()) + ".map");
  REQUIRE(map.saveAs(savedMapPath).is_success());
  appController.mapWindowManager().topMapWindow()->close();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  std::filesystem::remove(savedMapPath);
}

TEST_CASE("AutomationService explicit document view isolation")
{
  auto fixture = AppControllerFixture{};
  auto& appController = fixture.appController();
  auto& windowManager = appController.mapWindowManager();

  REQUIRE(windowManager
            .createDocument(
              mdl::QuakeGameInfo, mdl::MapFormat::Valve, vm::bbox3d{8192.0}, false)
            .is_success());
  REQUIRE(windowManager
            .createDocument(
              mdl::QuakeGameInfo, mdl::MapFormat::Valve, vm::bbox3d{8192.0}, false)
            .is_success());

  const auto windows = windowManager.mapWindows();
  REQUIRE(windows.size() == 2u);
  auto* targetWindow = windows[0];
  auto* foregroundWindow = windows[1];
  auto* targetView = targetWindow->currentMapViewBase();
  auto* foregroundView = foregroundWindow->currentMapViewBase();
  REQUIRE(targetView != nullptr);
  REQUIRE(foregroundView != nullptr);

  targetWindow->document().map().setCurrentMaterialName("target/material");
  foregroundWindow->document().map().setCurrentMaterialName("foreground/material");

  auto service = AutomationService{appController};
  REQUIRE(service.isListening());
  auto client = connectClient(service.serverName());

  const auto documentsResponse = sendRequest(*client, "documents.list", {});
  REQUIRE(documentsResponse.contains("result"));
  const auto documents = documentsResponse.value("result").toArray();
  REQUIRE(documents.size() == 2);
  const auto targetDocumentId = documents[0].toObject().value("id").toString();
  REQUIRE_FALSE(targetDocumentId.isEmpty());

  // QApplication broadcasts focusChanged to every MapWindow. Simulate the other
  // document coming to the foreground before explicitly capturing the target.
  targetWindow->focusChange(targetView, foregroundView);
  CHECK(targetWindow->currentMapViewBase() == targetView);

  const auto captureResponse = sendRequest(
    *client,
    "context.capture",
    QJsonObject{{"documentId", targetDocumentId}, {"screenshot", false}});
  REQUIRE(captureResponse.contains("result"));
  const auto capture = captureResponse.value("result").toObject();
  CHECK(capture.value("documentId") == targetDocumentId);
  CHECK(capture.value("currentMaterial") == "target/material");

  client.reset();
  for (auto* window : windows)
  {
    window->close();
  }
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

} // namespace tb::ui
