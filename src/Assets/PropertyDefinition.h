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

#ifndef __TrenchBroom__PropertyDefinition__
#define __TrenchBroom__PropertyDefinition__

#include "StringUtils.h"
#include "Model/EntityProperties.h"
#include "Exceptions.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class PropertyDefinition {
        public:
            typedef enum {
                TargetSourceProperty,
                TargetDestinationProperty,
                StringProperty,
                IntegerProperty,
                FloatProperty,
                ChoiceProperty,
                FlagsProperty
            } Type;
        private:
            String m_name;
            Type m_type;
            String m_description;
        public:
            PropertyDefinition(const String& name, const Type type, const String& description);
            virtual ~PropertyDefinition();
            
            const String& name() const;
            Type type() const;
            const String& description() const;
        };
        
        template <typename T>
        class PropertyDefinitionWithDefaultValue : public PropertyDefinition {
        private:
            bool m_hasDefaultValue;
            T m_defaultValue;
        public:
            bool hasDefaultValue() const {
                return m_hasDefaultValue;
            }
            
            const T& defaultValue() const {
                if (!hasDefaultValue())
                    throw EntityPropertyException(name() + " has no default value");
                return m_defaultValue;
            }
        protected:
            PropertyDefinitionWithDefaultValue(const String& name, const Type type, const String& description) :
            PropertyDefinition(name, type, description),
            m_hasDefaultValue(false) {}
            
            PropertyDefinitionWithDefaultValue(const String& name, const Type type, const String& description, const T& defaultValue) :
            PropertyDefinition(name, type, description),
            m_hasDefaultValue(true),
            m_defaultValue(defaultValue) {}
        };
        
        class StringPropertyDefinition : public PropertyDefinitionWithDefaultValue<String> {
        public:
            StringPropertyDefinition(const String& name, const String& description, const String& defaultValue);
            StringPropertyDefinition(const String& name, const String& description);
        };
        
        class IntegerPropertyDefinition : public PropertyDefinitionWithDefaultValue<int> {
        public:
            IntegerPropertyDefinition(const String& name, const String& description, const int defaultValue);
            IntegerPropertyDefinition(const String& name, const String& description);
        };
        
        class FloatPropertyDefinition : public PropertyDefinitionWithDefaultValue<float> {
        public:
            FloatPropertyDefinition(const String& name, const String& description, const float defaultValue);
            FloatPropertyDefinition(const String& name, const String& description);
        };
        
        class ChoicePropertyOption {
        public:
            typedef std::vector<ChoicePropertyOption> List;
        private:
            String m_value;
            String m_description;
        public:
            ChoicePropertyOption(const String& value, const String& description);
            const String& value() const;
            const String& description() const;
        };
        
        class ChoicePropertyDefinition : public PropertyDefinitionWithDefaultValue<size_t> {
        private:
            ChoicePropertyOption::List m_options;
        public:
            ChoicePropertyDefinition(const String& name, const String& description, const ChoicePropertyOption::List options, const size_t defaultValue);
            ChoicePropertyDefinition(const String& name, const String& description, const ChoicePropertyOption::List options);
            const ChoicePropertyOption::List& options() const;
        };
        
        class FlagsPropertyOption {
        public:
            typedef std::vector<FlagsPropertyOption> List;
        private:
            int m_value;
            String m_description;
            bool m_isDefault;
        public:
            FlagsPropertyOption(const int value, const String& description, const bool isDefault);
            int value() const;
            const String& description() const;
            bool isDefault() const;
        };
    
        class FlagsPropertyDefinition : public PropertyDefinition {
        private:
            FlagsPropertyOption::List m_options;
        public:
            FlagsPropertyDefinition(const String& name, const String& description, const int defaultValue);
            FlagsPropertyDefinition(const String& name, const String& description);

            int defaultValue() const;
            const FlagsPropertyOption::List& options() const;
            const FlagsPropertyOption* option(const int value) const;
            void addOption(const int value, const String& description, const bool isDefault);
        };
    }
}

#endif /* defined(__TrenchBroom__PropertyDefinition__) */
