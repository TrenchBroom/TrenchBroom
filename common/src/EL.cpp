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

#include "EL.h"

#include "CollectionUtils.h"

#include <cassert>
#include <cmath>

namespace TrenchBroom {
    namespace EL {
        String typeName(const ValueType type) {
            switch (type) {
                case Type_Boolean:
                    return "Boolean";
                case Type_String:
                    return "String";
                case Type_Number:
                    return "Number";
                case Type_Array:
                    return "Array";
                case Type_Map:
                    return "Map";
                case Type_Range:
                    return "Range";
                case Type_Null:
                    return "Null";
            }
        }

        ELException::ELException() throw() {}
        ELException::ELException(const String& str) throw() : ExceptionStream(str) {}
        ELException::~ELException() throw() {}
        
        ConversionError::ConversionError(const String& value, const ValueType from, const ValueType to) throw() :
        ELException("Cannot convert value '" + value + "' of type '" + typeName(from) + "' to type '" + typeName(to) + "'") {}

        ValueError::ValueError(const String& value, const ValueType from, const ValueType to) throw() :
        ELException("Cannot dereference value '" + value + "' of type '" + typeName(from) + "' as type '" + typeName(to) + "'") {}

        EvaluationError::EvaluationError(const String& msg) throw() :
        ELException(msg) {}

