/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__PropertyDefinition__
#define __TrenchBroom__PropertyDefinition__

#include "Model/EntityProperty.h"
#include "Utility/SharedPointer.h"
#include "Utility/String.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class PropertyDefinition {
        public:
            typedef std::tr1::shared_ptr<PropertyDefinition> Ptr;
            typedef std::map<String, Ptr> Map;
            typedef std::vector<Ptr> List;
            
            enum Type {
                TargetSourceProperty,
                TargetDestinationProperty,
                StringProperty,
                IntegerProperty,
                ChoiceProperty,
                FlagsProperty
            };
        private:
            String m_name;
            Type m_type;
            String m_description;
        public:
            PropertyDefinition(const String& name, Type type, const String& description) :
            m_name(name),
            m_type(type),
            m_description(description) {}
            
            virtual ~PropertyDefinition() {}
            
            inline const String& name() const {
                return m_name;
            }
            
            inline Type type() const {
                return m_type;
            }
            
            inline const String& description() const {
                return m_description;
            }
            
            virtual const Model::PropertyValue defaultPropertyValue() const {
                return "";
            }
        };

        class StringPropertyDefinition : public PropertyDefinition {
        private:
            String m_defaultValue;
        public:
            StringPropertyDefinition(const String& name, const String& description, const String& defaultValue) :
            PropertyDefinition(name, StringProperty, description),
            m_defaultValue(defaultValue) {}

            const Model::PropertyValue defaultPropertyValue() const {
                return m_defaultValue;
            }
        };
        
        class IntegerPropertyDefinition : public PropertyDefinition {
        private:
            int m_defaultValue;
        public:
            IntegerPropertyDefinition(const String& name, const String& description, int defaultValue) :
            PropertyDefinition(name, IntegerProperty, description),
            m_defaultValue(defaultValue) {}
            
            inline int defaultValue() const {
                return m_defaultValue;
            }
            
            const Model::PropertyValue defaultPropertyValue() const {
                StringStream buffer;
                buffer << m_defaultValue;
                return buffer.str();
            }
        };
        
        class FloatPropertyDefinition : public PropertyDefinition {
        private:
            float m_defaultValue;
        public:
            FloatPropertyDefinition(const String& name, const String& description, float defaultValue) :
            PropertyDefinition(name, IntegerProperty, description),
            m_defaultValue(defaultValue) {}
            
            inline float defaultValue() const {
                return m_defaultValue;
            }
            
            const Model::PropertyValue defaultPropertyValue() const {
                StringStream buffer;
                buffer << m_defaultValue;
                return buffer.str();
            }
        };

        class ChoicePropertyOption {
        public:
            typedef std::vector<ChoicePropertyOption> List;
        private:
            String m_value;
            String m_description;
        public:
            ChoicePropertyOption(const String& value, const String& description) :
            m_value(value),
            m_description(description) {}
            
            inline const String& value() const {
                return m_value;
            }
            
            inline const String& description() const {
                return m_description;
            }
        };

        class ChoicePropertyDefinition : public PropertyDefinition {
        private:
            int m_defaultValue;
            ChoicePropertyOption::List m_options;
        public:
            ChoicePropertyDefinition(const String& name, const String& description, int defaultValue) :
            PropertyDefinition(name, ChoiceProperty, description),
            m_defaultValue(defaultValue) {}
            
            inline int defaultValue() const {
                return m_defaultValue;
            }
            
            inline void addOption(const String& value, const String& description) {
                m_options.push_back(ChoicePropertyOption(value, description));
            }
            
            inline const ChoicePropertyOption::List& options() const {
                return m_options;
            }
            
            const Model::PropertyValue defaultPropertyValue() const {
                StringStream buffer;
                buffer << m_defaultValue;
                return buffer.str();
            }
        };
        
        class FlagsPropertyOption {
        public:
            typedef std::vector<FlagsPropertyOption> List;
        private:
            int m_value;
            String m_description;
            bool m_isDefault;
        public:
            FlagsPropertyOption(int value, const String& description, bool isDefault) :
            m_value(value),
            m_description(description),
            m_isDefault(isDefault) {}
            
            inline int value() const {
                return m_value;
            }
            
            inline const String& description() const {
                return m_description;
            }
            
            inline bool isDefault() const {
                return m_isDefault;
            }
        };
        
        class FlagsPropertyDefinition : public PropertyDefinition {
        private:
            FlagsPropertyOption::List m_options;
        public:
            FlagsPropertyDefinition(const String& name, const String& description) :
            PropertyDefinition(name, FlagsProperty, description) {}
            
            inline void addOption(int value, const String& description, bool isDefault) {
                m_options.push_back(FlagsPropertyOption(value, description, isDefault));
            }
            
            inline const FlagsPropertyOption::List& options() const {
                return m_options;
            }
            
            inline FlagsPropertyOption* option(int value) {
                FlagsPropertyOption::List::iterator it, end;
                for (it = m_options.begin(), end = m_options.end(); it != end; ++it) {
                    FlagsPropertyOption& option = *it;
                    if (option.value() == value)
                        return &option;
                }
                return NULL;
            }

            inline const FlagsPropertyOption* option(int value) const {
                FlagsPropertyOption::List::const_iterator it, end;
                for (it = m_options.begin(), end = m_options.end(); it != end; ++it) {
                    const FlagsPropertyOption& option = *it;
                    if (option.value() == value)
                        return &option;
                }
                return NULL;
            }
            
            const Model::PropertyValue defaultPropertyValue() const {
                int value = 0;
                FlagsPropertyOption::List::const_iterator it, end;
                for (it = m_options.begin(), end = m_options.end(); it != end; ++it) {
                    const FlagsPropertyOption& option = *it;
                    if (option.isDefault())
                        value |= option.value();
                }

                StringStream buffer;
                buffer << value;
                return buffer.str();
            }
        };
    }
}

#endif /* defined(__TrenchBroom__PropertyDefinition__) */
