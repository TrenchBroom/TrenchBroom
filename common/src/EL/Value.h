/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef Value_h
#define Value_h

#include "StringUtils.h"
#include "SharedPointer.h"
#include "EL/Types.h"

#include <algorithm>
#include <iterator>

namespace TrenchBroom {
    namespace EL {
        class ValueHolder {
        public:
            virtual ~ValueHolder();
            
            virtual ValueType type() const = 0;
            String describe() const;
            
            virtual const BooleanType& booleanValue() const;
            virtual const StringType&  stringValue()  const;
            virtual const NumberType&  numberValue()  const;
                          IntegerType  integerValue() const;
            virtual const ArrayType&   arrayValue()   const;
            virtual const MapType&     mapValue()     const;
            virtual const RangeType&   rangeValue()   const;
            
            virtual size_t length() const = 0;
            virtual bool convertibleTo(ValueType toType) const = 0;
            virtual ValueHolder* convertTo(ValueType toType) const = 0;
            
            virtual ValueHolder* clone() const = 0;
            
            virtual void appendToStream(std::ostream& str, bool multiline, const String& indent) const = 0;
        };
        
        class BooleanValueHolder : public ValueHolder {
        private:
            BooleanType m_value;
        public:
            BooleanValueHolder(const BooleanType& value);
            ValueType type() const;
            const BooleanType& booleanValue() const;
            size_t length() const;
            bool convertibleTo(ValueType toType) const;
            ValueHolder* convertTo(ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str, bool multiline, const String& indent) const;
        };

        class StringHolder : public ValueHolder {
        public:
            virtual ~StringHolder();
            
            ValueType type() const;
            const StringType& stringValue() const;
            size_t length() const;
            bool convertibleTo(ValueType toType) const;
            ValueHolder* convertTo(ValueType toType) const;
            void appendToStream(std::ostream& str, bool multiline, const String& indent) const;
        private:
            virtual const StringType& doGetValue() const = 0;
        };
        
        class StringValueHolder : public StringHolder {
        private:
            StringType m_value;
        public:
            StringValueHolder(const StringType& value);
            ValueHolder* clone() const;
        private:
            const StringType& doGetValue() const;
        };
        
        class StringReferenceHolder : public StringHolder {
        private:
            const StringType& m_value;
        public:
            StringReferenceHolder(const StringType& value);
            ValueHolder* clone() const;
        private:
            const StringType& doGetValue() const;
        };
        
        class NumberValueHolder : public ValueHolder {
        private:
            NumberType m_value;
        public:
            NumberValueHolder(const NumberType& value);
            ValueType type() const;
            const NumberType& numberValue() const;
            size_t length() const;
            bool convertibleTo(ValueType toType) const;
            ValueHolder* convertTo(ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str, bool multiline, const String& indent) const;
        };
        
        class ArrayValueHolder : public ValueHolder {
        private:
            ArrayType m_value;
        public:
            ArrayValueHolder(const ArrayType& value);
            ValueType type() const;
            const ArrayType& arrayValue() const;
            size_t length() const;
            bool convertibleTo(ValueType toType) const;
            ValueHolder* convertTo(ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str, bool multiline, const String& indent) const;
        };
        
        class MapValueHolder : public ValueHolder {
        private:
            MapType m_value;
        public:
            MapValueHolder(const MapType& value);
            ValueType type() const;
            const MapType& mapValue() const;
            size_t length() const;
            bool convertibleTo(ValueType toType) const;
            ValueHolder* convertTo(ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str, bool multiline, const String& indent) const;
        };
        
        class RangeValueHolder : public ValueHolder {
        private:
            RangeType m_value;
        public:
            RangeValueHolder(const RangeType& value);
            ValueType type() const;
            const RangeType& rangeValue() const;
            size_t length() const;
            bool convertibleTo(ValueType toType) const;
            ValueHolder* convertTo(ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str, bool multiline, const String& indent) const;
        };
        
        class NullValueHolder : public ValueHolder {
        public:
            ValueType type() const;
            const StringType& stringValue() const;
            const BooleanType& booleanValue() const;
            const NumberType& numberValue() const;
            const ArrayType& arrayValue() const;
            const MapType& mapValue() const;
            size_t length() const;
            bool convertibleTo(ValueType toType) const;
            ValueHolder* convertTo(ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str, bool multiline, const String& indent) const;
        };
        
        class UndefinedValueHolder : public ValueHolder {
        public:
            ValueType type() const;
            size_t length() const;
            bool convertibleTo(ValueType toType) const;
            ValueHolder* convertTo(ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str, bool multiline, const String& indent) const;
        };
        
        class Value {
        public:
            static const Value Null;
            static const Value Undefined;
            typedef std::set<Value> Set;
        private:
            typedef std::vector<size_t> IndexArray;
            typedef std::shared_ptr<ValueHolder> ValuePtr;
            ValuePtr m_value;
            size_t m_line;
            size_t m_column;
        private:
            Value(ValueHolder* holder, size_t line, size_t column);
        public:
            Value(const BooleanType& value, size_t line, size_t column);
            explicit Value(const BooleanType& value);
            
            Value(const StringType& value, size_t line, size_t column);
            explicit Value(const StringType& value);
            
            Value(const char* value, size_t line, size_t column);
            explicit Value(const char* value);
            
