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
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocalSocket>
#include <QTemporaryDir>
#include <QTest>

#include "base/PreferenceManager.h"
#include "gl/GlManager.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/EnvironmentConfig.h"
#include "mdl/GameConfigFixture.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h"
#include "prefs/Preferences.h"
#include "ui/AcceptanceView.h"
#include "ui/AppControllerFixture.h"
#include "ui/AutomationService.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/MapViewBase.h"
#include "ui/MapWindow.h"
#include "ui/MapWindowManager.h"
#include "ui/QPathUtils.h"

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

QJsonObject sendRequest(
  QLocalSocket& socket,
  const QString& method,
  QJsonObject params,
  const int responseTimeoutMs = 1000)
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
  while (!socket.canReadLine() && timeout.elapsed() < responseTimeoutMs)
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

void initializeQuakeGame(const WriteGameConfig& writeGameConfig)
{
  writeGameConfig(
    "Quake",
    R"({
      "version": 9,
      "name": "Quake",
      "fileformats": [{"format": "Valve"}],
      "filesystem": {
        "searchpath": "id1",
        "packageformat": {"extension": ".pak", "format": "idpak"}
      },
      "materials": {
        "root": "textures",
        "extensions": [".D"],
        "palette": "gfx/palette.lmp"
      },
      "entities": {"definitions": [], "defaultcolor": "0.6 0.6 0.6 1.0"}
    })",
    std::nullopt,
    std::nullopt);
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

