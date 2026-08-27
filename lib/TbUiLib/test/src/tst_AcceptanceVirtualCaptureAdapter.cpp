/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QApplication>
#include <QTemporaryDir>

#include "mdl/GameConfigFixture.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "ui/AcceptanceVirtualCaptureAdapter.h"
#include "ui/AppControllerFixture.h"
#include "ui/AutomationDocumentRegistry.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"
#include "ui/MapWindowManager.h"
#include "ui/QPathUtils.h"

#include <filesystem>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

AcceptanceVirtualCaptureRequest requestFor(const std::filesystem::path& path)
{
  return {
    path,
    AcceptanceCamera{
      AcceptanceProjection::Perspective,
      {0.0, -128.0, 64.0},
      {0.0, 1.0, 0.0},
      {0.0, 0.0, 1.0},
      90.0,
      1.0,
      65536.0,
      std::nullopt},
    {64, 48},
    "textured",
    {false, false, false}};
}

} // namespace

TEST_CASE("Acceptance virtual capture adapter")
{
  auto fixture = AppControllerFixture{[](const auto& writeGameConfig) {
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
  }};
  auto& appController = fixture.appController();
  auto& windowManager = appController.mapWindowManager();
  REQUIRE(windowManager
            .createDocument(
              mdl::QuakeGameInfo, mdl::MapFormat::Valve, vm::bbox3d{8192.0}, false)
            .is_success());
  auto* window = windowManager.topMapWindow();
  REQUIRE(window != nullptr);

  auto directory = QTemporaryDir{};
  REQUIRE(directory.isValid());
  const auto livePath = pathFromQString(directory.filePath("live.map"));
  const auto hiddenPath = pathFromQString(directory.filePath("hidden.map"));
  REQUIRE(window->document().map().saveAs(livePath).is_success());
  auto errorCode = std::error_code{};
  std::filesystem::copy_file(
    livePath, hiddenPath, std::filesystem::copy_options::none, errorCode);
  REQUIRE_FALSE(errorCode);

  auto documents = AutomationDocumentRegistry{};
  const auto liveDocumentId = documents.registerDocument(*window);
  auto adapter = AcceptanceVirtualCaptureAdapter{appController, documents};
  const auto* focusBefore = QApplication::focusWidget();
  const auto* topWindowBefore = windowManager.topMapWindow();
  const auto windowCountBefore = windowManager.mapWindows().size();

  CHECK(adapter.findDocument(liveDocumentId.toStdString()) == &window->document());
  CHECK(adapter.findDocument("hidden-not-captured") == nullptr);

  SECTION("reuses only the exact registered live path")
  {
    const auto result = adapter.capture(requestFor(livePath));

    REQUIRE(result.is_success());
    CHECK(result.value().document.path == livePath);
    CHECK(result.value().document.documentId == liveDocumentId.toStdString());
    CHECK(
      result.value().document.revision == window->document().map().modificationCount());
    CHECK(std::filesystem::is_regular_file(result.value().colorPath));
    CHECK(QApplication::focusWidget() == focusBefore);
    CHECK(windowManager.topMapWindow() == topWindowBefore);
    CHECK(windowManager.mapWindows().size() == windowCountBefore);
    std::filesystem::remove(result.value().colorPath);
  }

  SECTION("returns the requested EV6 depth buffer without changing UI state")
  {
    auto request = requestFor(livePath);
    request.depth = true;

    const auto result = adapter.capture(request);

    REQUIRE(result.is_success());
    REQUIRE(result.value().depthPath);
    CHECK(std::filesystem::is_regular_file(*result.value().depthPath));
    CHECK(QApplication::focusWidget() == focusBefore);
    CHECK(windowManager.topMapWindow() == topWindowBefore);
    CHECK(windowManager.mapWindows().size() == windowCountBefore);
    std::filesystem::remove(result.value().colorPath);
    std::filesystem::remove(*result.value().depthPath);
  }

  SECTION("loads a different explicit path into an adapter-owned hidden document")
  {
    const auto first = adapter.capture(requestFor(hiddenPath));
    const auto second = adapter.capture(requestFor(hiddenPath));

    const auto firstError =
      first.is_error() ? std::get<AcceptanceVirtualCaptureError>(first.error()).message
                       : std::string{};
    INFO(firstError);
    REQUIRE(first.is_success());
    const auto secondError =
      second.is_error() ? std::get<AcceptanceVirtualCaptureError>(second.error()).message
                        : std::string{};
    INFO(secondError);
    REQUIRE(second.is_success());
    CHECK(first.value().document.path == hiddenPath);
    CHECK(first.value().document.documentId.starts_with("hidden-"));
    CHECK(first.value().document.documentId == second.value().document.documentId);
    CHECK(first.value().document.revision == second.value().document.revision);
    const auto* hidden = adapter.findDocument(first.value().document.documentId);
    REQUIRE(hidden != nullptr);
    CHECK(hidden != &window->document());
    CHECK(hidden->map().path() == hiddenPath);
    CHECK(std::filesystem::is_regular_file(first.value().colorPath));
    CHECK(std::filesystem::is_regular_file(second.value().colorPath));
    CHECK(QApplication::focusWidget() == focusBefore);
    CHECK(windowManager.topMapWindow() == topWindowBefore);
    CHECK(windowManager.mapWindows().size() == windowCountBefore);
    std::filesystem::remove(first.value().colorPath);
    std::filesystem::remove(second.value().colorPath);
  }
}

} // namespace tb::ui
