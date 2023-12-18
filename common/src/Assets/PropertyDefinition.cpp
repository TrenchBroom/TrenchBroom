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

#include "PropertyDefinition.h"

#include "Macros.h"

#include "kdl/string_utils.h"
#include <kdl/reflection_impl.h>

#include <memory>
#include <string>

namespace TrenchBroom::Assets
{

PropertyDefinition::PropertyDefinition(
  std::string key,
  const PropertyDefinitionType type,
  std::string shortDescription,
  std::string longDescription,
  const bool readOnly)
  : m_key{std::move(key)}
  , m_type{type}
  , m_shortDescription{std::move(shortDescription)}
  , m_longDescription{std::move(longDescription)}
  , m_readOnly{readOnly}
{
}

PropertyDefinition::~PropertyDefinition() = default;

const std::string& PropertyDefinition::key() const
{
  return m_key;
}

PropertyDefinitionType PropertyDefinition::type() const
{
  return m_type;
}

const std::string& PropertyDefinition::shortDescription() const
{
  return m_shortDescription;
}

const std::string& PropertyDefinition::longDescription() const
{
  return m_longDescription;
}

bool PropertyDefinition::readOnly() const
{
  return m_readOnly;
}

bool PropertyDefinition::equals(const PropertyDefinition* other) const
{
  ensure(other != nullptr, "other is null");
  return type() == other->type() && key() == other->key() && doEquals(other);
}

bool PropertyDefinition::doEquals(const PropertyDefinition* /* other */) const
{
  return true;
}

std::string PropertyDefinition::defaultValue(const PropertyDefinition& definition)
{
  switch (definition.type())
  {
  case PropertyDefinitionType::StringProperty: {
    const auto& stringDef = static_cast<const StringPropertyDefinition&>(definition);
    return stringDef.hasDefaultValue() ? stringDef.defaultValue() : "";
  }
  case PropertyDefinitionType::BooleanProperty: {
    const auto& boolDef = static_cast<const BooleanPropertyDefinition&>(definition);
    return boolDef.hasDefaultValue() ? kdl::str_to_string(boolDef.defaultValue()) : "";
  }
  case PropertyDefinitionType::IntegerProperty: {
    const auto& intDef = static_cast<const IntegerPropertyDefinition&>(definition);
    return intDef.hasDefaultValue() ? kdl::str_to_string(intDef.defaultValue()) : "";
  }
  case PropertyDefinitionType::FloatProperty: {
    const auto& floatDef = static_cast<const FloatPropertyDefinition&>(definition);
    return floatDef.hasDefaultValue() ? kdl::str_to_string(floatDef.defaultValue()) : "";
  }
  case PropertyDefinitionType::ChoiceProperty: {
    const auto& choiceDef = static_cast<const ChoicePropertyDefinition&>(definition);
    return choiceDef.hasDefaultValue() ? kdl::str_to_string(choiceDef.defaultValue())
                                       : "";
  }
  case PropertyDefinitionType::FlagsProperty: {
    const auto& flagsDef = static_cast<const FlagsPropertyDefinition&>(definition);
    return kdl::str_to_string(flagsDef.defaultValue());
  }
  case PropertyDefinitionType::TargetSourceProperty:
  case PropertyDefinitionType::TargetDestinationProperty:
    return "";
    switchDefault();
  }
}

std::unique_ptr<PropertyDefinition> PropertyDefinition::clone(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly) const
{
  return doClone(
    std::move(key), std::move(shortDescription), std::move(longDescription), readOnly);
}

std::unique_ptr<PropertyDefinition> PropertyDefinition::doClone(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly) const
{
  return std::make_unique<PropertyDefinition>(
    std::move(key),
    type(),
    std::move(shortDescription),
    std::move(longDescription),
    readOnly);
}

StringPropertyDefinition::StringPropertyDefinition(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  const bool readOnly,
  std::optional<std::string> defaultValue)
  : PropertyDefinitionWithDefaultValue{
    std::move(key),
    PropertyDefinitionType::StringProperty,
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    std::move(defaultValue)}
{
}

std::unique_ptr<PropertyDefinition> StringPropertyDefinition::doClone(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly) const
{
  return std::make_unique<StringPropertyDefinition>(
    std::move(key),
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    m_defaultValue);
}

BooleanPropertyDefinition::BooleanPropertyDefinition(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  const bool readOnly,
  std::optional<bool> defaultValue)
  : PropertyDefinitionWithDefaultValue{
    std::move(key),
    PropertyDefinitionType::BooleanProperty,
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    std::move(defaultValue)}
{
}

std::unique_ptr<PropertyDefinition> BooleanPropertyDefinition::doClone(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly) const
{
  return std::make_unique<BooleanPropertyDefinition>(
    std::move(key),
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    m_defaultValue);
}

IntegerPropertyDefinition::IntegerPropertyDefinition(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  const bool readOnly,
  std::optional<int> defaultValue)
  : PropertyDefinitionWithDefaultValue{
    std::move(key),
    PropertyDefinitionType::IntegerProperty,
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    std::move(defaultValue)}
{
}

std::unique_ptr<PropertyDefinition> IntegerPropertyDefinition::doClone(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly) const
{
  return std::make_unique<IntegerPropertyDefinition>(
    std::move(key),
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    m_defaultValue);
}

FloatPropertyDefinition::FloatPropertyDefinition(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  const bool readOnly,
  std::optional<float> defaultValue)
  : PropertyDefinitionWithDefaultValue{
    std::move(key),
    PropertyDefinitionType::FloatProperty,
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    std::move(defaultValue)}
{
}

std::unique_ptr<PropertyDefinition> FloatPropertyDefinition::doClone(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly) const
{
  return std::make_unique<FloatPropertyDefinition>(
    std::move(key),
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    m_defaultValue);
}

ChoicePropertyOption::ChoicePropertyOption(std::string value, std::string description)
  : m_value{std::move(value)}
  , m_description{std::move(description)}
{
}

const std::string& ChoicePropertyOption::value() const
{
  return m_value;
}

const std::string& ChoicePropertyOption::description() const
{
  return m_description;
}

kdl_reflect_impl(ChoicePropertyOption);

ChoicePropertyDefinition::ChoicePropertyDefinition(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  ChoicePropertyOption::List options,
  const bool readOnly,
  std::optional<std::string> defaultValue)
  : PropertyDefinitionWithDefaultValue{std::move(key), PropertyDefinitionType::ChoiceProperty, std::move(shortDescription), std::move(longDescription), readOnly, std::move(defaultValue)}
  , m_options{std::move(options)}
{
}

const ChoicePropertyOption::List& ChoicePropertyDefinition::options() const
{
  return m_options;
}

bool ChoicePropertyDefinition::doEquals(const PropertyDefinition* other) const
{
  return options() == static_cast<const ChoicePropertyDefinition*>(other)->options();
}

std::unique_ptr<PropertyDefinition> ChoicePropertyDefinition::doClone(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly) const
{
  return std::make_unique<ChoicePropertyDefinition>(
    std::move(key),
    std::move(shortDescription),
    std::move(longDescription),
    options(),
    readOnly,
    m_defaultValue);
}

FlagsPropertyOption::FlagsPropertyOption(
  const int value,
  std::string shortDescription,
  std::string longDescription,
  const bool isDefault)
  : m_value{value}
  , m_shortDescription{std::move(shortDescription)}
  , m_longDescription{std::move(longDescription)}
  , m_isDefault(isDefault)
{
}

int FlagsPropertyOption::value() const
{
  return m_value;
}

const std::string& FlagsPropertyOption::shortDescription() const
{
  return m_shortDescription;
}

const std::string& FlagsPropertyOption::longDescription() const
{
  return m_longDescription;
}

bool FlagsPropertyOption::isDefault() const
{
  return m_isDefault;
}

kdl_reflect_impl(FlagsPropertyOption);

FlagsPropertyDefinition::FlagsPropertyDefinition(std::string key)
  : PropertyDefinition{
    std::move(key), PropertyDefinitionType::FlagsProperty, "", "", false}
{
}

int FlagsPropertyDefinition::defaultValue() const
{
  int value = 0;
  for (const auto& option : m_options)
  {
    if (option.isDefault())
    {
      value = value | option.value();
    }
  }
  return value;
}

const FlagsPropertyOption::List& FlagsPropertyDefinition::options() const
{
  return m_options;
}

const FlagsPropertyOption* FlagsPropertyDefinition::option(const int value) const
{
  for (const auto& option : m_options)
  {
    if (option.value() == value)
    {
      return &option;
    }
  }
  return nullptr;
}

void FlagsPropertyDefinition::addOption(
  const int value,
  std::string shortDescription,
  std::string longDescription,
  const bool isDefault)
{
  m_options.emplace_back(
    value, std::move(shortDescription), std::move(longDescription), isDefault);
}

bool FlagsPropertyDefinition::doEquals(const PropertyDefinition* other) const
{
  return options() == static_cast<const FlagsPropertyDefinition*>(other)->options();
}

std::unique_ptr<PropertyDefinition> FlagsPropertyDefinition::doClone(
  std::string key,
  std::string /* shortDescription */,
  std::string /* longDescription */,
  bool /* readOnly */) const
{
  auto result = std::make_unique<FlagsPropertyDefinition>(std::move(key));
  for (const auto& option : options())
  {
    result->addOption(
      option.value(),
      option.shortDescription(),
      option.longDescription(),
      option.isDefault());
  }
  return result;
}

UnknownPropertyDefinition::UnknownPropertyDefinition(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  const bool readOnly,
  std::optional<std::string> defaultValue)
  : StringPropertyDefinition{
    std::move(key),
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    std::move(defaultValue)}
{
}

std::unique_ptr<PropertyDefinition> UnknownPropertyDefinition::doClone(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly) const
{
  return std::make_unique<UnknownPropertyDefinition>(
    std::move(key),
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    m_defaultValue);
}

} // namespace TrenchBroom::Assets
