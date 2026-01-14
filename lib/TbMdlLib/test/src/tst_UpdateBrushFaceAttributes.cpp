/*
 Copyright (C) 2025 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl/Material.h"
#include "gl/Texture.h"
#include "gl/TextureResource.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceAttributes.h"
#include "mdl/CatchConfig.h"
#include "mdl/MapFormat.h"
#include "mdl/Matchers.h"
#include "mdl/UpdateBrushFaceAttributes.h"

#include "kd/k.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{

TEST_CASE("UpdateBrushFaceAttributes")
{
  constexpr auto Std = MapFormat::Standard;
  constexpr auto Vlv = MapFormat::Valve;
  constexpr auto UvU = UvAxis::u;
  constexpr auto UvV = UvAxis::v;
  constexpr auto UvBest = UvPolicy::best;
  constexpr auto UvNext = UvPolicy::next;
  constexpr auto UvPrev = UvPolicy::prev;
  constexpr auto UvFwd = UvDirection::forward;
  constexpr auto UvBwd = UvDirection::backward;

  SECTION("copyAll")
  {
    auto attributes = BrushFaceAttributes{"some_material"};
    attributes.setOffset({1, 2});
    attributes.setRotation(45.0f);
    attributes.setScale({2, 3});

    SECTION("with surface attributes and color unset")
    {
      CHECK(
        copyAll(attributes)
        == UpdateBrushFaceAttributes{
          .materialName = "some_material",
          .xOffset = SetValue{1.0f},
          .yOffset = SetValue{2.0f},
          .rotation = SetValue{45.0f},
          .xScale = SetValue{2.0f},
          .yScale = SetValue{3.0f},
          .color =
            std::optional<std::optional<Color>>{std::optional<Color>{std::nullopt}},
        });
    }

    SECTION("with surface attributes and color set")
    {
      attributes.setSurfaceFlags(2);
      attributes.setSurfaceContents(3);
      attributes.setSurfaceValue(11.0f);
      attributes.setColor(RgbaB{1, 2, 3, 4});

      CHECK(
        copyAll(attributes)
        == UpdateBrushFaceAttributes{
          .materialName = "some_material",
          .xOffset = SetValue{1.0f},
          .yOffset = SetValue{2.0f},
          .rotation = SetValue{45.0f},
          .xScale = SetValue{2.0f},
          .yScale = SetValue{3.0f},
          .surfaceFlags = SetFlags{2},
          .surfaceContents = SetFlags{3},
          .surfaceValue = SetValue{11.0f},
          .color = RgbaB{1, 2, 3, 4},
        });
    }
  }

  SECTION("copyAllExceptContentFlags")
  {
    auto attributes = BrushFaceAttributes{"some_material"};
    attributes.setOffset({1, 2});
    attributes.setRotation(45.0f);
    attributes.setScale({2, 3});
    attributes.setSurfaceFlags(2);
    attributes.setSurfaceContents(3);
    attributes.setSurfaceValue(11.0f);
    attributes.setColor(RgbaB{1, 2, 3, 4});

    CHECK(
      copyAllExceptContentFlags(attributes)
      == UpdateBrushFaceAttributes{
        .materialName = "some_material",
        .xOffset = SetValue{1.0f},
        .yOffset = SetValue{2.0f},
        .rotation = SetValue{45.0f},
        .xScale = SetValue{2.0f},
        .yScale = SetValue{3.0f},
        .surfaceFlags = SetFlags{2},
        .surfaceValue = SetValue{11.0f},
        .color = RgbaB{1, 2, 3, 4},
      });
  }

  SECTION("resetAll")
  {
    auto defaultAttributes = BrushFaceAttributes{"some_material"};
    defaultAttributes.setOffset({1, 2});
    defaultAttributes.setRotation(45.0f);
    defaultAttributes.setScale({2, 3});
    defaultAttributes.setSurfaceFlags(2);
    defaultAttributes.setSurfaceContents(3);
    defaultAttributes.setSurfaceValue(11.0f);
    defaultAttributes.setColor(RgbaB{1, 2, 3, 4});

    CHECK(
      resetAll(defaultAttributes)
      == UpdateBrushFaceAttributes{
        .xOffset = SetValue{0.0f},
        .yOffset = SetValue{0.0f},
        .rotation = SetValue{0.0f},
        .xScale = SetValue{2.0f},
        .yScale = SetValue{3.0f},
        .axis = ResetAxis{},
      });
  }

  SECTION("resetAllToParaxial")
  {
    auto defaultAttributes = BrushFaceAttributes{"some_material"};
    defaultAttributes.setOffset({1, 2});
    defaultAttributes.setRotation(45.0f);
    defaultAttributes.setScale({2, 3});
    defaultAttributes.setSurfaceFlags(2);
    defaultAttributes.setSurfaceContents(3);
    defaultAttributes.setSurfaceValue(11.0f);
    defaultAttributes.setColor(RgbaB{1, 2, 3, 4});

    CHECK(
      resetAllToParaxial(defaultAttributes)
      == UpdateBrushFaceAttributes{
        .xOffset = SetValue{0.0f},
        .yOffset = SetValue{0.0f},
        .rotation = SetValue{0.0f},
        .xScale = SetValue{2.0f},
        .yScale = SetValue{3.0f},
        .axis = ToParaxial{},
      });
  }

  SECTION("align")
  {
    using T = std::tuple<MapFormat, vm::vec2f, vm::vec2f, float, UvPolicy, float>;

    SECTION("Axis aligned rectangle (-Y normal)")
    {
      const auto
        [mapFormat,
         initialOffset,
         initialScale,
         initialRotation,
         policy,
         expectedRotation] = GENERATE_COPY(values<T>({
          {Std, {0, 0}, {1, 1}, 0.0f, UvBest, 0.0f},
          {Std, {0, 0}, {1, 1}, 15.0f, UvBest, 0.0f},
          {Std, {0, 0}, {1, 1}, 60.0f, UvBest, 90.0f},

          {Std, {0, 0}, {1, 1}, 0.0f, UvNext, 90.0f},
          {Std, {0, 0}, {1, 1}, 90.0f, UvNext, 180.0f},
          {Std, {0, 0}, {1, 1}, 180.0f, UvNext, 270.0f},
          {Std, {0, 0}, {1, 1}, 270.0f, UvNext, 0.0f},

          {Std, {0, 0}, {1, 1}, 0.0f, UvPrev, 270.0f},
          {Std, {0, 0}, {1, 1}, 270.0f, UvPrev, 180.0f},
          {Std, {0, 0}, {1, 1}, 180.0f, UvPrev, 90.0f},
          {Std, {0, 0}, {1, 1}, 90.0f, UvPrev, 0.0f},

          {Std, {0, 0}, {1, 1}, 15.0f, UvNext, 0.0f},
          {Std, {0, 0}, {1, 1}, 60.0f, UvNext, 90.0f},
          {Std, {0, 0}, {1, 1}, 15.0f, UvPrev, 0.0f},
          {Std, {0, 0}, {1, 1}, 60.0f, UvPrev, 90.0f},

          {Std, {12, -3}, {1.2f, 0.9f}, 0.0f, UvPrev, 270.0f},
          {Std, {12, -3}, {1.2f, 0.9f}, 15.0f, UvPrev, 0.0f},

          {Vlv, {0, 0}, {1, 1}, 0.0f, UvBest, 0.0f},
          {Vlv, {0, 0}, {1, 1}, 15.0f, UvBest, 0.0f},
          {Vlv, {0, 0}, {1, 1}, 60.0f, UvBest, 90.0f},

          {Vlv, {0, 0}, {1, 1}, 0.0f, UvNext, 270.0f},
          {Vlv, {0, 0}, {1, 1}, 270.0f, UvNext, 180.0f},
          {Vlv, {0, 0}, {1, 1}, 180.0f, UvNext, 90.0f},
          {Vlv, {0, 0}, {1, 1}, 90.0f, UvNext, 0.0f},

          {Vlv, {0, 0}, {1, 1}, 0.0f, UvPrev, 90.0f},
          {Vlv, {0, 0}, {1, 1}, 90.0f, UvPrev, 180.0f},
          {Vlv, {0, 0}, {1, 1}, 180.0f, UvPrev, 270.0f},
          {Vlv, {0, 0}, {1, 1}, 270.0f, UvPrev, 0.0f},

          {Vlv, {0, 0}, {1, 1}, 15.0f, UvNext, 0.0f},
          {Vlv, {0, 0}, {1, 1}, 60.0f, UvNext, 90.0f},
          {Vlv, {0, 0}, {1, 1}, 15.0f, UvPrev, 0.0f},
          {Vlv, {0, 0}, {1, 1}, 60.0f, UvPrev, 90.0f},

          {Vlv, {12, -3}, {1.2f, 0.9f}, 0.0f, UvPrev, 90.0f},
          {Vlv, {12, -3}, {1.2f, 0.9f}, 15.0f, UvPrev, 0.0f},
        }));

      CAPTURE(mapFormat, initialOffset, initialScale, initialRotation, policy);

      auto brushBuilder = BrushBuilder{mapFormat, vm::bbox3d{8192.0}};
      brushBuilder.createCuboid(vm::vec3d{32, 32, 32}, "material")
        | kdl::transform([&](auto brush) {
            const auto frontFaceIndex = brush.findFace(vm::vec3d{0, -1, 0});
            REQUIRE(frontFaceIndex);

            auto& frontFace = brush.face(*frontFaceIndex);
            evaluate(
              UpdateBrushFaceAttributes{
                .xOffset = SetValue{initialOffset.x()},
                .yOffset = SetValue{initialOffset.y()},
                .rotation = SetValue{initialRotation},
                .xScale = SetValue{initialScale.x()},
                .yScale = SetValue{initialScale.y()},
              },
              frontFace);

            CHECK_THAT(
              align(frontFace, policy),
              MatchesUpdateBrushFaceAttributes(
                UpdateBrushFaceAttributes{.rotation = SetValue{expectedRotation}}));
          })
        | kdl::transform_error([](const auto e) { FAIL(e); });
    }

    SECTION("Trapezoid (+Z normal)")
    {
      const auto
        [mapFormat,
         initialOffset,
         initialScale,
         initialRotation,
         policy,
         expectedRotation] = GENERATE_COPY(values<T>({
          {Std, {0, 0}, {1, 1}, 0.0f, UvBest, 0.0f},
          {Std, {0, 0}, {1, 1}, 15.0f, UvBest, 0.0f},
          {Std, {0, 0}, {1, 1}, 35.0f, UvBest, 45.0f},

          {Std, {0, 0}, {1, 1}, 0.0f, UvNext, 45.0f},
          {Std, {0, 0}, {1, 1}, 45.0f, UvNext, 180.0f},
          {Std, {0, 0}, {1, 1}, 180.0f, UvNext, 315.0f},
          {Std, {0, 0}, {1, 1}, 315.0f, UvNext, 0.0f},

          {Std, {0, 0}, {1, 1}, 0.0f, UvPrev, 315.0f},
          {Std, {0, 0}, {1, 1}, 315.0f, UvPrev, 180.0f},
          {Std, {0, 0}, {1, 1}, 180.0f, UvPrev, 45.0f},
          {Std, {0, 0}, {1, 1}, 45.0f, UvPrev, 0.0f},

          {Std, {12, -3}, {1.2f, 0.9f}, 0.0f, UvPrev, 315.0f},
          {Std, {12, -3}, {1.2f, 0.9f}, 315.0f, UvPrev, 180.0f},

          {Vlv, {0, 0}, {1, 1}, 0.0f, UvBest, 0.0f},
          {Vlv, {0, 0}, {1, 1}, 15.0f, UvBest, 0.0f},
          {Vlv, {0, 0}, {1, 1}, 35.0f, UvBest, 45.0f},

          {Vlv, {0, 0}, {1, 1}, 0.0f, UvNext, 315.0f},
          {Vlv, {0, 0}, {1, 1}, 315.0f, UvNext, 180.0f},
          {Vlv, {0, 0}, {1, 1}, 180.0f, UvNext, 45.0f},
          {Vlv, {0, 0}, {1, 1}, 45.0f, UvNext, 0.0f},

          {Vlv, {0, 0}, {1, 1}, 0.0f, UvPrev, 45.0f},
          {Vlv, {0, 0}, {1, 1}, 45.0f, UvPrev, 180.0f},
          {Vlv, {0, 0}, {1, 1}, 180.0f, UvPrev, 315.0f},
          {Vlv, {0, 0}, {1, 1}, 315.0f, UvPrev, 0.0f},

          {Vlv, {12, -3}, {1.2f, 0.9f}, 0.0f, UvPrev, 45.0f},
          {Vlv, {12, -3}, {1.2f, 0.9f}, 45.0f, UvPrev, 180.0f},
        }));

      CAPTURE(mapFormat, initialOffset, initialScale, initialRotation, policy);

      auto brushBuilder = BrushBuilder{mapFormat, vm::bbox3d{8192.0}};
      brushBuilder.createBrush(
        std::vector<vm::vec3d>{
          // top face
          {-48, 16, 0},
          {+48, 16, 0},
          {-16, -16, 0},
          {+16, -16, 0},
          // bottom face
          {-48, 16, -16},
          {+48, 16, -16},
          {-16, -16, -16},
          {+16, -16, -16},
        },
        "material")
        | kdl::transform([&](auto brush) {
            const auto topFaceIndex = brush.findFace(vm::vec3d{0, 0, 1});
            REQUIRE(topFaceIndex);

            auto& topFace = brush.face(*topFaceIndex);
            evaluate(
              UpdateBrushFaceAttributes{
                .xOffset = SetValue{initialOffset.x()},
                .yOffset = SetValue{initialOffset.y()},
                .rotation = SetValue{initialRotation},
                .xScale = SetValue{initialScale.x()},
                .yScale = SetValue{initialScale.y()},
              },
              topFace);

            CHECK_THAT(
              align(topFace, policy),
              MatchesUpdateBrushFaceAttributes(
                UpdateBrushFaceAttributes{.rotation = SetValue{expectedRotation}}));
          })
        | kdl::transform_error([](const auto e) { FAIL(e); });
    }

    SECTION("Slanted (+Z normal)")
    {
      const auto
        [mapFormat,
         initialOffset,
         initialScale,
         initialRotation,
         policy,
         expectedRotation] = GENERATE_COPY(values<T>({
          {Std, {0, 0}, {1, 1}, 0.0f, UvNext, 45.0f},
          {Std, {0, 0}, {1, 1}, 45.0f, UvNext, 180.0f},
          {Std, {0, 0}, {1, 1}, 180.0f, UvNext, 315.0f},
          {Std, {0, 0}, {1, 1}, 315.0f, UvNext, 0.0f},

          {Std, {0, 0}, {1, 1}, 0.0f, UvPrev, 315.0f},
          {Std, {0, 0}, {1, 1}, 315.0f, UvPrev, 180.0f},
          {Std, {0, 0}, {1, 1}, 180.0f, UvPrev, 45.0f},
          {Std, {0, 0}, {1, 1}, 45.0f, UvPrev, 0.0f},

          {Std, {12, -3}, {1.2f, 0.9f}, 0.0f, UvPrev, 315.0f},
          {Std, {12, -3}, {1.2f, 0.9f}, 315.0f, UvPrev, 180.0f},

          {Vlv, {0, 0}, {1, 1}, 0.0f, UvNext, 311.81f},
          {Vlv, {0, 0}, {1, 1}, 311.81f, UvNext, 180.0f},
          {Vlv, {0, 0}, {1, 1}, 180.0f, UvNext, 48.1897f},
          {Vlv, {0, 0}, {1, 1}, 48.1897f, UvNext, 0.0f},

          {Vlv, {0, 0}, {1, 1}, 0.0f, UvPrev, 48.1897f},
          {Vlv, {0, 0}, {1, 1}, 48.1897f, UvPrev, 180.0f},
          {Vlv, {0, 0}, {1, 1}, 180.0f, UvPrev, 311.81f},
          {Vlv, {0, 0}, {1, 1}, 311.81f, UvPrev, 0.0f},

          {Vlv, {12, -3}, {1.2f, 0.9f}, 0.0f, UvPrev, 48.1897f},
          {Vlv, {12, -3}, {1.2f, 0.9f}, 48.1897f, UvPrev, 180.0f},
        }));

      CAPTURE(mapFormat, initialOffset, initialScale, initialRotation, policy);

      auto brushBuilder = BrushBuilder{mapFormat, vm::bbox3d{8192.0}};
      brushBuilder.createBrush(
        std::vector<vm::vec3d>{
          // top face
          {-48, 16, 16},
          {+48, 16, 16},
          {-16, -16, 0},
          {+16, -16, 0},
          // bottom face
          {-48, 16, -16},
          {+48, 16, -16},
          {-16, -16, -16},
          {+16, -16, -16},
        },
        "material")
        | kdl::transform([&](auto brush) {
            const auto topFaceIndex = brush.findFace(vm::normalize(vm::vec3d{0, -1, 2}));
            REQUIRE(topFaceIndex);

            auto& topFace = brush.face(*topFaceIndex);
            evaluate(
              UpdateBrushFaceAttributes{
                .xOffset = SetValue{initialOffset.x()},
                .yOffset = SetValue{initialOffset.y()},
                .rotation = SetValue{initialRotation},
                .xScale = SetValue{initialScale.x()},
                .yScale = SetValue{initialScale.y()},
              },
              topFace);

            CHECK_THAT(
              align(topFace, policy),
              MatchesUpdateBrushFaceAttributes(
                UpdateBrushFaceAttributes{.rotation = SetValue{expectedRotation}}));
          })
        | kdl::transform_error([](const auto e) { FAIL(e); });
    }
  }

  SECTION("justify")
  {
    using T = std::tuple<
      MapFormat,
      vm::vec2d,
      vm::vec2d,
      double,
      UvAxis,
      UvDirection,
      UvPolicy,
      vm::vec3d,
      vm::vec2d>;

    SECTION("Rectangular off-center face (-Y normal)")
    {
      const auto
        [mapFormat,
         initialOffset,
         initialScale,
         initialRotation,
         axis,
         direction,
         policy,
         brushSize,
         expectedOffset] = GENERATE_COPY(values<T>({
          // UvAxis::u
          {Std, {0, 0}, {1, 1}, 0, UvU, UvFwd, UvBest, {64, 64, 64}, {16, 0}},
          {Std, {0, 0}, {1, 1}, 0, UvU, UvFwd, UvNext, {64, 64, 64}, {16, 0}},
          {Std, {0, 0}, {1, 1}, 0, UvU, UvFwd, UvPrev, {64, 64, 64}, {16, 0}},
          {Std, {16, 0}, {1, 1}, 0, UvU, UvFwd, UvBest, {64, 64, 64}, {16, 0}},
          {Std, {16, 0}, {1, 1}, 0, UvU, UvFwd, UvNext, {64, 64, 64}, {16, 0}},
          {Std, {16, 0}, {1, 1}, 0, UvU, UvFwd, UvPrev, {64, 64, 64}, {16, 0}},
          {Std, {16, 8}, {1, 1}, 0, UvU, UvFwd, UvBest, {64, 64, 64}, {16, 0}},

          {Std, {0, 0}, {1, 1}, 0, UvU, UvBwd, UvBest, {64, 64, 64}, {16, 0}},

          {Std, {0, 0}, {1.2, 0.9}, 0, UvU, UvFwd, UvBest, {64, 64, 64}, {24, 0}},
          {Std, {0, 0}, {1.2, 0.9}, 0, UvU, UvBwd, UvBest, {64, 64, 64}, {13.3333, 0}},

          {Std, {0, 0}, {1, 1}, 15, UvU, UvFwd, UvBest, {64, 64, 64}, {5.21225, 0}},

          {Vlv, {0, 0}, {1, 1}, 0, UvU, UvFwd, UvBest, {64, 64, 64}, {16, 0}},
          {Vlv, {0, 0}, {1, 1}, 0, UvU, UvBwd, UvBest, {64, 64, 64}, {16, 0}},

          {Vlv, {0, 0}, {1.2, 0.9}, 0, UvU, UvFwd, UvBest, {64, 64, 64}, {24.0, 0}},
          {Vlv, {0, 0}, {1.2, 0.9}, 0, UvU, UvBwd, UvBest, {64, 64, 64}, {16.0 / 1.2, 0}},

          {Vlv, {0, 0}, {1, 1}, 15, UvU, UvFwd, UvBest, {64, 64, 64}, {13.4945, 0}},

          // texture width is a multiple of brush width
          {Std, {0, 0}, {1, 1}, 0, UvU, UvFwd, UvBest, {16, 64, 64}, {40, 0}},
          {Std, {0, 0}, {1, 1}, 0, UvU, UvFwd, UvNext, {16, 64, 64}, {40, 0}},
          {Std, {0, 0}, {1, 1}, 0, UvU, UvFwd, UvPrev, {16, 64, 64}, {40, 0}},

          {Std, {40, 0}, {1, 1}, 0, UvU, UvFwd, UvBest, {16, 64, 64}, {40, 0}},
          {Std, {40, 0}, {1, 1}, 0, UvU, UvFwd, UvNext, {16, 64, 64}, {56, 0}},
          {Std, {40, 0}, {1, 1}, 0, UvU, UvFwd, UvPrev, {16, 64, 64}, {24, 0}},

          {Std, {56, 0}, {1, 1}, 0, UvU, UvBwd, UvBest, {16, 64, 64}, {56, 0}},
          {Std, {56, 0}, {1, 1}, 0, UvU, UvBwd, UvNext, {16, 64, 64}, {8, 0}},
          {Std, {56, 0}, {1, 1}, 0, UvU, UvBwd, UvPrev, {16, 64, 64}, {40, 0}},

          // texture width is a multiple of brush width, with scaling
          {Std, {0, 0}, {1.5, 1}, 0, UvU, UvFwd, UvBest, {24, 64, 64}, {45.333, 0}},
          {Std, {45.333, 0}, {1.5, 1}, 0, UvU, UvFwd, UvNext, {24, 64, 64}, {61.333, 0}},
          {Std, {45.333, 0}, {1.5, 1}, 0, UvU, UvFwd, UvPrev, {24, 64, 64}, {29.333, 0}},

          // UvAxis::v
          {Std, {0, 0}, {1, 1}, 0, UvV, UvFwd, UvBest, {64, 64, 64}, {0, 48}},
          {Std, {0, 8}, {1, 1}, 0, UvV, UvFwd, UvBest, {64, 64, 64}, {0, 48}},
          {Std, {16, 8}, {1, 1}, 0, UvV, UvFwd, UvBest, {64, 64, 64}, {0, 48}},

          {Std, {0, 0}, {1, 1}, 0, UvV, UvBwd, UvBest, {64, 64, 64}, {0, 48}},

          {Std, {0, 0}, {1.2, 0.9}, 0, UvV, UvFwd, UvBest, {64, 64, 64}, {0, 46.2222}},
          {Std, {0, 0}, {1.2, 0.9}, 0, UvV, UvBwd, UvBest, {64, 64, 64}, {0, 53.3333}},

          {Std, {0, 0}, {1, 1}, 15, UvV, UvFwd, UvBest, {64, 64, 64}, {0, 36.1219}},

          {Vlv, {0, 0}, {1, 1}, 0, UvV, UvFwd, UvBest, {64, 64, 64}, {0, 48}},
          {Vlv, {0, 0}, {1, 1}, 0, UvV, UvBwd, UvBest, {64, 64, 64}, {0, 48}},

          {Vlv, {0, 0}, {1.2, 0.9}, 0, UvV, UvFwd, UvBest, {64, 64, 64}, {0, 46.2222}},
          {Vlv, {0, 0}, {1.2, 0.9}, 0, UvV, UvBwd, UvBest, {64, 64, 64}, {0, 53.3333}},

          {Vlv, {0, 0}, {1, 1}, 15, UvV, UvFwd, UvBest, {64, 64, 64}, {0, 44.4041}},
        }));

      CAPTURE(
        mapFormat,
        initialOffset,
        initialScale,
        initialRotation,
        axis,
        direction,
        policy,
        brushSize);

      auto material =
        gl::Material{"material", gl::createTextureResource(gl::Texture{64, 64})};

      const auto worldBounds = vm::bbox3d{8192.0};
      auto brushBuilder = BrushBuilder{mapFormat, worldBounds};
      brushBuilder.createCuboid(brushSize, "material") | kdl::and_then([&](auto brush) {
        const auto transform = vm::translation_matrix(vm::vec3d{16, 0, 16});
        return brush.transform(worldBounds, transform, !K(lockMaterials))
               | kdl::transform([&]() { return std::move(brush); });
      }) | kdl::transform([&](auto brush) {
        const auto frontFaceIndex = brush.findFace(vm::vec3d{0, -1, 0});
        REQUIRE(frontFaceIndex);

        auto& frontFace = brush.face(*frontFaceIndex);
        frontFace.setMaterial(&material);

        evaluate(
          UpdateBrushFaceAttributes{
            .xOffset = SetValue{initialOffset.x()},
            .yOffset = SetValue{initialOffset.y()},
            .rotation = SetValue{initialRotation},
            .xScale = SetValue{initialScale.x()},
            .yScale = SetValue{initialScale.y()},
          },
          frontFace);

        CHECK_THAT(
          justify(frontFace, axis, direction, policy),
          MatchesUpdateBrushFaceAttributes(UpdateBrushFaceAttributes{
            .xOffset =
              axis == UvU ? std::optional{SetValue{expectedOffset.x()}} : std::nullopt,
            .yOffset =
              axis == UvV ? std::optional{SetValue{expectedOffset.y()}} : std::nullopt,
          }));
      }) | kdl::transform_error([](const auto e) { FAIL(e); });
    }
  }

  SECTION("fit")
  {
    using T = std::tuple<
      MapFormat,
      vm::vec2d,
      vm::vec2d,
      double,
      UvAxis,
      UvPolicy,
      vm::vec3d,
      vm::vec2d>;

    SECTION("Rectangular off-center face (-Y normal)")
    {
      const auto
        [mapFormat,
         initialOffset,
         initialScale,
         initialRotation,
         axis,
         policy,
         brushSize,
         expectedScale] = GENERATE_COPY(values<T>({
          // U axis
          // brush size == texture size
          {Std, {0, 0}, {1, 1}, 0, UvU, UvBest, {64, 64, 64}, {1, 0}},
          {Std, {0, 0}, {1.6, 1}, 0, UvU, UvBest, {64, 64, 64}, {1, 0}},
          {Std, {0, 0}, {2, 1}, 0, UvU, UvBest, {64, 64, 64}, {1, 0}},

          {Std, {0, 0}, {1, 1}, 0, UvU, UvNext, {64, 64, 64}, {2, 0}},
          {Std, {0, 0}, {1, 1}, 0, UvU, UvPrev, {64, 64, 64}, {0.5, 0}},
          {Std, {0, 8}, {1, 1}, 0, UvU, UvNext, {64, 64, 64}, {2, 0}},
          {Std, {16, 8}, {1, 1}, 0, UvU, UvNext, {64, 64, 64}, {2, 0}},

          {Std, {0, 0}, {1.2, 1}, 0, UvU, UvNext, {64, 64, 64}, {1, 0}},
          {Std, {0, 0}, {1.2, 1}, 0, UvU, UvPrev, {64, 64, 64}, {1, 0}},

          {Std, {0, 0}, {1.6, 1}, 0, UvU, UvNext, {64, 64, 64}, {1, 0}},
          {Std, {0, 0}, {1.6, 1}, 0, UvU, UvPrev, {64, 64, 64}, {1, 0}},

          {Std, {0, 0}, {2, 1}, 0, UvU, UvNext, {64, 64, 64}, {4, 0}},
          {Std, {0, 0}, {2, 1}, 0, UvU, UvPrev, {64, 64, 64}, {1, 0}},

          {Vlv, {0, 0}, {2, 1}, 0, UvU, UvNext, {64, 64, 64}, {4, 0}},
          {Vlv, {0, 0}, {2, 1}, 0, UvU, UvPrev, {64, 64, 64}, {1, 0}},

          // brush size != texture size
          {Std, {0, 0}, {1, 1}, 0, UvU, UvBest, {48, 48, 48}, {0.75, 0}},
          {Std, {0, 0}, {1.6, 1}, 0, UvU, UvBest, {48, 48, 48}, {0.75, 0}},
          {Std, {0, 0}, {2, 1}, 0, UvU, UvBest, {48, 48, 48}, {0.75, 0}},

          {Std, {0, 0}, {1, 1}, 0, UvU, UvNext, {48, 48, 48}, {0.75, 0}},
          {Std, {0, 0}, {1, 1}, 0, UvU, UvPrev, {48, 48, 48}, {0.75, 0}},
          {Std, {0, 8}, {1, 1}, 0, UvU, UvNext, {48, 48, 48}, {0.75, 0}},
          {Std, {16, 8}, {1, 1}, 0, UvU, UvNext, {48, 48, 48}, {0.75, 0}},

          {Std, {0, 0}, {0.75, 1}, 0, UvU, UvNext, {48, 48, 48}, {1.5, 0}},
          {Std, {0, 0}, {0.75, 1}, 0, UvU, UvPrev, {48, 48, 48}, {0.375, 0}},

          {Vlv, {0, 0}, {0.75, 1}, 0, UvU, UvNext, {48, 48, 48}, {1.5, 0}},
          {Vlv, {0, 0}, {0.75, 1}, 0, UvU, UvPrev, {48, 48, 48}, {0.375, 0}},

          // V axis
          // brush size == texture size
          {Std, {0, 0}, {1, 1.6}, 0, UvV, UvBest, {64, 64, 64}, {0, 1}},

          {Std, {0, 0}, {1, 1}, 0, UvV, UvNext, {64, 64, 64}, {0, 2}},
          {Std, {0, 0}, {1, 1}, 0, UvV, UvPrev, {64, 64, 64}, {0, 0.5}},

          // brush size != texture size
          {Std, {0, 0}, {1, 1.6}, 0, UvV, UvBest, {48, 48, 48}, {0, 0.75}},

          {Std, {0, 0}, {1, 1}, 0, UvV, UvNext, {48, 48, 48}, {0, 0.75}},
          {Std, {0, 0}, {1, 1}, 0, UvV, UvPrev, {48, 48, 48}, {0, 0.75}},

          {Std, {0, 0}, {1, 0.75}, 0, UvV, UvNext, {48, 48, 48}, {0, 1.5}},
          {Std, {0, 0}, {1, 0.75}, 0, UvV, UvPrev, {48, 48, 48}, {0, 0.375}},
        }));

      CAPTURE(mapFormat, initialOffset, initialScale, initialRotation, axis, policy);

      auto material =
        gl::Material{"material", gl::createTextureResource(gl::Texture{64, 64})};

      const auto worldBounds = vm::bbox3d{8192.0};
      auto brushBuilder = BrushBuilder{mapFormat, worldBounds};
      brushBuilder.createCuboid(brushSize, "material") | kdl::and_then([&](auto brush) {
        const auto transform = vm::translation_matrix(vm::vec3d{16, 0, 16});
        return brush.transform(worldBounds, transform, !K(lockMaterials))
               | kdl::transform([&]() { return std::move(brush); });
      }) | kdl::transform([&](auto brush) {
        const auto frontFaceIndex = brush.findFace(vm::vec3d{0, -1, 0});
        REQUIRE(frontFaceIndex);

        auto& frontFace = brush.face(*frontFaceIndex);
        frontFace.setMaterial(&material);

        evaluate(
          UpdateBrushFaceAttributes{
            .xOffset = SetValue{initialOffset.x()},
            .yOffset = SetValue{initialOffset.y()},
            .rotation = SetValue{initialRotation},
            .xScale = SetValue{initialScale.x()},
            .yScale = SetValue{initialScale.y()},
          },
          frontFace);

        CHECK_THAT(
          fit(frontFace, axis, policy),
          MatchesUpdateBrushFaceAttributes(UpdateBrushFaceAttributes{
            .xScale =
              axis == UvU ? std::optional{SetValue{expectedScale.x()}} : std::nullopt,
            .yScale =
              axis == UvV ? std::optional{SetValue{expectedScale.y()}} : std::nullopt,
          }));
      }) | kdl::transform_error([](const auto e) { FAIL(e); });
    }
  }

  SECTION("evaluate")
  {
    auto brushFace = BrushFace::create(
                       {0, 0, 0},
                       {0, 1, 0},
                       {1, 0, 0},
                       BrushFaceAttributes{"some_material"},
                       MapFormat::Quake2)
                       .value();

    SECTION("ValueOp")
    {
      using T = std::tuple<ValueOp, float, float>;

      const auto [valueOp, originalValue, expectedValue] = GENERATE(values<T>({
        {SetValue{2.0f}, 1.0f, 2.0f},
        {AddValue{2.0f}, 1.0f, 3.0f},
        {MultiplyValue{2.0f}, 3.0f, 6.0f},
      }));

      CAPTURE(valueOp, originalValue);

      const auto update = UpdateBrushFaceAttributes{.xOffset = valueOp};

      {
        auto attributes = brushFace.attributes();
        attributes.setXOffset(originalValue);
        brushFace.setAttributes(attributes);
      }

      evaluate(update, brushFace);

      CHECK(brushFace.attributes().xOffset() == expectedValue);
    }

    SECTION("FlagOp")
    {
      using T = std::tuple<FlagOp, std::optional<int>, std::optional<int>>;

      const auto [flagOp, originalFlags, expectedFlags] = GENERATE(values<T>({
        {SetFlags{std::nullopt}, 0xF1, std::nullopt},
        {SetFlags{0x0F}, std::nullopt, 0x0F},
        {SetFlags{0x0F}, 0xF1, 0x0F},
        {SetFlagBits{0x0F}, std::nullopt, 0x0F},
        {SetFlagBits{0x0F}, 0xF1, 0xFF},
        {ClearFlagBits{0x0F}, std::nullopt, 0x00},
        {ClearFlagBits{0x0F}, 0xF7, 0xF0},
      }));

      CAPTURE(flagOp, originalFlags);

      const auto update = UpdateBrushFaceAttributes{.surfaceFlags = flagOp};

      {
        auto attributes = brushFace.attributes();
        attributes.setSurfaceFlags(originalFlags);
        brushFace.setAttributes(attributes);
      }

      evaluate(update, brushFace);

      CHECK(brushFace.attributes().surfaceFlags() == expectedFlags);
    }

    SECTION("Full evaluation")
    {
      auto update = UpdateBrushFaceAttributes{
        .materialName = "other_material",
        .xOffset = SetValue{2.0f},
        .yOffset = SetValue{3.0f},
        .rotation = SetValue{45.0f},
        .xScale = SetValue{4.0f},
        .yScale = SetValue{5.0f},
        .surfaceFlags = SetFlags{0xFF},
        .surfaceContents = SetFlags{0xFF},
        .surfaceValue = SetValue{6.0f},
        .color = RgbaB{1, 2, 3, 4},
      };

      auto expectedAttributes = BrushFaceAttributes{"other_material"};
      expectedAttributes.setOffset({2, 3});
      expectedAttributes.setRotation(45.0f);
      expectedAttributes.setScale({4, 5});
      expectedAttributes.setSurfaceFlags(0xFF);
      expectedAttributes.setSurfaceContents(0xFF);
      expectedAttributes.setSurfaceValue(6.0f);
      expectedAttributes.setColor(RgbaB{1, 2, 3, 4});

      evaluate(update, brushFace);

      CHECK(brushFace.attributes() == expectedAttributes);
    }

    SECTION("No evaluation")
    {
      auto update = UpdateBrushFaceAttributes{
        .materialName = std::nullopt,
        .xOffset = std::nullopt,
        .yOffset = std::nullopt,
        .rotation = std::nullopt,
        .xScale = std::nullopt,
        .yScale = std::nullopt,
        .surfaceFlags = std::nullopt,
        .surfaceContents = std::nullopt,
        .surfaceValue = std::nullopt,
        .color = std::nullopt,
      };

      const auto expectedAttributes = brushFace.attributes();

      evaluate(update, brushFace);

      CHECK(brushFace.attributes() == expectedAttributes);
    }
  }
}

} // namespace tb::mdl
