/*
 Copyright (C) 2010 Kristian Duske

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

#include "PropertyDefinition.h"

#include "kdl/optional_utils.h"
#include "kdl/overload.h"
#include "kdl/reflection_impl.h"
#include "kdl/string_utils.h"
#include "kdl/vector_utils.h"

#include <algorithm>
#include <string>

namespace tb::mdl
{
namespace PropertyValueTypes
{

kdl_reflect_impl(TargetSource);
kdl_reflect_impl(TargetDestination);
kdl_reflect_impl(String);
kdl_reflect_impl(Boolean);
kdl_reflect_impl(Integer);
kdl_reflect_impl(Float);
kdl_reflect_impl(ChoiceOption);
kdl_reflect_impl(Choice);
kdl_reflect_impl(Flag);
kdl_reflect_impl(Flags);
kdl_reflect_impl(ColorPropertyValue);
kdl_reflect_impl(ColorComponent);
kdl_reflect_impl(Unknown);

const Flag* Flags::flag(const int flagValue) const
{
  const auto iFlag = std::ranges::find_if(
    flags, [&](const auto& flag) { return flag.value == flagValue; });
  return iFlag != flags.end() ? &*iFlag : nullptr;
}

bool Flags::isDefault(const int flagValue) const
{
  return (defaultValue & flagValue) != 0;
}

std::ostream& operator<<(std::ostream& lhs, const ColorValueType rhs)
{
  switch (rhs)
  {
  case ColorValueType::Any:
    lhs << "Any";
    break;
  case ColorValueType::Float:
    lhs << "Float";
    break;
  case ColorValueType::Byte:
    lhs << "Byte";
    break;
  }
  return lhs;
}

std::ostream& operator<<(std::ostream& lhs, ColorComponentType rhs)
{
  switch (rhs)
  {
  case ColorComponentType::Red:
    lhs << "Red";
    break;
  case ColorComponentType::Green:
    lhs << "Green";
    break;
  case ColorComponentType::Blue:
    lhs << "Blue";
    break;
  case ColorComponentType::Alpha:
    lhs << "Alpha";
    break;
  case ColorComponentType::LightBrightness:
    lhs << "Brightness";
    break;
  case ColorComponentType::Other:
    lhs << "Other";
    break;
  }
  return lhs;
}

std::vector<std::optional<float>> parseColorPropertyValueOptionalValues(
  const std::string_view& value, const size_t minimumNumValues)
{
  auto parsedValues = kdl::vec_transform(kdl::str_split(value, " "), kdl::str_to_float);
  if (parsedValues.size() < minimumNumValues)
    parsedValues.resize(minimumNumValues, std::nullopt);
  return parsedValues;
}

} // namespace PropertyValueTypes

std::ostream& operator<<(std::ostream& lhs, const PropertyValueType& rhs)
{
  std::visit([&](const auto& x) { lhs << x; }, rhs);
  return lhs;
}

kdl_reflect_impl(PropertyDefinition);

std::optional<std::string> PropertyDefinition::defaultValue(
  const PropertyDefinition& definition)
{
  using namespace std::string_literals;
  using namespace PropertyValueTypes;

  return std::visit(
    kdl::overload(
      [](const TargetSource&) -> std::optional<std::string> { return std::nullopt; },
      [](const TargetDestination&) -> std::optional<std::string> { return std::nullopt; },
      [](const String& value) { return value.defaultValue; },
      [](const Boolean& value) {
        return value.defaultValue | kdl::optional_transform([](const auto b) {
                 return b ? "true"s : "false"s;
               });
      },
      [](const Integer& value) {
        return value.defaultValue
               | kdl::optional_transform([](const auto i) { return std::to_string(i); });
      },
      [](const Float& value) {
        return value.defaultValue
               | kdl::optional_transform([](const auto f) { return std::to_string(f); });
      },
      [](const Choice& value) { return value.defaultValue; },
      [](const Flags& value) {
        return value.defaultValue != 0 ? std::optional{std::to_string(value.defaultValue)}
                                       : std::nullopt;
      },
      [](const ColorPropertyValue& value) -> std::optional<std::string> {
        if (value.components.empty())
          return std::nullopt;
        std::string defaultValue;
        for (const auto& component : value.components)
        {
          if (!component.defaultValue.has_value())
            break;
          const auto val = component.defaultValue.value();
          if (defaultValue.empty())
            defaultValue += " ";
          defaultValue += std::to_string(val);
        }
        if (defaultValue.empty())
          return std::nullopt;
        return defaultValue;
      },
      [](const Unknown& value) { return value.defaultValue; }),
    definition.valueType);
}

} // namespace tb::mdl
