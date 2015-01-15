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

#ifndef __TrenchBroom__AttributeDefinition__
#define __TrenchBroom__AttributeDefinition__

#include "StringUtils.h"
#include "Exceptions.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class AttributeDefinition {
        public:
            typedef enum {
                Type_TargetSourceAttribute,
                Type_TargetDestinationAttribute,
                Type_StringAttribute,
                Type_IntegerAttribute,
                Type_FloatAttribute,
                Type_ChoiceAttribute,
                Type_FlagsAttribute
            } Type;
        private:
            String m_name;
            Type m_type;
            String m_description;
        public:
            AttributeDefinition(const String& name, const Type type, const String& description);
            virtual ~AttributeDefinition();
            
            const String& name() const;
            Type type() const;
            const String& description() const;
            bool equals(const AttributeDefinition* other) const;
            
            static String defaultValue(const AttributeDefinition& definition);
        private:
            virtual bool doEquals(const AttributeDefinition* other) const;
        };
        
        template <typename T>
        class AttributeDefinitionWithDefaultValue : public AttributeDefinition {
        private:
            bool m_hasDefaultValue;
            T m_defaultValue;
        public:
            bool hasDefaultValue() const {
                return m_hasDefaultValue;
            }
            
            const T& defaultValue() const {
                if (!hasDefaultValue())
                    throw EntityAttributeException(name() + " has no default value");
                return m_defaultValue;
            }
        protected:
            AttributeDefinitionWithDefaultValue(const String& name, const Type type, const String& description) :
            AttributeDefinition(name, type, description),
            m_hasDefaultValue(false) {}
            
            AttributeDefinitionWithDefaultValue(const String& name, const Type type, const String& description, const T& defaultValue) :
            AttributeDefinition(name, type, description),
            m_hasDefaultValue(true),
            m_defaultValue(defaultValue) {}
        };
        
        class StringAttributeDefinition : public AttributeDefinitionWithDefaultValue<String> {
        public:
            StringAttributeDefinition(const String& name, const String& description, const String& defaultValue);
            StringAttributeDefinition(const String& name, const String& description);
        };
        
        class IntegerAttributeDefinition : public AttributeDefinitionWithDefaultValue<int> {
        public:
            IntegerAttributeDefinition(const String& name, const String& description, const int defaultValue);
            IntegerAttributeDefinition(const String& name, const String& description);
        };
        
        class FloatAttributeDefinition : public AttributeDefinitionWithDefaultValue<float> {
        public:
            FloatAttributeDefinition(const String& name, const String& description, const float defaultValue);
            FloatAttributeDefinition(const String& name, const String& description);
        };
        
        class ChoiceAttributeOption {
        public:
            typedef std::vector<ChoiceAttributeOption> List;
        private:
            String m_value;
            String m_description;
        public:
            ChoiceAttributeOption(const String& value, const String& description);
            bool operator==(const ChoiceAttributeOption& other) const;
            const String& value() const;
            const String& description() const;
        };
        
        class ChoiceAttributeDefinition : public AttributeDefinitionWithDefaultValue<size_t> {
        private:
            ChoiceAttributeOption::List m_options;
        public:
            ChoiceAttributeDefinition(const String& name, const String& description, const ChoiceAttributeOption::List& options, const size_t defaultValue);
            ChoiceAttributeDefinition(const String& name, const String& description, const ChoiceAttributeOption::List& options);
            const ChoiceAttributeOption::List& options() const;
        private:
            bool doEquals(const AttributeDefinition* other) const;
        };
        
        class FlagsAttributeOption {
        public:
            typedef std::vector<FlagsAttributeOption> List;
        private:
            int m_value;
            String m_description;
            bool m_isDefault;
        public:
            FlagsAttributeOption(const int value, const String& description, const bool isDefault);
            bool operator==(const FlagsAttributeOption& other) const;
            int value() const;
            const String& description() const;
            bool isDefault() const;
        };
    
        class FlagsAttributeDefinition : public AttributeDefinition {
        private:
            FlagsAttributeOption::List m_options;
        public:
            FlagsAttributeDefinition(const String& name, const String& description, const int defaultValue);
            FlagsAttributeDefinition(const String& name, const String& description);

            int defaultValue() const;
            const FlagsAttributeOption::List& options() const;
            const FlagsAttributeOption* option(const int value) const;
            void addOption(const int value, const String& description, const bool isDefault);
        private:
            bool doEquals(const AttributeDefinition* other) const;
        };
    }
}

#endif /* defined(__TrenchBroom__AttributeDefinition__) */
