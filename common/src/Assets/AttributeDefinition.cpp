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

#include "AttributeDefinition.h"

#include <algorithm>
#include <memory>

#include "StringStream.h"
#include "Macros.h"

namespace TrenchBroom {
    namespace Assets {
        AttributeDefinition::AttributeDefinition(const String& name, const Type type, const String& shortDescription, const String& longDescription, const bool readOnly) :
        m_name(name),
        m_type(type),
        m_shortDescription(shortDescription),
        m_longDescription(longDescription),
        m_readOnly(readOnly) {}

        AttributeDefinition::~AttributeDefinition() = default;

        const String& AttributeDefinition::name() const {
            return m_name;
        }

        AttributeDefinition::Type AttributeDefinition::type() const {
            return m_type;
        }

        const String& AttributeDefinition::shortDescription() const {
            return m_shortDescription;
        }

        const String& AttributeDefinition::longDescription() const {
            return m_longDescription;
        }

        bool AttributeDefinition::readOnly() const {
            return m_readOnly;
        }

        bool AttributeDefinition::equals(const AttributeDefinition* other) const {
            ensure(other != nullptr, "other is null");
            if (type() != other->type())
                return false;
            if (name() != other->name())
                return false;
            return doEquals(other);
        }

        bool AttributeDefinition::doEquals(const AttributeDefinition* /* other */) const {
            return true;
        }

        String AttributeDefinition::defaultValue(const AttributeDefinition& definition) {
            switch (definition.type()) {
                case Type_StringAttribute: {
                    const auto& stringDef = static_cast<const StringAttributeDefinition&>(definition);
                    if (!stringDef.hasDefaultValue())
                        return "";
                    return stringDef.defaultValue();
                }
                case Type_BooleanAttribute: {
                    const auto& boolDef = static_cast<const BooleanAttributeDefinition&>(definition);
                    if (!boolDef.hasDefaultValue())
                        return "";
                    StringStream str;
                    str << boolDef.defaultValue();
                    return str.str();
                }
                case Type_IntegerAttribute: {
                    const auto& intDef = static_cast<const IntegerAttributeDefinition&>(definition);
                    if (!intDef.hasDefaultValue())
                        return "";
                    StringStream str;
                    str << intDef.defaultValue();
                    return str.str();
                }
                case Type_FloatAttribute: {
                    const auto& floatDef = static_cast<const FloatAttributeDefinition&>(definition);
                    if (!floatDef.hasDefaultValue())
                        return "";
                    StringStream str;
                    str << floatDef.defaultValue();
                    return str.str();
                }
                case Type_ChoiceAttribute: {
                    const auto& choiceDef = static_cast<const ChoiceAttributeDefinition&>(definition);
                    if (!choiceDef.hasDefaultValue())
                        return "";
                    StringStream str;
                    str << choiceDef.defaultValue();
                    return str.str();
                }
                case Type_FlagsAttribute: {
                    const auto& flagsDef = static_cast<const FlagsAttributeDefinition&>(definition);
                    StringStream str;
                    str << flagsDef.defaultValue();
                    return str.str();
                }
                case Type_TargetSourceAttribute:
                case Type_TargetDestinationAttribute:
                    return "";
                switchDefault()
            }
        }

        AttributeDefinition* AttributeDefinition::clone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const {
            return doClone(name, shortDescription, longDescription, readOnly);
        }

        AttributeDefinition* AttributeDefinition::doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const {
            return new AttributeDefinition(name, type(), shortDescription, longDescription, readOnly);
        }

        StringAttributeDefinition::StringAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const bool readOnly, nonstd::optional<String> defaultValue) :
        AttributeDefinitionWithDefaultValue(name, Type_StringAttribute, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}

        AttributeDefinition* StringAttributeDefinition::doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const {
            return new StringAttributeDefinition(name, shortDescription, longDescription, readOnly, m_defaultValue);
        }

