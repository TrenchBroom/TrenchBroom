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

#pragma once

#include "EL/Types.h"

// FIXME: try to remove some of these headers
#include <iosfwd>
#include <variant>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace EL {
        class NullType {
        private:
            NullType();
        public:
            static const NullType Value;
        };
        
        class UndefinedType {
        private:
            UndefinedType();
        public:
            static const UndefinedType Value;
        };
        
        class Value {
        private:
            std::variant<BooleanType, StringType, NumberType, ArrayType, MapType, RangeType, NullType, UndefinedType> m_value;
            size_t m_line;
            size_t m_column;
        private:
            template <typename T>
            static ArrayType makeArray(const std::vector<T>& values, const size_t line, const size_t column) {
                ArrayType result;
                result.reserve(values.size());
                for (const auto& value : values) {
                    result.emplace_back(value, line, column);
                }
                return result;
            }
        public:
            static const Value Null;
            static const Value Undefined;
            
            Value();
        
            explicit Value(BooleanType value, size_t line = 0u, size_t column = 0u);
            explicit Value(StringType value, size_t line = 0u, size_t column = 0u);
            explicit Value(const char* value, size_t line = 0u, size_t column = 0u);
            explicit Value(NumberType value, size_t line = 0u, size_t column = 0u);
            explicit Value(int value, size_t line = 0u, size_t column = 0u);
            explicit Value(long value, size_t line = 0u, size_t column = 0u);
            explicit Value(size_t value, size_t line = 0u, size_t column = 0u);
            explicit Value(ArrayType value, size_t line = 0u, size_t column = 0u);
            explicit Value(MapType value, size_t line = 0u, size_t column = 0u);
            explicit Value(RangeType value, size_t line = 0u, size_t column = 0u);
            explicit Value(NullType value, size_t line = 0u, size_t column = 0u);
            explicit Value(UndefinedType value, size_t line = 0u, size_t column = 0u);
        
            template <typename T>
            explicit Value(const std::vector<T>& value, const size_t line = 0u, const size_t column = 0u) :
            m_value(makeArray(value, line, column)),
            m_line(line),
            m_column(column) {}
            
            Value(Value value, size_t line, size_t column);

            Value(const Value&) = default;
            Value(Value&&) = default;
            
            Value& operator=(const Value&) = default;
            Value& operator=(Value&&) = default;
        
            ValueType type() const;
            std::string typeName() const;
            std::string describe() const;
            
            size_t line() const;
            size_t column() const;

            const BooleanType& booleanValue() const;
            const StringType& stringValue() const;
            const NumberType& numberValue() const;
            IntegerType integerValue() const;
            const ArrayType& arrayValue() const;
            const MapType& mapValue() const;
            const RangeType& rangeValue() const;

            bool null() const;
            bool undefined() const;
            
            const std::vector<std::string> asStringList() const;
            const std::vector<std::string> asStringSet() const;

            size_t length() const;
            bool convertibleTo(ValueType toType) const;
            Value convertTo(ValueType toType) const;

            std::string asString(bool multiline = false) const;
            void appendToStream(std::ostream& str, bool multiline = true, const std::string& indent = "") const;

            bool contains(const Value& indexValue) const;
            bool contains(size_t index) const;
            bool contains(const std::string& key) const;
            std::vector<std::string> keys() const;

            Value operator[](const Value& indexValue) const;
            Value operator[](size_t index) const;
            Value operator[](int index) const;
            Value operator[](const std::string& key) const;
            Value operator[](const char* key) const;

            operator bool() const;
        };
        
        std::ostream& operator<<(std::ostream& stream, const Value& value);

        Value operator+(const Value& v);
        Value operator-(const Value& v);

        Value operator+(const Value& lhs, const Value& rhs);
        Value operator-(const Value& lhs, const Value& rhs);
        Value operator*(const Value& lhs, const Value& rhs);
        Value operator/(const Value& lhs, const Value& rhs);
        Value operator%(const Value& lhs, const Value& rhs);

        Value operator!(const Value& v);

        bool operator==(const Value& lhs, const Value& rhs);
        bool operator!=(const Value& lhs, const Value& rhs);
        bool operator<(const Value& lhs, const Value& rhs);
        bool operator<=(const Value& lhs, const Value& rhs);
        bool operator>(const Value& lhs, const Value& rhs);
        bool operator>=(const Value& lhs, const Value& rhs);

        int compare(const Value& lhs, const Value& rhs);
        int compareAsBooleans(const Value& lhs, const Value& rhs);
        int compareAsNumbers(const Value& lhs, const Value& rhs);

        Value operator~(const Value& v);

        Value operator&(const Value& lhs, const Value& rhs);
        Value operator|(const Value& lhs, const Value& rhs);
        Value operator^(const Value& lhs, const Value& rhs);
        Value operator<<(const Value& lhs, const Value& rhs);
        Value operator>>(const Value& lhs, const Value& rhs);
    }
}

