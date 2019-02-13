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

#ifndef TrenchBroom_AttributeDefinition
#define TrenchBroom_AttributeDefinition

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
                Type_BooleanAttribute,
                Type_IntegerAttribute,
                Type_FloatAttribute,
                Type_ChoiceAttribute,
                Type_FlagsAttribute
            } Type;
        private:
            String m_name;
            Type m_type;
            String m_shortDescription;
            String m_longDescription;
            bool m_readOnly;
        public:
            AttributeDefinition(const String& name, Type type, const String& shortDescription, const String& longDescription, bool readOnly);
            virtual ~AttributeDefinition();
            
            const String& name() const;
            Type type() const;
            const String& shortDescription() const;
            const String& longDescription() const;
            String fullDescription() const;
            bool readOnly() const;
            
            static String safeFullDescription(const AttributeDefinition* definition);
            
            bool equals(const AttributeDefinition* other) const;
            
            static String defaultValue(const AttributeDefinition& definition);

            AttributeDefinition* clone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const;
        private:
            virtual bool doEquals(const AttributeDefinition* other) const;
            virtual AttributeDefinition* doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const;
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
            AttributeDefinitionWithDefaultValue(const String& name, Type type, const String& shortDescription, const String& longDescription, bool readOnly) :
            AttributeDefinition(name, type, shortDescription, longDescription, readOnly),
            m_hasDefaultValue(false) {}
            
            AttributeDefinitionWithDefaultValue(const String& name, Type type, const String& shortDescription, const String& longDescription, const T& defaultValue, bool readOnly) :
            AttributeDefinition(name, type, shortDescription, longDescription, readOnly),
            m_hasDefaultValue(true),
            m_defaultValue(defaultValue) {}
        };
        
        class StringAttributeDefinition : public AttributeDefinitionWithDefaultValue<String> {
        public:
            StringAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const String& defaultValue, bool readOnly);
            StringAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, bool readOnly);
        private:
            AttributeDefinition* doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const override;
        };

        class BooleanAttributeDefinition : public AttributeDefinitionWithDefaultValue<bool> {
        public:
            BooleanAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, bool defaultValue, bool readOnly);
            BooleanAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, bool readOnly);
        private:
            AttributeDefinition* doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const override;
        };

        class IntegerAttributeDefinition : public AttributeDefinitionWithDefaultValue<int> {
        public:
            IntegerAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, int defaultValue, bool readOnly);
            IntegerAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, bool readOnly);
        private:
            AttributeDefinition* doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const override;
        };
        
        class FloatAttributeDefinition : public AttributeDefinitionWithDefaultValue<float> {
        public:
            FloatAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, float defaultValue, bool readOnly);
            FloatAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, bool readOnly);
        private:
            AttributeDefinition* doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const override;
        };
        
        class ChoiceAttributeOption {
        public:
            using List = std::vector<ChoiceAttributeOption>;
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
            ChoiceAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const ChoiceAttributeOption::List& options, const size_t defaultValue, bool readOnly);
            ChoiceAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const ChoiceAttributeOption::List& options, bool readOnly);
            const ChoiceAttributeOption::List& options() const;
        private:
            bool doEquals(const AttributeDefinition* other) const override;
            AttributeDefinition* doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const override;
        };
        
        class FlagsAttributeOption {
        public:
            using List = std::vector<FlagsAttributeOption>;
        private:
            int m_value;
            String m_shortDescription;
            String m_longDescription;
            bool m_isDefault;
        public:
            FlagsAttributeOption(int value, const String& shortDescription, const String& longDescription, bool isDefault);
            bool operator==(const FlagsAttributeOption& other) const;
            int value() const;
            const String& shortDescription() const;
            const String& longDescription() const;
            bool isDefault() const;
        };
    
        class FlagsAttributeDefinition : public AttributeDefinition {
        private:
            FlagsAttributeOption::List m_options;
        public:
            explicit FlagsAttributeDefinition(const String& name);

            int defaultValue() const;
            const FlagsAttributeOption::List& options() const;
            const FlagsAttributeOption* option(int value) const;
            void addOption(int value, const String& shortDescription, const String& longDescription, bool isDefault);
        private:
            bool doEquals(const AttributeDefinition* other) const override;
            AttributeDefinition* doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const override;
        };
        
        class UnknownAttributeDefinition : public StringAttributeDefinition {
        public:
            UnknownAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, const String& defaultValue, bool readOnly);
            UnknownAttributeDefinition(const String& name, const String& shortDescription, const String& longDescription, bool readOnly);
        private:
            AttributeDefinition* doClone(const String& name, const String& shortDescription, const String& longDescription, bool readOnly) const override;
        };
    }
}

#endif /* defined(TrenchBroom_AttributeDefinition) */
