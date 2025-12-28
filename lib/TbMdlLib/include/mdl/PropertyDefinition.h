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

#pragma once

#include "Color.h"

#include "kd/reflection_impl.h"

#include <optional>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

namespace tb::mdl
{

namespace PropertyValueTypes
{
struct LinkSource
{
  kdl_reflect_decl_empty(LinkSource);
};

struct LinkTarget
{
  kdl_reflect_decl_empty(LinkTarget);
};

struct String
{
  std::optional<std::string> defaultValue = std::nullopt;

  kdl_reflect_decl(String, defaultValue);
};

struct Boolean
{
  std::optional<bool> defaultValue = std::nullopt;

  kdl_reflect_decl(Boolean, defaultValue);
};

struct Integer
{
  std::optional<int> defaultValue = std::nullopt;

  kdl_reflect_decl(Integer, defaultValue);
};

struct Float
{
  std::optional<float> defaultValue = std::nullopt;

  kdl_reflect_decl(Float, defaultValue);
};

struct ChoiceOption
{
  std::string value;
  std::string description;

  kdl_reflect_decl(ChoiceOption, value, description);
};

struct Choice
{
  std::vector<ChoiceOption> options;
  std::optional<std::string> defaultValue = std::nullopt;

  kdl_reflect_decl(Choice, options, defaultValue);
};

struct Flag
{
  int value;
  std::string shortDescription;
  std::string longDescription;

  kdl_reflect_decl(Flag, value, shortDescription, longDescription);
};

struct Flags
{
  std::vector<Flag> flags;
  int defaultValue = 0;

  const Flag* flag(int flagValue) const;
  bool isDefault(int flagValue) const;

  kdl_reflect_decl(Flags, defaultValue, flags);
};

struct Origin
{
  std::optional<std::string> defaultValue = std::nullopt;

  kdl_reflect_decl(Origin, defaultValue);
};

enum class IOParameterType
{
  Void,
  String,
  Integer,
  Float,
  Boolean,
  EHandle,
};

std::ostream& operator<<(std::ostream& lhs, IOParameterType rhs);

struct Input
{
  IOParameterType parameterType;

  kdl_reflect_decl(Input, parameterType);
};

struct Output
{
  IOParameterType parameterType;

  kdl_reflect_decl(Output, parameterType);
};

template <typename T>
struct Color
{
  using Type = T;

  std::optional<std::string> defaultValue = std::nullopt;

  kdl_reflect_inline(Color, defaultValue);
};

struct Unknown
{
  std::optional<std::string> defaultValue = std::nullopt;

  kdl_reflect_decl(Unknown, defaultValue);
};

} // namespace PropertyValueTypes

using PropertyValueType = std::variant<
  PropertyValueTypes::LinkTarget,
  PropertyValueTypes::LinkSource,
  PropertyValueTypes::String,
  PropertyValueTypes::Boolean,
  PropertyValueTypes::Integer,
  PropertyValueTypes::Float,
  PropertyValueTypes::Choice,
  PropertyValueTypes::Flags,
  PropertyValueTypes::Origin,
  PropertyValueTypes::Input,
  PropertyValueTypes::Output,
  PropertyValueTypes::Color<RgbF>,
  PropertyValueTypes::Color<RgbB>,
  PropertyValueTypes::Color<Rgb>,
  PropertyValueTypes::Unknown>;

std::ostream& operator<<(std::ostream& lhs, const PropertyValueType& rhs);

struct PropertyDefinition
{
  std::string key;
  PropertyValueType valueType;
  std::string shortDescription;
  std::string longDescription;
  bool readOnly = false;

  static std::optional<std::string> defaultValue(const PropertyDefinition& definition);

  kdl_reflect_decl(
    PropertyDefinition, key, valueType, shortDescription, longDescription, readOnly);
};

} // namespace tb::mdl
