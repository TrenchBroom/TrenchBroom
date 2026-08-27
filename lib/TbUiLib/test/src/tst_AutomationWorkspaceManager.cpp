/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QSaveFile>

#include "base/PreferenceManager.h"
#include "fs/TestEnvironment.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GameConfigFixture.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "mdl/Map_Nodes.h"
#include "mdl/WorldNode.h"
#include "prefs/Preferences.h"
#include "ui/AppControllerFixture.h"
#include "ui/AutomationWorkspaceManager.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"
#include "ui/MapWindowManager.h"
#include "ui/QPathUtils.h"

#include <ranges>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

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

bool setWorldMessage(mdl::Map& map, const std::string& message)
{
  auto world = map.worldNode().entity();
  world.addOrUpdateProperty("message", message);
  return mdl::updateNodeContents(
    map,
    "Set Workspace Message",
    {{&map.worldNode(), mdl::NodeContents{std::move(world)}}});
}

size_t visibleMapWindowCount(const MapWindowManager& windowManager)
{
  return static_cast<size_t>(std::ranges::count_if(
    windowManager.mapWindows(), [](const auto* window) { return window->isVisible(); }));
}

bool removeMapMetadata(const std::filesystem::path& manifestPath)
{
  auto input = QFile{pathAsQString(manifestPath)};
  if (!input.open(QIODevice::ReadOnly))
  {
    return false;
  }
  auto parseError = QJsonParseError{};
  auto document = QJsonDocument::fromJson(input.readAll(), &parseError);
  if (parseError.error != QJsonParseError::NoError || !document.isObject())
  {
    return false;
  }
  auto json = document.object();
  json.remove("mapMetadata");

  auto output = QSaveFile{pathAsQString(manifestPath)};
  return output.open(QIODevice::WriteOnly)
         && output.write(QJsonDocument{json}.toJson(QJsonDocument::Indented)) >= 0
         && output.commit();
}

} // namespace

