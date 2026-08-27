/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QJsonArray>
#include <QJsonObject>

#include "ui/AutomationRenderRequest.h"
#include "ui/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui::automation
{
namespace
{

QJsonObject perspectiveRequest()
{
  return {
    {"camera",
     QJsonObject{
       {"projection", "perspective"},
       {"position", QJsonArray{10.0, 20.0, 30.0}},
       {"direction", QJsonArray{0.0, 1.0, 0.0}},
       {"up", QJsonArray{0.0, 0.0, 1.0}},
       {"verticalFov", 75.0},
       {"near", 1.0},
       {"far", 65536.0},
     }},
    {"size", QJsonArray{1600, 900}},
    {"renderMode", "textured"},
    {"overlays",
     QJsonObject{{"brushEdges", true}, {"selection", false}, {"grid", false}}},
    {"outputs", QJsonObject{{"depth", false}}},
  };
}

} // namespace

TEST_CASE("Automation render request JSON")
{
  SECTION("round trips a perspective camera")
  {
    const auto request = renderRequestFromJson(perspectiveRequest());
    REQUIRE(request);
    CHECK(request->camera.projection == AutomationProjection::Perspective);
    CHECK(request->camera.direction == vm::vec3d{0.0, 1.0, 0.0});
    CHECK(request->camera.up == vm::vec3d{0.0, 0.0, 1.0});
    REQUIRE(request->camera.verticalFov);
    CHECK(*request->camera.verticalFov == 75.0);
    CHECK_FALSE(request->camera.zoom);
    CHECK(request->size.width == 1600);
    CHECK(request->size.height == 900);

    const auto json = renderRequestToJson(*request);
    CHECK(json == perspectiveRequest());
  }

  SECTION("normalizes a non-unit camera basis")
  {
    auto json = perspectiveRequest();
    auto camera = json.value("camera").toObject();
    camera.insert("direction", QJsonArray{0.0, 2.0, 0.0});
    camera.insert("up", QJsonArray{0.0, 1.0, 3.0});
    json.insert("camera", camera);

    const auto request = renderRequestFromJson(json);
    REQUIRE(request);
    CHECK(request->camera.direction == vm::vec3d{0.0, 1.0, 0.0});
    CHECK(request->camera.up == vm::vec3d{0.0, 0.0, 1.0});
  }

  SECTION("round trips an orthographic camera")
  {
    auto json = perspectiveRequest();
    auto camera = json.value("camera").toObject();
    camera.insert("projection", "orthographic");
    camera.remove("verticalFov");
    camera.insert("zoom", 4.0);
    json.insert("camera", camera);

    const auto request = renderRequestFromJson(json);
    REQUIRE(request);
    CHECK(request->camera.projection == AutomationProjection::Orthographic);
    CHECK_FALSE(request->camera.verticalFov);
    REQUIRE(request->camera.zoom);
    CHECK(*request->camera.zoom == 4.0);
    CHECK(renderRequestToJson(*request) == json);
  }

  SECTION("rejects malformed, degenerate, and unsupported requests")
  {
    auto json = perspectiveRequest();
    CHECK_FALSE(renderRequestFromJson(QJsonObject{}));

    auto camera = json.value("camera").toObject();
    camera.insert("direction", QJsonArray{0.0, 0.0, 0.0});
    json.insert("camera", camera);
    CHECK_FALSE(renderRequestFromJson(json));

    json = perspectiveRequest();
    camera = json.value("camera").toObject();
    camera.insert("up", QJsonArray{0.0, 2.0, 0.0});
    json.insert("camera", camera);
    CHECK_FALSE(renderRequestFromJson(json));

    json = perspectiveRequest();
    json.insert("size", QJsonArray{AutomationMaxImageDimension + 1, 900});
    CHECK_FALSE(renderRequestFromJson(json));

    json = perspectiveRequest();
    json.insert("renderMode", "depth");
    CHECK_FALSE(renderRequestFromJson(json));
  }

  SECTION("serializes output metadata deterministically")
  {
    const auto output = AutomationRenderOutput{
      "/tmp/automation/capture.png",
      {1600, 900},
      AutomationCaptureMode::Offscreen,
      AutomationDepthOutput{"/tmp/automation/capture.pfm", {1600, 900}}};
    CHECK(
      renderOutputToJson(output)
      == QJsonObject{
        {"imagePath", "/tmp/automation/capture.png"},
        {"size", QJsonArray{1600, 900}},
        {"captureMode", "offscreen"},
        {"depth",
         QJsonObject{
           {"path", "/tmp/automation/capture.pfm"},
           {"size", QJsonArray{1600, 900}},
           {"format", "pfm-f32-linear-camera-space"},
           {"noHit", "infinity"},
         }},
      });
  }

  SECTION("accepts an opt-in depth output and rejects malformed output options")
  {
    auto json = perspectiveRequest();
    json.insert("outputs", QJsonObject{{"depth", true}});
    const auto request = renderRequestFromJson(json);
    REQUIRE(request);
    CHECK(request->outputs.depth);
    CHECK(renderRequestToJson(*request) == json);

    json.remove("outputs");
    const auto legacyRequest = renderRequestFromJson(json);
    REQUIRE(legacyRequest);
    CHECK_FALSE(legacyRequest->outputs.depth);

    json.insert("outputs", QJsonObject{{"depth", "yes"}});
    CHECK_FALSE(renderRequestFromJson(json));
  }
}

} // namespace tb::ui::automation
