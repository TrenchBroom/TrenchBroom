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

#include "mdl/CatchConfig.h"
#include "mdl/EntityColorPropertyValue.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{

TEST_CASE("parseEntityColorPropertyValue")
{
  const auto entityDefinition = EntityDefinition{
    "some_entity",
    Color{},
    "",
    {
      PropertyDefinition{"colorStr", PropertyValueTypes::String{}, "", ""},
      PropertyDefinition{"color1", PropertyValueTypes::Color<RgbF>{}, "", ""},
      PropertyDefinition{"color255", PropertyValueTypes::Color<RgbB>{}, "", ""},
      PropertyDefinition{"colorAny", PropertyValueTypes::Color<Rgb>{}, "", ""},
    }};

  using T = std::tuple<
    std::optional<EntityDefinition>,
    std::string,
    std::string,
    Result<EntityColorPropertyValue>>;

  const auto [definition, propertyKey, propertyValue, expectedResult] =
    GENERATE_COPY(values<T>({
      {std::nullopt,
       "colorStr",
       "0 0 0",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {}}},
      {std::nullopt,
       "colorStr",
       "0 0 0 0",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {0.0f}}},
      {entityDefinition,
       "colorStr",
       "0 0 0",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {}}},
      {entityDefinition,
       "colorStr",
       "0 0 0 0",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {0.0f}}},
      {entityDefinition,
       "colorAny",
       "0 0 0",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {}}},
      {entityDefinition,
       "colorAny",
       "0 0 0 0",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {0.0f}}},
      {entityDefinition,
       "color1",
       "0 0 0",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {}}},
      {entityDefinition,
       "color1",
       "0 0 0 0",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {0.0f}}},
      {entityDefinition,
       "color255",
       "0 0 0",
       EntityColorPropertyValue{RgbB{0, 0, 0}, {}}},
      {entityDefinition,
       "color255",
       "0 0 0 0",
       EntityColorPropertyValue{RgbB{0, 0, 0}, {0.0f}}},
    }));

  CAPTURE(definition, propertyKey, propertyValue);

  CHECK(
    parseEntityColorPropertyValue(
      definition ? &*definition : nullptr, propertyKey, propertyValue)
    == expectedResult);
}

TEST_CASE("entityColorPropertyToString")
{
  const auto entityDefinition = EntityDefinition{
    "some_entity",
    Color{},
    "",
    {
      PropertyDefinition{"colorStr", PropertyValueTypes::String{}, "", ""},
      PropertyDefinition{"color1", PropertyValueTypes::Color<RgbF>{}, "", ""},
      PropertyDefinition{"color255", PropertyValueTypes::Color<RgbB>{}, "", ""},
      PropertyDefinition{"colorAny", PropertyValueTypes::Color<Rgb>{}, "", ""},
    }};

  using T = std::tuple<
    std::optional<EntityDefinition>,
    std::string,
    EntityColorPropertyValue,
    Result<std::string>>;

  const auto [definition, propertyKey, colorValue, expectedResult] =
    GENERATE_COPY(values<T>({
      {std::nullopt,
       "colorStr",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {}},
       "0 0 0"},
      {std::nullopt,
       "colorStr",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {0.0f}},
       "0 0 0 0"},
      {entityDefinition,
       "colorStr",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {}},
       "0 0 0"},
      {entityDefinition,
       "colorStr",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {0.0f}},
       "0 0 0 0"},
      {entityDefinition,
       "colorAny",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {}},
       "0 0 0"},
      {entityDefinition,
       "colorAny",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {0.0f}},
       "0 0 0 0"},
      {entityDefinition,
       "color1",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {}},
       "0 0 0"},
      {entityDefinition,
       "color1",
       EntityColorPropertyValue{RgbF{0.0f, 0.0f, 0.0f}, {0.0f}},
       "0 0 0 0"},
      {entityDefinition,
       "color255",
       EntityColorPropertyValue{RgbB{0, 0, 0}, {}},
       "0 0 0"},
      {entityDefinition,
       "color255",
       EntityColorPropertyValue{RgbB{0, 0, 0}, {0.0f}},
       "0 0 0 0"},
    }));

  CAPTURE(definition, propertyKey, colorValue);

  CHECK(
    entityColorPropertyToString(
      definition ? &*definition : nullptr, propertyKey, colorValue)
    == expectedResult);
}

} // namespace tb::mdl
