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
#include "mdl/CatchConfig.h"
#include "mdl/MapFormat.h"
#include "mdl/Matchers.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "mdl/UvAlignment.h"

#include "kd/k.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{
namespace
{

BrushFace createBrushFace(
  std::string materialName,
  const UvAttributes& uvAttributes,
  const SurfaceAttributes& surfaceAttributes = {})
{
  return BrushFace::create(
           {0, 0, 0},
           {0, 1, 0},
           {1, 0, 0},
           std::move(materialName),
           uvAttributes,
           surfaceAttributes,
           MapFormat::Quake2)
         | kdl::value();
}

} // namespace

TEST_CASE("UpdateBrushFaceAttributes")
{

  SECTION("copyAll")
  {
    const auto uvAttributes = UvAttributes{
      .offset = {1, 2},
      .scale = {2, 3},
      .rotation = 45.0f,
    };
    auto surfaceAttributes = SurfaceAttributes{};

    SECTION("with surface attributes and color unset")
    {
      CHECK(
        copyAll(createBrushFace("some_material", uvAttributes, surfaceAttributes))
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
      surfaceAttributes = SurfaceAttributes{
        .contents = 3,
        .flags = 2,
        .value = 11.0f,
        .color = RgbaB{1, 2, 3, 4},
      };

      CHECK(
        copyAll(createBrushFace("some_material", uvAttributes, surfaceAttributes))
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
    const auto uvAttributes = UvAttributes{
      .offset = {1, 2},
      .scale = {2, 3},
      .rotation = 45.0f,
    };
    const auto surfaceAttributes = SurfaceAttributes{
      .contents = 3,
      .flags = 2,
      .value = 11.0f,
      .color = RgbaB{1, 2, 3, 4},
    };

    CHECK(
      copyAllExceptContentFlags(
        createBrushFace("some_material", uvAttributes, surfaceAttributes))
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
    const auto defaultUvAttributes = UvAttributes{
      .offset = {1, 2},
      .scale = {2, 3},
      .rotation = 45.0f,
    };

    CHECK(
      resetAll(defaultUvAttributes)
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
    const auto defaultUvAttributes = UvAttributes{
      .offset = {1, 2},
      .scale = {2, 3},
      .rotation = 45.0f,
    };

    CHECK(
      resetAllToParaxial(defaultUvAttributes)
      == UpdateBrushFaceAttributes{
        .xOffset = SetValue{0.0f},
        .yOffset = SetValue{0.0f},
        .rotation = SetValue{0.0f},
        .xScale = SetValue{2.0f},
        .yScale = SetValue{3.0f},
        .axis = ToParaxial{},
      });
  }

  SECTION("evaluate")
  {
    auto brushFace = createBrushFace("some_material", UvAttributes{});

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
        brushFace.setUvAttributes({.offset = {originalValue, 0.0f}});
      }

      evaluate(update, brushFace);

      CHECK(brushFace.uvAttributes().offset.x() == expectedValue);
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
        brushFace.setSurfaceAttributes({.flags = originalFlags});
      }

      evaluate(update, brushFace);

      CHECK(brushFace.surfaceAttributes().flags == expectedFlags);
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

      const auto expectedUvAttributes = UvAttributes{
        .offset = {2, 3},
        .scale = {4, 5},
        .rotation = 45.0f,
      };
      const auto expectedSurfaceAttributes = SurfaceAttributes{
        .contents = 0xFF,
        .flags = 0xFF,
        .value = 6.0f,
        .color = RgbaB{1, 2, 3, 4},
      };

      evaluate(update, brushFace);

      CHECK(brushFace.materialName() == "other_material");
      CHECK(brushFace.uvAttributes() == expectedUvAttributes);
      CHECK(brushFace.surfaceAttributes() == expectedSurfaceAttributes);
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

      const auto expectedMaterialName = brushFace.materialName();
      const auto expectedUvAttributes = brushFace.uvAttributes();
      const auto expectedSurfaceAttributes = brushFace.surfaceAttributes();

      evaluate(update, brushFace);

      CHECK(brushFace.materialName() == expectedMaterialName);
      CHECK(brushFace.uvAttributes() == expectedUvAttributes);
      CHECK(brushFace.surfaceAttributes() == expectedSurfaceAttributes);
    }
  }
}

} // namespace tb::mdl
