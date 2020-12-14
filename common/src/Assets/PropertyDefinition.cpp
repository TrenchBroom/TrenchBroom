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

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace TrenchBroom {
    namespace Assets {
        AttributeDefinition::AttributeDefinition(const std::string& name, const PropertyDefinitionType type, const std::string& shortDescription, const std::string& longDescription, const bool readOnly) :
        m_name(name),
        m_type(type),
        m_shortDescription(shortDescription),
        m_longDescription(longDescription),
        m_readOnly(readOnly) {}

        AttributeDefinition::~AttributeDefinition() = default;

        const std::string& AttributeDefinition::name() const {
            return m_name;
        }

        PropertyDefinitionType AttributeDefinition::type() const {
            return m_type;
        }

        const std::string& AttributeDefinition::shortDescription() const {
            return m_shortDescription;
        }

        const std::string& AttributeDefinition::longDescription() const {
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

        std::string AttributeDefinition::defaultValue(const AttributeDefinition& definition) {
            switch (definition.type()) {
                case PropertyDefinitionType::StringProperty: {
                    const auto& stringDef = static_cast<const StringAttributeDefinition&>(definition);
                    if (!stringDef.hasDefaultValue())
                        return "";
                    return stringDef.defaultValue();
                }
                case PropertyDefinitionType::BooleanProperty: {
                    const auto& boolDef = static_cast<const BooleanAttributeDefinition&>(definition);
                    if (!boolDef.hasDefaultValue())
                        return "";
                    std::stringstream str;
                    str << boolDef.defaultValue();
                    return str.str();
                }
                case PropertyDefinitionType::IntegerProperty: {
                    const auto& intDef = static_cast<const IntegerAttributeDefinition&>(definition);
                    if (!intDef.hasDefaultValue())
                        return "";
                    std::stringstream str;
                    str << intDef.defaultValue();
                    return str.str();
                }
                case PropertyDefinitionType::FloatProperty: {
                    const auto& floatDef = static_cast<const FloatAttributeDefinition&>(definition);
                    if (!floatDef.hasDefaultValue())
                        return "";
                    std::stringstream str;
                    str << floatDef.defaultValue();
                    return str.str();
                }
                case PropertyDefinitionType::ChoiceProperty: {
                    const auto& choiceDef = static_cast<const ChoiceAttributeDefinition&>(definition);
                    if (!choiceDef.hasDefaultValue())
                        return "";
                    std::stringstream str;
                    str << choiceDef.defaultValue();
                    return str.str();
                }
                case PropertyDefinitionType::FlagsProperty: {
                    const auto& flagsDef = static_cast<const FlagsAttributeDefinition&>(definition);
                    std::stringstream str;
                    str << flagsDef.defaultValue();
                    return str.str();
                }
                case PropertyDefinitionType::TargetSourceProperty:
                case PropertyDefinitionType::TargetDestinationProperty:
                    return "";
                switchDefault()
            }
        }

        AttributeDefinition* AttributeDefinition::clone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return doClone(name, shortDescription, longDescription, readOnly);
        }

        AttributeDefinition* AttributeDefinition::doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return new AttributeDefinition(name, type(), shortDescription, longDescription, readOnly);
        }

        StringAttributeDefinition::StringAttributeDefinition(const std::string& name, const std::string& shortDescription, const std::string& longDescription, const bool readOnly, std::optional<std::string> defaultValue) :
        AttributeDefinitionWithDefaultValue(name, PropertyDefinitionType::StringProperty, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}

        AttributeDefinition* StringAttributeDefinition::doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return new StringAttributeDefinition(name, shortDescription, longDescription, readOnly, m_defaultValue);
        }

        BooleanAttributeDefinition::BooleanAttributeDefinition(const std::string& name, const std::string& shortDescription, const std::string& longDescription, const bool readOnly, std::optional<bool> defaultValue) :
        AttributeDefinitionWithDefaultValue(name, PropertyDefinitionType::BooleanProperty, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}

        AttributeDefinition* BooleanAttributeDefinition::doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return new BooleanAttributeDefinition(name, shortDescription, longDescription, readOnly, m_defaultValue);
        }

        IntegerAttributeDefinition::IntegerAttributeDefinition(const std::string& name, const std::string& shortDescription, const std::string& longDescription, const bool readOnly, std::optional<int> defaultValue) :
        AttributeDefinitionWithDefaultValue(name, PropertyDefinitionType::IntegerProperty, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}

        AttributeDefinition* IntegerAttributeDefinition::doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return new IntegerAttributeDefinition(name, shortDescription, longDescription, readOnly, m_defaultValue);
        }

        FloatAttributeDefinition::FloatAttributeDefinition(const std::string& name, const std::string& shortDescription, const std::string& longDescription, const bool readOnly, std::optional<float> defaultValue) :
        AttributeDefinitionWithDefaultValue(name, PropertyDefinitionType::FloatProperty, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}

        AttributeDefinition* FloatAttributeDefinition::doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return new FloatAttributeDefinition(name, shortDescription, longDescription, readOnly, m_defaultValue);
        }

        ChoiceAttributeOption::ChoiceAttributeOption(const std::string& value, const std::string& description) :
        m_value(value),
        m_description(description) {}

        const std::string& ChoiceAttributeOption::value() const {
            return m_value;
        }

        const std::string& ChoiceAttributeOption::description() const {
            return m_description;
        }

        ChoiceAttributeDefinition::ChoiceAttributeDefinition(const std::string& name, const std::string& shortDescription, const std::string& longDescription, const ChoiceAttributeOption::List& options, const bool readOnly, std::optional<std::string> defaultValue) :
        AttributeDefinitionWithDefaultValue(name, PropertyDefinitionType::ChoiceProperty, shortDescription, longDescription, readOnly, std::move(defaultValue)),
        m_options(options) {}

        const ChoiceAttributeOption::List& ChoiceAttributeDefinition::options() const {
            return m_options;
        }

        bool operator==(const ChoiceAttributeOption& lhs, const ChoiceAttributeOption& rhs) {
            return lhs.value() == rhs.value()
                && lhs.description() == rhs.description();
        }
        
        bool operator!=(const ChoiceAttributeOption& lhs, const ChoiceAttributeOption& rhs) {
            return !(lhs == rhs);
        }

        std::ostream& operator<<(std::ostream& str, const ChoiceAttributeOption& opt) {
            str << "ChoiceAttributeOption{"
                << "value: " << opt.value() << ", "
                << "description: '" << opt.description() << "}";
            return str;
        }

        bool ChoiceAttributeDefinition::doEquals(const AttributeDefinition* other) const {
            return options() == static_cast<const ChoiceAttributeDefinition*>(other)->options();
        }

        AttributeDefinition* ChoiceAttributeDefinition::doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return new ChoiceAttributeDefinition(name, shortDescription, longDescription, options(), readOnly, m_defaultValue);
        }

        FlagsAttributeOption::FlagsAttributeOption(const int value, const std::string& shortDescription, const std::string& longDescription, const bool isDefault) :
        m_value(value),
        m_shortDescription(shortDescription),
        m_longDescription(longDescription),
        m_isDefault(isDefault) {}

        int FlagsAttributeOption::value() const {
            return m_value;
        }

        const std::string& FlagsAttributeOption::shortDescription() const {
            return m_shortDescription;
        }

        const std::string& FlagsAttributeOption::longDescription() const {
            return m_longDescription;
        }

        bool FlagsAttributeOption::isDefault() const {
            return m_isDefault;
        }

        bool operator==(const FlagsAttributeOption& lhs, const FlagsAttributeOption& rhs) {
            return lhs.value() == rhs.value()
                && lhs.shortDescription() == rhs.shortDescription()
                && lhs.longDescription() == rhs.longDescription()
                && lhs.isDefault() == rhs.isDefault();
        }
        
        bool operator!=(const FlagsAttributeOption& lhs, const FlagsAttributeOption& rhs) {
            return !(lhs == rhs);
        }

        std::ostream& operator<<(std::ostream& str, const FlagsAttributeOption& opt) {
            str << "FlagAttributeOption{"
                << "value: " << opt.value() << ", "
                << "shortDescription: '" << opt.shortDescription() << "', "
                << "longDescription: '" << opt.longDescription() << "', "
                << "isDefault: " << opt.isDefault() << "}";
            return str;
        }

        FlagsAttributeDefinition::FlagsAttributeDefinition(const std::string& name) :
        AttributeDefinition(name, PropertyDefinitionType::FlagsProperty, "", "", false) {}

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

        const FlagsAttributeOption* FlagsAttributeDefinition::option(const int value) const {
            for (const auto& option : m_options) {
                if (option.value() == value) {
                    return &option;
                }
            }
            return nullptr;
        }

        void FlagsAttributeDefinition::addOption(const int value, const std::string& shortDescription, const std::string& longDescription, const bool isDefault) {
            m_options.push_back(FlagsAttributeOption(value, shortDescription, longDescription, isDefault));
        }

        bool FlagsAttributeDefinition::doEquals(const AttributeDefinition* other) const {
            return options() == static_cast<const FlagsAttributeDefinition*>(other)->options();
        }

        AttributeDefinition* FlagsAttributeDefinition::doClone(const std::string& name, const std::string& /* shortDescription */, const std::string& /* longDescription */, bool /* readOnly */) const {
            auto result = std::make_unique<FlagsAttributeDefinition>(name);
            for (const auto& option : options()) {
                result->addOption(option.value(), option.shortDescription(), option.longDescription(), option.isDefault());
            }
            return result.release();
        }

        UnknownAttributeDefinition::UnknownAttributeDefinition(const std::string& name, const std::string& shortDescription, const std::string& longDescription, const bool readOnly, std::optional<std::string> defaultValue) :
        StringAttributeDefinition(name, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}


        AttributeDefinition* UnknownAttributeDefinition::doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return new UnknownAttributeDefinition(name, shortDescription, longDescription, readOnly, m_defaultValue);
        }
    }
}
