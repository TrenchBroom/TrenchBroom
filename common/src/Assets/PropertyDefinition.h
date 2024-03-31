/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Ensure.h"

#include "kdl/reflection_decl.h"

#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace TrenchBroom::Assets
{
class ChoiceOption;
class FlagOption;

enum class PropertyDefinitionType
{
  TargetSourceProperty,
  TargetDestinationProperty,
  StringProperty,
  BooleanProperty,
  IntegerProperty,
  FloatProperty,
  ChoiceProperty,
  FlagsProperty,
  UnknownProperty
};

enum class IOType
{
  Void,
  Bool,
  Float,
  Integer,
  String,
  Unknown
};

enum class IODirection
{
  Input,
  Output
};

using PropertyTypeVariant = std::variant<PropertyDefinitionType, IOType>;
using PropertyDefaultValueVariant =
  std::variant<std::monostate, float, int, bool, std::string, ChoiceOption>;
using PropertyOptionVariant = std::variant<std::monostate, ChoiceOption, FlagOption>;


class ChoiceOption
{
public:
  using List = std::vector<ChoiceOption>;

private:
  std::string m_value;
  std::string m_description;

public:
  ChoiceOption(std::string value, std::string description);
  ~ChoiceOption();
  const std::string& value() const;
  const std::string& description() const;

  kdl_reflect_decl(ChoiceOption, m_value, m_description);
};

class FlagOption
{
public:
  using List = std::vector<PropertyOptionVariant>;

private:
  int m_value;
  std::string m_shortDescription;
  std::string m_longDescription;
  bool m_defaultState;

public:
  FlagOption(
    int value,
    std::string shortDescription,
    std::string longDescription,
    bool defaultState);
  ~FlagOption();

  int value() const;
  const std::string& shortDescription() const;
  const std::string& longDescription() const;
  bool defaultState() const;

  kdl_reflect_decl(FlagOption, m_shortDescription, m_longDescription, m_defaultState);
};

class PropertyDefinition
{
private:
  std::string m_key;
  PropertyTypeVariant m_type;
  std::string m_shortDescription;
  std::string m_longDescription;
  bool m_readOnly;
  PropertyDefaultValueVariant m_defaultValue = std::monostate{};
  std::optional<IODirection> m_ioDirection;

  std::vector<PropertyOptionVariant> m_options;

public:
  PropertyDefinition(
    std::string key,
    PropertyTypeVariant type,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly,
    PropertyDefaultValueVariant defaultValue);
  virtual ~PropertyDefinition();

  const std::string& key() const;
  PropertyTypeVariant type() const;
  const std::string& shortDescription() const;
  const std::string& longDescription() const;
  const std::optional<IODirection> ioDirection() const;

  bool readOnly() const;
  bool isIO() const;
  PropertyDefaultValueVariant defaultValue();

  void setOptions(std::vector<PropertyOptionVariant> options);
  void setIODirection(IODirection direction);

  bool equals(const PropertyDefinition* other) const;

  std::unique_ptr<PropertyDefinition> clone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly);
};


} // namespace TrenchBroom::Assets
