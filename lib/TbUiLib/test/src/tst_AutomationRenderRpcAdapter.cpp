/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QApplication>
#include <QFile>
#include <QJsonArray>

#include "mdl/BrushNode.h"
#include "mdl/GameConfigFixture.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "mdl/Map_Nodes.h"
#include "mdl/TestFactory.h"
#include "ui/AppControllerFixture.h"
#include "ui/AutomationDocumentRegistry.h"
#include "ui/AutomationRenderRpcAdapter.h"
#include "ui/CatchConfig.h"
#include "ui/LocalJsonRpcServer.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"
#include "ui/MapWindowManager.h"
#include "ui/QPathUtils.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

QJsonObject renderParams(const QString& documentId)
{
  return {
    {"documentId", documentId},
    {"camera",
     QJsonObject{
       {"projection", "perspective"},
       {"position", QJsonArray{-128.0, 0.0, 0.0}},
       {"direction", QJsonArray{1.0, 0.0, 0.0}},
       {"up", QJsonArray{0.0, 0.0, 1.0}},
       {"verticalFov", 90.0},
       {"near", 1.0},
       {"far", 65536.0},
     }},
    {"size", QJsonArray{64, 48}},
    {"renderMode", "textured"},
    {"overlays",
     QJsonObject{{"brushEdges", false}, {"selection", false}, {"grid", false}}},
  };
}

QJsonObject pickParams(const QString& documentId, const double x, const double y)
{
  auto params = renderParams(documentId);
  params.insert("x", x);
  params.insert("y", y);
  return params;
}

} // namespace

TEST_CASE("Automation render RPC adapter")
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

  auto documents = AutomationDocumentRegistry{};
  const auto targetDocumentId = documents.registerDocument(*windows[0]);
  const auto foregroundDocumentId = documents.registerDocument(*windows[1]);
  auto adapter = AutomationRenderRpcAdapter{appController};
  const auto* focusBefore = QApplication::focusWidget();
  const auto targetRevision = windows[0]->document().map().modificationCount();

  SECTION("requires an explicit, live document identity")
  {
    auto missing = renderParams({});
    CHECK_FALSE(adapter.handle("render.context", missing, documents).isSuccess());

    auto unknown = renderParams("document-stale");
    CHECK_FALSE(adapter.handle("render.context", unknown, documents).isSuccess());
  }

  SECTION("returns context for the explicit document instead of the foreground document")
  {
    const auto response =
      adapter.handle("render.context", renderParams(targetDocumentId), documents);

    REQUIRE(response.isSuccess());
    const auto result = response.result().toObject();
    CHECK(result.value("documentId") == targetDocumentId);
    CHECK(result.value("revision") == static_cast<qint64>(targetRevision));
    CHECK(
      result.value("camera").toObject().value("position").toArray()
      == QJsonArray{-128.0, 0.0, 0.0});
    CHECK(result.value("imagePath").isUndefined());
    CHECK(result.value("documentId") != foregroundDocumentId);
  }

  SECTION("captures a unique offscreen image for the explicit document")
  {
    const auto response =
      adapter.handle("render.capture", renderParams(targetDocumentId), documents);

    REQUIRE(response.isSuccess());
    const auto result = response.result().toObject();
    const auto imagePath = pathFromQString(result.value("imagePath").toString());
    REQUIRE_FALSE(imagePath.empty());
    CHECK(QFile::exists(pathAsQString(imagePath)));
    CHECK(result.value("documentId") == targetDocumentId);
    CHECK(result.value("revision") == static_cast<qint64>(targetRevision));
    CHECK(QApplication::focusWidget() == focusBefore);
    CHECK(windows[0]->document().map().modificationCount() == targetRevision);
    QFile::remove(pathAsQString(imagePath));
  }

  SECTION("picks the explicit document without changing foreground focus")
  {
    auto& targetMap = windows[0]->document().map();
    auto* targetBrush = mdl::createBrushNode(targetMap);
    mdl::addNodes(targetMap, {{&mdl::parentForNodes(targetMap), {targetBrush}}});
    const auto pickRevision = targetMap.modificationCount();
    const auto response =
      adapter.handle("render.pick", pickParams(targetDocumentId, 32.0, 24.0), documents);

    REQUIRE(response.isSuccess());
    const auto result = response.result().toObject();
    const auto hits = result.value("hits").toArray();
    REQUIRE_FALSE(hits.isEmpty());
    const auto firstHit = hits.at(0).toObject();
    CHECK(result.value("documentId") == targetDocumentId);
    CHECK(result.value("revision") == static_cast<qint64>(pickRevision));
    CHECK(
      firstHit.value("node").toObject().value("name").toString()
      == QString::fromStdString(targetBrush->name()));
    CHECK(firstHit.contains("faceIndex"));
    CHECK(QApplication::focusWidget() == focusBefore);
    CHECK(targetMap.modificationCount() == pickRevision);
  }

  SECTION("rejects invalid virtual pick pixels")
  {
    const auto response =
      adapter.handle("render.pick", pickParams(targetDocumentId, 64.0, 24.0), documents);

    CHECK_FALSE(response.isSuccess());
  }
}

} // namespace tb::ui
