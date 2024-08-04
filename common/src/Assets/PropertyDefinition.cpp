/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU Generals Public License as published by
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

#include "kdl/reflection_impl.h"
#include "kdl/string_utils.h"

#include <memory>
#include <string>
#include <utility>

namespace TrenchBroom::Assets
{


ChoicePropertyOption::ChoicePropertyOption(std::string value, std::string description)
  : m_value{std::move(value)}
  , m_description{std::move(description)}
{
}
ChoicePropertyOption::~ChoicePropertyOption() = default;

const std::string& ChoicePropertyOption::value() const
{
  return m_value;
}

const std::string& ChoicePropertyOption::description() const
{
  return m_description;
}


FlagsPropertyOption::FlagsPropertyOption(
  int value, std::string shortDescription, std::string longDescription, bool defaultState)
  : m_value{value}
  , m_shortDescription{std::move(shortDescription)}
  , m_longDescription{std::move(longDescription)}
  , m_defaultState{defaultState}
{
}
FlagsPropertyOption::~FlagsPropertyOption() = default;

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

bool FlagsPropertyOption::defaultState() const
{
  return m_defaultState;
}


PropertyDefinition::PropertyDefinition(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly,
  std::optional<PropertyType> defaultValue)
  : m_key{std::move(key)}
  , m_shortDescription{std::move(shortDescription)}
  , m_longDescription{std::move(longDescription)}
  , m_readOnly{readOnly}
  , m_defaultValue{std::move(defaultValue)}
  , m_type{PropertyDefinitionType::UnknownProperty}
  , m_ioType{std::nullopt}
{
}
PropertyDefinition::PropertyDefinition(
  std::string key,
  PropertyDefinitionType type,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly,
  std::optional<PropertyType> defaultValue,
  std::optional<IOType> ioType)
  : m_key{std::move(key)}
  , m_shortDescription{std::move(shortDescription)}
  , m_longDescription{std::move(longDescription)}
  , m_readOnly{readOnly}
  , m_defaultValue{std::move(defaultValue)}
  , m_type{std::move(type)}
  , m_ioType{std::move(ioType)}
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

std::string PropertyDefinition::defaultValue(const PropertyDefinition& definition)
{
  switch (definition.type())
  {
  case PropertyDefinitionType::StringProperty: {
    const auto& stringDef = static_cast<const StringPropertyDefinition&>(definition);
    return stringDef.defaultValue().value_or("");
  }
  case PropertyDefinitionType::BooleanProperty: {
    const auto& boolDef = static_cast<const BooleanPropertyDefinition&>(definition);
    return boolDef.hasDefaultValue() ? kdl::str_to_string(boolDef.defaultValue().value())
                                     : "";
  }
  case PropertyDefinitionType::IntegerProperty: {
    const auto& intDef = static_cast<const IntegerPropertyDefinition&>(definition);
    return intDef.hasDefaultValue() ? kdl::str_to_string(intDef.defaultValue().value())
                                    : "";
  }
  case PropertyDefinitionType::FloatProperty: {
    const auto& floatDef = static_cast<const FloatPropertyDefinition&>(definition);
    return floatDef.hasDefaultValue()
             ? kdl::str_to_string(floatDef.defaultValue().value())
             : "";
  }
  case PropertyDefinitionType::ChoiceProperty: {
    const auto& choiceDef = static_cast<const ChoicePropertyDefinition&>(definition);
    return choiceDef.hasDefaultValue()
             ? kdl::str_to_string(choiceDef.defaultValue().value())
             : "";
  }
  case PropertyDefinitionType::FlagsProperty: {
    const auto& flagsDef = static_cast<const FlagsPropertyDefinition&>(definition);
    return kdl::str_to_string(flagsDef.defaultValue().value());
  }
  case PropertyDefinitionType::TargetSourceProperty:
  case PropertyDefinitionType::TargetDestinationProperty:
    return "";
  case PropertyDefinitionType::InputProperty:
  case PropertyDefinitionType::OutputProperty:
    return "";
    switchDefault();
  }
}

std::optional<PropertyDefinition::OptionsType> PropertyDefinition::options() const
{
  return m_options;
}

std::optional<IOType> PropertyDefinition::ioType() const
{
  return m_ioType;
}

bool PropertyDefinition::equals(const PropertyDefinition* other) const
{
  ensure(other != nullptr, "other is null");
  return type() == other->type() && key() == other->key() && doEquals(other);
}

bool PropertyDefinition::doEquals(const PropertyDefinition* other) const
{
  return true;
}

std::unique_ptr<PropertyDefinition> PropertyDefinition::clone(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly) const
{
  return doClone(key, shortDescription, longDescription, readOnly);
}

std::unique_ptr<PropertyDefinition> PropertyDefinition::doClone(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly) const
{
  return std::make_unique<PropertyDefinition>(
    std::move(key), std::move(shortDescription), std::move(longDescription), readOnly);
}


} // namespace TrenchBroom::Assets
