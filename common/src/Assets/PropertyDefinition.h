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
#include <utility>
#include <vector>

namespace TrenchBroom::Assets
{

class ChoicePropertyOption;
class FlagsPropertyOption;
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

using NoValue = char;
using NoDefaultValue = char;

using kdl::ct_map::ct_pair;
using PropertyTypeMap = kdl::ct_map::ct_map<
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::BooleanProperty, bool>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::IntegerProperty, int>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::FloatProperty, float>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::StringProperty, std::string>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::UnknownProperty, std::string>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::FlagsProperty, NoDefaultValue>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::ChoiceProperty, std::string>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::OutputProperty, NoValue>,
  ct_pair<PropertyDefinitionType, PropertyDefinitionType::InputProperty, NoValue>,
  ct_pair<
    PropertyDefinitionType,
    PropertyDefinitionType::TargetDestinationProperty,
    std::string>,
  ct_pair<
    PropertyDefinitionType,
    PropertyDefinitionType::TargetSourceProperty,
    std::string>>;

class ChoicePropertyOption
{
public:
  using List = std::vector<ChoicePropertyOption>;

private:
  std::string m_value;
  std::string m_description;

public:
  ChoicePropertyOption(std::string value, std::string description);
  ~ChoicePropertyOption();

  const std::string& value() const;
  const std::string& description() const;

  kdl_reflect_decl(ChoicePropertyOption, m_value, m_description);
};

class FlagsPropertyOption
{
public:
  using List = std::vector<FlagsPropertyOption>;

private:
  int m_value;
  std::string m_shortDescription;
  std::string m_longDescription;
  bool m_defaultState;

public:
  FlagsPropertyOption(
    int value,
    std::string shortDescription,
    std::string longDescription,
    bool defaultState);
  ~FlagsPropertyOption();

  int value() const;
  const std::string& shortDescription() const;
  const std::string& longDescription() const;
  bool defaultState() const;

  kdl_reflect_decl(
    FlagsPropertyOption, m_shortDescription, m_longDescription, m_defaultState);
};


class PropertyDefinition
{
public:
  using PropertyType = std::string;
  using OptionType = std::string;
  using OptionsType = std::vector<OptionType>;

private:
  std::string m_key;
  std::string m_shortDescription;
  std::string m_longDescription;
  bool m_readOnly;

  std::optional<PropertyType> m_defaultValue;
  std::optional<OptionsType> m_options;

protected:
  PropertyDefinitionType m_type;
  std::optional<IOType> m_ioType;

public:
  PropertyDefinition(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly,
    std::optional<PropertyType> defaultValue = {});
  PropertyDefinition(
    std::string key,
    PropertyDefinitionType type,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly,
    std::optional<PropertyType> defaultValue,
    std::optional<IOType> ioType);
  virtual ~PropertyDefinition();

  const std::string& key() const;
  virtual PropertyDefinitionType type() const;
  const std::string& shortDescription() const;
  const std::string& longDescription() const;
  bool readOnly() const;

  static std::string defaultValue(const PropertyDefinition& definition);
  virtual bool hasDefaultValue() const { return m_defaultValue.has_value(); };

  std::optional<OptionsType> options() const;
  std::optional<IOType> ioType() const;

