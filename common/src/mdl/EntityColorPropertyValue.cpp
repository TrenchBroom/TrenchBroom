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

#include "mdl/EntityColorPropertyValue.h"

#include "mdl/EntityDefinition.h"

#include "kd/reflection_impl.h"
#include "kd/result_fold.h"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>


namespace tb::mdl
{
namespace
{

template <std::ranges::range R>
Result<std::vector<float>> parseExtraColorComponents(const R& parts)
{
  return parts | std::views::transform([](const auto part) -> Result<float> {
           auto value = 0.0f;
           const auto ec =
             vm::from_chars(part.data(), part.data() + part.length(), value).ec;
           if (ec == std::errc{})
           {
             return value;
           }
           return Error{fmt::format("Failed to '{}' as float", part)};
         })
         | kdl::fold;
}

/**
 * Parse the default value of a color property definition.
 */
template <typename ColorType>
Result<EntityColorPropertyValue> parseEntityColorPropertyValue(const std::string_view str)
{
  auto parts = kdl::str_split(str, " ");
  return ColorType::parseComponents(parts) | kdl::and_then([&](auto color) {
           auto extraParts = parts | std::views::drop(color.numComponents());
           return parseExtraColorComponents(extraParts)
                  | kdl::transform([&](auto extraComponents) {
                      return EntityColorPropertyValue{Rgb{color}, extraComponents};
                    });
         });
}

std::string entityColorPropertyToString(
  const Rgb& color, const std::vector<float>& extraComponents)
{
  return !extraComponents.empty()
           ? fmt::format("{} {}", color.toString(), fmt::join(extraComponents, " "))
           : color.toString();
}

} // namespace

kdl_reflect_impl(EntityColorPropertyValue);

Result<EntityColorPropertyValue> parseEntityColorPropertyValue(
  const EntityDefinition* entityDefinition,
  const std::string& propertyKey,
  const std::string& propertyValue)
{
  if (
    const auto* propertyDefinition = getPropertyDefinition(entityDefinition, propertyKey))
  {
    return std::visit(
      kdl::overload(
        [&]<typename ColorType>(const PropertyValueTypes::Color<ColorType>&) {
          return parseEntityColorPropertyValue<ColorType>(propertyValue);
        },
        [&](const PropertyValueTypes::String&) {
          return parseEntityColorPropertyValue<Rgb>(propertyValue);
        },
        [&](const PropertyValueTypes::Unknown&) {
          return parseEntityColorPropertyValue<Rgb>(propertyValue);
        },
        [](const auto& valueType) -> Result<EntityColorPropertyValue> {
          return Error{fmt::format(
            "Cannot convert property of type {} to color", fmt::streamed(valueType))};
        }),
      propertyDefinition->valueType);
  }

  return parseEntityColorPropertyValue<Rgb>(propertyValue);
};

Result<std::string> entityColorPropertyToString(
  const EntityDefinition* entityDefinition,
  const std::string& propertyKey,
  const EntityColorPropertyValue& entityColorPropertyValue)
{
  if (
    const auto* propertyDefinition = getPropertyDefinition(entityDefinition, propertyKey))
  {
    return std::visit(
             kdl::overload(
               [&](const PropertyValueTypes::Color<Rgb>&) -> Result<Rgb> {
                 return entityColorPropertyValue.color;
               },
               [&]<typename ColorType>(
                 const PropertyValueTypes::Color<ColorType>&) -> Result<Rgb> {
                 return Rgb{entityColorPropertyValue.color.to<ColorType>()};
               },
               [&](const PropertyValueTypes::String&) -> Result<Rgb> {
                 return entityColorPropertyValue.color;
               },
               [&](const PropertyValueTypes::Unknown&) -> Result<Rgb> {
                 return entityColorPropertyValue.color;
               },
               [](const auto& valueType) -> Result<Rgb> {
                 return Error{fmt::format(
                   "Cannot convert color property of type {} to string",
                   fmt::streamed(valueType))};
               }),
             propertyDefinition->valueType)
           | kdl::transform([&](const auto& color) {
               return entityColorPropertyToString(
                 color, entityColorPropertyValue.extraComponents);
             });
  }
  else
  {
    return entityColorPropertyToString(
      entityColorPropertyValue.color, entityColorPropertyValue.extraComponents);
  }
}

} // namespace tb::mdl