        BooleanAttributeDefinition::BooleanAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const bool readOnly, nonstd::optional<bool> defaultValue) :
        AttributeDefinitionWithDefaultValue(name, Type_BooleanAttribute, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}

        AttributeDefinition* BooleanAttributeDefinition::doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const {
            return new BooleanAttributeDefinition(name, shortDescription, longDescription, readOnly, m_defaultValue);
        }

        IntegerAttributeDefinition::IntegerAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const bool readOnly, nonstd::optional<int> defaultValue) :
        AttributeDefinitionWithDefaultValue(name, Type_IntegerAttribute, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}

        AttributeDefinition* IntegerAttributeDefinition::doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const {
            return new IntegerAttributeDefinition(name, shortDescription, longDescription, readOnly, m_defaultValue);
        }

        FloatAttributeDefinition::FloatAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const bool readOnly, nonstd::optional<float> defaultValue) :
        AttributeDefinitionWithDefaultValue(name, Type_FloatAttribute, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}

        AttributeDefinition* FloatAttributeDefinition::doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const {
            return new FloatAttributeDefinition(name, shortDescription, longDescription, readOnly, m_defaultValue);
        }

        ChoiceAttributeOption::ChoiceAttributeOption(const String& value, const String& description) :
        m_value(value),
        m_description(description) {}

        bool ChoiceAttributeOption::operator==(const ChoiceAttributeOption& other) const {
            return m_value == other.m_value && m_description == other.m_description;
        }

        const String& ChoiceAttributeOption::value() const {
            return m_value;
        }

        const String& ChoiceAttributeOption::description() const {
            return m_description;
        }

        ChoiceAttributeDefinition::ChoiceAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const ChoiceAttributeOption::List& options, const bool readOnly, nonstd::optional<String> defaultValue) :
        AttributeDefinitionWithDefaultValue(name, Type_ChoiceAttribute, shortDescription, longDescription, readOnly, std::move(defaultValue)),
        m_options(options) {}

        const ChoiceAttributeOption::List& ChoiceAttributeDefinition::options() const {
            return m_options;
        }

        bool ChoiceAttributeDefinition::doEquals(const AttributeDefinition* other) const {
            return options() == static_cast<const ChoiceAttributeDefinition*>(other)->options();
        }

        AttributeDefinition* ChoiceAttributeDefinition::doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const {
            return new ChoiceAttributeDefinition(name, shortDescription, longDescription, options(), readOnly, m_defaultValue);
        }

        FlagsAttributeOption::FlagsAttributeOption(const int value, const String& shortDescription, const String& longDescription, const bool isDefault) :
        m_value(value),
        m_shortDescription(shortDescription),
        m_longDescription(longDescription),
        m_isDefault(isDefault) {}

        bool FlagsAttributeOption::operator==(const FlagsAttributeOption& other) const {
            return (m_value == other.m_value &&
                    m_shortDescription == other.m_shortDescription &&
                    m_longDescription == other.m_longDescription &&
                    m_isDefault == other.m_isDefault);
        }

        int FlagsAttributeOption::value() const {
            return m_value;
        }

        const String& FlagsAttributeOption::shortDescription() const {
            return m_shortDescription;
        }

        const String& FlagsAttributeOption::longDescription() const {
            return m_longDescription;
        }

        bool FlagsAttributeOption::isDefault() const {
            return m_isDefault;
        }

        FlagsAttributeDefinition::FlagsAttributeDefinition(const String& name) :
        AttributeDefinition(name, Type_FlagsAttribute, EmptyString, EmptyString, false) {}

        int FlagsAttributeDefinition::defaultValue() const {
            int value = 0;
            for (const FlagsAttributeOption& option : m_options) {
                if (option.isDefault())
                    value |= option.value();
            }
            return value;
        }

        const FlagsAttributeOption::List& FlagsAttributeDefinition::options() const {
            return m_options;
        }

        struct FindFlagByValue {
            int value;
            explicit FindFlagByValue(const int i_value) : value(i_value) {}
            bool operator()(const FlagsAttributeOption& option) const {
                return option.value() == value;
            }
        };

        const FlagsAttributeOption* FlagsAttributeDefinition::option(const int value) const {
            auto it = std::find_if(std::begin(m_options), std::end(m_options), FindFlagByValue(value));
            if (it == std::end(m_options)) {
                return nullptr;
            } else {
                return &(*it);
            }
        }

        void FlagsAttributeDefinition::addOption(const int value, const String& shortDescription, const String& longDescription, const bool isDefault) {
            m_options.push_back(FlagsAttributeOption(value, shortDescription, longDescription, isDefault));
        }

        bool FlagsAttributeDefinition::doEquals(const AttributeDefinition* other) const {
            return options() == static_cast<const FlagsAttributeDefinition*>(other)->options();
        }

        AttributeDefinition* FlagsAttributeDefinition::doClone(const String& name, const String& /* shortDescription */, const String& /* longDescription */, bool /* readOnly */) const {
            auto result = std::make_unique<FlagsAttributeDefinition>(name);
            for (const auto& option : options()) {
                result->addOption(option.value(), option.shortDescription(), option.longDescription(), option.isDefault());
            }
            return result.release();
        }

        UnknownAttributeDefinition::UnknownAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const bool readOnly, nonstd::optional<String> defaultValue) :
        StringAttributeDefinition(name, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}


        AttributeDefinition* UnknownAttributeDefinition::doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const {
            return new UnknownAttributeDefinition(name, shortDescription, longDescription, readOnly, m_defaultValue);
        }
    }
}
