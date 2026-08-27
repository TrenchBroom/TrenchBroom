/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "AutomationJson.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/MapFormat.h"
#include "mdl/SurfaceAttributes.h"
#include "mdl/UvAttributes.h"
#include "mdl/WorldNode.h"
#include "ui/CatchConfig.h"

#include <limits>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui::automation
{
namespace
{

struct NodeTree
{
  mdl::WorldNode world{{}, {}, mdl::MapFormat::Standard};

  NodeTree()
  {
    auto* castle = new mdl::GroupNode{mdl::Group{"castle"}};
    castle->addChild(
      new mdl::EntityNode{mdl::Entity{{{"classname", "light"}, {"origin", "0 0 0"}}}});
    castle->addChild(
      new mdl::EntityNode{mdl::Entity{{{"classname", "info_player_start"}}}});
    castle->addChild(
      new mdl::EntityNode{mdl::Entity{{{"classname", "light"}, {"origin", "128 0 0"}}}});
    const auto builder = mdl::BrushBuilder{
      world.mapFormat(), vm::bbox3d{{-1024, -1024, -1024}, {1024, 1024, 1024}}};
    auto brush =
      builder.createCuboid(vm::bbox3d{{256, 0, 0}, {320, 64, 64}}, "agua") | kdl::value();
    castle->addChild(new mdl::BrushNode{std::move(brush)});
    world.defaultLayer()->addChild(castle);
  }
};

} // namespace

TEST_CASE("Automation node queries")
{
  auto tree = NodeTree{};

  SECTION("retains the legacy search result")
  {
    const auto result = queryNodes(tree.world, "info_player_start", 200);

    REQUIRE(result.size() == 1);
    CHECK(result[0].toObject().value("path").toArray() == QJsonArray{0, 0, 1});
  }

  SECTION("paginates a filtered descendant subtree")
  {
    const auto result = queryNodes(
      tree.world,
      NodeQuery{
        .pattern = "light",
        .ancestorPath = mdl::NodePath{{0, 0}},
        .offset = 0,
        .limit = 1,
      });

    REQUIRE(result.nodes.size() == 1);
    CHECK(result.nodes[0].toObject().value("path").toArray() == QJsonArray{0, 0, 0});
    CHECK(result.total == 2);
    CHECK(result.truncated);
    REQUIRE(result.nextOffset == 1);
  }

  SECTION("aggregates all matching nodes without returning node details")
  {
    const auto result =
      queryNodes(tree.world, NodeQuery{.pattern = "light", .aggregate = true});

    CHECK(result.nodes.empty());
    CHECK(result.total == 2);
    CHECK(result.aggregate.value("total") == 2);
    CHECK(result.aggregate.value("byType").toObject().value("entity") == 2);
    CHECK(result.aggregate.value("byClassname").toObject().value("light") == 2);
  }

  SECTION("filters results by logical bounds")
  {
    const auto result = queryNodes(
      tree.world,
      NodeQuery{
        .pattern = "light",
        .bounds = vm::bbox3d{{-16.0, -16.0, -16.0}, {16.0, 16.0, 16.0}},
      });

    REQUIRE(result.nodes.size() == 1);
    CHECK(result.nodes[0].toObject().value("path").toArray() == QJsonArray{0, 0, 0});
  }

  SECTION("composes exact type material and classname filters")
  {
    const auto brushes = queryNodes(tree.world, NodeQuery{.types = {"brush"}});
    REQUIRE(brushes.nodes.size() == 1);
    CHECK(brushes.nodes[0].toObject().value("path").toArray() == QJsonArray{0, 0, 3});

    const auto water = queryNodes(
      tree.world, NodeQuery{.types = {"brush"}, .materials = {"agua", "lava"}});
    REQUIRE(water.nodes.size() == 1);
    CHECK(water.nodes[0].toObject().value("materials").toArray() == QJsonArray{"agua"});

    const auto lights =
      queryNodes(tree.world, NodeQuery{.types = {"entity"}, .classnames = {"light"}});
    CHECK(lights.total == 2);

    const auto noMatch =
      queryNodes(tree.world, NodeQuery{.types = {"brush"}, .classnames = {"light"}});
    CHECK(noMatch.total == 0);
  }

  SECTION("returns compact path-only pages")
  {
    const auto result = queryNodes(
      tree.world, NodeQuery{.types = {"entity"}, .pathsOnly = true, .limit = 1});

    REQUIRE(result.nodes.size() == 1);
    CHECK(result.nodes[0].toArray() == QJsonArray{0, 0, 0});
    CHECK(result.total == 3);
    CHECK(result.truncated);
    REQUIRE(result.nextOffset == 1);
  }
}

TEST_CASE("Automation integer decoding")
{
  SECTION("accepts non-negative integral values that fit in size_t")
  {
    CHECK(sizeFromJson(0) == 0u);
    CHECK(sizeFromJson(42) == 42u);
  }

  SECTION("rejects values that cannot safely identify a node, face, or revision")
  {
    CHECK_FALSE(sizeFromJson(-1));
    CHECK_FALSE(sizeFromJson(0.5));
    CHECK_FALSE(sizeFromJson(std::numeric_limits<double>::infinity()));
    CHECK_FALSE(sizeFromJson(static_cast<double>(std::numeric_limits<size_t>::max())));
  }

  SECTION("does not truncate malformed node path elements")
  {
    CHECK_FALSE(nodePathFromJson(QJsonArray{0.5}));
    CHECK_FALSE(nodePathFromJson(QJsonArray{-1}));
    CHECK_FALSE(nodePathFromJson(
      QJsonArray{static_cast<double>(std::numeric_limits<size_t>::max())}));
  }
}

TEST_CASE("Automation brush-face serialization")
{
  auto world = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};
  const auto builder = mdl::BrushBuilder{
    world.mapFormat(), vm::bbox3d{{-1024, -1024, -1024}, {1024, 1024, 1024}}};
  auto brush =
    builder.createCuboid(vm::bbox3d{{0, 0, 0}, {32, 64, 128}}, "stone") | kdl::value();
  const auto topFaceIndex = brush.findFace(vm::vec3d{0, 0, 1});
  REQUIRE(topFaceIndex);
  brush.face(*topFaceIndex).setMaterialName("agua");
  brush.face(*topFaceIndex)
    .setUvAttributes(
      mdl::UvAttributes{.offset = {12, 24}, .scale = {2, 4}, .rotation = 45});
  brush.face(*topFaceIndex)
    .setSurfaceAttributes(
      mdl::SurfaceAttributes{
        .contents = 1,
        .flags = 2,
        .value = 3.0f,
        .color = RgbaF{0.25f, 0.5f, 0.75f, 1.0f},
      });
  auto* node = new mdl::BrushNode{std::move(brush)};
  world.defaultLayer()->addChild(node);

  const auto result = brushToJson(*node, world);
  CHECK(result.value("path").toArray() == QJsonArray{0, 0});
  CHECK(result.value("bounds").toObject().value("min").toArray() == QJsonArray{0, 0, 0});
  CHECK(
    result.value("bounds").toObject().value("max").toArray() == QJsonArray{32, 64, 128});
  CHECK(result.value("vertices").toArray().size() == 8);
  const auto faces = result.value("faces").toArray();
  REQUIRE(faces.size() == 6);
  const auto topFace = faces[static_cast<qsizetype>(*topFaceIndex)].toObject();
  CHECK(topFace.value("index") == static_cast<qint64>(*topFaceIndex));
  CHECK(
    topFace.value("boundary").toObject().value("normal").toArray()
    == QJsonArray{0, 0, 1});
  CHECK(topFace.value("vertices").toArray().size() == 4);
  CHECK(topFace.value("material") == "agua");
  const auto surface = topFace.value("surface").toObject();
  CHECK(surface.value("contents") == 1);
  CHECK(surface.value("flags") == 2);
  CHECK(surface.value("value") == 3.0);
  CHECK(surface.value("color").toArray() == QJsonArray{0.25, 0.5, 0.75, 1.0});
  const auto uv = topFace.value("uv").toObject();
  CHECK(uv.value("offset").toArray() == QJsonArray{12, 24});
  CHECK(uv.value("scale").toArray() == QJsonArray{2, 4});
  CHECK(uv.value("rotation") == 45.0);
  CHECK(uv.value("uAxis").toArray().size() == 3);
  CHECK(uv.value("vAxis").toArray().size() == 3);
}