        IndexError::IndexError(const Value& indexableValue, const Value& indexValue) throw() :
        EvaluationError("Cannot index value '" + indexableValue.description() + "' of type '" + indexableValue.typeName() + " with '" + indexValue.description() + "' of type '" + typeName(indexValue.type()) + "'") {}

        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, const size_t outOfBoundsIndex) throw() :
        IndexError(indexableValue, indexValue) {
            *this << ": Index value " << outOfBoundsIndex << " is out of bounds";
        }

        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, const String& outOfBoundsIndex) throw() :
        IndexError(indexableValue, indexValue) {
            *this << ": Key '" << outOfBoundsIndex << "' not found";
        }

        ValueHolder::~ValueHolder() {}
        
        const BooleanType& ValueHolder::booleanValue() const { throw ValueError(description(), type(), Type_Boolean); }
        const StringType&  ValueHolder::stringValue()  const { throw ValueError(description(), type(), Type_String); }
        const NumberType&  ValueHolder::numberValue()  const { throw ValueError(description(), type(), Type_Number); }
        const ArrayType&   ValueHolder::arrayValue()   const { throw ValueError(description(), type(), Type_Array); }
        const MapType&     ValueHolder::mapValue()     const { throw ValueError(description(), type(), Type_Map); }
        const RangeType&   ValueHolder::rangeValue()   const { throw ValueError(description(), type(), Type_Range); }
        
        String ValueHolder::asString() const {
            StringStream str;
            str.precision(17);
            appendToStream(str);
            return str.str();
        }
        
        BooleanValueHolder::BooleanValueHolder(const BooleanType& value) : m_value(value) {}
        ValueType BooleanValueHolder::type() const { return Type_Boolean; }
        String BooleanValueHolder::description() const { return asString(); }
        const BooleanType& BooleanValueHolder::booleanValue() const { return m_value; }
        size_t BooleanValueHolder::length() const { return 1; }

        ValueHolder* BooleanValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case Type_Boolean:
                    return new BooleanValueHolder(m_value);
                case Type_String:
                    return new StringValueHolder(m_value ? "true" : "false" );
                case Type_Number:
                    return new NumberValueHolder(m_value ? 1.0 : 0.0);
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                    break;
            }
            
            throw ConversionError(description(), type(), toType);
        }
        
        ValueHolder* BooleanValueHolder::clone() const { return new BooleanValueHolder(m_value); }
        void BooleanValueHolder::appendToStream(std::ostream& str) const { str  << (m_value ? "true" : "false"); }

        StringValueHolder::StringValueHolder(const StringType& value) : m_value(value) {}
        ValueType StringValueHolder::type() const { return Type_String; }
        String StringValueHolder::description() const { return m_value; }
        const StringType& StringValueHolder::stringValue() const { return m_value; }
        size_t StringValueHolder::length() const { return m_value.length(); }
        
        ValueHolder* StringValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case Type_Boolean:
                    return new BooleanValueHolder(!StringUtils::caseInsensitiveEqual(m_value, "false") && !m_value.empty());
                case Type_String:
                    return new StringValueHolder(m_value);
                case Type_Number: {
                    if (m_value.empty())
                        throw ConversionError(description(), type(), toType);
                    const char* begin = m_value.c_str();
                    char* end;
                    const NumberType value = std::strtod(begin, &end);
                    if (value == 0.0 && end == begin)
                        throw ConversionError(description(), type(), toType);
                    return new NumberValueHolder(value);
                }
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                    break;
            }
            
            throw ConversionError(description(), type(), toType);
        }
        
        ValueHolder* StringValueHolder::clone() const { return new StringValueHolder(m_value); }
        void StringValueHolder::appendToStream(std::ostream& str) const { str << "\"" << m_value << "\""; }


        
        NumberValueHolder::NumberValueHolder(const NumberType& value) : m_value(value) {}
        ValueType NumberValueHolder::type() const { return Type_Number; }
        String NumberValueHolder::description() const { return asString(); }
        const NumberType& NumberValueHolder::numberValue() const { return m_value; }
        size_t NumberValueHolder::length() const { return 1; }
        
        ValueHolder* NumberValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case Type_Boolean:
                    return new BooleanValueHolder(m_value != 0.0);
                case Type_String:
                    return new StringValueHolder(asString());
                case Type_Number:
                    return new NumberValueHolder(m_value);
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                    break;
            }
            
            throw ConversionError(description(), type(), toType);
        }
        
        ValueHolder* NumberValueHolder::clone() const { return new NumberValueHolder(m_value); }
        void NumberValueHolder::appendToStream(std::ostream& str) const { str << m_value; }


        
        ArrayValueHolder::ArrayValueHolder(const ArrayType& value) : m_value(value) {}
        ValueType ArrayValueHolder::type() const { return Type_Array; }
        String ArrayValueHolder::description() const { return asString(); }
        const ArrayType& ArrayValueHolder::arrayValue() const { return m_value; }
        size_t ArrayValueHolder::length() const { return m_value.size(); }
        
        ValueHolder* ArrayValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case Type_Array:
                    return new ArrayValueHolder(m_value);
                case Type_Boolean:
                case Type_String:
                case Type_Number:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                    break;
            }
            
            throw ConversionError(description(), type(), toType);
        }
        
        ValueHolder* ArrayValueHolder::clone() const { return new ArrayValueHolder(m_value); }
        
        void ArrayValueHolder::appendToStream(std::ostream& str) const {
            str << "[";
            for (size_t i = 0; i < m_value.size(); ++i) {
                m_value[i].appendToStream(str);
                if (i < m_value.size() - 1)
                    str << ",";
            }
            str << "]";
        }
        
        
        MapValueHolder::MapValueHolder(const MapType& value) : m_value(value) {}
        ValueType MapValueHolder::type() const { return Type_Map; }
        String MapValueHolder::description() const { return asString(); }
        const MapType& MapValueHolder::mapValue() const { return m_value; }
        size_t MapValueHolder::length() const { return m_value.size(); }
        
        ValueHolder* MapValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case Type_Map:
                    return new MapValueHolder(m_value);
                case Type_Boolean:
                case Type_String:
                case Type_Number:
                case Type_Array:
                case Type_Range:
                case Type_Null:
                    break;
            }
            
            throw ConversionError(description(), type(), toType);
        }
        
        ValueHolder* MapValueHolder::clone() const { return new MapValueHolder(m_value); }

        void MapValueHolder::appendToStream(std::ostream& str) const {
            str << "{";
            MapType::const_iterator it, end;
            size_t i = 0;
            for (it = m_value.begin(), end = m_value.end(); it != end; ++it) {
                str << "\"" << it->first << "\"" << ":";
                it->second.appendToStream(str);
                if (i++ < m_value.size() - 1)
                    str << ",";
            }
            str << "}";
        }
        
        
        RangeValueHolder::RangeValueHolder(const RangeType& value) : m_value(value) {}
        ValueType RangeValueHolder::type() const { return Type_Range; }
        String RangeValueHolder::description() const { return asString(); }
        const RangeType& RangeValueHolder::rangeValue() const { return m_value; }
        size_t RangeValueHolder::length() const { return m_value.size(); }
        
        ValueHolder* RangeValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case Type_Range:
                    return new RangeValueHolder(m_value);
                case Type_Boolean:
                case Type_String:
                case Type_Number:
                case Type_Array:
                case Type_Map:
                case Type_Null:
                    break;
            }
            
            throw ConversionError(description(), type(), toType);
        }
        
        ValueHolder* RangeValueHolder::clone() const { return new RangeValueHolder(m_value); }
        
        void RangeValueHolder::appendToStream(std::ostream& str) const {
            str << "[";
            for (size_t i = 0; i < m_value.size(); ++i) {
                str << m_value[i];
                if (i < m_value.size() - 1)
                    str << ",";
            }
            str << "]";
        }
        
        
        ValueType NullValueHolder::type() const { return Type_Null; }
        String NullValueHolder::description() const { return "null"; }
        size_t NullValueHolder::length() const { return 0; }
        
        ValueHolder* NullValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case Type_Boolean:
                    return new BooleanValueHolder(false);
                case Type_Null:
                    return new NullValueHolder();
                case Type_Number:
                case Type_String:
                case Type_Array:
                case Type_Map:
                case Type_Range:
                    break;
            }
            
            throw ConversionError(description(), type(), toType);
        }
        
        ValueHolder* NullValueHolder::clone() const { return new NullValueHolder(); }
        void NullValueHolder::appendToStream(std::ostream& str) const { str << "null"; }

        
        const Value Value::Null = Value();
        
        Value::Value(ValueHolder* holder)      : m_value(holder) {}
        Value::Value(const BooleanType& value) : m_value(new BooleanValueHolder(value)) {}
        Value::Value(const StringType& value)  : m_value(new StringValueHolder(value)) {}
        Value::Value(const char* value)        : m_value(new StringValueHolder(String(value))) {}
        Value::Value(const NumberType& value)  : m_value(new NumberValueHolder(value)) {}
        Value::Value(int value)                : m_value(new NumberValueHolder(static_cast<NumberType>(value))) {}
        Value::Value(long value)               : m_value(new NumberValueHolder(static_cast<NumberType>(value))) {}
        Value::Value(size_t value)             : m_value(new NumberValueHolder(static_cast<NumberType>(value))) {}
        Value::Value(const ArrayType& value)   : m_value(new ArrayValueHolder(value)) {}
        Value::Value(const MapType& value)     : m_value(new MapValueHolder(value)) {}
        Value::Value(const RangeType& value)   : m_value(new RangeValueHolder(value)) {}
        Value::Value()                         : m_value(new NullValueHolder()) {}

        ValueType Value::type() const {
            return m_value->type();
        }

        String Value::typeName() const {
            return EL::typeName(type());
        }

        String Value::description() const {
            return m_value->description();
        }

        const StringType& Value::stringValue() const {
            return m_value->stringValue();
        }
        
        const BooleanType& Value::booleanValue() const {
            return m_value->booleanValue();
        }

        const NumberType& Value::numberValue() const {
            return m_value->numberValue();
        }
        
        const ArrayType& Value::arrayValue() const {
            return m_value->arrayValue();
        }
        
        const MapType& Value::mapValue() const {
            return m_value->mapValue();
        }

        const RangeType& Value::rangeValue() const {
            return m_value->rangeValue();
        }

        size_t Value::length() const {
            return m_value->length();
        }

        Value Value::convertTo(const ValueType toType) const {
            if (type() == toType)
                return *this;
            return Value(m_value->convertTo(toType));
        }

        void Value::appendToStream(std::ostream& str) const {
            m_value->appendToStream(str);
        }
        
        std::ostream& operator<<(std::ostream& stream, const Value& value) {
            value.appendToStream(stream);
            return stream;
        }

        Value Value::operator[](const Value& indexValue) const {
            switch (type()) {
                case Type_String:
                    switch (indexValue.type()) {
                        case Type_Boolean:
                        case Type_Number: {
                            const StringType& str = stringValue();
                            const size_t index = computeIndex(indexValue, str.length());
                            if (index >= str.length())
                                throw IndexOutOfBoundsError(*this, indexValue, index);
                            StringStream result;
                            result << str[index];
                            return Value(result.str());
                        }
                        case Type_Array:
                        case Type_Range: {
                            const StringType& str = stringValue();
                            const IndexList indices = computeIndexArray(indexValue, str.length());
                            StringStream result;
                            for (size_t i = 0; i < indices.size(); ++i) {
                                const size_t index = indices[i];
                                if (index >= str.length())
                                    throw IndexOutOfBoundsError(*this, indexValue, index);
                                result << str[index];
                            }
                            return Value(result.str());
                        }
                        case Type_String:
                        case Type_Map:
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_Array:
                    switch (indexValue.type()) {
                        case Type_Boolean:
                        case Type_Number: {
                            const ArrayType& array = arrayValue();
                            const size_t index = computeIndex(indexValue, array.size());
                            if (index >= array.size())
                                throw IndexOutOfBoundsError(*this, indexValue, index);
                            return array[index];
                        }
                        case Type_Array:
                        case Type_Range: {
                            const ArrayType& array = arrayValue();
                            const IndexList indices = computeIndexArray(indexValue, array.size());
                            ArrayType result;
                            result.reserve(indices.size());
                            for (size_t i = 0; i < indices.size(); ++i) {
                                const size_t index = indices[i];
                                if (index >= array.size())
                                    throw IndexOutOfBoundsError(*this, indexValue, index);
                                result.push_back(array[index]);
                            }
                            return Value(result);
                        }
                        case Type_String:
                        case Type_Map:
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_Map:
                    switch (indexValue.type()) {
                        case Type_String: {
                            const MapType& map = mapValue();
                            const String& key = indexValue.stringValue();
                            const MapType::const_iterator it = map.find(key);
                            if (it == map.end())
                                throw IndexOutOfBoundsError(*this, indexValue, key);
                            return it->second;
                        }
                        case Type_Array: {
                            const MapType& map = mapValue();
                            const ArrayType& keys = indexValue.arrayValue();
                            MapType result;
                            for (size_t i = 0; i < keys.size(); ++i) {
                                const Value& keyValue = keys[i];
                                if (keyValue.type() != Type_String)
                                    throw ConversionError(keyValue.description(), keyValue.type(), Type_String);
                                const String& key = keyValue.stringValue();
                                const MapType::const_iterator it = map.find(key);
                                if (it == map.end())
                                    throw IndexOutOfBoundsError(*this, indexValue, key);
                                result.insert(std::make_pair(key, it->second));
                            }
                            return result;
                        }
                        case Type_Boolean:
                        case Type_Number:
                        case Type_Map:
                        case Type_Range:
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_Boolean:
                case Type_Number:
                case Type_Range:
                case Type_Null:
                    break;
            }
            
            throw IndexError(*this, indexValue);
        }
        
        Value::IndexList Value::computeIndexArray(const Value& indexValue, const size_t indexableSize) const {
            IndexList result;
            computeIndexArray(indexValue, indexableSize, result);
            return result;
        }

        void Value::computeIndexArray(const Value& indexValue, const size_t indexableSize, IndexList& result) const {
            switch (indexValue.type()) {
                case Type_Array: {
                    const ArrayType& indexArray = indexValue.arrayValue();
                    result.reserve(result.size() + indexArray.size());
                    for (size_t i = 0; i < indexArray.size(); ++i)
                        computeIndexArray(indexArray[i], indexableSize, result);
                    break;
                }
                case Type_Range: {
                    const RangeType& range = indexValue.rangeValue();
                    result.reserve(result.size() + range.size());
                    for (size_t i = 0; i < range.size(); ++i)
                        result.push_back(computeIndex(range[i], indexableSize));
                    break;
                }
                case Type_Boolean:
                case Type_Number:
                case Type_String:
                case Type_Map:
                case Type_Null:
                    result.push_back(computeIndex(indexValue, indexableSize));
                    break;
            }
        }

        size_t Value::computeIndex(const Value& indexValue, const size_t indexableSize) const {
            return computeIndex(static_cast<long>(indexValue.convertTo(Type_Number).numberValue()), indexableSize);
        }

        size_t Value::computeIndex(const long index, const size_t indexableSize) const {
            const long size  = static_cast<long>(indexableSize);
            if ((index >= 0 && index <   size) ||
                (index <  0 && index >= -size ))
                return static_cast<size_t>((size + index % size) % size);
            return static_cast<size_t>(size);
        }

        Value Value::operator+() const {
            switch (type()) {
                case Type_Boolean:
                case Type_Number:
                    return convertTo(Type_Number).numberValue();
                case Type_String:
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                    throw EvaluationError("Cannot apply unary plus to value '" + description() + "' of type '" + typeName());
            }
        }
        
        Value Value::operator-() const {
            switch (type()) {
                case Type_Boolean:
                case Type_Number:
                    return -convertTo(Type_Number).numberValue();
                case Type_String:
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                    throw EvaluationError("Cannot negate value '" + description() + "' of type '" + typeName());
            }
        }

        Value operator+(const Value& lhs, const Value& rhs) {
            switch (lhs.type()) {
                case Type_Boolean:
                case Type_Number:
                    switch (rhs.type()) {
                        case Type_Boolean:
                        case Type_Number:
                            return Value(lhs.convertTo(Type_Number).numberValue() + rhs.convertTo(Type_Number).numberValue());
                        case Type_String:
                        case Type_Array:
                        case Type_Map:
                        case Type_Range:
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_String:
                    switch (rhs.type()) {
                        case Type_String:
                            return Value(lhs.convertTo(Type_String).stringValue() + rhs.convertTo(Type_String).stringValue());
                        case Type_Boolean:
                        case Type_Number:
                        case Type_Array:
                        case Type_Map:
                        case Type_Range:
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                    break;
            }
            
            throw EvaluationError("Cannot add value '" + rhs.description() + "' of type '" + typeName(rhs.type()) + " to value '" + lhs.description() + "' of type '" + typeName(lhs.type()) + "'");
        }
        
        Value operator-(const Value& lhs, const Value& rhs) {
            switch (lhs.type()) {
                case Type_Boolean:
                case Type_Number:
                    switch (rhs.type()) {
                        case Type_Boolean:
                        case Type_Number:
                            return Value(lhs.convertTo(Type_Number).numberValue() - rhs.convertTo(Type_Number).numberValue());
                        case Type_String:
                        case Type_Array:
                        case Type_Map:
                        case Type_Range:
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_String:
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                    break;
            }
            
            throw EvaluationError("Cannot subtract value '" + rhs.description() + "' of type '" + typeName(rhs.type()) + " from value '" + lhs.description() + "' of type '" + typeName(lhs.type()) + "'");
        }

        Value operator*(const Value& lhs, const Value& rhs) {
            switch (lhs.type()) {
                case Type_Boolean:
                case Type_Number:
                    switch (rhs.type()) {
                        case Type_Boolean:
                        case Type_Number:
                            return Value(lhs.convertTo(Type_Number).numberValue() * rhs.convertTo(Type_Number).numberValue());
                        case Type_String:
                        case Type_Array:
                        case Type_Map:
                        case Type_Range:
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_String:
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                    break;
            }
            
            throw EvaluationError("Cannot subtract value '" + rhs.description() + "' of type '" + typeName(rhs.type()) + " from value '" + lhs.description() + "' of type '" + typeName(lhs.type()) + "'");
        }
        
        Value operator/(const Value& lhs, const Value& rhs) {
            switch (lhs.type()) {
                case Type_Boolean:
                case Type_Number:
                    switch (rhs.type()) {
                        case Type_Boolean:
                        case Type_Number:
                            return Value(lhs.convertTo(Type_Number).numberValue() / rhs.convertTo(Type_Number).numberValue());
                        case Type_String:
                        case Type_Array:
                        case Type_Map:
                        case Type_Range:
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_String:
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                    break;
            }
            
            throw EvaluationError("Cannot subtract value '" + rhs.description() + "' of type '" + typeName(rhs.type()) + " from value '" + lhs.description() + "' of type '" + typeName(lhs.type()) + "'");
        }
        
        Value operator%(const Value& lhs, const Value& rhs) {
            switch (lhs.type()) {
                case Type_Boolean:
                case Type_Number:
                    switch (rhs.type()) {
                        case Type_Boolean:
                        case Type_Number:
                            return Value(std::fmod(lhs.convertTo(Type_Number).numberValue(), rhs.convertTo(Type_Number).numberValue()));
                        case Type_String:
                        case Type_Array:
                        case Type_Map:
                        case Type_Range:
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_String:
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                    break;
            }
            
            throw EvaluationError("Cannot subtract value '" + rhs.description() + "' of type '" + typeName(rhs.type()) + " from value '" + lhs.description() + "' of type '" + typeName(lhs.type()) + "'");
        }

        Value::operator bool() const {
            return convertTo(Type_Boolean).booleanValue();
        }

        Value Value::operator!() const {
            return !convertTo(Type_Boolean).booleanValue();
        }

        bool operator==(const Value& lhs, const Value& rhs) {
            return compare(lhs, rhs) == 0;
        }
        
        bool operator!=(const Value& lhs, const Value& rhs) {
            return compare(lhs, rhs) != 0;
        }
        
        bool operator<(const Value& lhs, const Value& rhs) {
            return compare(lhs, rhs) < 0;
        }
        
        bool operator<=(const Value& lhs, const Value& rhs) {
            return compare(lhs, rhs) <= 0;
        }
        
        bool operator>(const Value& lhs, const Value& rhs) {
            return compare(lhs, rhs) > 0;
        }
        
        bool operator>=(const Value& lhs, const Value& rhs) {
            return compare(lhs, rhs) >= 0;
        }

        int compare(const Value& lhs, const Value& rhs) {
            switch (lhs.type()) {
                case Type_String:
                    switch (rhs.type()) {
                        case Type_Boolean:
                        case Type_Number:
                        case Type_String:
                            return lhs.stringValue().compare(rhs.convertTo(Type_String).stringValue());
                        case Type_Null:
                            return 1;
                        case Type_Array:
                        case Type_Map:
                        case Type_Range:
                            break;
                    }
                    break;
                case Type_Boolean:
                case Type_Number:
                    switch (rhs.type()) {
                        case Type_Boolean:
                        case Type_Number: {
                            const NumberType diff = lhs.convertTo(Type_Number).numberValue() - rhs.convertTo(Type_Number).numberValue();
                            if (diff < 0.0)
                                return -1;
                            else if (diff > 0.0)
                                return 1;
                            return 0;
                        }
                        case Type_String:
                            return lhs.convertTo(Type_String).stringValue().compare(rhs.stringValue());
                        case Type_Null:
                            return 1;
                        case Type_Array:
                        case Type_Map:
                        case Type_Range:
                            break;
                    }
                    break;
                case Type_Null:
                    if (rhs.type() == Type_Null)
                        return 0;
                    return -1;
                case Type_Array:
                    if (rhs.type() == Type_Array)
                        return VectorUtils::compare(lhs.arrayValue(), rhs.arrayValue());
                    break;
                case Type_Map:
                    if (rhs.type() == Type_Map)
                        return MapUtils::compare(lhs.mapValue(), rhs.mapValue());
                    break;
                case Type_Range:
                    if (rhs.type() == Type_Range)
                        return VectorUtils::compare(lhs.rangeValue(), rhs.rangeValue());
                    break;
            }
            throw EvaluationError("Cannot compare value '" + lhs.description() + "' of type '" + typeName(lhs.type()) + " to value '" + rhs.description() + "' of type '" + typeName(rhs.type()) + "'");
        }

        const Value& EvaluationContext::variableValue(const String& name) const {
            VariableTable::const_iterator it = m_variables.find(name);
            if (it == m_variables.end())
                throw EvaluationError("Unknown variable '" + name + "'");
            return it->second;
        }
        
        void EvaluationContext::defineVariable(const String& name, const Value& value) {
            m_variables.insert(std::make_pair(name, value));
        }
        
        InternalEvaluationContext::InternalEvaluationContext(const EvaluationContext& context) :
        m_context(context) {}
        
        const Value& InternalEvaluationContext::variableValue(const String& name) const {
            VariableTable::const_iterator it = m_variables.find(name);
            if (it == m_variables.end())
                return m_context.variableValue(name);
            assert(!it->second.empty());
            return it->second.back();
        }
        
        void InternalEvaluationContext::pushVariable(const String& name, const Value& value) {
            VariableTable::iterator it = m_variables.find(name);
            if (it == m_variables.end())
                m_variables.insert(std::make_pair(name, ValueStack(1, value)));
            else
                it->second.push_back(value);
        }
        
        void InternalEvaluationContext::popVariable(const String& name) {
            VariableTable::iterator it = m_variables.find(name);
            if (it == m_variables.end())
                throw EvaluationError("Unknown variable '" + name + "'");
            assert(!it->second.empty());
            it->second.pop_back();
            if (it->second.empty())
                m_variables.erase(it);
        }

        Expression::Expression(ExpressionBase* expression) :
        m_expression(expression) {
            assert(m_expression.get() != NULL);
        }
        
        void Expression::optimize() {
            ExpressionBase* optimized = m_expression->optimize();
            if (optimized != NULL && optimized != m_expression.get())
                m_expression.reset(optimized);
        }

        Value Expression::evaluate(const EvaluationContext& context) const {
            InternalEvaluationContext internalContext(context);
            return m_expression->evaluate(internalContext);
        }

        void ExpressionBase::replaceExpression(ExpressionBase*& oldExpression, ExpressionBase* newExpression) {
            if (newExpression != NULL && newExpression != oldExpression) {
                delete oldExpression;
                oldExpression = newExpression;
            }
        }
        
        ExpressionBase::ExpressionBase() {}
        ExpressionBase::~ExpressionBase() {}

        ExpressionBase* ExpressionBase::reorderByPrecedence() {
            return doReorderByPrecedence();
        }
        
        ExpressionBase* ExpressionBase::reorderByPrecedence(BinaryOperator* parent) {
            return doReorderByPrecedence(parent);
        }

        ExpressionBase* ExpressionBase::clone() const {
            return doClone();
        }

        ExpressionBase* ExpressionBase::optimize() {
            return doOptimize();
        }

        Value ExpressionBase::evaluate(InternalEvaluationContext& context) const {
            return doEvaluate(context);
        }

        ExpressionBase* ExpressionBase::doReorderByPrecedence() {
            return this;
        }
        
        ExpressionBase* ExpressionBase::doReorderByPrecedence(BinaryOperator* parent) {
            return parent;
        }

        LiteralExpression::LiteralExpression(const Value& value) :
        m_value(value) {}
        
        ExpressionBase* LiteralExpression::create(const Value& value) {
            return new LiteralExpression(value);
        }

        ExpressionBase* LiteralExpression::doClone() const {
            return new LiteralExpression(m_value);
        }

        ExpressionBase* LiteralExpression::doOptimize() {
            return this;
        }

        Value LiteralExpression::doEvaluate(InternalEvaluationContext& context) const {
            return m_value;
        }

        VariableExpression::VariableExpression(const String& variableName) :
        m_variableName(variableName) {}

        ExpressionBase* VariableExpression::create(const String& variableName) {
            return new VariableExpression(variableName);
        }
        
        ExpressionBase* VariableExpression::doClone() const {
            return new VariableExpression(m_variableName);
        }

        ExpressionBase* VariableExpression::doOptimize() {
            return NULL;
        }

        Value VariableExpression::doEvaluate(InternalEvaluationContext& context) const {
            return context.variableValue(m_variableName);
        }

        ArrayExpression::ArrayExpression(const ExpressionBase::List& elements) :
        m_elements(elements) {}
        
        ExpressionBase* ArrayExpression::create(const ExpressionBase::List& elements) {
            return new ArrayExpression(elements);
        }
        
        ArrayExpression::~ArrayExpression() {
            ListUtils::clearAndDelete(m_elements);
        }

        ExpressionBase* ArrayExpression::doClone() const {
            ExpressionBase::List clones;
            ExpressionBase::List::const_iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                const ExpressionBase* element = *it;
                clones.push_back(element->clone());
            }
            
            return new ArrayExpression(clones);
        }
        
        ExpressionBase* ArrayExpression::doOptimize() {
            bool allOptimized = true;
            
            ExpressionBase::List::iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                ExpressionBase*& expression = *it;
                ExpressionBase* optimized = expression->optimize();
                replaceExpression(expression, optimized);
                allOptimized &= optimized != NULL;
            }
            
            if (allOptimized) {
                InternalEvaluationContext context((EvaluationContext()));
                return LiteralExpression::create(evaluate(context));
            }
            
            return NULL;
        }

        Value ArrayExpression::doEvaluate(InternalEvaluationContext& context) const {
            ArrayType array;
            ExpressionBase::List::const_iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                const ExpressionBase* element = *it;
                const Value value = element->evaluate(context);
                if (value.type() == Type_Range) {
                    const RangeType& range = value.rangeValue();
                    array.reserve(array.size() + range.size());
                    for (size_t i = 0; i < range.size(); ++i)
                        array.push_back(Value(range[i]));
                } else {
                    array.push_back(value);
                }
            }
            
            return Value(array);
        }

        MapExpression::MapExpression(const ExpressionBase::Map& elements) :
        m_elements(elements) {}
        
        ExpressionBase* MapExpression::create(const ExpressionBase::Map& elements) {
            return new MapExpression(elements);
        }
        
        MapExpression::~MapExpression() {
            MapUtils::clearAndDelete(m_elements);
        }

        ExpressionBase* MapExpression::doClone() const {
            ExpressionBase::Map clones;
            ExpressionBase::Map::const_iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                const String& key = it->first;
                const ExpressionBase* value = it->second;
                clones.insert(std::make_pair(key, value->clone()));
            }
            
            return new MapExpression(clones);
        }

        
        ExpressionBase* MapExpression::doOptimize() {
            bool allOptimized = true;
            
            ExpressionBase::Map::iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                ExpressionBase*& expression = it->second;
                ExpressionBase* optimized = expression->optimize();
                replaceExpression(expression, optimized);
                allOptimized &= optimized != NULL;
            }
            
            if (allOptimized) {
                InternalEvaluationContext context((EvaluationContext()));
                return LiteralExpression::create(evaluate(context));
            }
            
            return NULL;
        }
        
        Value MapExpression::doEvaluate(InternalEvaluationContext& context) const {
            MapType map;
            ExpressionBase::Map::const_iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                const String& key = it->first;
                const ExpressionBase* value = it->second;
                map.insert(std::make_pair(key, value->evaluate(context)));
            }
            
            return Value(map);
        }

        UnaryOperator::UnaryOperator(ExpressionBase* operand) :
        m_operand(operand) {
            assert(m_operand != NULL);
        }

        UnaryOperator::~UnaryOperator() {
            delete m_operand;
        }

        ExpressionBase* UnaryOperator::doOptimize() {
            ExpressionBase* optimized = m_operand->optimize();
            replaceExpression(m_operand, optimized);
            
            if (optimized != NULL) {
                InternalEvaluationContext context((EvaluationContext()));
                return LiteralExpression::create(evaluate(context));
            }
            
            return NULL;
        }

        UnaryPlusOperator::UnaryPlusOperator(ExpressionBase* operand) :
        UnaryOperator(operand) {}

        ExpressionBase* UnaryPlusOperator::create(ExpressionBase* operand) {
            return new UnaryPlusOperator(operand);
        }

        ExpressionBase* UnaryPlusOperator::doClone() const {
            return new UnaryPlusOperator(m_operand->clone());
        }

        Value UnaryPlusOperator::doEvaluate(InternalEvaluationContext& context) const {
            return +m_operand->evaluate(context);
        }

        UnaryMinusOperator::UnaryMinusOperator(ExpressionBase* operand) :
        UnaryOperator(operand) {}
        
        ExpressionBase* UnaryMinusOperator::create(ExpressionBase* operand) {
            return new UnaryMinusOperator(operand);
        }

        ExpressionBase* UnaryMinusOperator::doClone() const {
            return new UnaryMinusOperator(m_operand->clone());
        }

        Value UnaryMinusOperator::doEvaluate(InternalEvaluationContext& context) const {
            return -m_operand->evaluate(context);
        }
        
        NegationOperator::NegationOperator(ExpressionBase* operand) :
        UnaryOperator(operand) {}

        ExpressionBase* NegationOperator::create(ExpressionBase* operand) {
            return new NegationOperator(operand);
        }

        ExpressionBase* NegationOperator::doClone() const  {
            return new NegationOperator(m_operand->clone());
        }
        
        Value NegationOperator::doEvaluate(InternalEvaluationContext& context) const {
            return !m_operand->evaluate(context);
        }

        GroupingOperator::GroupingOperator(ExpressionBase* operand) :
        UnaryOperator(operand) {}

        ExpressionBase* GroupingOperator::create(ExpressionBase* operand) {
            return new GroupingOperator(operand);
        }

        ExpressionBase* GroupingOperator::doClone() const {
            return new GroupingOperator(m_operand->clone());
        }
        
        Value GroupingOperator::doEvaluate(InternalEvaluationContext& context) const {
            return m_operand->evaluate(context);
        }

        SubscriptOperator::SubscriptOperator(ExpressionBase* indexableOperand, ExpressionBase* indexOperand) :
        m_indexableOperand(indexableOperand),
        m_indexOperand(indexOperand) {
            assert(m_indexableOperand != NULL);
            assert(m_indexOperand != NULL);
        }
        
        SubscriptOperator::~SubscriptOperator() {
            delete m_indexableOperand;
            delete m_indexOperand;
        }

        ExpressionBase* SubscriptOperator::create(ExpressionBase* indexableOperand, ExpressionBase* indexOperand) {
            return (new SubscriptOperator(indexableOperand, indexOperand))->reorderByPrecedence();
        }
        
        ExpressionBase* SubscriptOperator::doClone() const {
            return new SubscriptOperator(m_indexableOperand->clone(), m_indexOperand->clone());
        }
        
        ExpressionBase* SubscriptOperator::doOptimize() {
            ExpressionBase* indexableOptimized = m_indexableOperand->optimize();
            ExpressionBase* indexOptimized = m_indexOperand->optimize();
            
            replaceExpression(m_indexableOperand, indexableOptimized);
            replaceExpression(m_indexOperand, indexOptimized);
            
            if (indexableOptimized != NULL && indexOptimized != NULL) {
                InternalEvaluationContext context((EvaluationContext()));
                return LiteralExpression::create(evaluate(context));
            }
            
            return NULL;
        }

        Value SubscriptOperator::doEvaluate(InternalEvaluationContext& context) const {
            const Value indexableValue = m_indexableOperand->evaluate(context);
            context.pushVariable(RangeOperator::AutoRangeParameterName(), Value(indexableValue.length()-1));
            const Value indexValue = m_indexOperand->evaluate(context);
            context.popVariable(RangeOperator::AutoRangeParameterName());
            return indexableValue[indexValue];
        }

        BinaryOperator::BinaryOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand) :
        m_leftOperand(leftOperand),
        m_rightOperand(rightOperand) {
            assert(m_leftOperand != NULL);
            assert(m_rightOperand != NULL);
        }

        BinaryOperator::~BinaryOperator() {
            delete m_leftOperand;
            delete m_rightOperand;
        }

        ExpressionBase* BinaryOperator::doReorderByPrecedence() {
            ExpressionBase* result = m_leftOperand->reorderByPrecedence(this);
            if (result == this)
                result = m_rightOperand->reorderByPrecedence(this);
            return result;
        }
        
        ExpressionBase* BinaryOperator::doReorderByPrecedence(BinaryOperator* parent) {
            assert(parent->m_leftOperand == this || parent->m_rightOperand == this);
            if (parent->m_leftOperand == this && precedence() < parent->precedence())
                return parent->rotateLeftUp(this);
            if (parent->m_rightOperand == this && precedence() < parent->precedence())
                return parent->rotateRightUp(this);
            return parent;
        }
        

        BinaryOperator* BinaryOperator::rotateLeftUp(BinaryOperator* leftOperand) {
            assert(m_leftOperand == leftOperand);
            
            m_leftOperand = leftOperand->m_rightOperand;
            leftOperand->m_rightOperand = this;
            
            return leftOperand;
        }
        
        BinaryOperator* BinaryOperator::rotateRightUp(BinaryOperator* rightOperand) {
            assert(m_rightOperand == rightOperand);
            
            m_rightOperand = rightOperand->m_leftOperand;
            rightOperand->m_leftOperand = this;
            
            return rightOperand;
        }
        
        ExpressionBase* BinaryOperator::doOptimize() {
            ExpressionBase* leftOptimized = m_leftOperand->optimize();
            ExpressionBase* rightOptimized = m_rightOperand->optimize();
            
            replaceExpression(m_leftOperand, leftOptimized);
            replaceExpression(m_rightOperand, rightOptimized);
            
            if (leftOptimized != NULL && rightOptimized != NULL) {
                InternalEvaluationContext context((EvaluationContext()));
                return LiteralExpression::create(evaluate(context));
            }
            
            return NULL;
        }
        
        struct BinaryOperator::Traits {
            size_t precedence;
            bool associative;
            bool commutative;
            
            Traits(const size_t i_precedence, const bool i_associative, const bool i_commutative) :
            precedence(i_precedence),
            associative(i_associative),
            commutative(i_commutative) {}
        };
        
        BinaryOperator::Traits BinaryOperator::traits() const {
            return doGetTraits();
        }

        size_t BinaryOperator::precedence() const {
            return traits().precedence;
        }
        
        bool BinaryOperator::associative() const {
            return traits().associative;
        }
        
        bool BinaryOperator::commutative() const {
            return traits().commutative;
        }

        AdditionOperator::AdditionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}

        ExpressionBase* AdditionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand) {
            return (new AdditionOperator(leftOperand, rightOperand))->reorderByPrecedence();
        }

        ExpressionBase* AdditionOperator::doClone() const {
            return new AdditionOperator(m_leftOperand->clone(), m_rightOperand->clone());
        }
        
        Value AdditionOperator::doEvaluate(InternalEvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return leftValue + rightValue;
        }

        BinaryOperator::Traits AdditionOperator::doGetTraits() const {
            return Traits(4, true, true);
        }
        
        SubtractionOperator::SubtractionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}

        ExpressionBase* SubtractionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand) {
            return (new SubtractionOperator(leftOperand, rightOperand))->reorderByPrecedence();
        }

        ExpressionBase* SubtractionOperator::doClone() const {
            return new SubtractionOperator(m_leftOperand->clone(), m_rightOperand->clone());
        }
        
        Value SubtractionOperator::doEvaluate(InternalEvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return leftValue - rightValue;
        }
        
        BinaryOperator::Traits SubtractionOperator::doGetTraits() const {
            return Traits(4, false, false);
        }
        
        MultiplicationOperator::MultiplicationOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}
        
        ExpressionBase* MultiplicationOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand) {
            return (new MultiplicationOperator(leftOperand, rightOperand))->reorderByPrecedence();
        }

        ExpressionBase* MultiplicationOperator::doClone() const {
            return new MultiplicationOperator(m_leftOperand->clone(), m_rightOperand->clone());
        }
        
        Value MultiplicationOperator::doEvaluate(InternalEvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return leftValue * rightValue;
        }
        
        BinaryOperator::Traits MultiplicationOperator::doGetTraits() const {
            return Traits(5, true, true);
        }

        DivisionOperator::DivisionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}
        
        ExpressionBase* DivisionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand) {
            return (new DivisionOperator(leftOperand, rightOperand))->reorderByPrecedence();
        }

        ExpressionBase* DivisionOperator::doClone() const {
            return new DivisionOperator(m_leftOperand->clone(), m_rightOperand->clone());
        }
        
        Value DivisionOperator::doEvaluate(InternalEvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return leftValue / rightValue;
        }
        
        BinaryOperator::Traits DivisionOperator::doGetTraits() const {
            return Traits(5, false, false);
        }

        ModulusOperator::ModulusOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}
        
        ExpressionBase* ModulusOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand) {
            return (new ModulusOperator(leftOperand, rightOperand))->reorderByPrecedence();
        }

        ExpressionBase* ModulusOperator::doClone() const {
            return new ModulusOperator(m_leftOperand->clone(), m_rightOperand->clone());
        }
        
        Value ModulusOperator::doEvaluate(InternalEvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return leftValue % rightValue;
        }
        
        BinaryOperator::Traits ModulusOperator::doGetTraits() const {
            return Traits(5, false, false);
        }
        
        const String& RangeOperator::AutoRangeParameterName() {
            static const String Name = "__AutoRangeParameter";
            return Name;
        }

        ConjunctionOperator::ConjunctionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}

        ExpressionBase* ConjunctionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand) {
            return new ConjunctionOperator(leftOperand, rightOperand);
        }

        ExpressionBase* ConjunctionOperator::doClone() const {
            return new ConjunctionOperator(m_leftOperand->clone(), m_rightOperand->clone());
        }
        
        Value ConjunctionOperator::doEvaluate(InternalEvaluationContext& context) const {
            return m_leftOperand->evaluate(context) && m_rightOperand->evaluate(context);
        }
        
        BinaryOperator::Traits ConjunctionOperator::doGetTraits() const {
            return Traits(2, true, true);
        }
        
        DisjunctionOperator::DisjunctionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}
        
        ExpressionBase* DisjunctionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand) {
            return new DisjunctionOperator(leftOperand, rightOperand);
        }
        
        ExpressionBase* DisjunctionOperator::doClone() const {
            return new DisjunctionOperator(m_leftOperand->clone(), m_rightOperand->clone());
        }
        
        Value DisjunctionOperator::doEvaluate(InternalEvaluationContext& context) const {
            return m_leftOperand->evaluate(context) || m_rightOperand->evaluate(context);
        }
        
        BinaryOperator::Traits DisjunctionOperator::doGetTraits() const {
            return Traits(1, true, true);
        }

        ComparisonOperator::ComparisonOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const Op op) :
        BinaryOperator(leftOperand, rightOperand),
        m_op(op) {}

        ExpressionBase* ComparisonOperator::createLess(ExpressionBase* leftOperand, ExpressionBase* rightOperand) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_Less);
        }
        
        ExpressionBase* ComparisonOperator::createLessOrEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_LessOrEqual);
        }
        
        ExpressionBase* ComparisonOperator::createEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_Equal);
        }
        
        ExpressionBase* ComparisonOperator::createInequal(ExpressionBase* leftOperand, ExpressionBase* rightOperand) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_Inequal);
        }
        
        ExpressionBase* ComparisonOperator::createGreaterOrEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_GreaterOrEqual);
        }
        
        ExpressionBase* ComparisonOperator::createGreater(ExpressionBase* leftOperand, ExpressionBase* rightOperand) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_Greater);
        }
        
        ExpressionBase* ComparisonOperator::doClone() const {
            return new ComparisonOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_op);
        }
        
        Value ComparisonOperator::doEvaluate(InternalEvaluationContext& context) const {
            switch (m_op) {
                case Op_Less:
                    return m_leftOperand->evaluate(context) < m_rightOperand->evaluate(context);
                case Op_LessOrEqual:
                    return m_leftOperand->evaluate(context) <= m_rightOperand->evaluate(context);
                case Op_Equal:
                    return m_leftOperand->evaluate(context) == m_rightOperand->evaluate(context);
                case Op_Inequal:
                    return m_leftOperand->evaluate(context) != m_rightOperand->evaluate(context);
                case Op_GreaterOrEqual:
                    return m_leftOperand->evaluate(context) >= m_rightOperand->evaluate(context);
                case Op_Greater:
                    return m_leftOperand->evaluate(context) > m_rightOperand->evaluate(context);
            }
        }
        
        BinaryOperator::Traits ComparisonOperator::doGetTraits() const {
            switch (m_op) {
                case Op_Less:
                case Op_LessOrEqual:
                case Op_Greater:
                case Op_GreaterOrEqual:
                    return Traits(3, false, false);
                case Op_Equal:
                case Op_Inequal:
                    return Traits(3, true, false);
            }
        }

        RangeOperator::RangeOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}
        
        ExpressionBase* RangeOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand) {
            return (new RangeOperator(leftOperand, rightOperand))->reorderByPrecedence();
        }
        
        ExpressionBase* RangeOperator::createAutoRangeWithLeftOperand(ExpressionBase* leftOperand) {
            return create(leftOperand, VariableExpression::create(AutoRangeParameterName()));
        }
        
        ExpressionBase* RangeOperator::createAutoRangeWithRightOperand(ExpressionBase* rightOperand) {
            return create(VariableExpression::create(AutoRangeParameterName()), rightOperand);
        }

        ExpressionBase* RangeOperator::doClone() const {
            return new RangeOperator(m_leftOperand->clone(), m_rightOperand->clone());
        }
        
        Value RangeOperator::doEvaluate(InternalEvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);

            const long from = static_cast<long>(leftValue.convertTo(Type_Number).numberValue());
            const long to = static_cast<long>(rightValue.convertTo(Type_Number).numberValue());
            
            RangeType range;
            if (from <= to) {
                range.reserve(static_cast<size_t>(to - from + 1));
                for (long i = from; i <= to; ++i) {
                    assert(range.capacity() > range.size());
                    range.push_back(i);
                }
            } else if (to < from) {
                range.reserve(static_cast<size_t>(from - to + 1));
                for (long i = from; i >= to; --i) {
                    assert(range.capacity() > range.size());
                    range.push_back(i);
                }
            }
            assert(range.capacity() == range.size());

            return Value(range);
        }
        
        BinaryOperator::Traits RangeOperator::doGetTraits() const {
            return Traits(0, false, false);
        }
    }
}
