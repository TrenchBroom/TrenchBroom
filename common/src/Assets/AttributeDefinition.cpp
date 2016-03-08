/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "CollectionUtils.h"
#include "Macros.h"

namespace TrenchBroom {
    namespace Assets {
        AttributeDefinition::AttributeDefinition(const String& name, const Type type, const String& shortDescription, const String& longDescription) :
        m_name(name),
        m_type(type),
        m_shortDescription(shortDescription),
        m_longDescription(longDescription) {}
        
        AttributeDefinition::~AttributeDefinition() {}
        
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
        
        String AttributeDefinition::fullDescription() const {
            StringStream result;
            if (!m_shortDescription.empty() && !m_longDescription.empty()) {
                result << m_shortDescription << std::endl << std::endl << m_longDescription;
            } else if (!m_shortDescription.empty()) {
                result << m_shortDescription;
            } else if (!m_longDescription.empty()) {
                result << m_longDescription;
            } else {
                result << "No description found";
            }
            return result.str();
        }

        String AttributeDefinition::safeFullDescription(const AttributeDefinition* definition) {
            return definition == NULL ? EmptyString : definition->fullDescription();
        }

        bool AttributeDefinition::equals(const AttributeDefinition* other) const {
            assert(other != NULL);
            if (type() != other->type())
                return false;
            if (name() != other->name())
                return false;
            return doEquals(other);
        }

        bool AttributeDefinition::doEquals(const AttributeDefinition* other) const {
            return true;
        }

        String AttributeDefinition::defaultValue(const AttributeDefinition& definition) {
            switch (definition.type()) {
                case Type_StringAttribute: {
                    const StringAttributeDefinition& stringDef = static_cast<const StringAttributeDefinition&>(definition);
                    if (!stringDef.hasDefaultValue())
                        return "";
                    return stringDef.defaultValue();
                }
                case Type_IntegerAttribute: {
                    const IntegerAttributeDefinition& intDef = static_cast<const IntegerAttributeDefinition&>(definition);
                    if (!intDef.hasDefaultValue())
                        return "";
                    StringStream str;
                    str << intDef.defaultValue();
                    return str.str();
                }
                case Type_FloatAttribute: {
                    const FloatAttributeDefinition& floatDef = static_cast<const FloatAttributeDefinition&>(definition);
                    if (!floatDef.hasDefaultValue())
                        return "";
                    StringStream str;
                    str << floatDef.defaultValue();
                    return str.str();
                }
                case Type_ChoiceAttribute: {
                    const ChoiceAttributeDefinition& choiceDef = static_cast<const ChoiceAttributeDefinition&>(definition);
                    if (!choiceDef.hasDefaultValue())
                        return "";
                    StringStream str;
                    str << choiceDef.defaultValue();
                    return str.str();
                }
                case Type_FlagsAttribute: {
                    const FlagsAttributeDefinition& flagsDef = static_cast<const FlagsAttributeDefinition&>(definition);
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

        StringAttributeDefinition::StringAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const String& defaultValue) :
        AttributeDefinitionWithDefaultValue(name, Type_StringAttribute, shortDescription, longDescription, defaultValue) {}
        
        StringAttributeDefinition::StringAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription) :
        AttributeDefinitionWithDefaultValue(name, Type_StringAttribute, shortDescription, longDescription) {}

        IntegerAttributeDefinition::IntegerAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const int defaultValue) :
        AttributeDefinitionWithDefaultValue(name, Type_IntegerAttribute, shortDescription, longDescription, defaultValue) {}
        
        IntegerAttributeDefinition::IntegerAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription) :
        AttributeDefinitionWithDefaultValue(name, Type_IntegerAttribute, shortDescription, longDescription) {}

        FloatAttributeDefinition::FloatAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const float defaultValue) :
        AttributeDefinitionWithDefaultValue(name, Type_FloatAttribute, shortDescription, longDescription, defaultValue) {}
        
        FloatAttributeDefinition::FloatAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription) :
        AttributeDefinitionWithDefaultValue(name, Type_FloatAttribute, shortDescription, longDescription) {}

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

        ChoiceAttributeDefinition::ChoiceAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const ChoiceAttributeOption::List& options, const size_t defaultValue) :
        AttributeDefinitionWithDefaultValue(name, Type_ChoiceAttribute, shortDescription, longDescription, defaultValue),
        m_options(options) {}
        
        ChoiceAttributeDefinition::ChoiceAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const ChoiceAttributeOption::List& options) :
        AttributeDefinitionWithDefaultValue(name, Type_ChoiceAttribute, shortDescription, longDescription),
        m_options(options) {}
        
        const ChoiceAttributeOption::List& ChoiceAttributeDefinition::options() const {
            return m_options;
        }

        bool ChoiceAttributeDefinition::doEquals(const AttributeDefinition* other) const {
            return options() == static_cast<const ChoiceAttributeDefinition*>(other)->options();
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

        FlagsAttributeDefinition::FlagsAttributeDefinition(const String& name, const int defaultValue) :
        AttributeDefinition(name, Type_FlagsAttribute, EmptyString, EmptyString) {}
        
        FlagsAttributeDefinition::FlagsAttributeDefinition(const String& name) :
        AttributeDefinition(name, Type_FlagsAttribute, EmptyString, EmptyString) {}
        
        int FlagsAttributeDefinition::defaultValue() const {
            int value = 0;
            FlagsAttributeOption::List::const_iterator it, end;
            for (it = m_options.begin(), end = m_options.end(); it != end; ++it) {
                const FlagsAttributeOption& option = *it;
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
            FindFlagByValue(const int i_value) : value(i_value) {}
            bool operator()(const FlagsAttributeOption& option) const {
                return option.value() == value;
            }
        };

        const FlagsAttributeOption* FlagsAttributeDefinition::option(const int value) const {
            return VectorUtils::findIf(m_options, FindFlagByValue(value));
        }

        void FlagsAttributeDefinition::addOption(const int value, const String& shortDescription, const String& longDescription, const bool isDefault) {
            m_options.push_back(FlagsAttributeOption(value, shortDescription, longDescription, isDefault));
        }

        bool FlagsAttributeDefinition::doEquals(const AttributeDefinition* other) const {
            return options() == static_cast<const FlagsAttributeDefinition*>(other)->options();
        }

        UnknownAttributeDefinition::UnknownAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const String& defaultValue) :
        StringAttributeDefinition(name, shortDescription, longDescription, defaultValue) {}
        
        UnknownAttributeDefinition::UnknownAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription) :
        StringAttributeDefinition(name, shortDescription, longDescription) {}
    }
}