  bool equals(const PropertyDefinition* other) const;
  std::unique_ptr<PropertyDefinition> clone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly) const;

private:
  virtual std::unique_ptr<PropertyDefinition> doClone(
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
    FlagsPropertyOption,
    std::conditional_t<
      P == PropertyDefinitionType::ChoiceProperty,
      ChoicePropertyOption,
      NoValue>>;
  using OptionsType = std::conditional_t<
    std::is_same_v<OptionType, NoValue>,
    std::string,
    std::vector<OptionType>>;
  using DefaultValueType =
    std::conditional_t<P == PropertyDefinitionType::FlagsProperty, int, PropertyType>;

private:
  std::optional<DefaultValueType> m_defaultValue;
  std::optional<OptionsType> m_options;


public:
  explicit PropertyDefinitionT(
    std::string key,
    std::string shortDescription = std::string{},
    std::string longDescription = std::string{},
    bool readOnly = false,
    std::optional<DefaultValueType> defaultValue = {})
    : PropertyDefinition(key, shortDescription, longDescription, readOnly)
    , m_defaultValue{std::move(defaultValue)}
    , m_options{OptionsType{}}
  {
    m_type = P;
  }

  PropertyDefinitionT(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly,
    IOType ioType)
    : PropertyDefinition(
        key, P, shortDescription, longDescription, readOnly, std::nullopt, ioType)
    , m_defaultValue{std::nullopt}
    , m_options{OptionsType{}} {};

  ~PropertyDefinitionT() override = default;

  PropertyDefinitionType type() const override { return m_type; };
  std::optional<DefaultValueType> defaultValue() const { return m_defaultValue; };


  bool hasDefaultValue() const override { return m_defaultValue.has_value(); };

  const std::optional<OptionsType> options() const { return m_options; };
  const OptionType* option(const int value) const { return nullptr; };
  void setOptions(OptionsType options) { m_options = options; };
  void addOption(const OptionType* new_item) { return; };

private:
  std::unique_ptr<PropertyDefinition> doClone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly) const override
  {
    return std::make_unique<PropertyDefinitionT<P>>(
      key, shortDescription, longDescription, readOnly);
  };

  bool doEquals(const PropertyDefinitionT<P>* other) const { return true; };
};

inline std::ostream& operator<<(std::ostream& lhs, const ChoicePropertyOption& rhs)
{
  lhs << rhs.value() << " " << rhs.description();
  return lhs;
}
inline std::ostream& operator<<(std::ostream& lhs, const FlagsPropertyOption& rhs)
{
  lhs << rhs.value() << rhs.shortDescription();
  return lhs;
}


template <>
inline const FlagsPropertyDefinition::OptionType* FlagsPropertyDefinition::option(
  const int value) const
{
  if (m_options.has_value() == false)
  {
    return nullptr;
  }
  for (const auto& option : *m_options)
  {
    if (option.value() == value)
    {
      return &option;
    }
  }

  return nullptr;
}

template <>
inline std::optional<FlagsPropertyDefinition::DefaultValueType> FlagsPropertyDefinition::
  defaultValue() const
{
  int value = 0;
  for (const auto& option : *m_options)
  {
    if (option.defaultState())
    {
      value |= option.value();
    }
  }

  return value;
}

template <>
inline void FlagsPropertyDefinition::addOption(const OptionType* new_item)
{
  m_options->emplace_back(
    new_item->value(),
    new_item->shortDescription(),
    new_item->longDescription(),
    new_item->defaultState());
}

template <>
inline std::unique_ptr<PropertyDefinition> FlagsPropertyDefinition::doClone(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly) const
{
  auto cloneProperty = std::make_unique<FlagsPropertyDefinition>(
    key, shortDescription, longDescription, readOnly);
  cloneProperty->setOptions(*m_options);
  return cloneProperty;
}

template <>
inline void ChoicePropertyDefinition::addOption(const OptionType* new_item)
{
  m_options->emplace_back(new_item->value(), new_item->description());
}

template <>
inline bool ChoicePropertyDefinition::doEquals(const PropertyDefinitionT* other) const
{
  if (options().has_value() && other->options().has_value())
  {
    const auto this_val = options().value();
    const auto other_val = other->options().value();
    return this_val == other_val;
  }
  else
  {
    return options().has_value() == other->options().has_value();
  }
}

template <>
inline std::unique_ptr<PropertyDefinition> ChoicePropertyDefinition::doClone(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly) const
{
  auto cloneProperty = std::make_unique<ChoicePropertyDefinition>(
    key, shortDescription, longDescription, readOnly);
  cloneProperty->setOptions(*m_options);
  return cloneProperty;
}


} // namespace TrenchBroom::Assets
