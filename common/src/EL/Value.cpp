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

#include "Value.h"

#include "CollectionUtils.h"
#include "Base/ColUtils.h"
#include "Base/VecUtils.h"
#include "EL/ELExceptions.h"
#include "StringStream.h"
#include "StringUtils.h"

#include <algorithm>
#include <cmath>
#include <iterator>

namespace TrenchBroom {
    namespace EL {
        ValueHolder::~ValueHolder() {}

        String ValueHolder::describe() const {
            StringStream str;
            appendToStream(str, false, "");
            return str.str();
        }

        const BooleanType& ValueHolder::booleanValue() const { throw DereferenceError(describe(), type(), ValueType::Boolean); }
        const StringType&  ValueHolder::stringValue()  const { throw DereferenceError(describe(), type(), ValueType::String); }
        const NumberType&  ValueHolder::numberValue()  const { throw DereferenceError(describe(), type(), ValueType::Number); }
              IntegerType  ValueHolder::integerValue() const { return static_cast<IntegerType>(numberValue()); }
        const ArrayType&   ValueHolder::arrayValue()   const { throw DereferenceError(describe(), type(), ValueType::Array); }
        const MapType&     ValueHolder::mapValue()     const { throw DereferenceError(describe(), type(), ValueType::Map); }
        const RangeType&   ValueHolder::rangeValue()   const { throw DereferenceError(describe(), type(), ValueType::Range); }

        BooleanValueHolder::BooleanValueHolder(const BooleanType& value) : m_value(value) {}
        ValueType BooleanValueHolder::type() const { return ValueType::Boolean; }
        const BooleanType& BooleanValueHolder::booleanValue() const { return m_value; }
        size_t BooleanValueHolder::length() const { return 1; }

        bool BooleanValueHolder::convertibleTo(ValueType toType) const {
            switch (toType) {
                case ValueType::Boolean:
                case ValueType::String:
                case ValueType::Number:
                    return true;
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Undefined:
                case ValueType::Null:
                    break;
            }

            return false;
        }

        ValueHolder* BooleanValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case ValueType::Boolean:
                    return new BooleanValueHolder(m_value);
                case ValueType::String:
                    return new StringValueHolder(m_value ? "true" : "false" );
                case ValueType::Number:
                    return new NumberValueHolder(m_value ? 1.0 : 0.0);
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Undefined:
                case ValueType::Null:
                    break;
            }