TEST_CASE("Automation footprint extraction")
{
  auto world = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};
  const auto builder = mdl::BrushBuilder{
    world.mapFormat(), vm::bbox3d{{-1024, -1024, -1024}, {1024, 1024, 1024}}};
  const auto makeBrush = [&](const vm::bbox3d& bounds) {
    auto brush = builder.createCuboid(bounds, "stone") | kdl::value();
    const auto topFace = brush.findFace(vm::vec3d{0, 0, 1});
    REQUIRE(topFace);
    brush.face(*topFace).setMaterialName("agua");
    return brush;
  };
  auto* first = new mdl::BrushNode{makeBrush({{0, 0, 0}, {32, 64, 128}})};
  auto* duplicate = new mdl::BrushNode{makeBrush({{0, 0, 0}, {32, 64, 128}})};
  auto* second = new mdl::BrushNode{makeBrush({{64, 0, 0}, {96, 32, 128}})};
  world.defaultLayer()->addChild(first);
  world.defaultLayer()->addChild(duplicate);
  world.defaultLayer()->addChild(second);

  const auto footprints = extractFootprints(
    world,
    {second, duplicate, first},
    FootprintFaceSelector{vm::axis::z, 128.0, std::string{"agua"}});

  SECTION("canonicalizes and coalesces matching convex face polygons")
  {
    REQUIRE(footprints.size() == 2);
    const auto& firstFootprint = footprints[0];
    CHECK(
      firstFootprint.vertices
      == std::vector<vm::vec3d>{{0, 0, 128}, {32, 0, 128}, {32, 64, 128}, {0, 64, 128}});
    REQUIRE(firstFootprint.sources.size() == 2);
    CHECK(firstFootprint.sources[0].path.indices == std::vector<size_t>{0, 0});
    CHECK(firstFootprint.sources[1].path.indices == std::vector<size_t>{0, 1});
    CHECK(firstFootprint.bounds.min == vm::vec3d{0, 0, 128});
    CHECK(firstFootprint.bounds.max == vm::vec3d{32, 64, 128});
  }

  SECTION("serializes source faces and face bounds without mutating the world")
  {
    const auto json = footprintsToJson(footprints);
    REQUIRE(json.size() == 2);
    const auto firstJson = json[0].toObject();
    CHECK(
      firstJson.value("vertices").toArray()
      == QJsonArray{
        QJsonArray{0, 0, 128},
        QJsonArray{32, 0, 128},
        QJsonArray{32, 64, 128},
        QJsonArray{0, 64, 128}});
    CHECK(firstJson.value("sources").toArray().size() == 2);
    CHECK(
      firstJson.value("bounds").toObject().value("max").toArray()
      == QJsonArray{32, 64, 128});
    const auto topFace = first->brush().findFace(vm::vec3d{0, 0, 1});
    REQUIRE(topFace);
    CHECK(first->brush().face(*topFace).materialName() == "agua");
  }

  SECTION("requires the matching material when one is selected")
  {
    CHECK(
      extractFootprints(
        world, {first}, FootprintFaceSelector{vm::axis::z, 128.0, std::string{"stone"}})
        .empty());

    const auto bottom = extractFootprints(
      world, {first}, FootprintFaceSelector{vm::axis::z, 0.0, std::string{"stone"}});
    REQUIRE(bottom.size() == 1);
    CHECK(
      bottom[0].vertices
      == std::vector<vm::vec3d>{{0, 0, 0}, {32, 0, 0}, {32, 64, 0}, {0, 64, 0}});
  }
}

