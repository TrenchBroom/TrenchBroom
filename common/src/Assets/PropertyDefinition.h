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

#include <kdl/reflection_decl.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom::Assets
{

enum class PropertyDefinitionType
{
  TargetSourceProperty,
  TargetDestinationProperty,
  StringProperty,
  BooleanProperty,
  IntegerProperty,
  FloatProperty,
  ChoiceProperty,
  FlagsProperty
};

class PropertyDefinition
{
public:
private:
  std::string m_key;
  PropertyDefinitionType m_type;
  std::string m_shortDescription;
  std::string m_longDescription;
  bool m_readOnly;

public:
  PropertyDefinition(
    std::string key,
    PropertyDefinitionType type,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly);
  virtual ~PropertyDefinition();

  const std::string& key() const;
  PropertyDefinitionType type() const;
  const std::string& shortDescription() const;
  const std::string& longDescription() const;

  bool readOnly() const;

  bool equals(const PropertyDefinition* other) const;

  static std::string defaultValue(const PropertyDefinition& definition);

  std::unique_ptr<PropertyDefinition> clone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly) const;

private:
  virtual bool doEquals(const PropertyDefinition* other) const;
  virtual std::unique_ptr<PropertyDefinition> doClone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly) const;
};

template <typename T>
class PropertyDefinitionWithDefaultValue : public PropertyDefinition
{
protected:
  std::optional<T> m_defaultValue;

public:
  bool hasDefaultValue() const { return m_defaultValue.has_value(); }

  const T& defaultValue() const
  {
    ensure(hasDefaultValue(), "property definition has no default value");
    return *m_defaultValue;
  }

protected:
  PropertyDefinitionWithDefaultValue(
    std::string key,
    PropertyDefinitionType type,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly,
    std::optional<T> defaultValue = std::nullopt)
    : PropertyDefinition(key, type, shortDescription, longDescription, readOnly)
    , m_defaultValue(std::move(defaultValue))
  {
  }
};

class StringPropertyDefinition : public PropertyDefinitionWithDefaultValue<std::string>
{
public:
  StringPropertyDefinition(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly,
    std::optional<std::string> defaultValue = std::nullopt);

private:
  std::unique_ptr<PropertyDefinition> doClone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly) const override;
};

class BooleanPropertyDefinition : public PropertyDefinitionWithDefaultValue<bool>
{
public:
  BooleanPropertyDefinition(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly,
    std::optional<bool> defaultValue = std::nullopt);

private:
  std::unique_ptr<PropertyDefinition> doClone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly) const override;
};

class IntegerPropertyDefinition : public PropertyDefinitionWithDefaultValue<int>
{
public:
  IntegerPropertyDefinition(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly,
    std::optional<int> defaultValue = std::nullopt);

private:
  std::unique_ptr<PropertyDefinition> doClone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly) const override;
};

class FloatPropertyDefinition : public PropertyDefinitionWithDefaultValue<float>
{
public:
  FloatPropertyDefinition(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly,
    std::optional<float> defaultValue = std::nullopt);

private:
  std::unique_ptr<PropertyDefinition> doClone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly) const override;
};

class ChoicePropertyOption
{
public:
  using List = std::vector<ChoicePropertyOption>;

private:
  std::string m_value;
  std::string m_description;

public:
  ChoicePropertyOption(std::string value, std::string description);
  const std::string& value() const;
  const std::string& description() const;

  kdl_reflect_decl(ChoicePropertyOption, m_value, m_description);
};

class ChoicePropertyDefinition : public PropertyDefinitionWithDefaultValue<std::string>
{
private:
  ChoicePropertyOption::List m_options;

public:
  ChoicePropertyDefinition(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    ChoicePropertyOption::List options,
    bool readOnly,
    std::optional<std::string> defaultValue = std::nullopt);
  const ChoicePropertyOption::List& options() const;

private:
  bool doEquals(const PropertyDefinition* other) const override;
  std::unique_ptr<PropertyDefinition> doClone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly) const override;
};

class FlagsPropertyOption
{
public:
  using List = std::vector<FlagsPropertyOption>;

private:
  int m_value;
  std::string m_shortDescription;
  std::string m_longDescription;
  bool m_isDefault;

public:
  FlagsPropertyOption(
    int value, std::string shortDescription, std::string longDescription, bool isDefault);
  int value() const;
  const std::string& shortDescription() const;
  const std::string& longDescription() const;
  bool isDefault() const;

  kdl_reflect_decl(
    FlagsPropertyOption, m_shortDescription, m_longDescription, m_isDefault);
};

class FlagsPropertyDefinition : public PropertyDefinition
{
private:
  FlagsPropertyOption::List m_options;

public:
  explicit FlagsPropertyDefinition(std::string key);

  int defaultValue() const;
  const FlagsPropertyOption::List& options() const;
  const FlagsPropertyOption* option(int value) const;
  void addOption(
    int value, std::string shortDescription, std::string longDescription, bool isDefault);

private:
  bool doEquals(const PropertyDefinition* other) const override;
  std::unique_ptr<PropertyDefinition> doClone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly) const override;
};

class UnknownPropertyDefinition : public StringPropertyDefinition
{
public:
  UnknownPropertyDefinition(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly,
    std::optional<std::string> defaultValue = std::nullopt);

private:
  std::unique_ptr<PropertyDefinition> doClone(
    std::string key,
    std::string shortDescription,
    std::string longDescription,
    bool readOnly) const override;
};

} // namespace TrenchBroom::Assets
