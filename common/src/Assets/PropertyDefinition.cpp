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

PropertyDefinition::PropertyDefinition(
  std::string key,
  PropertyTypeVariant type,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly,
  PropertyDefaultValueVariant defaultValue)
  : m_key{std::move(key)}
  , m_type{type}
  , m_shortDescription{std::move(shortDescription)}
  , m_longDescription{std::move(longDescription)}
  , m_readOnly{readOnly}
  , m_defaultValue{std::move(defaultValue)}
{
}

PropertyDefinition::~PropertyDefinition() = default;

const std::string& PropertyDefinition::key() const
{
  return m_key;
}

PropertyTypeVariant PropertyDefinition::type() const
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
const std::optional<IODirection> PropertyDefinition::ioDirection() const
{
  return m_ioDirection;
}

bool PropertyDefinition::readOnly() const
{
  return m_readOnly;
}
bool PropertyDefinition::isIO() const
{
  return m_type.index() == 1;
}

PropertyDefaultValueVariant PropertyDefinition::defaultValue()
{
  return m_defaultValue;
}
void PropertyDefinition::setOptions(std::vector<PropertyOptionVariant> options)
{
  m_options = options;
}
void PropertyDefinition::setIODirection(IODirection direction)
{
  m_ioDirection = direction;
}


bool PropertyDefinition::equals(const PropertyDefinition* other) const
{
  ensure(other != nullptr, "other is null");
  return (type() == other->type() && key() == other->key());
}

std::unique_ptr<PropertyDefinition> PropertyDefinition::clone(
  std::string key,
  std::string shortDescription,
  std::string longDescription,
  bool readOnly)
{
  return std::make_unique<PropertyDefinition>(
    std::move(key),
    type(),
    std::move(shortDescription),
    std::move(longDescription),
    readOnly,
    defaultValue());
}

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

const std::string& FlagOption::shortDescription() const
{
  return m_shortDescription;
}

const std::string& FlagOption::longDescription() const
{
  return m_longDescription;
}

int FlagOption::value() const
{
  return m_value;
}

bool FlagOption::defaultState() const
{
  return m_defaultState;
}


} // namespace TrenchBroom::Assets
