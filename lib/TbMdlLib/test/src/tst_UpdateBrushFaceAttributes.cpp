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

#include <limits>

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
        REQUIRE(
          brushFace.setUvAttributes({.offset = {originalValue, 0.0f}}).is_success());
      }

      REQUIRE(evaluate(update, brushFace).is_success());

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

      REQUIRE(evaluate(update, brushFace).is_success());

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

      REQUIRE(evaluate(update, brushFace).is_success());

      CHECK(brushFace.materialName() == "other_material");
      CHECK(brushFace.uvAttributes() == expectedUvAttributes);
      CHECK(brushFace.surfaceAttributes() == expectedSurfaceAttributes);
    }

    SECTION("rejects an extreme rotation and leaves the face unchanged")
    {
      const auto max = std::numeric_limits<float>::max();
      REQUIRE(brushFace.setUvAttributes({.rotation = max}).is_success());

      const auto expectedMaterialName = brushFace.materialName();
      const auto expectedUvAttributes = brushFace.uvAttributes();
      const auto expectedSurfaceAttributes = brushFace.surfaceAttributes();

      // adding another huge delta overflows the rotation to infinity -- this is the
      // original crash this branch set out to fix
      const auto update = UpdateBrushFaceAttributes{
        .materialName = "other_material",
        .rotation = AddValue{max},
      };
      CHECK(!evaluate(update, brushFace));

      CHECK(brushFace.materialName() == expectedMaterialName);
      CHECK(brushFace.uvAttributes() == expectedUvAttributes);
      CHECK(brushFace.surfaceAttributes() == expectedSurfaceAttributes);
    }

    SECTION("rejects an extreme negative rotation without hanging")
    {
      const auto lowest = std::numeric_limits<float>::lowest();
      REQUIRE(brushFace.setUvAttributes({.rotation = lowest}).is_success());

      // adding another huge negative delta overflows the rotation to -infinity; this is
      // a regression guard for the normalize_degrees hang this branch also fixed
      const auto update = UpdateBrushFaceAttributes{.rotation = AddValue{lowest}};
      CHECK(!evaluate(update, brushFace));
    }

    SECTION("rejects an extreme scale via SetValue")
    {
      const auto expectedUvAttributes = brushFace.uvAttributes();

      const auto update =
        UpdateBrushFaceAttributes{.xScale = SetValue{1e30f}, .yScale = SetValue{1e30f}};
      CHECK(!evaluate(update, brushFace));
      CHECK(brushFace.uvAttributes() == expectedUvAttributes);
    }

    SECTION("rejects an extreme scale accumulated via repeated MultiplyValue")
    {
      REQUIRE(brushFace.setUvAttributes({.scale = {1e15f, 1e15f}}).is_success());
      const auto expectedUvAttributes = brushFace.uvAttributes();

      const auto update = UpdateBrushFaceAttributes{
        .xScale = MultiplyValue{1e15f}, .yScale = MultiplyValue{1e15f}};
      CHECK(!evaluate(update, brushFace));
      CHECK(brushFace.uvAttributes() == expectedUvAttributes);
    }

    SECTION("accepts a scale of 0")
    {
      // InvalidUvScaleValidator relies on zero-scale faces being able to load and be
      // edited so it can flag and fix them
      const auto update =
        UpdateBrushFaceAttributes{.xScale = SetValue{0.0f}, .yScale = SetValue{0.0f}};
      REQUIRE(evaluate(update, brushFace).is_success());
      CHECK(brushFace.uvAttributes().scale == vm::vec2f{0, 0});
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

      REQUIRE(evaluate(update, brushFace).is_success());

      CHECK(brushFace.materialName() == expectedMaterialName);
      CHECK(brushFace.uvAttributes() == expectedUvAttributes);
      CHECK(brushFace.surfaceAttributes() == expectedSurfaceAttributes);
    }
  }
}

} // namespace tb::mdl
