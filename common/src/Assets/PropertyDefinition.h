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

#include "kdl/ct_map.h"
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
class PropertyDefinition;

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
  OutputProperty,
  InputProperty,
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

template <PropertyDefinitionType T>
class PropertyDefinitionT;

using FlagsPropertyDefinition =
  PropertyDefinitionT<PropertyDefinitionType::FlagsProperty>;
using ChoicePropertyDefinition =
  PropertyDefinitionT<PropertyDefinitionType::ChoiceProperty>;
using FloatPropertyDefinition =
  PropertyDefinitionT<PropertyDefinitionType::FloatProperty>;
using IntegerPropertyDefinition =
  PropertyDefinitionT<PropertyDefinitionType::IntegerProperty>;
using StringPropertyDefinition =
  PropertyDefinitionT<PropertyDefinitionType::StringProperty>;
using UnknownPropertyDefinition =
  PropertyDefinitionT<PropertyDefinitionType::UnknownProperty>;
using BooleanPropertyDefinition =
  PropertyDefinitionT<PropertyDefinitionType::BooleanProperty>;


using kdl::ct_map::ct_pair;
using PropertyTypeMap = kdl::ct_map::ct_map<
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::BooleanProperty, bool>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::IntegerProperty, int>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::FloatProperty, float>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::StringProperty, std::string>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::UnknownProperty, std::string>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::FlagsProperty, char>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::ChoiceProperty, std::string>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::OutputProperty, char>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::InputProperty, char>,
  ct_pair<
    PropertyDefinitionType,
    PropertyDefinitionType::TargetDestinationProperty,
    char>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::TargetSourceProperty, char>>;

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
  using List = std::vector<FlagOption>;

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
public:
  using PropertyType = std::string;
  using OptionType = std::string;
  using OptionsType = std::vector<OptionType>;

private:
  std::string m_key;
  PropertyDefinitionType m_type;
  std::string m_shortDescription;
  std::string m_longDescription;
  bool m_readOnly;

  std::optional<PropertyType> m_defaultValue;
  std::optional<OptionsType> m_options;

protected:
  std::optional<IOType> m_ioType;

public:
  PropertyDefinition(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly,
    std::optional<PropertyType> defaultValue = {});
  virtual ~PropertyDefinition();

  const std::string& key() const;
  PropertyDefinitionType type() const;
  const std::string& shortDescription() const;
  const std::string& longDescription() const;
  bool readOnly() const;

  const std::optional<PropertyType> defaultValue() const;
  bool hasDefaultValue() const { return m_defaultValue.has_value(); };

  std::optional<OptionsType> options() const;
  std::optional<IOType> ioType() const;

  bool equals(const PropertyDefinition* other) const;
  std::unique_ptr<PropertyDefinition> clone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly) const;

private:
  std::unique_ptr<PropertyDefinition> doClone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly) const;

  bool doEquals(const PropertyDefinition* other) const;
};

template <PropertyDefinitionType P>
class PropertyDefinitionT : public PropertyDefinition
{
public:
  using PropertyType = PropertyTypeMap::find_type<PropertyDefinitionType, P>;
  using OptionType = std::conditional_t<
    P == PropertyDefinitionType::FlagsProperty,
    FlagOption,
    std::conditional_t<P == PropertyDefinitionType::ChoiceProperty, ChoiceOption, char>>;
  using OptionsType = std::
    conditional_t<std::is_same_v<OptionType, char>, std::string, std::vector<OptionType>>;

private:
  std::optional<PropertyType> m_defaultValue;
  std::optional<OptionsType> m_options;


public:
  PropertyDefinitionT(
    std::string key,
    std::string shortDescription = std::string{},
    std::string longDescription = std::string{},
    bool readOnly = false,
    std::optional<PropertyType> defaultValue = {});

  PropertyDefinitionT(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly,
    IOType ioType);

  ~PropertyDefinitionT() override;

  std::optional<PropertyType> defaultValue() const;

  const std::optional<OptionsType> options() const;
  const OptionType* option(const int value) const;
  void setOptions(OptionsType options);
  void addOption(const OptionType* new_item);

private:
  std::unique_ptr<PropertyDefinition> doClone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly) const;
  bool doEquals(const PropertyDefinitionT<P>* other) const;
};

inline std::ostream& operator<<(std::ostream& lhs, const ChoiceOption& rhs)
{
  lhs << rhs.value() << " " << rhs.description();
  return lhs;
}
inline std::ostream& operator<<(std::ostream& lhs, const FlagOption& rhs)
{
  lhs << rhs.value() << rhs.shortDescription();
  return lhs;
}

template <PropertyDefinitionType P>
PropertyDefinitionT<P>::PropertyDefinitionT(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly,
  std::optional<PropertyType> defaultValue)
  : PropertyDefinition(key, shortDescription, longDescription, readOnly)
  , m_defaultValue{defaultValue}
  , m_options()
{
}

template <PropertyDefinitionType P>
PropertyDefinitionT<P>::PropertyDefinitionT(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly,
  IOType ioType)
  : PropertyDefinition(key, shortDescription, longDescription, readOnly)
  , m_defaultValue{std::nullopt}
  , m_options{std::nullopt}
{
  m_ioType = ioType;
}

template <PropertyDefinitionType P>
PropertyDefinitionT<P>::~PropertyDefinitionT() = default;

template <PropertyDefinitionType P>
const std::optional<typename PropertyDefinitionT<P>::OptionsType> PropertyDefinitionT<
  P>::options() const
{
  return m_options;
}

template <PropertyDefinitionType P>
const typename PropertyDefinitionT<P>::OptionType* PropertyDefinitionT<P>::option(
  const int value) const
{
  return nullptr;
}

template <PropertyDefinitionType P>
std::optional<typename PropertyDefinitionT<P>::PropertyType> PropertyDefinitionT<
  P>::defaultValue() const
{
  return m_defaultValue;
}

template <PropertyDefinitionType P>
void PropertyDefinitionT<P>::setOptions(OptionsType options)
{
  m_options = options;
}

template <PropertyDefinitionType P>
void PropertyDefinitionT<P>::addOption(const OptionType* new_item)
{
  return;
}

template <PropertyDefinitionType P>
std::unique_ptr<PropertyDefinition> PropertyDefinitionT<P>::doClone(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly) const
{
  return std::make_unique<PropertyDefinitionT<P>>(
    key, shortDescription, longDescription, readOnly);
}

template <PropertyDefinitionType P>
bool PropertyDefinitionT<P>::doEquals(const PropertyDefinitionT<P>* other) const
{
  return true;
}

template <>
inline const typename FlagsPropertyDefinition::OptionType* FlagsPropertyDefinition::
  option(const int value) const
{
  const auto option_list = m_options.value_or(OptionsType{});
  for (const auto& option : option_list)
  {
    if (option.value() == value)
    {
      return &option;
    }
  }

  return nullptr;
}

template <>
inline void PropertyDefinitionT<PropertyDefinitionType::FlagsProperty>::addOption(
  const OptionType* new_item)
{
  m_options->emplace_back(
    new_item->value(),
    new_item->shortDescription(),
    new_item->longDescription(),
    new_item->defaultState());
}

template <>
inline void ChoicePropertyDefinition::addOption(const OptionType* new_item);

template <>
inline bool ChoicePropertyDefinition::doEquals(const PropertyDefinitionT* other) const;

} // namespace TrenchBroom::Assets