TEST_CASE("AppController automatic update policy follows the build configuration")
{
#if defined(TB_ENABLE_UPDATE_CHECKS)
  CHECK(AppController::automaticUpdatesEnabledForBuild());
#else
  CHECK_FALSE(AppController::automaticUpdatesEnabledForBuild());
#endif
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

TEST_CASE("AutomationService workspace lifecycle RPC")
{
  setPref(
    Preferences::RendererFontPath,
    std::filesystem::path{__FILE__}
        .parent_path()
        .parent_path()
        .parent_path()
        .parent_path()
        .parent_path()
      / "app/TrenchBroom/resources/fonts/SourceSansPro-Regular.otf");
  auto fixture = AppControllerFixture{initializeQuakeGame};
  auto& appController = fixture.appController();
  auto& windowManager = appController.mapWindowManager();
  REQUIRE(windowManager
            .createDocument(
              mdl::QuakeGameInfo, mdl::MapFormat::Valve, vm::bbox3d{8192.0}, false)
            .is_success());
  auto* sourceWindow = windowManager.topMapWindow();
  REQUIRE(sourceWindow != nullptr);
  const auto sourcePath =
    std::filesystem::temp_directory_path()
    / ("trenchbroom-automation-workspace-rpc-"
       + std::to_string(QCoreApplication::applicationPid()) + ".map");
  REQUIRE(sourceWindow->document().map().saveAs(sourcePath).is_success());

  auto workspaceDirectory = std::filesystem::path{};
  {
    auto service = AutomationService{appController};
    auto client = connectClient(service.serverName());
    const auto documents =
      sendRequest(*client, "documents.list", {}).value("result").toArray();
    REQUIRE(documents.size() == 1);
    const auto sourceDocumentId = documents[0].toObject().value("id").toString();
    REQUIRE_FALSE(sourceDocumentId.isEmpty());

    const auto missingStatus = sendRequest(*client, "workspace.status", {});
    REQUIRE(missingStatus.contains("error"));
    CHECK(missingStatus.value("error").toObject().value("code") == -32602);

    const auto unknownStatus = sendRequest(
      *client, "workspace.status", QJsonObject{{"workspaceId", "does-not-exist"}});
    REQUIRE(unknownStatus.contains("error"));
    CHECK(unknownStatus.value("error").toObject().value("code") == -32020);

    const auto forked = sendRequest(
      *client,
      "workspace.fork",
      QJsonObject{{"documentId", sourceDocumentId}, {"name", "RPC lifecycle"}},
      5000);
    REQUIRE(forked.contains("result"));
    const auto forkResult = forked.value("result").toObject();
    const auto workspaceId = forkResult.value("workspaceId").toString();
    REQUIRE_FALSE(workspaceId.isEmpty());
    workspaceDirectory = pathFromQString(forkResult.value("directory").toString());
    CHECK(workspaceDirectory.string().starts_with(
      appController.environmentConfig().userDataFolderPath.string()));
    CHECK(forkResult.value("runtimeStatus") == "attached");
    CHECK(forkResult.value("sourceDocumentId") == sourceDocumentId);

    const auto checkpointed = sendRequest(
      *client, "workspace.checkpoint", QJsonObject{{"workspaceId", workspaceId}});
    REQUIRE(checkpointed.contains("result"));
    CHECK(
      checkpointed.value("result").toObject().value("checkpointGeneration").toInteger()
      == 1);

    const auto closed =
      sendRequest(*client, "workspace.close", QJsonObject{{"workspaceId", workspaceId}});
    REQUIRE(closed.contains("result"));
    CHECK(closed.value("result").toObject().value("runtimeStatus") == "dormant");
    CHECK(
      closed.value("result").toObject().value("branchDocumentId").toString().isEmpty());

    const auto recovered = sendRequest(
      *client, "workspace.recover", QJsonObject{{"workspaceId", workspaceId}});
    REQUIRE(recovered.contains("result"));
    CHECK_FALSE(recovered.value("result")
                  .toObject()
                  .value("branchDocumentId")
                  .toString()
                  .isEmpty());
    CHECK(recovered.value("result").toObject().value("runtimeStatus") == "attached");

    const auto attached = sendRequest(
      *client,
      "workspace.attachSource",
      QJsonObject{{"workspaceId", workspaceId}, {"documentId", sourceDocumentId}});
    REQUIRE(attached.contains("result"));
    CHECK(
      attached.value("result").toObject().value("sourceDocumentId") == sourceDocumentId);

    const auto invalidRename = sendRequest(
      *client,
      "workspace.rename",
      QJsonObject{{"workspaceId", workspaceId}, {"name", "   "}});
    REQUIRE(invalidRename.contains("error"));
    CHECK(invalidRename.value("error").toObject().value("code") == -32602);

    const auto renamed = sendRequest(
      *client,
      "workspace.rename",
      QJsonObject{{"workspaceId", workspaceId}, {"name", "RPC lifecycle repaired"}});
    REQUIRE(renamed.contains("result"));
    CHECK(renamed.value("result").toObject().value("name") == "RPC lifecycle repaired");
    CHECK(
      renamed.value("result").toObject().value("checkpointGeneration").toInteger() == 1);

    const auto abandoned = sendRequest(
      *client, "workspace.abandon", QJsonObject{{"workspaceId", workspaceId}});
    REQUIRE(abandoned.contains("result"));
    CHECK(abandoned.value("result").toObject().value("state") == "abandoned");
    CHECK(
      abandoned.value("result").toObject().value("checkpointGeneration").toInteger()
      == 1);

    const auto repeatedAbandon = sendRequest(
      *client, "workspace.abandon", QJsonObject{{"workspaceId", workspaceId}});
    REQUIRE(repeatedAbandon.contains("error"));
    CHECK(repeatedAbandon.value("error").toObject().value("code") == -32020);
    const auto rejectedCheckpoint = sendRequest(
      *client, "workspace.checkpoint", QJsonObject{{"workspaceId", workspaceId}});
    REQUIRE(rejectedCheckpoint.contains("error"));
    CHECK(rejectedCheckpoint.value("error").toObject().value("code") == -32020);
    const auto status =
      sendRequest(*client, "workspace.status", QJsonObject{{"workspaceId", workspaceId}});
    REQUIRE(status.contains("result"));
    CHECK(status.value("result").toObject().value("state") == "abandoned");
    CHECK(
      status.value("result").toObject().value("checkpointGeneration").toInteger() == 1);

    const auto finalClose =
      sendRequest(*client, "workspace.close", QJsonObject{{"workspaceId", workspaceId}});
    REQUIRE(finalClose.contains("result"));
    client.reset();
  }

  sourceWindow->close();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  std::filesystem::remove(sourcePath);
  std::filesystem::remove_all(workspaceDirectory);
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
  const auto foregroundDocumentId = documents[1].toObject().value("id").toString();
  REQUIRE_FALSE(foregroundDocumentId.isEmpty());

  const auto cameraRequest = automation::AutomationRenderRequest{
    {automation::AutomationProjection::Perspective,
     {128.0, 64.0, 32.0},
     {1.0, 0.0, 0.0},
     {0.0, 0.0, 1.0},
     75.0,
     std::nullopt,
     1.0,
     65536.0},
    {64, 48},
    automation::AutomationRenderMode::Textured,
    {},
    {},
    std::nullopt};
  const auto cameraCreated = sendRequest(
    *client,
    "cameras.create",
    QJsonObject{
      {"documentId", targetDocumentId},
      {"request", automation::renderRequestToJson(cameraRequest)},
    });
  REQUIRE(cameraCreated.contains("result"));
  const auto cameraId =
    cameraCreated.value("result").toObject().value("cameraId").toString();
  REQUIRE_FALSE(cameraId.isEmpty());
  const auto cameraRead =
    sendRequest(*client, "cameras.get", QJsonObject{{"cameraId", cameraId}});
  REQUIRE(cameraRead.contains("result"));
  CHECK(cameraRead.value("result").toObject().value("documentId") == targetDocumentId);

  const auto targetViewsResponse =
    sendRequest(*client, "views.list", QJsonObject{{"documentId", targetDocumentId}});
  REQUIRE(targetViewsResponse.contains("result"));
  const auto targetViews = targetViewsResponse.value("result").toArray();
  REQUIRE_FALSE(targetViews.isEmpty());
  const auto targetViewId = targetViews.at(0).toObject().value("viewId").toString();
  REQUIRE_FALSE(targetViewId.isEmpty());

  const auto foregroundViewsResponse =
    sendRequest(*client, "views.list", QJsonObject{{"documentId", foregroundDocumentId}});
  REQUIRE(foregroundViewsResponse.contains("result"));
  const auto foregroundViews = foregroundViewsResponse.value("result").toArray();
  REQUIRE_FALSE(foregroundViews.isEmpty());
  const auto foregroundViewId =
    foregroundViews.at(0).toObject().value("viewId").toString();
  REQUIRE_FALSE(foregroundViewId.isEmpty());

  // QApplication broadcasts focusChanged to every MapWindow. Simulate the other
  // document coming to the foreground before explicitly capturing the target.
  targetWindow->focusChange(targetView, foregroundView);
  CHECK(targetWindow->currentMapViewBase() == targetView);

  const auto captureResponse = sendRequest(
    *client,
    "context.capture",
    QJsonObject{
      {"documentId", targetDocumentId},
      {"viewId", targetViewId},
      {"screenshot", false},
    });
  REQUIRE(captureResponse.contains("result"));
  const auto capture = captureResponse.value("result").toObject();
  CHECK(capture.value("documentId") == targetDocumentId);
  CHECK(capture.value("viewId") == targetViewId);
  CHECK(capture.value("currentMaterial") == "target/material");

  const auto wrongOwnerResponse = sendRequest(
    *client,
    "context.capture",
    QJsonObject{
      {"documentId", targetDocumentId},
      {"viewId", foregroundViewId},
      {"screenshot", false},
    });
  CHECK(wrongOwnerResponse.contains("error"));

  const auto implicitViewResponse =
    sendRequest(*client, "context.capture", QJsonObject{{"screenshot", false}});
  CHECK(implicitViewResponse.contains("error"));

  const auto cameraSetResponse = sendRequest(
    *client,
    "view.camera.set",
    QJsonObject{
      {"documentId", targetDocumentId},
      {"viewId", targetViewId},
      {"position", QJsonArray{128.0, 64.0, 32.0}},
      {"direction", QJsonArray{1.0, 0.0, 0.0}},
      {"up", QJsonArray{0.0, 0.0, 1.0}},
    });
  REQUIRE(cameraSetResponse.contains("result"));
  const auto cameraSet = cameraSetResponse.value("result").toObject();
  CHECK(cameraSet.value("documentId") == targetDocumentId);
  CHECK(cameraSet.value("viewId") == targetViewId);

  const auto frameResponse = sendRequest(
    *client,
    "view.frame",
    QJsonObject{
      {"documentId", targetDocumentId},
      {"viewId", targetViewId},
      {"bounds",
       QJsonObject{
         {"min", QJsonArray{-16.0, -16.0, -16.0}},
         {"max", QJsonArray{16.0, 16.0, 16.0}}}},
    });
  REQUIRE(frameResponse.contains("result"));
  CHECK(frameResponse.value("result").toObject().value("viewId") == targetViewId);

  const auto pickResponse = sendRequest(
    *client,
    "view.pick",
    QJsonObject{
      {"documentId", targetDocumentId},
      {"viewId", targetViewId},
      {"x", 0.0},
      {"y", 0.0},
    });
  REQUIRE(pickResponse.contains("result"));
  CHECK(pickResponse.value("result").toObject().value("documentId") == targetDocumentId);
  CHECK(pickResponse.value("result").toObject().value("viewId") == targetViewId);

  targetWindow->close();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  const auto staleDocumentResponse =
    sendRequest(*client, "views.list", QJsonObject{{"documentId", targetDocumentId}});
  CHECK(staleDocumentResponse.contains("error"));
  const auto staleCameraResponse =
    sendRequest(*client, "cameras.get", QJsonObject{{"cameraId", cameraId}});
  CHECK(staleCameraResponse.contains("error"));

  client.reset();
  foregroundWindow->close();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_CASE("AutomationService discovery records are isolated per service instance")
{
  auto fixture = AppControllerFixture{};
  auto first = std::make_unique<AutomationService>(fixture.appController());
  auto second = std::make_unique<AutomationService>(fixture.appController());
  REQUIRE(first->isListening());
  REQUIRE(second->isListening());
  REQUIRE_FALSE(first->discoveryPath().empty());
  REQUIRE_FALSE(second->discoveryPath().empty());
  CHECK(first->discoveryPath() != second->discoveryPath());
  CHECK(QFile::exists(pathAsQString(first->discoveryPath())));
  CHECK(QFile::exists(pathAsQString(second->discoveryPath())));

  first.reset();
  CHECK(QFile::exists(pathAsQString(second->discoveryPath())));
}

TEST_CASE("AutomationService acceptance RPC requires an explicit project")
{
  auto fixture = AppControllerFixture{};
  auto service = AutomationService{fixture.appController()};
  REQUIRE(service.isListening());
  auto client = connectClient(service.serverName());

  const auto missingProject = sendRequest(*client, "acceptance.views.list", {});
  REQUIRE(missingProject.contains("error"));
  CHECK(missingProject.value("error").toObject().value("code") == -32602);

  auto directory = QTemporaryDir{};
  REQUIRE(directory.isValid());
  const auto projectPath = directory.filePath("acceptance.json");
  const auto listed = sendRequest(
    *client, "acceptance.views.list", QJsonObject{{"projectPath", projectPath}});
  REQUIRE(listed.contains("result"));
  const auto result = listed.value("result").toObject();
  CHECK(result.value("projectPath") == projectPath);
  CHECK(result.value("revision") == 0);
  CHECK(result.value("items").toArray().isEmpty());

  const auto context = QJsonObject{
    {"id", "unrest-rebuild"},
    {"name", "Unrest rebuild"},
    {"reference", QJsonObject{{"documentPath", "unrest.map"}}},
    {"candidate", QJsonObject{{"documentPath", "unrest_rebuilt.map"}}},
    {"alignment", QJsonObject{{"type", "identity"}}},
  };
  const auto createdContext = sendRequest(
    *client,
    "acceptance.contexts.create",
    QJsonObject{
      {"projectPath", projectPath},
      {"expectedRevision", 0},
      {"context", context},
    });
  REQUIRE(createdContext.contains("result"));
  CHECK(createdContext.value("result").toObject().value("revision") == 1);

  const auto listedContexts = sendRequest(
    *client, "acceptance.contexts.list", QJsonObject{{"projectPath", projectPath}});
  REQUIRE(listedContexts.contains("result"));
  const auto contexts =
    listedContexts.value("result").toObject().value("items").toArray();
  REQUIRE(contexts.size() == 1);
  CHECK(contexts.at(0).toObject().value("id") == "unrest-rebuild");

  const auto unknown =
    sendRequest(*client, "acceptance.unknown", QJsonObject{{"projectPath", projectPath}});
  REQUIRE(unknown.contains("error"));
  CHECK(unknown.value("error").toObject().value("code") == -32601);
}

TEST_CASE(
  "AutomationService evaluates a one-shot assertion only for an explicit document")
{
  auto fixture = AppControllerFixture{};
  auto& appController = fixture.appController();
  REQUIRE(appController.mapWindowManager()
            .createDocument(
              mdl::QuakeGameInfo, mdl::MapFormat::Valve, vm::bbox3d{8192.0}, false)
            .is_success());
  auto service = AutomationService{appController};
  auto client = connectClient(service.serverName());
  const auto documents =
    sendRequest(*client, "documents.list", {}).value("result").toArray();
  REQUIRE(documents.size() == 1);
  const auto document = documents.at(0).toObject();
  const auto documentId = document.value("id").toString();
  REQUIRE_FALSE(documentId.isEmpty());

  auto directory = QTemporaryDir{};
  REQUIRE(directory.isValid());
  const auto assertion = QJsonObject{
    {"id", "clear"},
    {"type", "clearSightline"},
    {"configuration",
     QJsonObject{
       {"origin", QJsonArray{0.0, 0.0, 0.0}},
       {"target", QJsonArray{0.0, 64.0, 0.0}},
     }},
  };
  const auto identity = QJsonObject{
    {"path", document.value("path")},
    {"documentId", documentId},
    {"revision", document.value("revision")},
  };
  const auto projectPath = directory.filePath("acceptance.json");
  const auto evaluated = sendRequest(
    *client,
    "acceptance.assertions.evaluate",
    QJsonObject{
      {"projectPath", projectPath},
      {"document", identity},
      {"assertion", assertion},
      {"context", QJsonObject{{"geometrySpace", "reference"}}},
    });
  REQUIRE(evaluated.contains("result"));
  const auto result = evaluated.value("result").toObject();
  CHECK(result.value("status") == "passed");
  CHECK(result.value("document").toObject().value("documentId") == documentId);
  CHECK(result.value("report").toObject().value("clearRays") == 1);

  const auto missingId = sendRequest(
    *client,
    "acceptance.assertions.evaluate",
    QJsonObject{
      {"projectPath", projectPath},
      {"document", QJsonObject{}},
      {"assertion", assertion}});
  REQUIRE(missingId.contains("error"));
  CHECK(missingId.value("error").toObject().value("code") == -32602);

  auto unknownIdentity = identity;
  unknownIdentity.insert("documentId", "does-not-exist");
  const auto unknown = sendRequest(
    *client,
    "acceptance.assertions.evaluate",
    QJsonObject{
      {"projectPath", projectPath},
      {"document", unknownIdentity},
      {"assertion", assertion}});
  REQUIRE(unknown.contains("result"));
  CHECK(unknown.value("result").toObject().value("status") == "error");
}

TEST_CASE("AutomationService headless acceptance capture RPC")
{
  auto fixture = AppControllerFixture{initializeQuakeGame};
  auto& appController = fixture.appController();
  auto directory = QTemporaryDir{};
  REQUIRE(directory.isValid());
  const auto sourcePath = pathFromQString(directory.filePath("source.map"));
  const auto targetPath = pathFromQString(directory.filePath("target.map"));
  const auto projectPath = pathFromQString(directory.filePath("acceptance.json"));

  {
    auto document = MapDocument::createDocument(
                      appController.environmentConfig(),
                      mdl::QuakeGameInfo,
                      mdl::MapFormat::Valve,
                      vm::bbox3d{8192.0},
                      appController.taskManager(),
                      appController.glManager().resourceManager())
                    | kdl::value();
    REQUIRE(document->map().saveAs(sourcePath).is_success());
  }
  auto copyError = std::error_code{};
  std::filesystem::copy_file(sourcePath, targetPath, {}, copyError);
  REQUIRE_FALSE(copyError);

  auto project = AcceptanceProject{};
  project.views = {
    {"source", "Source", {}, {64, 48}, "textured", {}},
    {"target", "Target", {}, {64, 48}, "textured", {}},
  };
  project.comparisons = {{
    "comparison",
    "Comparison",
    {"source.map", "source"},
    {"target.map", "target"},
    {},
    {},
    {{"pixels", AcceptanceMetricType::Silhouette, std::nullopt, {}}},
    {},
    "headless-pair",
  }};
  project.contexts = {{
    "headless-pair",
    "Headless reference and candidate",
    "source.map",
    "target.map",
    {},
  }};
  const auto projectJson = acceptanceProjectToJson(project);

  const auto* focusBefore = QApplication::focusWidget();
  const auto windowCountBefore = appController.mapWindowManager().mapWindows().size();
  auto service = AutomationService{appController};
  auto client = connectClient(service.serverName());
  const auto pathParam = pathAsQString(projectPath);

  auto revision = 0;
  const auto createdContext = sendRequest(
    *client,
    "acceptance.contexts.create",
    QJsonObject{
      {"projectPath", pathParam},
      {"expectedRevision", revision++},
      {"context", projectJson.value("contexts").toArray().at(0)},
    });
  REQUIRE(createdContext.contains("result"));
  for (const auto& view : projectJson.value("views").toArray())
  {
    const auto response = sendRequest(
      *client,
      "acceptance.views.create",
      QJsonObject{
        {"projectPath", pathParam},
        {"expectedRevision", revision++},
        {"view", view},
      });
    REQUIRE(response.contains("result"));
  }
  const auto createdComparison = sendRequest(
    *client,
    "acceptance.comparisons.create",
    QJsonObject{
      {"projectPath", pathParam},
      {"expectedRevision", revision},
      {"comparison", projectJson.value("comparisons").toArray().at(0)},
    });
  REQUIRE(createdComparison.contains("result"));

  const auto captured = sendRequest(
    *client,
    "acceptance.capture",
    QJsonObject{{"projectPath", pathParam}, {"comparisonId", "comparison"}});
  REQUIRE(captured.contains("result"));
  const auto capture = captured.value("result").toObject().value("capture").toObject();
  CHECK(capture.value("comparisonId") == "comparison");
  const auto imageComparison = capture.value("imageComparison").toObject();
  CHECK(imageComparison.value("passed") == true);
  const auto metricReports = imageComparison.value("metrics").toArray();
  REQUIRE(metricReports.size() == 1);
  CHECK(metricReports.at(0).toObject().value("passed") == true);

  const auto geometryComparison = sendRequest(
    *client,
    "acceptance.geometry.compare",
    QJsonObject{
      {"projectPath", pathParam},
      {"contextId", "headless-pair"},
      {"bounds",
       QJsonObject{
         {"min", QJsonArray{-1.0, -1.0, -1.0}},
         {"max", QJsonArray{1.0, 1.0, 1.0}},
       }},
      {"cellSize", 1.0},
    });
  REQUIRE(geometryComparison.contains("result"));
  const auto geometryReport =
    geometryComparison.value("result").toObject().value("comparison").toObject();
  CHECK(geometryReport.value("occupancyModel") == "brushVolumesV1");
  CHECK(geometryReport.value("referenceDocument")
          .toObject()
          .value("id")
          .toString()
          .startsWith("hidden-"));
  CHECK(geometryReport.value("candidateDocument")
          .toObject()
          .value("id")
          .toString()
          .startsWith("hidden-"));
  CHECK(
    geometryReport.value("referenceDocument").toObject().value("id")
    != geometryReport.value("candidateDocument").toObject().value("id"));
  CHECK(geometryReport.value("totalCells") == 8);
  CHECK(geometryReport.value("referenceOnly").toObject().value("cellCount") == 0);
  CHECK(geometryReport.value("candidateOnly").toObject().value("cellCount") == 0);
  CHECK(capture.value("reference")
          .toObject()
          .value("document")
          .toObject()
          .value("documentId")
          .toString()
          .startsWith("hidden-"));
  CHECK(QApplication::focusWidget() == focusBefore);
  CHECK(appController.mapWindowManager().mapWindows().size() == windowCountBefore);
}

} // namespace tb::ui
