/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "CollectionUtils.h"

namespace TrenchBroom {
    namespace Assets {
        PropertyDefinition::PropertyDefinition(const String& name, const Type type, const String& description) :
        m_name(name),
        m_type(type),
        m_description(description) {}
        
        PropertyDefinition::~PropertyDefinition() {}
        
        const String& PropertyDefinition::name() const {
            return m_name;
        }
        
        PropertyDefinition::Type PropertyDefinition::type() const {
            return m_type;
        }
        
        const String& PropertyDefinition::description() const {
            return m_description;
        }

        String PropertyDefinition::defaultValue(const PropertyDefinition& definition) {
            switch (definition.type()) {
                case StringProperty: {
                    const StringPropertyDefinition& stringDef = static_cast<const StringPropertyDefinition&>(definition);
                    if (!stringDef.hasDefaultValue())
                        return "";
                    return stringDef.defaultValue();
                }
                case IntegerProperty: {
                    const IntegerPropertyDefinition& intDef = static_cast<const IntegerPropertyDefinition&>(definition);
                    if (!intDef.hasDefaultValue())
                        return "";
                    StringStream str;
                    str << intDef.defaultValue();
                    return str.str();
                }
                case FloatProperty: {
                    const FloatPropertyDefinition& floatDef = static_cast<const FloatPropertyDefinition&>(definition);
                    if (!floatDef.hasDefaultValue())
                        return "";
                    StringStream str;
                    str << floatDef.defaultValue();
                    return str.str();
                }
                case ChoiceProperty: {
                    const ChoicePropertyDefinition& choiceDef = static_cast<const ChoicePropertyDefinition&>(definition);
                    if (!choiceDef.hasDefaultValue())
                        return "";
                    StringStream str;
                    str << choiceDef.defaultValue();
                    return str.str();
                }
                case FlagsProperty: {
                    const FlagsPropertyDefinition& flagsDef = static_cast<const FlagsPropertyDefinition&>(definition);
                    StringStream str;
                    str << flagsDef.defaultValue();
                    return str.str();
                }
                default:
                    return "";
            }
        }

        StringPropertyDefinition::StringPropertyDefinition(const String& name, const String& description, const String& defaultValue) :
        PropertyDefinitionWithDefaultValue(name, StringProperty, description, defaultValue) {}
        
        StringPropertyDefinition::StringPropertyDefinition(const String& name, const String& description) :
        PropertyDefinitionWithDefaultValue(name, StringProperty, description) {}

        IntegerPropertyDefinition::IntegerPropertyDefinition(const String& name, const String& description, const int defaultValue) :
        PropertyDefinitionWithDefaultValue(name, IntegerProperty, description, defaultValue) {}
        
        IntegerPropertyDefinition::IntegerPropertyDefinition(const String& name, const String& description) :
        PropertyDefinitionWithDefaultValue(name, IntegerProperty, description) {}

        FloatPropertyDefinition::FloatPropertyDefinition(const String& name, const String& description, const float defaultValue) :
        PropertyDefinitionWithDefaultValue(name, FloatProperty, description, defaultValue) {}
        
        FloatPropertyDefinition::FloatPropertyDefinition(const String& name, const String& description) :
        PropertyDefinitionWithDefaultValue(name, FloatProperty, description) {}

        ChoicePropertyOption::ChoicePropertyOption(const String& value, const String& description) :
        m_value(value),
        m_description(description) {}
        
        const String& ChoicePropertyOption::value() const {
            return m_value;
        }
        
        const String& ChoicePropertyOption::description() const {
            return m_description;
        }

        ChoicePropertyDefinition::ChoicePropertyDefinition(const String& name, const String& description, const ChoicePropertyOption::List options, const size_t defaultValue) :
        PropertyDefinitionWithDefaultValue(name, ChoiceProperty, description, defaultValue),
        m_options(options) {}
        
        ChoicePropertyDefinition::ChoicePropertyDefinition(const String& name, const String& description, const ChoicePropertyOption::List options) :
        PropertyDefinitionWithDefaultValue(name, ChoiceProperty, description),
        m_options(options) {}
        
        const ChoicePropertyOption::List& ChoicePropertyDefinition::options() const {
            return m_options;
        }

        FlagsPropertyOption::FlagsPropertyOption(const int value, const String& description, const bool isDefault) :
        m_value(value),
        m_description(description),
        m_isDefault(isDefault) {}
        
        int FlagsPropertyOption::value() const {
            return m_value;
        }
        
        const String& FlagsPropertyOption::description() const {
            return m_description;
        }

        bool FlagsPropertyOption::isDefault() const {
            return m_isDefault;
        }

        FlagsPropertyDefinition::FlagsPropertyDefinition(const String& name, const String& description, const int defaultValue) :
        PropertyDefinition(name, FlagsProperty, description) {}
        
        FlagsPropertyDefinition::FlagsPropertyDefinition(const String& name, const String& description) :
        PropertyDefinition(name, FlagsProperty, description) {}
        
        int FlagsPropertyDefinition::defaultValue() const {
            int value = 0;
            FlagsPropertyOption::List::const_iterator it, end;
            for (it = m_options.begin(), end = m_options.end(); it != end; ++it) {
                const FlagsPropertyOption& option = *it;
                if (option.isDefault())
                    value |= option.value();
            }
            return value;
        }

        const FlagsPropertyOption::List& FlagsPropertyDefinition::options() const {
            return m_options;
        }
        
        struct FindFlagByValue {
            int value;
            FindFlagByValue(const int i_value) : value(i_value) {}
            bool operator()(const FlagsPropertyOption& option) const {
                return option.value() == value;
            }
        };

        const FlagsPropertyOption* FlagsPropertyDefinition::option(const int value) const {
            return VectorUtils::findIf(m_options, FindFlagByValue(value));
        }

        void FlagsPropertyDefinition::addOption(const int value, const String& description, const bool isDefault) {
            m_options.push_back(FlagsPropertyOption(value, description, isDefault));
        }
    }
}
