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

#include "mdl/BrushFace.h"
#include "mdl/BrushFaceAttributes.h"
#include "mdl/MapFormat.h"
#include "mdl/UpdateBrushFaceAttributes.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{

TEST_CASE("UpdateBrushFaceAttributes")
{
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
        });
    }

    SECTION("with surface attributes and color set")
    {
      attributes.setSurfaceFlags(2);
      attributes.setSurfaceContents(3);
      attributes.setSurfaceValue(11.0f);
      attributes.setColor(Color{1.0f, 2.0f, 3.0f, 4.0f});

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
          .color = Color{1.0f, 2.0f, 3.0f, 4.0f},
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
    attributes.setColor(Color{1.0f, 2.0f, 3.0f, 4.0f});

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
        .color = Color{1.0f, 2.0f, 3.0f, 4.0f},
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
    defaultAttributes.setColor(Color{1.0f, 2.0f, 3.0f, 4.0f});

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
    defaultAttributes.setColor(Color{1.0f, 2.0f, 3.0f, 4.0f});

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
        .color = Color{1.0f, 2.0f, 3.0f, 4.0f},
      };

      auto expectedAttributes = BrushFaceAttributes{"other_material"};
      expectedAttributes.setOffset({2, 3});
      expectedAttributes.setRotation(45.0f);
      expectedAttributes.setScale({4, 5});
      expectedAttributes.setSurfaceFlags(0xFF);
      expectedAttributes.setSurfaceContents(0xFF);
      expectedAttributes.setSurfaceValue(6.0f);
      expectedAttributes.setColor(Color{1.0f, 2.0f, 3.0f, 4.0f});

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
