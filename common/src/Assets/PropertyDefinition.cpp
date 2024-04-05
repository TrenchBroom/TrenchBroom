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

ChoiceOption::ChoiceOption(std::string value, std::string description)
  : m_value{std::move(value)}
  , m_description{std::move(description)}
{
}
ChoiceOption::~ChoiceOption() = default;


const std::string& ChoiceOption::value() const
{
  return m_value;
}

const std::string& ChoiceOption::description() const
{
  return m_description;
}


FlagOption::FlagOption(
  int value, std::string shortDescription, std::string longDescription, bool defaultState)
  : m_value{value}
  , m_shortDescription{std::move(shortDescription)}
  , m_longDescription{std::move(longDescription)}
  , m_defaultState{defaultState}
{
}
FlagOption::~FlagOption() = default;

int FlagOption::value() const
{
  return m_value;
}
const std::string& FlagOption::shortDescription() const
{
  return m_shortDescription;
}

const std::string& FlagOption::longDescription() const
{
  return m_longDescription;
}

bool FlagOption::defaultState() const
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
  , m_type{PropertyDefinitionType::UnknownProperty}
  , m_shortDescription{std::move(shortDescription)}
  , m_longDescription{std::move(longDescription)}
  , m_readOnly{readOnly}
  , m_defaultValue{std::move(defaultValue)}
  , m_ioType{std::nullopt}
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
const std::optional<PropertyDefinition::PropertyType> PropertyDefinition::defaultValue()
  const
{
  return m_defaultValue;
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


} // namespace TrenchBroom::Assets
