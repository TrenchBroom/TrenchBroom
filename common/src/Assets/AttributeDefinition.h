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

#include "Ensure.h"

#include <nonstd/optional.hpp>
#include <string>
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
            std::string m_name;
            Type m_type;
            std::string m_shortDescription;
            std::string m_longDescription;
            bool m_readOnly;
        public:
            AttributeDefinition(const std::string& name, Type type, const std::string& shortDescription, const std::string& longDescription, bool readOnly);
            virtual ~AttributeDefinition();

            const std::string& name() const;
            Type type() const;
            const std::string& shortDescription() const;
            const std::string& longDescription() const;

            bool readOnly() const;

            bool equals(const AttributeDefinition* other) const;

            static std::string defaultValue(const AttributeDefinition& definition);

            AttributeDefinition* clone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const;
        private:
            virtual bool doEquals(const AttributeDefinition* other) const;
            virtual AttributeDefinition* doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const;
        };

        template <typename T>
        class AttributeDefinitionWithDefaultValue : public AttributeDefinition {
        protected:
            nonstd::optional<T> m_defaultValue;
        public:
            bool hasDefaultValue() const {
                return m_defaultValue.has_value();
            }

            const T& defaultValue() const {
                ensure(hasDefaultValue(), "attribute definition has no default value");
                return *m_defaultValue;
            }
        protected:
            AttributeDefinitionWithDefaultValue(const std::string& name, Type type, const std::string& shortDescription, const std::string& longDescription, bool readOnly, nonstd::optional<T> defaultValue = nonstd::nullopt) :
            AttributeDefinition(name, type, shortDescription, longDescription, readOnly),
            m_defaultValue(std::move(defaultValue)) {}
        };

        class StringAttributeDefinition : public AttributeDefinitionWithDefaultValue<std::string> {
        public:
            StringAttributeDefinition(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly, nonstd::optional<std::string> defaultValue = nonstd::nullopt);
        private:
            AttributeDefinition* doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const override;
        };

        class BooleanAttributeDefinition : public AttributeDefinitionWithDefaultValue<bool> {
        public:
            BooleanAttributeDefinition(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly, nonstd::optional<bool> defaultValue = nonstd::nullopt);
        private:
            AttributeDefinition* doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const override;
        };

        class IntegerAttributeDefinition : public AttributeDefinitionWithDefaultValue<int> {
        public:
            IntegerAttributeDefinition(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly, nonstd::optional<int> defaultValue = nonstd::nullopt);
        private:
            AttributeDefinition* doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const override;
        };

        class FloatAttributeDefinition : public AttributeDefinitionWithDefaultValue<float> {
        public:
            FloatAttributeDefinition(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly, nonstd::optional<float> defaultValue = nonstd::nullopt);
        private:
            AttributeDefinition* doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const override;
        };

        class ChoiceAttributeOption {
        public:
            using List = std::vector<ChoiceAttributeOption>;
        private:
            std::string m_value;
            std::string m_description;
        public:
            ChoiceAttributeOption(const std::string& value, const std::string& description);
            bool operator==(const ChoiceAttributeOption& other) const;
            const std::string& value() const;
            const std::string& description() const;
        };

        class ChoiceAttributeDefinition : public AttributeDefinitionWithDefaultValue<std::string> {
        private:
            ChoiceAttributeOption::List m_options;
        public:
            ChoiceAttributeDefinition(const std::string& name, const std::string& shortDescription, const std::string& longDescription, const ChoiceAttributeOption::List& options, bool readOnly, nonstd::optional<std::string> defaultValue = nonstd::nullopt);
            const ChoiceAttributeOption::List& options() const;
        private:
            bool doEquals(const AttributeDefinition* other) const override;
            AttributeDefinition* doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const override;
        };

        class FlagsAttributeOption {
        public:
            using List = std::vector<FlagsAttributeOption>;
        private:
            int m_value;
            std::string m_shortDescription;
            std::string m_longDescription;
            bool m_isDefault;
        public:
            FlagsAttributeOption(int value, const std::string& shortDescription, const std::string& longDescription, bool isDefault);
            bool operator==(const FlagsAttributeOption& other) const;
            int value() const;
            const std::string& shortDescription() const;
            const std::string& longDescription() const;
            bool isDefault() const;
        };

        class FlagsAttributeDefinition : public AttributeDefinition {
        private:
            FlagsAttributeOption::List m_options;
        public:
            explicit FlagsAttributeDefinition(const std::string& name);

            int defaultValue() const;
            const FlagsAttributeOption::List& options() const;
            const FlagsAttributeOption* option(int value) const;
            void addOption(int value, const std::string& shortDescription, const std::string& longDescription, bool isDefault);
        private:
            bool doEquals(const AttributeDefinition* other) const override;
            AttributeDefinition* doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const override;
        };

        class UnknownAttributeDefinition : public StringAttributeDefinition {
        public:
            UnknownAttributeDefinition(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly, nonstd::optional<std::string> defaultValue = nonstd::nullopt);
        private:
            AttributeDefinition* doClone(const std::string& name, const std::string& shortDescription, const std::string& longDescription, bool readOnly) const override;
        };
    }
}

#endif /* defined(TrenchBroom_AttributeDefinition) */