TEST_CASE("Automation planar profile JSON")
{
  const auto params = QJsonObject{
    {"axis", "z"},
    {"gridSize", 16},
    {"material", "stone"},
    {"contour",
     QJsonArray{
       QJsonArray{1, 1}, QJsonArray{127, 1}, QJsonArray{127, 127}, QJsonArray{1, 127}}},
    {"bands",
     QJsonArray{
       QJsonObject{{"inset", 17}, {"bottom", -1}, {"top", 31}, {"role", "curb"}}}},
    {"core", QJsonObject{{"bottom", -31}, {"top", -1}, {"material", "paving"}}},
  };

  SECTION("snaps all planar quantities and assigns material and role defaults")
  {
    const auto profile = planarProfileSpecFromJson(params);
    REQUIRE(profile);
    CHECK(
      profile->contour == std::vector<vm::vec2d>{{0, 0}, {128, 0}, {128, 128}, {0, 128}});
    REQUIRE(profile->bands.size() == 1u);
    CHECK(profile->bands[0].inset == 16.0);
    CHECK(profile->bands[0].bottom == 0.0);
    CHECK(profile->bands[0].top == 32.0);
    CHECK(profile->bands[0].materialName == "stone");
    CHECK(profile->bands[0].role == "curb");
    REQUIRE(profile->core);
    CHECK(profile->core->bottom == -32.0);
    CHECK(profile->core->top == 0.0);
    CHECK(profile->core->materialName == "paving");
    CHECK(profile->core->role == "core");
  }

  SECTION("rejects unsupported axes and incomplete profile parts")
  {
    auto unsupportedAxis = params;
    unsupportedAxis.insert("axis", "x");
    CHECK_FALSE(planarProfileSpecFromJson(unsupportedAxis));

    auto missingMaterial = params;
    missingMaterial.remove("material");
    CHECK_FALSE(planarProfileSpecFromJson(missingMaterial));

    auto missingInset = params;
    auto bands = missingInset.value("bands").toArray();
    auto band = bands[0].toObject();
    band.remove("inset");
    bands[0] = band;
    missingInset.insert("bands", bands);
    CHECK_FALSE(planarProfileSpecFromJson(missingInset));
  }
}

