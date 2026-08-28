/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "gl/GlManager.h"
#include "mdl/GameConfigFixture.h"
#include "mdl/MapFormat.h"
#include "ui/AppControllerFixture.h"
#include "ui/AutomationDocumentRegistry.h"
#include "ui/AutomationViewRegistry.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"

#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

std::unique_ptr<MapWindow> makeWindow(AppController& appController)
{
  auto document = MapDocument::createDocument(
                    appController.environmentConfig(),
                    mdl::QuakeGameInfo,
                    mdl::MapFormat::Valve,
                    vm::bbox3d{8192.0},
                    appController.taskManager(),
                    appController.glManager().resourceManager())
                  | kdl::value();
  return std::make_unique<MapWindow>(appController, std::move(document));
}

} // namespace

TEST_CASE("Automation document and view identity registries")
{
  auto fixture = AppControllerFixture{};
  auto& appController = fixture.appController();

  SECTION("view IDs are opaque, document-owned, and invalidated with their window")
  {
    auto documents = AutomationDocumentRegistry{};
    auto views = AutomationViewRegistry{};
    auto first = makeWindow(appController);
    auto second = makeWindow(appController);

    const auto firstDocumentId = documents.registerDocument(*first);
    const auto secondDocumentId = documents.registerDocument(*second);
    REQUIRE(firstDocumentId.startsWith("document-"));
    REQUIRE(secondDocumentId.startsWith("document-"));
    CHECK(firstDocumentId != secondDocumentId);

    auto* firstView = first->currentMapViewBase();
    auto* secondView = second->currentMapViewBase();
    REQUIRE(firstView != nullptr);
    REQUIRE(secondView != nullptr);
    const auto firstViewId = views.registerView(firstDocumentId, *firstView);
    const auto secondViewId = views.registerView(secondDocumentId, *secondView);
    REQUIRE(firstViewId.startsWith("view-3d-"));
    CHECK(firstViewId != secondViewId);
    CHECK(views.findView(firstDocumentId, firstViewId) == firstView);
    CHECK(views.findView(secondDocumentId, firstViewId) == nullptr);
    CHECK(views.views(firstDocumentId).size() == 1u);

    first.reset();
    CHECK(documents.findWindow(firstDocumentId) == nullptr);
    CHECK(views.findView(firstDocumentId, firstViewId) == nullptr);
    CHECK(documents.documentId(*second) == secondDocumentId);
    CHECK(views.findView(secondDocumentId, secondViewId) == secondView);
  }

  SECTION("destroyed document IDs are never minted again")
  {
    auto tokens = std::vector<QString>{"same", "same", "fresh"};
    auto nextToken = [&tokens] {
      const auto token = tokens.front();
      tokens.erase(tokens.begin());
      return token;
    };
    auto documents = AutomationDocumentRegistry{nextToken};

    auto first = makeWindow(appController);
    const auto firstId = documents.registerDocument(*first);
    CHECK(firstId == "document-same");
    first.reset();

    auto second = makeWindow(appController);
    const auto secondId = documents.registerDocument(*second);
    CHECK(secondId == "document-fresh");
    CHECK(documents.findWindow(firstId) == nullptr);
  }

  SECTION("view IDs are not rebound after a destroyed view")
  {
    auto tokens = std::vector<QString>{"same", "same", "fresh"};
    auto nextToken = [&tokens] {
      const auto token = tokens.front();
      tokens.erase(tokens.begin());
      return token;
    };
    auto views = AutomationViewRegistry{nextToken};

    auto first = makeWindow(appController);
    const auto firstId = views.registerView("document-a", *first->currentMapViewBase());
    CHECK(firstId == "view-3d-same");
    first.reset();

    auto second = makeWindow(appController);
    const auto secondId = views.registerView("document-a", *second->currentMapViewBase());
    CHECK(secondId == "view-3d-fresh");
    CHECK(views.findView("document-a", firstId) == nullptr);
  }
}

} // namespace tb::ui