            throw ConversionError(describe(), type(), toType);
        }

        ValueHolder* BooleanValueHolder::clone() const { return new BooleanValueHolder(m_value); }
        void BooleanValueHolder::appendToStream(std::ostream& str, const bool /* multiline */, const String& /* indent */) const { str << (m_value ? "true" : "false"); }

        StringHolder::~StringHolder() {}
        ValueType StringHolder::type() const { return ValueType::String; }
        const StringType& StringHolder::stringValue() const { return doGetValue(); }
        size_t StringHolder::length() const { return doGetValue().length(); }

        bool StringHolder::convertibleTo(const ValueType toType) const {
            switch (toType) {
                case ValueType::Boolean:
                case ValueType::String:
                    return true;
                case ValueType::Number: {
                    if (StringUtils::isBlank(doGetValue()))
                        return true;
                    const char* begin = doGetValue().c_str();
                    char* end;
                    const NumberType value = std::strtod(begin, &end);
                    if (value == 0.0 && end == begin)
                        return false;
                    return true;
                }
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            return false;
        }

        ValueHolder* StringHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case ValueType::Boolean:
                    return new BooleanValueHolder(!StringUtils::caseSensitiveEqual(doGetValue(), "false") && !doGetValue().empty());
                case ValueType::String:
                    return new StringValueHolder(doGetValue());
                case ValueType::Number: {
                    if (StringUtils::isBlank(doGetValue()))
                        return new NumberValueHolder(0.0);
                    const char* begin = doGetValue().c_str();
                    char* end;
                    const NumberType value = std::strtod(begin, &end);
                    if (value == 0.0 && end == begin)
                        throw ConversionError(describe(), type(), toType);
                    return new NumberValueHolder(value);
                }
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            throw ConversionError(describe(), type(), toType);
        }

        void StringHolder::appendToStream(std::ostream& str, const bool /* multiline */, const String& /* indent */) const {
            // Unescaping happens in IO::ELParser::parseLiteral
            str << "\"" << StringUtils::escape(doGetValue(), "\\\"") << "\"";
        }



        StringValueHolder::StringValueHolder(const StringType& value) : m_value(value) {}
        ValueHolder* StringValueHolder::clone() const { return new StringValueHolder(m_value); }
        const StringType& StringValueHolder::doGetValue() const { return m_value; }



        StringReferenceHolder::StringReferenceHolder(const StringType& value) : m_value(value) {}
        ValueHolder* StringReferenceHolder::clone() const { return new StringReferenceHolder(m_value); }
        const StringType& StringReferenceHolder::doGetValue() const { return m_value; }



        NumberValueHolder::NumberValueHolder(const NumberType& value) : m_value(value) {}
        ValueType NumberValueHolder::type() const { return ValueType::Number; }
        const NumberType& NumberValueHolder::numberValue() const { return m_value; }
        size_t NumberValueHolder::length() const { return 1; }

        bool NumberValueHolder::convertibleTo(const ValueType toType) const {
            switch (toType) {
                case ValueType::Boolean:
                case ValueType::String:
                case ValueType::Number:
                    return true;
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            return false;
        }

        ValueHolder* NumberValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case ValueType::Boolean:
                    return new BooleanValueHolder(m_value != 0.0);
                case ValueType::String:
                    return new StringValueHolder(describe());
                case ValueType::Number:
                    return new NumberValueHolder(m_value);
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            throw ConversionError(describe(), type(), toType);
        }

        ValueHolder* NumberValueHolder::clone() const { return new NumberValueHolder(m_value); }

        void NumberValueHolder::appendToStream(std::ostream& str, const bool /* multiline */, const String& /* indent */) const {
            if (std::abs(m_value - std::round(m_value)) < RoundingThreshold) {
                str.precision(0);
                str.setf(std::ios::fixed);
            } else {
                str.precision(17);
                str.unsetf(std::ios::fixed);
            }
            str << m_value;
        }



        ArrayValueHolder::ArrayValueHolder(const ArrayType& value) : m_value(value) {}
        ValueType ArrayValueHolder::type() const { return ValueType::Array; }
        const ArrayType& ArrayValueHolder::arrayValue() const { return m_value; }
        size_t ArrayValueHolder::length() const { return m_value.size(); }

        bool ArrayValueHolder::convertibleTo(const ValueType toType) const {
            switch (toType) {
                case ValueType::Array:
                    return true;
                case ValueType::Boolean:
                case ValueType::String:
                case ValueType::Number:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            return false;
        }

        ValueHolder* ArrayValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case ValueType::Array:
                    return new ArrayValueHolder(m_value);
                case ValueType::Boolean:
                case ValueType::String:
                case ValueType::Number:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
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
                else
                    str << " ";
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
                if (multiline)
                    str << indent;
                else
                    str << " ";
                str << "]";
            }
        }


        MapValueHolder::MapValueHolder(const MapType& value) : m_value(value) {}
        ValueType MapValueHolder::type() const { return ValueType::Map; }
        const MapType& MapValueHolder::mapValue() const { return m_value; }
        size_t MapValueHolder::length() const { return m_value.size(); }

        bool MapValueHolder::convertibleTo(const ValueType toType) const {
            switch (toType) {
                case ValueType::Map:
                    return true;
                case ValueType::Boolean:
                case ValueType::String:
                case ValueType::Number:
                case ValueType::Array:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            return false;
        }

        ValueHolder* MapValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case ValueType::Map:
                    return new MapValueHolder(m_value);
                case ValueType::Boolean:
                case ValueType::String:
                case ValueType::Number:
                case ValueType::Array:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
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
                else
                    str << " ";

                size_t i = 0;
                for (const auto& entry : m_value) {
                    str << childIndent << "\"" << entry.first << "\"" << ": ";
                    entry.second.appendToStream(str, multiline, childIndent);
                    if (i++ < m_value.size() - 1) {
                        str << ",";
                        if (!multiline)
                            str << " ";
                    }
                    if (multiline)
                        str << "\n";
                }
                if (multiline)
                    str << indent;
                else
                    str << " ";
                str << "}";
            }
        }


        RangeValueHolder::RangeValueHolder(const RangeType& value) : m_value(value) {}
        ValueType RangeValueHolder::type() const { return ValueType::Range; }
        const RangeType& RangeValueHolder::rangeValue() const { return m_value; }
        size_t RangeValueHolder::length() const { return m_value.size(); }

        bool RangeValueHolder::convertibleTo(const ValueType toType) const {
            switch (toType) {
                case ValueType::Range:
                    return true;
                case ValueType::Boolean:
                case ValueType::String:
                case ValueType::Number:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            return false;
        }

        ValueHolder* RangeValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case ValueType::Range:
                    return new RangeValueHolder(m_value);
                case ValueType::Boolean:
                case ValueType::String:
                case ValueType::Number:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            throw ConversionError(describe(), type(), toType);
        }

        ValueHolder* RangeValueHolder::clone() const { return new RangeValueHolder(m_value); }

        void RangeValueHolder::appendToStream(std::ostream& str, const bool /* multiline */, const String& /* indent */) const {
            str << "[";
            for (size_t i = 0; i < m_value.size(); ++i) {
                str << m_value[i];
                if (i < m_value.size() - 1)
                    str << ", ";
            }
            str << "]";
        }


        ValueType NullValueHolder::type() const { return ValueType::Null; }
        size_t NullValueHolder::length() const { return 0; }
        const StringType& NullValueHolder::stringValue() const   { static const StringType result;         return result; }
        const BooleanType& NullValueHolder::booleanValue() const { static const BooleanType result(false); return result; }
        const NumberType& NullValueHolder::numberValue() const   { static const NumberType result(0.0);    return result; }
        const ArrayType& NullValueHolder::arrayValue() const     { static const ArrayType result(0);       return result; }
        const MapType& NullValueHolder::mapValue() const         { static const MapType result;            return result; }

        bool NullValueHolder::convertibleTo(const ValueType toType) const {
            switch (toType) {
                case ValueType::Boolean:
                case ValueType::Null:
                case ValueType::Number:
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Map:
                    return true;
                case ValueType::Range:
                case ValueType::Undefined:
                    break;
            }

            return false;
        }

        ValueHolder* NullValueHolder::convertTo(const ValueType toType) const {
            switch (toType) {
                case ValueType::Boolean:
                    return new BooleanValueHolder(false);
                case ValueType::Null:
                    return new NullValueHolder();
                case ValueType::Number:
                    return new NumberValueHolder(0.0);
                case ValueType::String:
                    return new StringValueHolder("");
                case ValueType::Array:
                    return new ArrayValueHolder(ArrayType(0));
                case ValueType::Map:
                    return new MapValueHolder(MapType());
                case ValueType::Range:
                case ValueType::Undefined:
                    break;
            }

            throw ConversionError(describe(), type(), toType);
        }

        ValueHolder* NullValueHolder::clone() const { return new NullValueHolder(); }
        void NullValueHolder::appendToStream(std::ostream& str, const bool /* multiline */, const String& /* indent */) const { str << "null"; }


        ValueType UndefinedValueHolder::type() const { return ValueType::Undefined; }
        size_t UndefinedValueHolder::length() const { return 0; }
        bool UndefinedValueHolder::convertibleTo(const ValueType /* toType */) const { return false; }
        ValueHolder* UndefinedValueHolder::convertTo(const ValueType toType) const { throw ConversionError(describe(), type(), toType); }
        ValueHolder* UndefinedValueHolder::clone() const { return new UndefinedValueHolder(); }
        void UndefinedValueHolder::appendToStream(std::ostream& str, const bool /* multiline */, const String& /* indent */) const { str << "undefined"; }


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

        Value::Value()                                                                 : m_value(new NullValueHolder()), m_line(0), m_column(0) {}

        Value Value::ref(const StringType& value, const size_t line, const size_t column) {
            return Value(new StringReferenceHolder(value), line, column);
        }

        Value Value::ref(const StringType& value) {
            return ref(value, 0, 0);
        }

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

        IntegerType Value::integerValue() const {
            return m_value->integerValue();
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
            return type() == ValueType::Null;
        }

        bool Value::undefined() const {
            return type() == ValueType::Undefined;
        }

        const StringList Value::asStringList() const {
            const ArrayType& array = arrayValue();
            StringList result;
            result.reserve(array.size());

            std::transform(std::begin(array), std::end(array), std::back_inserter(result),
                           [](const Value& entry) { return entry.convertTo(ValueType::String).stringValue(); });

            return result;
        }

        const StringSet Value::asStringSet() const {
            const ArrayType& array = arrayValue();
            StringSet result;

            std::transform(std::begin(array), std::end(array), std::inserter(result, result.begin()),
                           [](const Value& entry) { return entry.convertTo(ValueType::String).stringValue(); });

            return result;
        }

        size_t Value::length() const {
            return m_value->length();
        }

        bool Value::convertibleTo(const ValueType toType) const {
            if (type() == toType)
                return true;
            return m_value->convertibleTo(toType);
        }

        Value Value::convertTo(const ValueType toType) const {
            if (type() == toType)
                return *this;
            return Value(m_value->convertTo(toType), m_line, m_column);
        }

        String Value::asString(const bool multiline) const {
            StringStream str;
            appendToStream(str, multiline);
            return str.str();
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
                case ValueType::String: {
                    switch (indexValue.type()) {
                        case ValueType::Boolean:
                        case ValueType::Number: {
                            const size_t index = computeIndex(indexValue, length());
                            return index < length();
                        }
                        case ValueType::Array:
                        case ValueType::Range: {
                            const IndexList indices = computeIndexArray(indexValue, length());
                            for (size_t i = 0; i < indices.size(); ++i) {
                                const size_t index = indices[i];
                                if (index >= length())
                                    return false;
                            }
                            return true;
                        }
                        case ValueType::String:
                        case ValueType::Map:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }
                    break;
                }
                case ValueType::Array:
                    switch (indexValue.type()) {
                        case ValueType::Boolean:
                        case ValueType::Number: {
                            const size_t index = computeIndex(indexValue, length());
                            return index < length();
                        }
                        case ValueType::Array:
                        case ValueType::Range: {
                            const IndexList indices = computeIndexArray(indexValue, length());
                            for (size_t i = 0; i < indices.size(); ++i) {
                                const size_t index = indices[i];
                                if (index >= length())
                                    return false;
                            }
                            return true;
                        }
                        case ValueType::String:
                        case ValueType::Map:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }
                    break;
                case ValueType::Map:
                    switch (indexValue.type()) {
                        case ValueType::String: {
                            const MapType& map = mapValue();
                            const String& key = indexValue.stringValue();
                            const MapType::const_iterator it = map.find(key);
                            return it != std::end(map);
                        }
                        case ValueType::Array: {
                            const MapType& map = mapValue();
                            const ArrayType& keys = indexValue.arrayValue();
                            for (size_t i = 0; i < keys.size(); ++i) {
                                const Value& keyValue = keys[i];
                                if (keyValue.type() != ValueType::String)
                                    throw ConversionError(keyValue.describe(), keyValue.type(), ValueType::String);
                                const String& key = keyValue.stringValue();
                                const MapType::const_iterator it = map.find(key);
                                if (it == std::end(map))
                                    return false;
                            }
                            return true;
                        }
                        case ValueType::Boolean:
                        case ValueType::Number:
                        case ValueType::Map:
                        case ValueType::Range:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }
                    break;
                case ValueType::Boolean:
                case ValueType::Number:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }
            return false;
        }

        bool Value::contains(const size_t index) const {
            switch (type()) {
                case ValueType::String:
                case ValueType::Array:
                    return index < length();
                case ValueType::Map:
                case ValueType::Boolean:
                case ValueType::Number:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }
            return false;
        }

        bool Value::contains(const String& key) const {
            const MapType& map = mapValue();
            const MapType::const_iterator it = map.find(key);
            return it != std::end(map);
        }

        StringSet Value::keys() const {
            return MapUtils::keySet(mapValue());
        }

        Value Value::operator[](const Value& indexValue) const {
            switch (type()) {
                case ValueType::String:
                    switch (indexValue.type()) {
                        case ValueType::Boolean:
                        case ValueType::Number: {
                            const StringType& str = stringValue();
                            const size_t index = computeIndex(indexValue, str.length());
                            StringStream result;
                            if (index < str.length())
                                result << str[index];
                            return Value(result.str(), m_line, m_column);
                        }
                        case ValueType::Array:
                        case ValueType::Range: {
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
                        case ValueType::String:
                        case ValueType::Map:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }
                    break;
                case ValueType::Array:
                    switch (indexValue.type()) {
                        case ValueType::Boolean:
                        case ValueType::Number: {
                            const ArrayType& array = arrayValue();
                            const size_t index = computeIndex(indexValue, array.size());
                            if (index >= array.size())
                                throw IndexOutOfBoundsError(*this, indexValue, index);
                            return array[index];
                        }
                        case ValueType::Array:
                        case ValueType::Range: {
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
                        case ValueType::String:
                        case ValueType::Map:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }
                    break;
                case ValueType::Map:
                    switch (indexValue.type()) {
                        case ValueType::String: {
                            const MapType& map = mapValue();
                            const String& key = indexValue.stringValue();
                            const MapType::const_iterator it = map.find(key);
                            if (it == std::end(map))
                                return Value::Undefined;
                            return it->second;
                        }
                        case ValueType::Array: {
                            const MapType& map = mapValue();
                            const ArrayType& keys = indexValue.arrayValue();
                            MapType result;
                            for (size_t i = 0; i < keys.size(); ++i) {
                                const Value& keyValue = keys[i];
                                if (keyValue.type() != ValueType::String)
                                    throw ConversionError(keyValue.describe(), keyValue.type(), ValueType::String);
                                const String& key = keyValue.stringValue();
                                const MapType::const_iterator it = map.find(key);
                                if (it != std::end(map))
                                    result.insert(std::make_pair(key, it->second));
                            }
                            return Value(result, m_line, m_column);
                        }
                        case ValueType::Boolean:
                        case ValueType::Number:
                        case ValueType::Map:
                        case ValueType::Range:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }
                    break;
                case ValueType::Boolean:
                case ValueType::Number:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            throw IndexError(*this, indexValue);
        }

        Value Value::operator[](const size_t index) const {
            switch (type()) {
                case ValueType::String: {
                    const StringType& str = stringValue();
                    StringStream result;
                    if (index < str.length())
                        result << str[index];
                    return Value(result.str());
                }
                case ValueType::Array: {
                    const ArrayType& array = arrayValue();
                    if (index >= array.size())
                        throw IndexOutOfBoundsError(*this, index);
                    return array[index];
                }
                case ValueType::Map:
                case ValueType::Boolean:
                case ValueType::Number:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            throw IndexError(*this, index);
        }

        Value Value::operator[](const int index) const {
            assert(index >= 0);
            return this->operator[](static_cast<size_t>(index));
        }

        Value Value::operator[](const String& key) const {
            return this->operator[](key.c_str());
        }

        Value Value::operator[](const char* key) const {
            switch (type()) {
                case ValueType::Map: {
                    const MapType& map = mapValue();
                    const MapType::const_iterator it = map.find(key);
                    if (it == std::end(map))
                        return Value::Null;
                    return it->second;
                }
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Boolean:
                case ValueType::Number:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
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
                case ValueType::Array: {
                    const ArrayType& indexArray = indexValue.arrayValue();
                    result.reserve(result.size() + indexArray.size());
                    for (size_t i = 0; i < indexArray.size(); ++i)
                        computeIndexArray(indexArray[i], indexableSize, result);
                    break;
                }
                case ValueType::Range: {
                    const RangeType& range = indexValue.rangeValue();
                    result.reserve(result.size() + range.size());
                    for (size_t i = 0; i < range.size(); ++i)
                        result.push_back(computeIndex(range[i], indexableSize));
                    break;
                }
                case ValueType::Boolean:
                case ValueType::Number:
                case ValueType::String:
                case ValueType::Map:
                case ValueType::Null:
                case ValueType::Undefined:
                    result.push_back(computeIndex(indexValue, indexableSize));
                    break;
            }
        }

        size_t Value::computeIndex(const Value& indexValue, const size_t indexableSize) const {
            return computeIndex(static_cast<long>(indexValue.convertTo(ValueType::Number).numberValue()), indexableSize);
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
                case ValueType::Boolean:
                case ValueType::Number:
                    return Value(convertTo(ValueType::Number).numberValue());
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }
            throw EvaluationError("Cannot apply unary plus to value '" + describe() + "' of type '" + typeName());
        }

        Value Value::operator-() const {
            switch (type()) {
                case ValueType::Boolean:
                case ValueType::Number:
                    return Value(-convertTo(ValueType::Number).numberValue());
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }
            throw EvaluationError("Cannot negate value '" + describe() + "' of type '" + typeName());
        }

        Value operator+(const Value& lhs, const Value& rhs) {
            switch (lhs.type()) {
                case ValueType::Boolean:
                case ValueType::Number:
                    switch (rhs.type()) {
                        case ValueType::Boolean:
                        case ValueType::Number:
                            return Value(lhs.convertTo(ValueType::Number).numberValue() + rhs.convertTo(ValueType::Number).numberValue());
                        case ValueType::String:
                        case ValueType::Array:
                        case ValueType::Map:
                        case ValueType::Range:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }
                    break;
                case ValueType::String:
                    switch (rhs.type()) {
                        case ValueType::String:
                            return Value(lhs.convertTo(ValueType::String).stringValue() + rhs.convertTo(ValueType::String).stringValue());
                        case ValueType::Boolean:
                        case ValueType::Number:
                        case ValueType::Array:
                        case ValueType::Map:
                        case ValueType::Range:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }
                    break;
                case ValueType::Array:
                    switch (rhs.type()) {
                        case ValueType::Array:
                            return Value(VecUtils::concat(lhs.arrayValue(), rhs.arrayValue()));
                        case ValueType::Boolean:
                        case ValueType::Number:
                        case ValueType::String:
                        case ValueType::Map:
                        case ValueType::Range:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }
                    break;
                case ValueType::Map:
                    switch (rhs.type()) {
                        case ValueType::Map:
                            return Value(MapUtils::concatenate(lhs.mapValue(), rhs.mapValue()));
                        case ValueType::Boolean:
                        case ValueType::Number:
                        case ValueType::String:
                        case ValueType::Array:
                        case ValueType::Range:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }
                    break;
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            throw EvaluationError("Cannot add value '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + " to value '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + "'");
        }

        Value operator-(const Value& lhs, const Value& rhs) {
            switch (lhs.type()) {
                case ValueType::Boolean:
                case ValueType::Number:
                    switch (rhs.type()) {
                        case ValueType::Boolean:
                        case ValueType::Number:
                            return Value(lhs.convertTo(ValueType::Number).numberValue() - rhs.convertTo(ValueType::Number).numberValue());
                        case ValueType::String:
                        case ValueType::Array:
                        case ValueType::Map:
                        case ValueType::Range:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }
                    break;
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            throw EvaluationError("Cannot subtract value '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + " from value '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + "'");
        }

        Value operator*(const Value& lhs, const Value& rhs) {
            switch (lhs.type()) {
                case ValueType::Boolean:
                case ValueType::Number:
                    switch (rhs.type()) {
                        case ValueType::Boolean:
                        case ValueType::Number:
                            return Value(lhs.convertTo(ValueType::Number).numberValue() * rhs.convertTo(ValueType::Number).numberValue());
                        case ValueType::String:
                        case ValueType::Array:
                        case ValueType::Map:
                        case ValueType::Range:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }
                    break;
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            throw EvaluationError("Cannot subtract value '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + " from value '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + "'");
        }

        Value operator/(const Value& lhs, const Value& rhs) {
            switch (lhs.type()) {
                case ValueType::Boolean:
                case ValueType::Number:
                    switch (rhs.type()) {
                        case ValueType::Boolean:
                        case ValueType::Number:
                            return Value(lhs.convertTo(ValueType::Number).numberValue() / rhs.convertTo(ValueType::Number).numberValue());
                        case ValueType::String:
                        case ValueType::Array:
                        case ValueType::Map:
                        case ValueType::Range:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }
                    break;
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            throw EvaluationError("Cannot subtract value '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + " from value '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + "'");
        }

        Value operator%(const Value& lhs, const Value& rhs) {
            switch (lhs.type()) {
                case ValueType::Boolean:
                case ValueType::Number:
                    switch (rhs.type()) {
                        case ValueType::Boolean:
                        case ValueType::Number:
                            return Value(std::fmod(lhs.convertTo(ValueType::Number).numberValue(), rhs.convertTo(ValueType::Number).numberValue()));
                        case ValueType::String:
                        case ValueType::Array:
                        case ValueType::Map:
                        case ValueType::Range:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }
                    break;
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }

            throw EvaluationError("Cannot compute moduls of value '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + " and value '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'");
        }

        Value::operator bool() const {
            switch (type()) {
                case ValueType::Boolean:
                    return booleanValue();
                case ValueType::Number:
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }
            throw ConversionError(describe(), type(), ValueType::Boolean);
        }

        Value Value::operator!() const {
            switch (type()) {
                case ValueType::Boolean:
                    return Value(!booleanValue());
                case ValueType::Number:
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }
            throw ConversionError(describe(), type(), ValueType::Boolean);
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
                case ValueType::Boolean:
                    switch (rhs.type()) {
                        case ValueType::Boolean:
                        case ValueType::Number:
                        case ValueType::String:
                            return compareAsBooleans(lhs, rhs);
                        case ValueType::Null:
                        case ValueType::Undefined:
                            return 1;
                        case ValueType::Array:
                        case ValueType::Map:
                        case ValueType::Range:
                            break;
                    }
                    break;
                case ValueType::Number:
                    switch (rhs.type()) {
                        case ValueType::Boolean:
                            return compareAsBooleans(lhs, rhs);
                        case ValueType::Number:
                        case ValueType::String:
                            return compareAsNumbers(lhs, rhs);
                        case ValueType::Null:
                        case ValueType::Undefined:
                            return 1;
                        case ValueType::Array:
                        case ValueType::Map:
                        case ValueType::Range:
                            break;
                    }
                    break;
                case ValueType::String:
                    switch (rhs.type()) {
                        case ValueType::Boolean:
                            return compareAsBooleans(lhs, rhs);
                        case ValueType::Number:
                            return compareAsNumbers(lhs, rhs);
                        case ValueType::String:
                            return lhs.stringValue().compare(rhs.convertTo(ValueType::String).stringValue());
                        case ValueType::Null:
                        case ValueType::Undefined:
                            return 1;
                        case ValueType::Array:
                        case ValueType::Map:
                        case ValueType::Range:
                            break;
                    }
                    break;
                case ValueType::Null:
                    if (rhs.type() == ValueType::Null)
                        return 0;
                    return -1;
                case ValueType::Undefined:
                    if (rhs.type() == ValueType::Undefined)
                        return 0;
                    return -1;
                case ValueType::Array:
                    switch (rhs.type()) {
                        case ValueType::Array:
                            return ColUtils::lexicographicalCompare(lhs.arrayValue(), rhs.arrayValue());
                        case ValueType::Null:
                        case ValueType::Undefined:
                            return 1;
                        case ValueType::Boolean:
                        case ValueType::Number:
                        case ValueType::String:
                        case ValueType::Map:
                        case ValueType::Range:
                            break;
                    }
                    break;
                case ValueType::Map:
                    switch (rhs.type()) {
                        case ValueType::Map:
                            return MapUtils::compare(lhs.mapValue(), rhs.mapValue());
                        case ValueType::Null:
                        case ValueType::Undefined:
                            return 1;
                        case ValueType::Boolean:
                        case ValueType::Number:
                        case ValueType::String:
                        case ValueType::Array:
                        case ValueType::Range:
                            break;
                    }
                    break;
                case ValueType::Range:
                    switch (rhs.type()) {
                        case ValueType::Range:
                            return ColUtils::lexicographicalCompare(lhs.rangeValue(), rhs.rangeValue());
                        case ValueType::Null:
                        case ValueType::Undefined:
                            return 1;
                        case ValueType::Boolean:
                        case ValueType::Number:
                        case ValueType::String:
                        case ValueType::Array:
                        case ValueType::Map:
                            break;
                    }
                    break;
            }
            throw EvaluationError("Cannot compare value '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + " to value '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'");
        }

        int compareAsBooleans(const Value& lhs, const Value& rhs) {
            const bool lhsValue = lhs.convertTo(ValueType::Boolean).booleanValue();
            const bool rhsValue = rhs.convertTo(ValueType::Boolean).booleanValue();
            if (lhsValue == rhsValue)
                return 0;
            if (lhsValue)
                return 1;
            return -1;
        }

        int compareAsNumbers(const Value& lhs, const Value& rhs) {
            const NumberType diff = lhs.convertTo(ValueType::Number).numberValue() - rhs.convertTo(ValueType::Number).numberValue();
            if (diff < 0.0)
                return -1;
            else if (diff > 0.0)
                return 1;
            return 0;
        }

        Value Value::operator~() const {
            switch (type()) {
                case ValueType::Number:
                    return Value(~integerValue());
                case ValueType::Boolean:
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }
            throw ConversionError(describe(), type(), ValueType::Boolean);
        }

        Value operator&(const Value& lhs, const Value& rhs) {
            if (lhs.convertibleTo(ValueType::Number) && rhs.convertibleTo(ValueType::Number)) {
                const IntegerType lhsInt = lhs.convertTo(ValueType::Number).integerValue();
                const IntegerType rhsInt = rhs.convertTo(ValueType::Number).integerValue();
                return Value(lhsInt & rhsInt);
            }
            throw EvaluationError("Cannot apply operator & to '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + " and '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'");
        }

        Value operator|(const Value& lhs, const Value& rhs) {
            if (lhs.convertibleTo(ValueType::Number) && rhs.convertibleTo(ValueType::Number)) {
                const IntegerType lhsInt = lhs.convertTo(ValueType::Number).integerValue();
                const IntegerType rhsInt = rhs.convertTo(ValueType::Number).integerValue();
                return Value(lhsInt | rhsInt);
            }
            throw EvaluationError("Cannot apply operator | to '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + " and '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'");
        }

        Value operator^(const Value& lhs, const Value& rhs) {
            if (lhs.convertibleTo(ValueType::Number) && rhs.convertibleTo(ValueType::Number)) {
                const IntegerType lhsInt = lhs.convertTo(ValueType::Number).integerValue();
                const IntegerType rhsInt = rhs.convertTo(ValueType::Number).integerValue();
                return Value(lhsInt ^ rhsInt);
            }
            throw EvaluationError("Cannot apply operator ^ to '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + " and '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'");
        }

        Value operator<<(const Value& lhs, const Value& rhs) {
            if (lhs.convertibleTo(ValueType::Number) && rhs.convertibleTo(ValueType::Number)) {
                const IntegerType lhsInt = lhs.convertTo(ValueType::Number).integerValue();
                const IntegerType rhsInt = rhs.convertTo(ValueType::Number).integerValue();
                return Value(lhsInt << rhsInt);
            }
            throw EvaluationError("Cannot apply operator << to '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + " and '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'");
        }

        Value operator>>(const Value& lhs, const Value& rhs) {
            if (lhs.convertibleTo(ValueType::Number) && rhs.convertibleTo(ValueType::Number)) {
                const IntegerType lhsInt = lhs.convertTo(ValueType::Number).integerValue();
                const IntegerType rhsInt = rhs.convertTo(ValueType::Number).integerValue();
                return Value(lhsInt >> rhsInt);
            }
            throw EvaluationError("Cannot apply operator >> to '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + " and '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'");
        }
    }
}