TEST_CASE("AutomationWorkspaceManager durable lifecycle")
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
  auto environment = fs::TestEnvironment{};
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
    / ("trenchbroom-automation-workspace-"
       + std::to_string(QCoreApplication::applicationPid()) + ".map");
  REQUIRE(sourceWindow->document().map().saveAs(sourcePath).is_success());

  const auto workspaceRoot = environment.dir() / "workspaces";
  auto manager = AutomationWorkspaceManager{appController, workspaceRoot};
  const auto* focusBeforeFork = QApplication::focusWidget();
  const auto visibleWindowsBeforeFork = visibleMapWindowCount(windowManager);
  const auto forked = manager.fork(*sourceWindow, "north vault");
  REQUIRE(forked);
  const auto workspaceId = forked.workspace->id;
  REQUIRE(forked.workspace->sourceWindow == sourceWindow);
  REQUIRE(forked.workspace->branchWindow);
  CHECK_FALSE(forked.workspace->branchWindow->isVisible());
  CHECK(QApplication::focusWidget() == focusBeforeFork);
  CHECK(visibleMapWindowCount(windowManager) == visibleWindowsBeforeFork);
  CHECK(forked.workspace->runtimeStatus == AutomationWorkspaceRuntimeStatus::Attached);
  CHECK_FALSE(forked.workspace->manifest.nodeIdentities.empty());
  CHECK(forked.workspace->manifest.checkpointGeneration == 0u);

  auto& branchMap = forked.workspace->branchWindow->document().map();
  REQUIRE(setWorldMessage(branchMap, "workspace branch change"));
  CHECK(branchMap.modified());

  const auto checkpointed = manager.checkpoint(workspaceId);
  REQUIRE(checkpointed);
  CHECK(checkpointed.workspace->manifest.checkpointGeneration == 1u);

  const auto closed = manager.close(workspaceId);
  REQUIRE(closed);
  CHECK_FALSE(closed.workspace->sourceWindow);
  CHECK_FALSE(closed.workspace->branchWindow);
  CHECK(closed.workspace->runtimeStatus == AutomationWorkspaceRuntimeStatus::Dormant);
  CHECK(visibleMapWindowCount(windowManager) == visibleWindowsBeforeFork);
  CHECK(QApplication::activeModalWidget() == nullptr);
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

  auto recoveredManager = AutomationWorkspaceManager{appController, workspaceRoot};
  const auto discovered = recoveredManager.workspaces();
  REQUIRE(discovered.size() == 1u);
  CHECK(discovered.front()->id == workspaceId);
  CHECK(discovered.front()->runtimeStatus == AutomationWorkspaceRuntimeStatus::Dormant);
  CHECK(discovered.front()->manifest.checkpointGeneration == 1u);

  const auto recovered = recoveredManager.recover(workspaceId);
  REQUIRE(recovered);
  REQUIRE(recovered.workspace->branchWindow);
  CHECK_FALSE(recovered.workspace->branchWindow->isVisible());
  CHECK(QApplication::focusWidget() == focusBeforeFork);
  CHECK_FALSE(recovered.workspace->sourceWindow);
  CHECK_FALSE(recovered.workspace->branchWindow->document().map().modified());
  CHECK(visibleMapWindowCount(windowManager) == visibleWindowsBeforeFork);

  const auto attached = recoveredManager.attachSource(workspaceId, *sourceWindow);
  REQUIRE(attached);
  REQUIRE(attached.workspace->branchWindow);
  CHECK_FALSE(attached.workspace->branchWindow->isVisible());
  CHECK(QApplication::focusWidget() == focusBeforeFork);
  CHECK(attached.workspace->sourceWindow == sourceWindow);
  CHECK(attached.workspace->runtimeStatus == AutomationWorkspaceRuntimeStatus::Attached);
  const auto* model = recoveredManager.model(workspaceId);
  REQUIRE(model != nullptr);
  CHECK_FALSE(model->changes().empty());

  const auto renamed = recoveredManager.rename(workspaceId, "north vault repaired");
  REQUIRE(renamed);
  CHECK(renamed.workspace->manifest.name == "north vault repaired");
  CHECK(renamed.workspace->manifest.checkpointGeneration == 1u);

  const auto abandoned = recoveredManager.abandon(workspaceId);
  REQUIRE(abandoned);
  CHECK(
    abandoned.workspace->manifest.state == AutomationWorkspaceLifecycleState::Abandoned);
  CHECK(abandoned.workspace->manifest.checkpointGeneration == 1u);
  CHECK_FALSE(abandoned.workspace->branchWindow->document().map().modified());
  const auto manifestPath = workspaceRoot / workspaceId.toStdString() / "workspace.json";
  const auto beforeInvalidTransition = environment.loadFile(manifestPath);
  CHECK_FALSE(recoveredManager.abandon(workspaceId));
  CHECK_FALSE(recoveredManager.checkpoint(workspaceId));
  CHECK_FALSE(recoveredManager.attachSource(workspaceId, *sourceWindow));
  CHECK(environment.loadFile(manifestPath) == beforeInvalidTransition);

  const auto finalClose = recoveredManager.close(workspaceId);
  REQUIRE(finalClose);
  CHECK_FALSE(finalClose.workspace->branchWindow);
  CHECK(visibleMapWindowCount(windowManager) == visibleWindowsBeforeFork);
  CHECK(QApplication::activeModalWidget() == nullptr);

  const auto appliedFork = recoveredManager.fork(*sourceWindow, "applied north vault");
  REQUIRE(appliedFork);
  const auto appliedWorkspaceId = appliedFork.workspace->id;
  REQUIRE(setWorldMessage(
    appliedFork.workspace->branchWindow->document().map(), "applied workspace branch"));
  const auto applied = recoveredManager.merge(appliedWorkspaceId, true);
  REQUIRE(applied.applied);
  CHECK(applied.plan.conflicts.empty());
  const auto* appliedWorkspace = recoveredManager.find(appliedWorkspaceId);
  REQUIRE(appliedWorkspace != nullptr);
  CHECK(appliedWorkspace->manifest.state == AutomationWorkspaceLifecycleState::Merged);
  REQUIRE(sourceWindow->document().map().worldNode().entity().property("message"));
  CHECK(
    *sourceWindow->document().map().worldNode().entity().property("message")
    == "applied workspace branch");
  CHECK_FALSE(appliedWorkspace->branchWindow->document().map().modified());
  const auto appliedClose = recoveredManager.close(appliedWorkspaceId);
  REQUIRE(appliedClose);
  CHECK_FALSE(appliedClose.workspace->branchWindow);
  CHECK(QApplication::activeModalWidget() == nullptr);
  REQUIRE(sourceWindow->document().map().saveAs(sourcePath).is_success());

  const auto workspaceDirectory = workspaceRoot / workspaceId.toStdString();
  REQUIRE(removeMapMetadata(workspaceDirectory / "workspace.json"));
  REQUIRE(removeMapMetadata(workspaceDirectory / "snapshots" / "1" / "workspace.json"));
  auto legacyManager = AutomationWorkspaceManager{appController, workspaceRoot};
  const auto legacyWithoutContext = legacyManager.recover(workspaceId);
  CHECK_FALSE(legacyWithoutContext);
  CHECK(
    QString::fromStdString(legacyWithoutContext.error)
      .contains("requires documentId context"));
  CHECK(visibleMapWindowCount(windowManager) == visibleWindowsBeforeFork);
  const auto legacyRecovered = legacyManager.recover(workspaceId, *sourceWindow);
  REQUIRE(legacyRecovered);
  REQUIRE(legacyRecovered.workspace->branchWindow);
  CHECK_FALSE(legacyRecovered.workspace->branchWindow->isVisible());
  CHECK_FALSE(legacyRecovered.workspace->branchWindow->document().map().modified());
  const auto legacyClose = legacyManager.close(workspaceId);
  REQUIRE(legacyClose);
  CHECK_FALSE(legacyClose.workspace->branchWindow);
  CHECK(QApplication::activeModalWidget() == nullptr);
  CHECK(std::filesystem::is_regular_file(workspaceDirectory / "workspace.json"));
  CHECK(
    std::filesystem::is_regular_file(
      workspaceDirectory / "snapshots" / "1" / "branch.map"));

  sourceWindow->close();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  CHECK(QApplication::activeModalWidget() == nullptr);
  std::filesystem::remove(sourcePath);
}

