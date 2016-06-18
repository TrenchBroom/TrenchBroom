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
                case Type_Undefined:
                    return "Undefined";
            }
        }

        ValueType typeForName(const String& type) {
            if (type == "Boolean")
                return Type_Boolean;
            if (type == "String")
                return Type_String;
            if (type == "Number")
                return Type_Number;
            if (type == "Array")
                return Type_Array;
            if (type == "Map")
                return Type_Map;
            if (type == "Range")
                return Type_Range;
            if (type == "Undefined")
                return Type_Undefined;
            assert(false);
            return Type_Null;
        }

        ELException::ELException() throw() {}
        ELException::ELException(const String& str) throw() : ExceptionStream(str) {}
        ELException::~ELException() throw() {}
        
        ConversionError::ConversionError(const String& value, const ValueType from, const ValueType to) throw() :
        ELException("Cannot convert value '" + value + "' of type '" + typeName(from) + "' to type '" + typeName(to) + "'") {}

        DereferenceError::DereferenceError(const String& value, const ValueType from, const ValueType to) throw() :
        ELException("Cannot dereference value '" + value + "' of type '" + typeName(from) + "' as type '" + typeName(to) + "'") {}

        EvaluationError::EvaluationError(const String& msg) throw() :
        ELException(msg) {}

        IndexError::IndexError(const Value& indexableValue, const Value& indexValue) throw() :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + " with '" + indexValue.describe() + "' of type '" + typeName(indexValue.type()) + "'") {}

        IndexError::IndexError(const Value& indexableValue, const size_t index) throw() :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + " with integral index") {}

        IndexError::IndexError(const Value& indexableValue, const String& key) throw() :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + " with string index") {}
        
        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, const size_t outOfBoundsIndex) throw() :
        IndexError(indexableValue, indexValue) {
            *this << ": Index value " << outOfBoundsIndex << " is out of bounds";
        }

        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, const String& outOfBoundsIndex) throw() :
        IndexError(indexableValue, indexValue) {
            *this << ": Key '" << outOfBoundsIndex << "' not found";
        }
        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const size_t index) throw() :
        IndexError(indexableValue, index) {
            *this << ": Index value " << index << " is out of bounds";
        }
        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const String& key) throw() :
        IndexError(indexableValue, key) {
            *this << ": Key '" << key << "' not found";
        }

        ValueHolder::~ValueHolder() {}
        
        String ValueHolder::describe() const {
            StringStream str;
            appendToStream(str, false, "");
            return str.str();
        }

        const BooleanType& ValueHolder::booleanValue() const { throw DereferenceError(describe(), type(), Type_Boolean); }
        const StringType&  ValueHolder::stringValue()  const { throw DereferenceError(describe(), type(), Type_String); }
        const NumberType&  ValueHolder::numberValue()  const { throw DereferenceError(describe(), type(), Type_Number); }
        const ArrayType&   ValueHolder::arrayValue()   const { throw DereferenceError(describe(), type(), Type_Array); }
        const MapType&     ValueHolder::mapValue()     const { throw DereferenceError(describe(), type(), Type_Map); }
        const RangeType&   ValueHolder::rangeValue()   const { throw DereferenceError(describe(), type(), Type_Range); }
        
        BooleanValueHolder::BooleanValueHolder(const BooleanType& value) : m_value(value) {}
        ValueType BooleanValueHolder::type() const { return Type_Boolean; }
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
                case Type_Undefined:
                case Type_Null:
                    break;
            }
            
            throw ConversionError(describe(), type(), toType);
        }
        
        ValueHolder* BooleanValueHolder::clone() const { return new BooleanValueHolder(m_value); }
        void BooleanValueHolder::appendToStream(std::ostream& str, const bool multiline, const String& indent) const { str << (m_value ? "true" : "false"); }

        StringValueHolder::StringValueHolder(const StringType& value) : m_value(value) {}
        ValueType StringValueHolder::type() const { return Type_String; }
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
                        throw ConversionError(describe(), type(), toType);
                    const char* begin = m_value.c_str();
                    char* end;
                    const NumberType value = std::strtod(begin, &end);
                    if (value == 0.0 && end == begin)
                        throw ConversionError(describe(), type(), toType);
                    return new NumberValueHolder(value);
                }
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                case Type_Undefined:
                    break;
            }
            
            throw ConversionError(describe(), type(), toType);
        }
        
        ValueHolder* StringValueHolder::clone() const { return new StringValueHolder(m_value); }
        void StringValueHolder::appendToStream(std::ostream& str, const bool multiline, const String& indent) const { str << "\"" << m_value << "\""; }


        
        NumberValueHolder::NumberValueHolder(const NumberType& value) : m_value(value) {}
        ValueType NumberValueHolder::type() const { return Type_Number; }
        const NumberType& NumberValueHolder::numberValue() const { return m_value; }
        size_t NumberValueHolder::length() const { return 1; }
        
        ValueHolder* NumberValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case Type_Boolean:
                    return new BooleanValueHolder(m_value != 0.0);
                case Type_String:
                    return new StringValueHolder(describe());
                case Type_Number:
                    return new NumberValueHolder(m_value);
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                case Type_Undefined:
                    break;
            }
            
            throw ConversionError(describe(), type(), toType);
        }
        
        ValueHolder* NumberValueHolder::clone() const { return new NumberValueHolder(m_value); }
        void NumberValueHolder::appendToStream(std::ostream& str, const bool multiline, const String& indent) const {
            str.precision(17);
            str << m_value;
        }


        
        ArrayValueHolder::ArrayValueHolder(const ArrayType& value) : m_value(value) {}
        ValueType ArrayValueHolder::type() const { return Type_Array; }
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
                case Type_Undefined:
                    break;
            }
            
            throw ConversionError(describe(), type(), toType);
        }
        
        ValueHolder* ArrayValueHolder::clone() const { return new ArrayValueHolder(m_value); }
        
        void ArrayValueHolder::appendToStream(std::ostream& str, const bool multiline, const String& indent) const {
            if (m_value.empty()) {
                str << "[]";
            } else {
                const String childIndent = multiline ? indent + "\t" : "";
                str << "[";
                if (multiline)
                    str << "\n";
                for (size_t i = 0; i < m_value.size(); ++i) {
                    str << childIndent;
                    m_value[i].appendToStream(str, multiline, childIndent);
                    if (i < m_value.size() - 1) {
                        str << ",";
                        if (!multiline)
                            str << " ";
                    }
                    if (multiline)
                        str << "\n";
                }
                str << indent << "]";
            }
        }
        
        
        MapValueHolder::MapValueHolder(const MapType& value) : m_value(value) {}
        ValueType MapValueHolder::type() const { return Type_Map; }
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
                case Type_Undefined:
                    break;
            }
            
            throw ConversionError(describe(), type(), toType);
        }
        
        ValueHolder* MapValueHolder::clone() const { return new MapValueHolder(m_value); }

        void MapValueHolder::appendToStream(std::ostream& str, const bool multiline, const String& indent) const {
            if (m_value.empty()) {
                str << "{}";
            } else {
                const String childIndent = multiline ? indent + "\t" : "";
                str << "{";
                if (multiline)
                    str << "\n";
                MapType::const_iterator it, end;
                size_t i = 0;
                for (it = m_value.begin(), end = m_value.end(); it != end; ++it) {
                    str << childIndent << "\"" << it->first << "\"" << ": ";
                    it->second.appendToStream(str, multiline, childIndent);
                    if (i++ < m_value.size() - 1) {
                        str << ",";
                        if (!multiline)
                            str << " ";
                    }
                    if (multiline)
                        str << "\n";
                }
                str << indent << "}";
            }
        }
        
        
        RangeValueHolder::RangeValueHolder(const RangeType& value) : m_value(value) {}
        ValueType RangeValueHolder::type() const { return Type_Range; }
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
                case Type_Undefined:
                    break;
            }
            
            throw ConversionError(describe(), type(), toType);
        }
        
        ValueHolder* RangeValueHolder::clone() const { return new RangeValueHolder(m_value); }
        
        void RangeValueHolder::appendToStream(std::ostream& str, const bool multiline, const String& indent) const {
            str << "[";
            for (size_t i = 0; i < m_value.size(); ++i) {
                str << m_value[i];
                if (i < m_value.size() - 1)
                    str << ", ";
            }
            str << "]";
        }
        
        
        ValueType NullValueHolder::type() const { return Type_Null; }
        size_t NullValueHolder::length() const { return 0; }
        const StringType& NullValueHolder::stringValue() const   { static const StringType result;         return result; }
        const BooleanType& NullValueHolder::booleanValue() const { static const BooleanType result(false); return result; }
        const NumberType& NullValueHolder::numberValue() const   { static const NumberType result(0.0);    return result; }
        const ArrayType& NullValueHolder::arrayValue() const     { static const ArrayType result(0);       return result; }
        const MapType& NullValueHolder::mapValue() const         { static const MapType result;            return result; }
        const RangeType& NullValueHolder::rangeValue() const     { static const RangeType result(0);       return result; }
        
        ValueHolder* NullValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case Type_Boolean:
                    return new BooleanValueHolder(false);
                case Type_Null:
                    return new NullValueHolder();
                case Type_Number:
                    return new NumberValueHolder(0.0);
                case Type_String:
                    return new StringValueHolder("");
                case Type_Array:
                    return new ArrayValueHolder(ArrayType(0));
                case Type_Map:
                    return new MapValueHolder(MapType());
                case Type_Range:
                    return new RangeValueHolder(RangeType(0));
                case Type_Undefined:
                    break;
            }
            
            throw ConversionError(describe(), type(), toType);
        }
        
        ValueHolder* NullValueHolder::clone() const { return new NullValueHolder(); }
        void NullValueHolder::appendToStream(std::ostream& str, const bool multiline, const String& indent) const { str << "null"; }

        
        ValueType UndefinedValueHolder::type() const { return Type_Undefined; }
        size_t UndefinedValueHolder::length() const { return 0; }
        ValueHolder* UndefinedValueHolder::convertTo(const ValueType toType) const { throw ConversionError(describe(), type(), toType); }
        ValueHolder* UndefinedValueHolder::clone() const { return new UndefinedValueHolder(); }
        void UndefinedValueHolder::appendToStream(std::ostream& str, const bool multiline, const String& indent) const { str << "undefined"; }

        
        const Value Value::Null = Value(new NullValueHolder(), 0, 0);
        const Value Value::Undefined = Value(new UndefinedValueHolder(), 0, 0);
        
        Value::Value(ValueHolder* holder, const size_t line, const size_t column)      : m_value(holder), m_line(line), m_column(column) {}
        
        Value::Value(const BooleanType& value, const size_t line, const size_t column) : m_value(new BooleanValueHolder(value)), m_line(line), m_column(column) {}
        Value::Value(const BooleanType& value)                                         : m_value(new BooleanValueHolder(value)), m_line(0), m_column(0) {}
        
        Value::Value(const StringType& value, const size_t line, const size_t column)  : m_value(new StringValueHolder(value)), m_line(line), m_column(column) {}
        Value::Value(const StringType& value)                                          : m_value(new StringValueHolder(value)), m_line(0), m_column(0) {}

        Value::Value(const char* value, const size_t line, const size_t column)        : m_value(new StringValueHolder(String(value))), m_line(line), m_column(column) {}
        Value::Value(const char* value)                                                : m_value(new StringValueHolder(String(value))), m_line(0), m_column(0) {}

        Value::Value(const NumberType& value, const size_t line, const size_t column)  : m_value(new NumberValueHolder(value)), m_line(line), m_column(column) {}
        Value::Value(const NumberType& value)                                          : m_value(new NumberValueHolder(value)), m_line(0), m_column(0) {}

        Value::Value(const int value, const size_t line, const size_t column)          : m_value(new NumberValueHolder(static_cast<NumberType>(value))), m_line(line), m_column(column) {}
        Value::Value(const int value)                                                  : m_value(new NumberValueHolder(static_cast<NumberType>(value))), m_line(0), m_column(0) {}

        Value::Value(const long value, const size_t line, const size_t column)         : m_value(new NumberValueHolder(static_cast<NumberType>(value))), m_line(line), m_column(column) {}
        Value::Value(const long value)                                                 : m_value(new NumberValueHolder(static_cast<NumberType>(value))), m_line(0), m_column(0) {}

        Value::Value(const size_t value, const size_t line, const size_t column)       : m_value(new NumberValueHolder(static_cast<NumberType>(value))), m_line(line), m_column(column) {}
        Value::Value(const size_t value)                                               : m_value(new NumberValueHolder(static_cast<NumberType>(value))), m_line(0), m_column(0) {}

        Value::Value(const ArrayType& value, const size_t line, const size_t column)   : m_value(new ArrayValueHolder(value)), m_line(line), m_column(column) {}
        Value::Value(const ArrayType& value)                                           : m_value(new ArrayValueHolder(value)), m_line(0), m_column(0) {}

        Value::Value(const MapType& value, const size_t line, const size_t column)     : m_value(new MapValueHolder(value)), m_line(line), m_column(column) {}
        Value::Value(const MapType& value)                                             : m_value(new MapValueHolder(value)), m_line(0), m_column(0) {}

        Value::Value(const RangeType& value, const size_t line, const size_t column)   : m_value(new RangeValueHolder(value)), m_line(line), m_column(column) {}
        Value::Value(const RangeType& value)                                           : m_value(new RangeValueHolder(value)), m_line(0), m_column(0) {}

        Value::Value(const Value& other, const size_t line, const size_t column)       : m_value(other.m_value), m_line(line), m_column(column) {}
        Value::Value(const Value& other)                                               : m_value(other.m_value), m_line(other.m_line), m_column(other.m_column) {}

        Value::Value()                                                                 : m_value(new NullValueHolder()), m_line(0), m_column(0) {}

        ValueType Value::type() const {
            return m_value->type();
        }

        String Value::typeName() const {
            return EL::typeName(type());
        }

        String Value::describe() const {
            return m_value->describe();
        }

        size_t Value::line() const {
            return m_line;
        }
        
        size_t Value::column() const {
            return m_column;
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

        bool Value::null() const {
            return type() == Type_Null;
        }

        const StringList Value::asStringList() const {
            const ArrayType& array = arrayValue();
            StringList result;
            result.reserve(array.size());
            
            ArrayType::const_iterator it, end;
            for (it = array.begin(), end = array.end(); it != end; ++it) {
                const Value& entry = *it;
                result.push_back(entry.convertTo(Type_String).stringValue());
            }
            
            return result;
        }

        const StringSet Value::asStringSet() const {
            const ArrayType& array = arrayValue();
            StringSet result;
            
            ArrayType::const_iterator it, end;
            for (it = array.begin(), end = array.end(); it != end; ++it) {
                const Value& entry = *it;
                result.insert(entry.convertTo(Type_String).stringValue());
            }
            
            return result;
        }

        size_t Value::length() const {
            return m_value->length();
        }

        Value Value::convertTo(const ValueType toType) const {
            if (type() == toType)
                return *this;
            return Value(m_value->convertTo(toType), m_line, m_column);
        }

        void Value::appendToStream(std::ostream& str, const bool multiline, const String& indent) const {
            m_value->appendToStream(str, multiline, indent);
        }
        
        std::ostream& operator<<(std::ostream& stream, const Value& value) {
            value.appendToStream(stream);
            return stream;
        }

        bool Value::contains(const Value& indexValue) const {
            switch (type()) {
                case Type_String: {
                    switch (indexValue.type()) {
                        case Type_Boolean:
                        case Type_Number: {
                            const size_t index = computeIndex(indexValue, length());
                            return index < length();
                        }
                        case Type_Array:
                        case Type_Range: {
                            const IndexList indices = computeIndexArray(indexValue, length());
                            for (size_t i = 0; i < indices.size(); ++i) {
                                const size_t index = indices[i];
                                if (index >= length())
                                    return false;
                            }
                            return true;
                        }
                        case Type_String:
                        case Type_Map:
                        case Type_Null:
                        case Type_Undefined:
                            break;
                    }
                    break;
                }
                case Type_Array:
                    switch (indexValue.type()) {
                        case Type_Boolean:
                        case Type_Number: {
                            const size_t index = computeIndex(indexValue, length());
                            return index < length();
                        }
                        case Type_Array:
                        case Type_Range: {
                            const IndexList indices = computeIndexArray(indexValue, length());
                            for (size_t i = 0; i < indices.size(); ++i) {
                                const size_t index = indices[i];
                                if (index >= length())
                                    return false;
                            }
                            return true;
                        }
                        case Type_String:
                        case Type_Map:
                        case Type_Null:
                        case Type_Undefined:
                            break;
                    }
                    break;
                case Type_Map:
                    switch (indexValue.type()) {
                        case Type_String: {
                            const MapType& map = mapValue();
                            const String& key = indexValue.stringValue();
                            const MapType::const_iterator it = map.find(key);
                            return it != map.end();
                        }
                        case Type_Array: {
                            const MapType& map = mapValue();
                            const ArrayType& keys = indexValue.arrayValue();
                            for (size_t i = 0; i < keys.size(); ++i) {
                                const Value& keyValue = keys[i];
                                if (keyValue.type() != Type_String)
                                    throw ConversionError(keyValue.describe(), keyValue.type(), Type_String);
                                const String& key = keyValue.stringValue();
                                const MapType::const_iterator it = map.find(key);
                                if (it == map.end())
                                    return false;
                            }
                            return true;
                        }
                        case Type_Boolean:
                        case Type_Number:
                        case Type_Map:
                        case Type_Range:
                        case Type_Null:
                        case Type_Undefined:
                            break;
                    }
                    break;
                case Type_Boolean:
                case Type_Number:
                case Type_Range:
                case Type_Null:
                case Type_Undefined:
                    break;
            }
            return false;
        }

        bool Value::contains(const size_t index) const {
            switch (type()) {
                case Type_String:
                case Type_Array:
                    return index < length();
                case Type_Map:
                case Type_Boolean:
                case Type_Number:
                case Type_Range:
                case Type_Null:
                case Type_Undefined:
                    break;
            }
            return false;
        }
        
        bool Value::contains(const String& key) const {
            const MapType& map = mapValue();
            const MapType::const_iterator it = map.find(key);
            return it != map.end();
        }

        StringSet Value::keys() const {
            return MapUtils::keySet(mapValue());
        }

        Value Value::operator[](const Value& indexValue) const {
            switch (type()) {
                case Type_String:
                    switch (indexValue.type()) {
                        case Type_Boolean:
                        case Type_Number: {
                            const StringType& str = stringValue();
                            const size_t index = computeIndex(indexValue, str.length());
                            StringStream result;
                            if (index < str.length())
                                result << str[index];
                            return Value(result.str(), m_line, m_column);
                        }
                        case Type_Array:
                        case Type_Range: {
                            const StringType& str = stringValue();
                            const IndexList indices = computeIndexArray(indexValue, str.length());
                            StringStream result;
                            for (size_t i = 0; i < indices.size(); ++i) {
                                const size_t index = indices[i];
                                if (index < str.length())
                                    result << str[index];
                            }
                            return Value(result.str(), m_line, m_column);
                        }
                        case Type_String:
                        case Type_Map:
                        case Type_Null:
                        case Type_Undefined:
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
                            return Value(result, m_line, m_column);
                        }
                        case Type_String:
                        case Type_Map:
                        case Type_Null:
                        case Type_Undefined:
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
                                return Value::Undefined;
                            return it->second;
                        }
                        case Type_Array: {
                            const MapType& map = mapValue();
                            const ArrayType& keys = indexValue.arrayValue();
                            MapType result;
                            for (size_t i = 0; i < keys.size(); ++i) {
                                const Value& keyValue = keys[i];
                                if (keyValue.type() != Type_String)
                                    throw ConversionError(keyValue.describe(), keyValue.type(), Type_String);
                                const String& key = keyValue.stringValue();
                                const MapType::const_iterator it = map.find(key);
                                if (it != map.end())
                                    result.insert(std::make_pair(key, it->second));
                            }
                            return Value(result, m_line, m_column);
                        }
                        case Type_Boolean:
                        case Type_Number:
                        case Type_Map:
                        case Type_Range:
                        case Type_Null:
                        case Type_Undefined:
                            break;
                    }
                    break;
                case Type_Boolean:
                case Type_Number:
                case Type_Range:
                case Type_Null:
                case Type_Undefined:
                    break;
            }
            
            throw IndexError(*this, indexValue);
        }
        
        Value Value::operator[](const size_t index) const {
            switch (type()) {
                case Type_String: {
                    const StringType& str = stringValue();
                    StringStream result;
                    if (index < str.length())
                        result << str[index];
                    return Value(result.str());
                }
                case Type_Array: {
                    const ArrayType& array = arrayValue();
                    if (index >= array.size())
                        throw IndexOutOfBoundsError(*this, index);
                    return array[index];
                }
                case Type_Map:
                case Type_Boolean:
                case Type_Number:
                case Type_Range:
                case Type_Null:
                case Type_Undefined:
                    break;
            }
            
            throw IndexError(*this, index);
        }
        
        Value Value::operator[](const String& key) const {
            switch (type()) {
                case Type_Map: {
                    const MapType& map = mapValue();
                    const MapType::const_iterator it = map.find(key);
                    if (it == map.end())
                        return Value::Null;
                    return it->second;
                }
                case Type_String:
                case Type_Array:
                case Type_Boolean:
                case Type_Number:
                case Type_Range:
                case Type_Null:
                case Type_Undefined:
                    break;
            }
            
            throw IndexError(*this, key);
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
                case Type_Undefined:
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
                    return Value(convertTo(Type_Number).numberValue());
                case Type_String:
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                case Type_Undefined:
                    break;
            }
            throw EvaluationError("Cannot apply unary plus to value '" + describe() + "' of type '" + typeName());
        }
        
        Value Value::operator-() const {
            switch (type()) {
                case Type_Boolean:
                case Type_Number:
                    return Value(-convertTo(Type_Number).numberValue());
                case Type_String:
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                case Type_Undefined:
                    break;
            }
            throw EvaluationError("Cannot negate value '" + describe() + "' of type '" + typeName());
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
                        case Type_Undefined:
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
                        case Type_Undefined:
                            break;
                    }
                    break;
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                case Type_Undefined:
                    break;
            }
            
            throw EvaluationError("Cannot add value '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + " to value '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + "'");
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
                        case Type_Undefined:
                            break;
                    }
                    break;
                case Type_String:
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                case Type_Undefined:
                    break;
            }
            
            throw EvaluationError("Cannot subtract value '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + " from value '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + "'");
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
                        case Type_Undefined:
                            break;
                    }
                    break;
                case Type_String:
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                case Type_Undefined:
                    break;
            }
            
            throw EvaluationError("Cannot subtract value '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + " from value '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + "'");
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
                        case Type_Undefined:
                            break;
                    }
                    break;
                case Type_String:
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                case Type_Undefined:
                    break;
            }
            
            throw EvaluationError("Cannot subtract value '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + " from value '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + "'");
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
                        case Type_Undefined:
                            break;
                    }
                    break;
                case Type_String:
                case Type_Array:
                case Type_Map:
                case Type_Range:
                case Type_Null:
                case Type_Undefined:
                    break;
            }
            
            throw EvaluationError("Cannot subtract value '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + " from value '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + "'");
        }

        Value::operator bool() const {
            return convertTo(Type_Boolean).booleanValue();
        }

        Value Value::operator!() const {
            return Value(!convertTo(Type_Boolean).booleanValue());
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
                        case Type_Undefined:
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
                        case Type_Undefined:
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
                case Type_Undefined:
                    if (rhs.type() == Type_Undefined)
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
            throw EvaluationError("Cannot compare value '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + " to value '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'");
        }

        VariableStore::~VariableStore() {}

        VariableStore* VariableStore::clone() const {
            return doClone();
        }

        Value VariableStore::value(const String& name) const {
            return doGetValue(name);
        }

        const StringSet VariableStore::names() const {
            return doGetNames();
        }

        void VariableStore::declare(const String& name, const Value& value) {
            doDeclare(name, value);
        }
        
        void VariableStore::assign(const String& name, const Value& value) {
            doAssign(name, value);
        }

        VariableTable::VariableTable() {}

        VariableTable::VariableTable(const Table& variables) :
        m_variables(variables) {}

        VariableStore* VariableTable::doClone() const {
            return new VariableTable(m_variables);
        }

        Value VariableTable::doGetValue(const String& name) const {
            Table::const_iterator it = m_variables.find(name);
            if (it != m_variables.end())
                return it->second;
            return Value::Undefined;
        }

        StringSet VariableTable::doGetNames() const {
            return MapUtils::keySet(m_variables);
        }

        void VariableTable::doDeclare(const String& name, const Value& value) {
            if (!MapUtils::insertOrFail(m_variables, name, value))
                throw EvaluationError("Variable '" + name + "' already declared");
        }
        
        void VariableTable::doAssign(const String& name, const Value& value) {
            Table::iterator it = m_variables.find(name);
            if (it == m_variables.end())
                throw EvaluationError("Cannot assign to undeclared variable '" + name + "'");
            it->second = value;
        }

        EvaluationContext::EvaluationContext() :
        m_store(new VariableTable()) {}
        
        EvaluationContext::EvaluationContext(const VariableStore& store) :
        m_store(store.clone()) {}

        EvaluationContext::~EvaluationContext() {
            delete m_store;
        }
        
        Value EvaluationContext::variableValue(const String& name) const {
            return m_store->value(name);
        }
        
        void EvaluationContext::declareVariable(const String& name, const Value& value) {
            m_store->declare(name, value);
        }
        
        EvaluationStack::EvaluationStack(const EvaluationContext& next) :
        m_next(next) {}
        
        Value EvaluationStack::variableValue(const String& name) const {
            const Value& value = EvaluationContext::variableValue(name);
            if (value != Value::Undefined)
                return value;
            return m_next.variableValue(name);
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
            return m_expression->evaluate(context);
        }

        void ExpressionBase::replaceExpression(ExpressionBase*& oldExpression, ExpressionBase* newExpression) {
            if (newExpression != NULL && newExpression != oldExpression) {
                delete oldExpression;
                oldExpression = newExpression;
            }
        }
        
        ExpressionBase::ExpressionBase(const size_t line, const size_t column) : m_line(line), m_column(column) {}
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

        Value ExpressionBase::evaluate(const EvaluationContext& context) const {
            return doEvaluate(context);
        }

        ExpressionBase* ExpressionBase::doReorderByPrecedence() {
            return this;
        }
        
        ExpressionBase* ExpressionBase::doReorderByPrecedence(BinaryOperator* parent) {
            return parent;
        }

        LiteralExpression::LiteralExpression(const Value& value, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_value(value, line, column) {}
        
        ExpressionBase* LiteralExpression::create(const Value& value, const size_t line, const size_t column) {
            return new LiteralExpression(value, line, column);
        }

        ExpressionBase* LiteralExpression::doClone() const {
            return new LiteralExpression(m_value, m_line, m_column);
        }

        ExpressionBase* LiteralExpression::doOptimize() {
            return this;
        }

        Value LiteralExpression::doEvaluate(const EvaluationContext& context) const {
            return m_value;
        }

        VariableExpression::VariableExpression(const String& variableName, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_variableName(variableName) {}

        ExpressionBase* VariableExpression::create(const String& variableName, const size_t line, const size_t column) {
            return new VariableExpression(variableName, line, column);
        }
        
        ExpressionBase* VariableExpression::doClone() const {
            return new VariableExpression(m_variableName, m_line, m_column);
        }

        ExpressionBase* VariableExpression::doOptimize() {
            return NULL;
        }

        Value VariableExpression::doEvaluate(const EvaluationContext& context) const {
            return context.variableValue(m_variableName);
        }

        ArrayExpression::ArrayExpression(const ExpressionBase::List& elements, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_elements(elements) {}
        
        ExpressionBase* ArrayExpression::create(const ExpressionBase::List& elements, const size_t line, const size_t column) {
            return new ArrayExpression(elements, line, column);
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
            
            return new ArrayExpression(clones, m_line, m_column);
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
            
            if (allOptimized)
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);
            
            return NULL;
        }

        Value ArrayExpression::doEvaluate(const EvaluationContext& context) const {
            ArrayType array;
            ExpressionBase::List::const_iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                const ExpressionBase* element = *it;
                const Value value = element->evaluate(context);
                if (value.type() == Type_Range) {
                    const RangeType& range = value.rangeValue();
                    array.reserve(array.size() + range.size());
                    for (size_t i = 0; i < range.size(); ++i)
                        array.push_back(Value(range[i], value.line(), value.column()));
                } else {
                    array.push_back(value);
                }
            }
            
            return Value(array, m_line, m_column);
        }

        MapExpression::MapExpression(const ExpressionBase::Map& elements, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_elements(elements) {}
        
        ExpressionBase* MapExpression::create(const ExpressionBase::Map& elements, const size_t line, const size_t column) {
            return new MapExpression(elements, line, column);
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
            
            return new MapExpression(clones, m_line, m_column);
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
            
            if (allOptimized)
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);
            
            return NULL;
        }
        
        Value MapExpression::doEvaluate(const EvaluationContext& context) const {
            MapType map;
            ExpressionBase::Map::const_iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                const String& key = it->first;
                const ExpressionBase* expression = it->second;
                map.insert(std::make_pair(key, expression->evaluate(context)));
            }
            
            return Value(map, m_line, m_column);
        }

        UnaryOperator::UnaryOperator(ExpressionBase* operand, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_operand(operand) {
            assert(m_operand != NULL);
        }

        UnaryOperator::~UnaryOperator() {
            delete m_operand;
        }

        ExpressionBase* UnaryOperator::doOptimize() {
            ExpressionBase* optimized = m_operand->optimize();
            replaceExpression(m_operand, optimized);
            
            if (optimized != NULL)
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);
            
            return NULL;
        }

        UnaryPlusOperator::UnaryPlusOperator(ExpressionBase* operand, const size_t line, const size_t column) :
        UnaryOperator(operand, line, column) {}

        ExpressionBase* UnaryPlusOperator::create(ExpressionBase* operand, const size_t line, const size_t column) {
            return new UnaryPlusOperator(operand, line, column);
        }

        ExpressionBase* UnaryPlusOperator::doClone() const {
            return new UnaryPlusOperator(m_operand->clone(), m_line, m_column);
        }

        Value UnaryPlusOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(+m_operand->evaluate(context), m_line, m_column);
        }

        UnaryMinusOperator::UnaryMinusOperator(ExpressionBase* operand, const size_t line, const size_t column) :
        UnaryOperator(operand, line, column) {}
        
        ExpressionBase* UnaryMinusOperator::create(ExpressionBase* operand, const size_t line, const size_t column) {
            return new UnaryMinusOperator(operand, line, column);
        }

        ExpressionBase* UnaryMinusOperator::doClone() const {
            return new UnaryMinusOperator(m_operand->clone(), m_line, m_column);
        }

        Value UnaryMinusOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(-m_operand->evaluate(context), m_line, m_column);
        }
        
        NegationOperator::NegationOperator(ExpressionBase* operand, const size_t line, const size_t column) :
        UnaryOperator(operand, line, column) {}

        ExpressionBase* NegationOperator::create(ExpressionBase* operand, const size_t line, const size_t column) {
            return new NegationOperator(operand, line, column);
        }

        ExpressionBase* NegationOperator::doClone() const  {
            return new NegationOperator(m_operand->clone(), m_line, m_column);
        }
        
        Value NegationOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(!m_operand->evaluate(context), m_line, m_column);
        }

        GroupingOperator::GroupingOperator(ExpressionBase* operand, const size_t line, const size_t column) :
        UnaryOperator(operand, line, column) {}

        ExpressionBase* GroupingOperator::create(ExpressionBase* operand, const size_t line, const size_t column) {
            return new GroupingOperator(operand, line, column);
        }

        ExpressionBase* GroupingOperator::doClone() const {
            return new GroupingOperator(m_operand->clone(), m_line, m_column);
        }
        
        Value GroupingOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(m_operand->evaluate(context), m_line, m_column);
        }

        SubscriptOperator::SubscriptOperator(ExpressionBase* indexableOperand, ExpressionBase* indexOperand, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_indexableOperand(indexableOperand),
        m_indexOperand(indexOperand) {
            assert(m_indexableOperand != NULL);
            assert(m_indexOperand != NULL);
        }
        
        SubscriptOperator::~SubscriptOperator() {
            delete m_indexableOperand;
            delete m_indexOperand;
        }

        ExpressionBase* SubscriptOperator::create(ExpressionBase* indexableOperand, ExpressionBase* indexOperand, const size_t line, const size_t column) {
            return (new SubscriptOperator(indexableOperand, indexOperand, line, column))->reorderByPrecedence();
        }
        
        ExpressionBase* SubscriptOperator::doClone() const {
            return new SubscriptOperator(m_indexableOperand->clone(), m_indexOperand->clone(), m_line, m_column);
        }
        
        ExpressionBase* SubscriptOperator::doOptimize() {
            ExpressionBase* indexableOptimized = m_indexableOperand->optimize();
            ExpressionBase* indexOptimized = m_indexOperand->optimize();
            
            replaceExpression(m_indexableOperand, indexableOptimized);
            replaceExpression(m_indexOperand, indexOptimized);
            
            if (indexableOptimized != NULL && indexOptimized != NULL)
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);
            
            return NULL;
        }

        Value SubscriptOperator::doEvaluate(const EvaluationContext& context) const {
            const Value indexableValue = m_indexableOperand->evaluate(context);

            EvaluationStack stack(context);
            stack.declareVariable(RangeOperator::AutoRangeParameterName(), Value(indexableValue.length()-1, m_line, m_column));
            const Value indexValue = m_indexOperand->evaluate(stack);
            
            return indexableValue[indexValue];
        }

        BinaryOperator::BinaryOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        ExpressionBase(line, column),
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
            
            if (leftOptimized != NULL && rightOptimized != NULL)
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);
            
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

        AdditionOperator::AdditionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* AdditionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new AdditionOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* AdditionOperator::doClone() const {
            return new AdditionOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }
        
        Value AdditionOperator::doEvaluate(const EvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return Value(leftValue + rightValue, m_line, m_column);
        }

        BinaryOperator::Traits AdditionOperator::doGetTraits() const {
            return Traits(4, true, true);
        }
        
        SubtractionOperator::SubtractionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* SubtractionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new SubtractionOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* SubtractionOperator::doClone() const {
            return new SubtractionOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }
        
        Value SubtractionOperator::doEvaluate(const EvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return Value(leftValue - rightValue, m_line, m_column);
        }
        
        BinaryOperator::Traits SubtractionOperator::doGetTraits() const {
            return Traits(4, false, false);
        }
        
        MultiplicationOperator::MultiplicationOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}
        
        ExpressionBase* MultiplicationOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new MultiplicationOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* MultiplicationOperator::doClone() const {
            return new MultiplicationOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }
        
        Value MultiplicationOperator::doEvaluate(const EvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return Value(leftValue * rightValue, m_line, m_column);
        }
        
        BinaryOperator::Traits MultiplicationOperator::doGetTraits() const {
            return Traits(5, true, true);
        }

        DivisionOperator::DivisionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}
        
        ExpressionBase* DivisionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new DivisionOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* DivisionOperator::doClone() const {
            return new DivisionOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }
        
        Value DivisionOperator::doEvaluate(const EvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return Value(leftValue / rightValue, m_line, m_column);
        }
        
        BinaryOperator::Traits DivisionOperator::doGetTraits() const {
            return Traits(5, false, false);
        }

        ModulusOperator::ModulusOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}
        
        ExpressionBase* ModulusOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new ModulusOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* ModulusOperator::doClone() const {
            return new ModulusOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }
        
        Value ModulusOperator::doEvaluate(const EvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return Value(leftValue % rightValue, m_line, m_column);
        }
        
        BinaryOperator::Traits ModulusOperator::doGetTraits() const {
            return Traits(5, false, false);
        }
        
        const String& RangeOperator::AutoRangeParameterName() {
            static const String Name = "__AutoRangeParameter";
            return Name;
        }

        ConjunctionOperator::ConjunctionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* ConjunctionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new ConjunctionOperator(leftOperand, rightOperand, line, column);
        }

        ExpressionBase* ConjunctionOperator::doClone() const {
            return new ConjunctionOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }
        
        Value ConjunctionOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(m_leftOperand->evaluate(context) && m_rightOperand->evaluate(context), m_line, m_column);
        }
        
        BinaryOperator::Traits ConjunctionOperator::doGetTraits() const {
            return Traits(2, true, true);
        }
        
        DisjunctionOperator::DisjunctionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}
        
        ExpressionBase* DisjunctionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new DisjunctionOperator(leftOperand, rightOperand, line, column);
        }
        
        ExpressionBase* DisjunctionOperator::doClone() const {
            return new DisjunctionOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }
        
        Value DisjunctionOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(m_leftOperand->evaluate(context) || m_rightOperand->evaluate(context), m_line, m_column);
        }
        
        BinaryOperator::Traits DisjunctionOperator::doGetTraits() const {
            return Traits(1, true, true);
        }

        ComparisonOperator::ComparisonOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const Op op, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column),
        m_op(op) {}

        ExpressionBase* ComparisonOperator::createLess(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_Less, line, column);
        }
        
        ExpressionBase* ComparisonOperator::createLessOrEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_LessOrEqual, line, column);
        }
        
        ExpressionBase* ComparisonOperator::createEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_Equal, line, column);
        }
        
        ExpressionBase* ComparisonOperator::createInequal(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_Inequal, line, column);
        }
        
        ExpressionBase* ComparisonOperator::createGreaterOrEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_GreaterOrEqual, line, column);
        }
        
        ExpressionBase* ComparisonOperator::createGreater(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_Greater, line, column);
        }
        
        ExpressionBase* ComparisonOperator::doClone() const {
            return new ComparisonOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_op, m_line, m_column);
        }
        
        Value ComparisonOperator::doEvaluate(const EvaluationContext& context) const {
            switch (m_op) {
                case Op_Less:
                    return Value(m_leftOperand->evaluate(context) < m_rightOperand->evaluate(context), m_line, m_column);
                case Op_LessOrEqual:
                    return Value(m_leftOperand->evaluate(context) <= m_rightOperand->evaluate(context), m_line, m_column);
                case Op_Equal:
                    return Value(m_leftOperand->evaluate(context) == m_rightOperand->evaluate(context), m_line, m_column);
                case Op_Inequal:
                    return Value(m_leftOperand->evaluate(context) != m_rightOperand->evaluate(context), m_line, m_column);
                case Op_GreaterOrEqual:
                    return Value(m_leftOperand->evaluate(context) >= m_rightOperand->evaluate(context), m_line, m_column);
                case Op_Greater:
                    return Value(m_leftOperand->evaluate(context) > m_rightOperand->evaluate(context), m_line, m_column);
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

        RangeOperator::RangeOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}
        
        ExpressionBase* RangeOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new RangeOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }
        
        ExpressionBase* RangeOperator::createAutoRangeWithLeftOperand(ExpressionBase* leftOperand, const size_t line, const size_t column) {
            return create(leftOperand, VariableExpression::create(AutoRangeParameterName(), line, column), line, column);
        }
        
        ExpressionBase* RangeOperator::createAutoRangeWithRightOperand(ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return create(VariableExpression::create(AutoRangeParameterName(), line, column), rightOperand, line, column);
        }

        ExpressionBase* RangeOperator::doClone() const {
            return new RangeOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }
        
        Value RangeOperator::doEvaluate(const EvaluationContext& context) const {
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

            return Value(range, m_line, m_column);
        }
        
        BinaryOperator::Traits RangeOperator::doGetTraits() const {
            return Traits(0, false, false);
        }
    }
}