            Value(const NumberType& value, size_t line, size_t column);
            explicit Value(const NumberType& value);
            
            Value(int value, size_t line, size_t column);
            explicit Value(int value);
            
            Value(long value, size_t line, size_t column);
            explicit Value(long value);
            
            Value(size_t value, size_t line, size_t column);
            explicit Value(size_t value);
            
            Value(const ArrayType& value, size_t line, size_t column);
            explicit Value(const ArrayType& value);
            
            template <typename T>
            Value(const std::vector<T>& value, size_t line, size_t column) :
            m_value(new ArrayValueHolder(makeArray(value))),
            m_line(line),
            m_column(column){}
            
            template <typename T>
            explicit Value(const std::vector<T>& value) :
            m_value(new ArrayValueHolder(makeArray(value))),
            m_line(0),
            m_column(0) {}
            
            Value(const MapType& value, size_t line, size_t column);
            explicit Value(const MapType& value);
            
            template <typename T, typename C>
            Value(const std::map<String, T, C>& value, size_t line, size_t column) :
            m_value(new MapValueHolder(makeMap(value))),
            m_line(line),
            m_column(column) {}
            
            template <typename T, typename C>
            explicit Value(const std::map<String, T, C>& value) :
            m_value(new MapValueHolder(makeMap(value))),
            m_line(0),
            m_column(0) {}
            
            Value(const RangeType& value, size_t line, size_t column);
            explicit Value(const RangeType& value);
            
            Value(const Value& other, size_t line, size_t column);
            
            Value();
            
            static Value ref(const StringType& value, size_t line, size_t column);
            static Value ref(const StringType& value);
        private:
            template <typename T>
            ArrayType makeArray(const std::vector<T>& value) {
                ArrayType result;
                result.reserve(value.size());
                std::transform(std::begin(value), std::end(value), std::back_inserter(result),
                               [](const T& elem) { return EL::Value(elem); });
                return result;
            }
            
            template <typename T, typename C>
            MapType makeMap(const std::map<String, T, C>& value) {
                typedef typename std::map<String, T, C>::value_type Entry;
                MapType result;
                std::transform(std::begin(value), std::end(value), std::inserter(result, result.begin()),
                               [](const Entry& entry) { return std::make_pair(entry.first, EL::Value(entry.second)); });
                return result;
            }
        public:
            ValueType type() const;
            String typeName() const;
            String describe() const;
            
            size_t line() const;
            size_t column() const;
            
            const StringType& stringValue() const;
            const BooleanType& booleanValue() const;
            const NumberType& numberValue() const;
                  IntegerType integerValue() const;
            const ArrayType& arrayValue() const;
            const MapType& mapValue() const;
            const RangeType& rangeValue() const;
            bool null() const;
            bool undefined() const;
            
            const StringArray asStringList() const;
            const StringSet asStringSet() const;
            
            size_t length() const;
            bool convertibleTo(ValueType toType) const;
            Value convertTo(ValueType toType) const;
            
            String asString(bool multiline = false) const;
            void appendToStream(std::ostream& str, bool multiline = true, const String& indent = "") const;
            friend std::ostream& operator<<(std::ostream& stream, const Value& value);
            
            bool contains(const Value& indexValue) const;
            bool contains(size_t index) const;
            bool contains(const String& key) const;
            StringSet keys() const;
            
            Value operator[](const Value& indexValue) const;
            Value operator[](size_t index) const;
            Value operator[](int index) const;
            Value operator[](const String& key) const;
            Value operator[](const char* key) const;
        private:
            IndexArray computeIndexArray(const Value& indexValue, size_t indexableSize) const;
            void computeIndexArray(const Value& indexValue, size_t indexableSize, IndexArray& result) const;
            size_t computeIndex(const Value& indexValue, size_t indexableSize) const;
            size_t computeIndex(long index, size_t indexableSize) const;
        public:
            Value operator+() const;
            Value operator-() const;
            
            friend Value operator+(const Value& lhs, const Value& rhs);
            friend Value operator-(const Value& lhs, const Value& rhs);
            friend Value operator*(const Value& lhs, const Value& rhs);
            friend Value operator/(const Value& lhs, const Value& rhs);
            friend Value operator%(const Value& lhs, const Value& rhs);
            
            operator bool() const;
            Value operator!() const;
            
            friend bool operator==(const Value& lhs, const Value& rhs);
            friend bool operator!=(const Value& lhs, const Value& rhs);
            friend bool operator<(const Value& lhs, const Value& rhs);
            friend bool operator<=(const Value& lhs, const Value& rhs);
            friend bool operator>(const Value& lhs, const Value& rhs);
            friend bool operator>=(const Value& lhs, const Value& rhs);
        private:
            friend int compare(const Value& lhs, const Value& rhs);
            friend int compareAsBooleans(const Value& lhs, const Value& rhs);
            friend int compareAsNumbers(const Value& lhs, const Value& rhs);
        public:
            Value operator~() const;
            
            friend Value operator&(const Value& lhs, const Value& rhs);
            friend Value operator|(const Value& lhs, const Value& rhs);
            friend Value operator^(const Value& lhs, const Value& rhs);
            friend Value operator<<(const Value& lhs, const Value& rhs);
            friend Value operator>>(const Value& lhs, const Value& rhs);
        };
    }
}

#endif /* Value_h */