TEST_CASE("AutomationWorkspaceManager destructor closes hidden branches without prompts")
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
  auto environment = fs::TestEnvironment{};
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
    / ("trenchbroom-automation-workspace-destructor-"
       + std::to_string(QCoreApplication::applicationPid()) + ".map");
  REQUIRE(sourceWindow->document().map().saveAs(sourcePath).is_success());
  const auto workspaceRoot = environment.dir() / "workspaces";
  const auto windowCountBeforeFork = windowManager.mapWindows().size();
  const auto visibleWindowsBeforeFork = visibleMapWindowCount(windowManager);

  auto durableWorkspaceId = QString{};
  {
    auto manager = AutomationWorkspaceManager{appController, workspaceRoot};
    const auto forked = manager.fork(*sourceWindow, "destructor checkpoint");
    REQUIRE(forked);
    durableWorkspaceId = forked.workspace->id;
    REQUIRE(forked.workspace->branchWindow);
    CHECK_FALSE(forked.workspace->branchWindow->isVisible());
    REQUIRE(setWorldMessage(
      forked.workspace->branchWindow->document().map(), "destructor checkpoint"));
    CHECK(forked.workspace->branchWindow->document().map().modified());
  }
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  CHECK(windowManager.mapWindows().size() == windowCountBeforeFork);
  CHECK(visibleMapWindowCount(windowManager) == visibleWindowsBeforeFork);
  CHECK(QApplication::activeModalWidget() == nullptr);

  auto recoveredManager = AutomationWorkspaceManager{appController, workspaceRoot};
  const auto* durableWorkspace = recoveredManager.find(durableWorkspaceId);
  REQUIRE(durableWorkspace != nullptr);
  CHECK(durableWorkspace->manifest.checkpointGeneration == 1u);

  {
    auto manager = AutomationWorkspaceManager{appController, workspaceRoot};
    const auto forked = manager.fork(*sourceWindow, "destructor discard fallback");
    REQUIRE(forked);
    REQUIRE(forked.workspace->branchWindow);
    REQUIRE(setWorldMessage(
      forked.workspace->branchWindow->document().map(), "destructor discard fallback"));
    const auto workspaceDirectory = workspaceRoot / forked.workspace->id.toStdString();
    auto errorCode = std::error_code{};
    std::filesystem::remove_all(workspaceDirectory, errorCode);
    REQUIRE_FALSE(errorCode);
  }
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  CHECK(windowManager.mapWindows().size() == windowCountBeforeFork);
  CHECK(visibleMapWindowCount(windowManager) == visibleWindowsBeforeFork);
  CHECK(QApplication::activeModalWidget() == nullptr);

  {
    auto manager = AutomationWorkspaceManager{appController, workspaceRoot};
    const auto forked = manager.fork(*sourceWindow, "close discard fallback");
    REQUIRE(forked);
    const auto workspaceId = forked.workspace->id;
    REQUIRE(forked.workspace->branchWindow);
    REQUIRE(setWorldMessage(
      forked.workspace->branchWindow->document().map(), "close discard fallback"));
    const auto workspaceDirectory = workspaceRoot / workspaceId.toStdString();
    auto errorCode = std::error_code{};
    std::filesystem::remove_all(workspaceDirectory, errorCode);
    REQUIRE_FALSE(errorCode);

    const auto closed = manager.close(workspaceId);
    CHECK_FALSE(closed);
    CHECK(
      QString::fromStdString(closed.error)
        .contains("discarded unsaved hidden branch changes"));
    CHECK_FALSE(forked.workspace->branchWindow);
    CHECK(QApplication::activeModalWidget() == nullptr);
  }
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  CHECK(windowManager.mapWindows().size() == windowCountBeforeFork);
  CHECK(visibleMapWindowCount(windowManager) == visibleWindowsBeforeFork);
  CHECK(QApplication::activeModalWidget() == nullptr);

  sourceWindow->close();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  CHECK(QApplication::activeModalWidget() == nullptr);
  std::filesystem::remove(sourcePath);
}

} // namespace tb::ui