TEST_CASE("Automation profile extrusion JSON")
{
  const auto params = QJsonObject{
    {"plane", "xz"},
    {"gridSize", 16},
    {"profile", QJsonArray{QJsonArray{1, 1}, QJsonArray{127, 1}, QJsonArray{1, 127}}},
    {"interval", QJsonArray{-17, 17}},
    {"material", "stone"},
    {"role", "gable"},
  };

  SECTION("snaps profile and extrusion interval on an arbitrary plane")
  {
    const auto extrusion = profileExtrusionSpecFromJson(params);
    REQUIRE(extrusion);
    CHECK(extrusion->plane == mdl::ProfileExtrusionPlane::XZ);
    CHECK(extrusion->profile == std::vector<vm::vec2d>{{0, 0}, {128, 0}, {0, 128}});
    CHECK(extrusion->minimum == -16.0);
    CHECK(extrusion->maximum == 16.0);
    CHECK(extrusion->materialName == "stone");
    CHECK(extrusion->role == "gable");
  }

  SECTION("defaults the role and rejects incomplete inputs")
  {
    auto defaultRole = params;
    defaultRole.remove("role");
    const auto extrusion = profileExtrusionSpecFromJson(defaultRole);
    REQUIRE(extrusion);
    CHECK(extrusion->role == "profile");

    auto invalidPlane = params;
    invalidPlane.insert("plane", "z");
    CHECK_FALSE(profileExtrusionSpecFromJson(invalidPlane));
    auto invalidInterval = params;
    invalidInterval.insert("interval", QJsonArray{0});
    CHECK_FALSE(profileExtrusionSpecFromJson(invalidInterval));
    auto missingMaterial = params;
    missingMaterial.remove("material");
    CHECK_FALSE(profileExtrusionSpecFromJson(missingMaterial));
  }
}

} // namespace tb::ui::automation
