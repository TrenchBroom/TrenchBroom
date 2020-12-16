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
        PropertyDefinition::PropertyDefinition(const std::string& key, const PropertyDefinitionType type, const std::string& shortDescription, const std::string& longDescription, const bool readOnly) :
        m_key(key),
        m_type(type),
        m_shortDescription(shortDescription),
        m_longDescription(longDescription),
        m_readOnly(readOnly) {}

        PropertyDefinition::~PropertyDefinition() = default;

        const std::string& PropertyDefinition::key() const {
            return m_key;
        }

        PropertyDefinitionType PropertyDefinition::type() const {
            return m_type;
        }

        const std::string& PropertyDefinition::shortDescription() const {
            return m_shortDescription;
        }

        const std::string& PropertyDefinition::longDescription() const {
            return m_longDescription;
        }

        bool PropertyDefinition::readOnly() const {
            return m_readOnly;
        }

        bool PropertyDefinition::equals(const PropertyDefinition* other) const {
            ensure(other != nullptr, "other is null");
            if (type() != other->type())
                return false;
            if (key() != other->key())
                return false;
            return doEquals(other);
        }

        bool PropertyDefinition::doEquals(const PropertyDefinition* /* other */) const {
            return true;
        }

        std::string PropertyDefinition::defaultValue(const PropertyDefinition& definition) {
            switch (definition.type()) {
                case PropertyDefinitionType::StringProperty: {
                    const auto& stringDef = static_cast<const StringPropertyDefinition&>(definition);
                    if (!stringDef.hasDefaultValue())
                        return "";
                    return stringDef.defaultValue();
                }
                case PropertyDefinitionType::BooleanProperty: {
                    const auto& boolDef = static_cast<const BooleanPropertyDefinition&>(definition);
                    if (!boolDef.hasDefaultValue())
                        return "";
                    std::stringstream str;
                    str << boolDef.defaultValue();
                    return str.str();
                }
                case PropertyDefinitionType::IntegerProperty: {
                    const auto& intDef = static_cast<const IntegerPropertyDefinition&>(definition);
                    if (!intDef.hasDefaultValue())
                        return "";
                    std::stringstream str;
                    str << intDef.defaultValue();
                    return str.str();
                }
                case PropertyDefinitionType::FloatProperty: {
                    const auto& floatDef = static_cast<const FloatPropertyDefinition&>(definition);
                    if (!floatDef.hasDefaultValue())
                        return "";
                    std::stringstream str;
                    str << floatDef.defaultValue();
                    return str.str();
                }
                case PropertyDefinitionType::ChoiceProperty: {
                    const auto& choiceDef = static_cast<const ChoicePropertyDefinition&>(definition);
                    if (!choiceDef.hasDefaultValue())
                        return "";
                    std::stringstream str;
                    str << choiceDef.defaultValue();
                    return str.str();
                }
                case PropertyDefinitionType::FlagsProperty: {
                    const auto& flagsDef = static_cast<const FlagsPropertyDefinition&>(definition);
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

        PropertyDefinition* PropertyDefinition::clone(const std::string& key, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return doClone(key, shortDescription, longDescription, readOnly);
        }

        PropertyDefinition* PropertyDefinition::doClone(const std::string& key, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return new PropertyDefinition(key, type(), shortDescription, longDescription, readOnly);
        }

        StringPropertyDefinition::StringPropertyDefinition(const std::string& key, const std::string& shortDescription, const std::string& longDescription, const bool readOnly, std::optional<std::string> defaultValue) :
        PropertyDefinitionWithDefaultValue(key, PropertyDefinitionType::StringProperty, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}

        PropertyDefinition* StringPropertyDefinition::doClone(const std::string& key, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return new StringPropertyDefinition(key, shortDescription, longDescription, readOnly, m_defaultValue);
        }

        BooleanPropertyDefinition::BooleanPropertyDefinition(const std::string& key, const std::string& shortDescription, const std::string& longDescription, const bool readOnly, std::optional<bool> defaultValue) :
        PropertyDefinitionWithDefaultValue(key, PropertyDefinitionType::BooleanProperty, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}

        PropertyDefinition* BooleanPropertyDefinition::doClone(const std::string& key, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return new BooleanPropertyDefinition(key, shortDescription, longDescription, readOnly, m_defaultValue);
        }

        IntegerPropertyDefinition::IntegerPropertyDefinition(const std::string& key, const std::string& shortDescription, const std::string& longDescription, const bool readOnly, std::optional<int> defaultValue) :
        PropertyDefinitionWithDefaultValue(key, PropertyDefinitionType::IntegerProperty, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}

        PropertyDefinition* IntegerPropertyDefinition::doClone(const std::string& key, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return new IntegerPropertyDefinition(key, shortDescription, longDescription, readOnly, m_defaultValue);
        }

        FloatPropertyDefinition::FloatPropertyDefinition(const std::string& key, const std::string& shortDescription, const std::string& longDescription, const bool readOnly, std::optional<float> defaultValue) :
        PropertyDefinitionWithDefaultValue(key, PropertyDefinitionType::FloatProperty, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}

        PropertyDefinition* FloatPropertyDefinition::doClone(const std::string& key, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return new FloatPropertyDefinition(key, shortDescription, longDescription, readOnly, m_defaultValue);
        }

        ChoicePropertyOption::ChoicePropertyOption(const std::string& value, const std::string& description) :
        m_value(value),
        m_description(description) {}

        const std::string& ChoicePropertyOption::value() const {
            return m_value;
        }

        const std::string& ChoicePropertyOption::description() const {
            return m_description;
        }

        ChoicePropertyDefinition::ChoicePropertyDefinition(const std::string& key, const std::string& shortDescription, const std::string& longDescription, const ChoicePropertyOption::List& options, const bool readOnly, std::optional<std::string> defaultValue) :
        PropertyDefinitionWithDefaultValue(key, PropertyDefinitionType::ChoiceProperty, shortDescription, longDescription, readOnly, std::move(defaultValue)),
        m_options(options) {}

        const ChoicePropertyOption::List& ChoicePropertyDefinition::options() const {
            return m_options;
        }

        bool operator==(const ChoicePropertyOption& lhs, const ChoicePropertyOption& rhs) {
            return lhs.value() == rhs.value()
                && lhs.description() == rhs.description();
        }
        
        bool operator!=(const ChoicePropertyOption& lhs, const ChoicePropertyOption& rhs) {
            return !(lhs == rhs);
        }

        std::ostream& operator<<(std::ostream& str, const ChoicePropertyOption& opt) {
            str << "ChoicePropertyOption{"
                << "value: " << opt.value() << ", "
                << "description: '" << opt.description() << "}";
            return str;
        }

        bool ChoicePropertyDefinition::doEquals(const PropertyDefinition* other) const {
            return options() == static_cast<const ChoicePropertyDefinition*>(other)->options();
        }

        PropertyDefinition* ChoicePropertyDefinition::doClone(const std::string& key, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return new ChoicePropertyDefinition(key, shortDescription, longDescription, options(), readOnly, m_defaultValue);
        }

        FlagsPropertyOption::FlagsPropertyOption(const int value, const std::string& shortDescription, const std::string& longDescription, const bool isDefault) :
        m_value(value),
        m_shortDescription(shortDescription),
        m_longDescription(longDescription),
        m_isDefault(isDefault) {}

        int FlagsPropertyOption::value() const {
            return m_value;
        }

        const std::string& FlagsPropertyOption::shortDescription() const {
            return m_shortDescription;
        }

        const std::string& FlagsPropertyOption::longDescription() const {
            return m_longDescription;
        }

        bool FlagsPropertyOption::isDefault() const {
            return m_isDefault;
        }

        bool operator==(const FlagsPropertyOption& lhs, const FlagsPropertyOption& rhs) {
            return lhs.value() == rhs.value()
                && lhs.shortDescription() == rhs.shortDescription()
                && lhs.longDescription() == rhs.longDescription()
                && lhs.isDefault() == rhs.isDefault();
        }
        
        bool operator!=(const FlagsPropertyOption& lhs, const FlagsPropertyOption& rhs) {
            return !(lhs == rhs);
        }

        std::ostream& operator<<(std::ostream& str, const FlagsPropertyOption& opt) {
            str << "FlagPropertyOption{"
                << "value: " << opt.value() << ", "
                << "shortDescription: '" << opt.shortDescription() << "', "
                << "longDescription: '" << opt.longDescription() << "', "
                << "isDefault: " << opt.isDefault() << "}";
            return str;
        }

        FlagsPropertyDefinition::FlagsPropertyDefinition(const std::string& key) :
        PropertyDefinition(key, PropertyDefinitionType::FlagsProperty, "", "", false) {}

        int FlagsPropertyDefinition::defaultValue() const {
            int value = 0;
            for (const FlagsPropertyOption& option : m_options) {
                if (option.isDefault())
                    value |= option.value();
            }
            return value;
        }

        const FlagsPropertyOption::List& FlagsPropertyDefinition::options() const {
            return m_options;
        }

        const FlagsPropertyOption* FlagsPropertyDefinition::option(const int value) const {
            for (const auto& option : m_options) {
                if (option.value() == value) {
                    return &option;
                }
            }
            return nullptr;
        }

        void FlagsPropertyDefinition::addOption(const int value, const std::string& shortDescription, const std::string& longDescription, const bool isDefault) {
            m_options.push_back(FlagsPropertyOption(value, shortDescription, longDescription, isDefault));
        }

        bool FlagsPropertyDefinition::doEquals(const PropertyDefinition* other) const {
            return options() == static_cast<const FlagsPropertyDefinition*>(other)->options();
        }

        PropertyDefinition* FlagsPropertyDefinition::doClone(const std::string& key, const std::string& /* shortDescription */, const std::string& /* longDescription */, bool /* readOnly */) const {
            auto result = std::make_unique<FlagsPropertyDefinition>(key);
            for (const auto& option : options()) {
                result->addOption(option.value(), option.shortDescription(), option.longDescription(), option.isDefault());
            }
            return result.release();
        }

        UnknownPropertyDefinition::UnknownPropertyDefinition(const std::string& key, const std::string& shortDescription, const std::string& longDescription, const bool readOnly, std::optional<std::string> defaultValue) :
        StringPropertyDefinition(key, shortDescription, longDescription, readOnly, std::move(defaultValue)) {}


        PropertyDefinition* UnknownPropertyDefinition::doClone(const std::string& key, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const {
            return new UnknownPropertyDefinition(key, shortDescription, longDescription, readOnly, m_defaultValue);
        }
    }
}
