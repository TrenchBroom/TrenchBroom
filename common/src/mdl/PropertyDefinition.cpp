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

#include "kd/optional_utils.h"
#include "kd/overload.h"
#include "kd/reflection_impl.h"

#include <fmt/format.h>

#include <algorithm>
#include <string>

namespace tb::mdl
{
namespace PropertyValueTypes
{

kdl_reflect_impl(LinkSource);
kdl_reflect_impl(LinkTarget);
kdl_reflect_impl(String);
kdl_reflect_impl(Boolean);
kdl_reflect_impl(Integer);
kdl_reflect_impl(Float);
kdl_reflect_impl(ChoiceOption);
kdl_reflect_impl(Choice);
kdl_reflect_impl(Flag);
kdl_reflect_impl(Flags);
kdl_reflect_impl(Origin);
kdl_reflect_impl(Input);
kdl_reflect_impl(Output);
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

std::ostream& operator<<(std::ostream& lhs, IOParameterType rhs)
{
  switch (rhs)
  {
  case IOParameterType::Void:
    lhs << "Void";
    break;
  case IOParameterType::String:
    lhs << "String";
    break;
  case IOParameterType::Integer:
    lhs << "Integer";
    break;
  case IOParameterType::Float:
    lhs << "Float";
    break;
  case IOParameterType::Boolean:
    lhs << "Boolean";
    break;
  case IOParameterType::EHandle:
    lhs << "EHandle";
    break;
  }
  return lhs;
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

  return std::visit(
    kdl::overload(
      [](const PropertyValueTypes::LinkTarget&) -> std::optional<std::string> {
        return std::nullopt;
      },
      [](const PropertyValueTypes::LinkSource&) -> std::optional<std::string> {
        return std::nullopt;
      },
      [](const PropertyValueTypes::String& value) { return value.defaultValue; },
      [](const PropertyValueTypes::Boolean& value) {
        return value.defaultValue | kdl::optional_transform([](const auto b) {
                 return b ? "true"s : "false"s;
               });
      },
      [](const PropertyValueTypes::Integer& value) {
        return value.defaultValue
               | kdl::optional_transform([](const auto i) { return std::to_string(i); });
      },
      [](const PropertyValueTypes::Float& value) {
        return value.defaultValue
               | kdl::optional_transform([](const auto f) { return std::to_string(f); });
      },
      [](const PropertyValueTypes::Choice& value) { return value.defaultValue; },
      [](const PropertyValueTypes::Flags& value) {
        return value.defaultValue != 0 ? std::optional{std::to_string(value.defaultValue)}
                                       : std::nullopt;
      },
      [](const PropertyValueTypes::Origin& value) { return value.defaultValue; },
      [](const PropertyValueTypes::Input&) -> std::optional<std::string> {
        return std::nullopt;
      },
      [](const PropertyValueTypes::Output&) -> std::optional<std::string> {
        return std::nullopt;
      },
      []<typename T>(const PropertyValueTypes::Color<T>& value) {
        return value.defaultValue;
      },
      [](const PropertyValueTypes::Unknown& value) { return value.defaultValue; }),
    definition.valueType);
}

} // namespace tb::mdl
