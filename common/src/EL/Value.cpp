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

#include "EL/ELExceptions.h"

#include <kdl/collection_utils.h>
#include <kdl/map_utils.h>
#include <kdl/overload.h>
#include <kdl/string_compare.h>
#include <kdl/string_format.h>
#include <kdl/vector_set.h>
#include <kdl/vector_utils.h>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <sstream>
#include <string>

namespace TrenchBroom {
    namespace EL {
        NullType::NullType() = default;
        const NullType NullType::Value = NullType();
        
        UndefinedType::UndefinedType() = default;
        const UndefinedType UndefinedType::Value = UndefinedType();
    
        const Value Value::Null = Value(NullType::Value);
        const Value Value::Undefined = Value(UndefinedType::Value);
            
        Value::Value() :
        m_value(NullType::Value),
        m_line(0u),
        m_column(0u) {}

        Value::Value(const BooleanType value, const size_t line, const size_t column) :
        m_value(value),
        m_line(line),
        m_column(column) {}

        Value::Value(StringType value, const size_t line, const size_t column) :
        m_value(std::move(value)),
        m_line(line),
        m_column(column) {}

        Value::Value(const char* value, const size_t line, const size_t column) :
        m_value(StringType(value)),
        m_line(line),
        m_column(column) {}

        Value::Value(const NumberType value, const size_t line, const size_t column) :
        m_value(value),
        m_line(line),
        m_column(column) {}

        Value::Value(const int value, const size_t line, const size_t column) :
        m_value(static_cast<NumberType>(value)),
        m_line(line),
        m_column(column) {}
    
        Value::Value(const long value, const size_t line, const size_t column) :
        m_value(static_cast<NumberType>(value)),
        m_line(line),
        m_column(column) {}
    
        Value::Value(const size_t value, const size_t line, const size_t column) :
        m_value(static_cast<NumberType>(value)),
        m_line(line),
        m_column(column) {}
    
        Value::Value(ArrayType value, const size_t line, const size_t column) :
        m_value(std::move(value)),
        m_line(line),
        m_column(column) {}
    
        Value::Value(MapType value, const size_t line, const size_t column) :
        m_value(std::move(value)),
        m_line(line),
        m_column(column) {}
    
        Value::Value(RangeType value, const size_t line, const size_t column) :
        m_value(std::move(value)),
        m_line(line),
        m_column(column) {}
    
        Value::Value(NullType value, const size_t line, const size_t column) :
        m_value(value),
        m_line(line),
        m_column(column) {}
    
        Value::Value(UndefinedType value, const size_t line, const size_t column) :
        m_value(value),
        m_line(line),
        m_column(column) {}
    
        Value::Value(Value value, const size_t line, const size_t column) :
        m_value(std::move(value.m_value)),
        m_line(line),
        m_column(column) {}
        
        ValueType Value::type() const {
            return std::visit(kdl::overload{
                [](const BooleanType&)   { return ValueType::Boolean; },
                [](const StringType&)    { return ValueType::String; },
                [](const NumberType&)    { return ValueType::Number; },
                [](const ArrayType&)     { return ValueType::Array; },
                [](const MapType&)       { return ValueType::Map; },
                [](const RangeType&)     { return ValueType::Range; },
                [](const NullType&)      { return ValueType::Null; },
                [](const UndefinedType&) { return ValueType::Undefined; },
            }, m_value);
        }
        
        std::string Value::typeName() const {
            return EL::typeName(type());
        }
        
        std::string Value::describe() const {
            return asString(false);
        }
        
        size_t Value::line() const {
            return m_line;
        }
        
        size_t Value::column() const {
            return m_column;
        }

        const BooleanType& Value::booleanValue() const {
            return std::visit(kdl::overload {
                [&](const BooleanType& b) -> const BooleanType& { return b; },
                [&](const StringType&)    -> const BooleanType& { throw DereferenceError(describe(), type(), ValueType::String); },
                [&](const NumberType&)    -> const BooleanType& { throw DereferenceError(describe(), type(), ValueType::Number); },
                [&](const ArrayType&)     -> const BooleanType& { throw DereferenceError(describe(), type(), ValueType::Array); },
                [&](const MapType&)       -> const BooleanType& { throw DereferenceError(describe(), type(), ValueType::Map); },
                [&](const RangeType&)     -> const BooleanType& { throw DereferenceError(describe(), type(), ValueType::Range); },
                [&](const NullType&)      -> const BooleanType& { static const BooleanType b = false; return b; },
                [&](const UndefinedType&) -> const BooleanType& { throw DereferenceError(describe(), type(), ValueType::Undefined); },
            }, m_value);
        }
        
        const StringType& Value::stringValue() const {
            return std::visit(kdl::overload {
                [&](const BooleanType&)   -> const StringType& { throw DereferenceError(describe(), type(), ValueType::Boolean); },
                [&](const StringType& s)  -> const StringType& { return s; },
                [&](const NumberType&)    -> const StringType& { throw DereferenceError(describe(), type(), ValueType::Number); },
                [&](const ArrayType&)     -> const StringType& { throw DereferenceError(describe(), type(), ValueType::Array); },
                [&](const MapType&)       -> const StringType& { throw DereferenceError(describe(), type(), ValueType::Map); },
                [&](const RangeType&)     -> const StringType& { throw DereferenceError(describe(), type(), ValueType::Range); },
                [&](const NullType&)      -> const StringType& { static const StringType s; return s; },
                [&](const UndefinedType&) -> const StringType& { throw DereferenceError(describe(), type(), ValueType::Undefined); },
            }, m_value);
        }
        
        const NumberType& Value::numberValue() const {
            return std::visit(kdl::overload {
                [&](const BooleanType&)   -> const NumberType& { throw DereferenceError(describe(), type(), ValueType::Boolean); },
                [&](const StringType&)    -> const NumberType& { throw DereferenceError(describe(), type(), ValueType::String); },
                [&](const NumberType& n)  -> const NumberType& { return n; },
                [&](const ArrayType&)     -> const NumberType& { throw DereferenceError(describe(), type(), ValueType::Array); },
                [&](const MapType&)       -> const NumberType& { throw DereferenceError(describe(), type(), ValueType::Map); },
                [&](const RangeType&)     -> const NumberType& { throw DereferenceError(describe(), type(), ValueType::Range); },
                [&](const NullType&)      -> const NumberType& { static const NumberType n = 0.0; return n; },
                [&](const UndefinedType&) -> const NumberType& { throw DereferenceError(describe(), type(), ValueType::Undefined); },
            }, m_value);
        }
        
        IntegerType Value::integerValue() const {
            return static_cast<IntegerType>(numberValue());
        }
        
        const ArrayType& Value::arrayValue() const {
            return std::visit(kdl::overload {
                [&](const BooleanType&)   -> const ArrayType& { throw DereferenceError(describe(), type(), ValueType::Boolean); },
                [&](const StringType&)    -> const ArrayType& { throw DereferenceError(describe(), type(), ValueType::String); },
                [&](const NumberType&)    -> const ArrayType& { throw DereferenceError(describe(), type(), ValueType::Number); },
                [&](const ArrayType& a)   -> const ArrayType& { return a; },
                [&](const MapType&)       -> const ArrayType& { throw DereferenceError(describe(), type(), ValueType::Map); },
                [&](const RangeType&)     -> const ArrayType& { throw DereferenceError(describe(), type(), ValueType::Range); },
                [&](const NullType&)      -> const ArrayType& { static const ArrayType a(0); return a; },
                [&](const UndefinedType&) -> const ArrayType& { throw DereferenceError(describe(), type(), ValueType::Undefined); },
            }, m_value);
        }
        
        const MapType& Value::mapValue() const {
            return std::visit(kdl::overload {
                [&](const BooleanType&)   -> const MapType& { throw DereferenceError(describe(), type(), ValueType::Boolean); },
                [&](const StringType&)    -> const MapType& { throw DereferenceError(describe(), type(), ValueType::String); },
                [&](const NumberType&)    -> const MapType& { throw DereferenceError(describe(), type(), ValueType::Number); },
                [&](const ArrayType&)     -> const MapType& { throw DereferenceError(describe(), type(), ValueType::Array); },
                [&](const MapType& m)     -> const MapType& { return m; },
                [&](const RangeType&)     -> const MapType& { throw DereferenceError(describe(), type(), ValueType::Range); },
                [&](const NullType&)      -> const MapType& { static const MapType m; return m; },
                [&](const UndefinedType&) -> const MapType& { throw DereferenceError(describe(), type(), ValueType::Undefined); },
            }, m_value);
        }
        
        const RangeType& Value::rangeValue() const {
            return std::visit(kdl::overload {
                [&](const BooleanType&)   -> const RangeType& { throw DereferenceError(describe(), type(), ValueType::Boolean); },
                [&](const StringType&)    -> const RangeType& { throw DereferenceError(describe(), type(), ValueType::String); },
                [&](const NumberType&)    -> const RangeType& { throw DereferenceError(describe(), type(), ValueType::Number); },
                [&](const ArrayType&)     -> const RangeType& { throw DereferenceError(describe(), type(), ValueType::Array); },
                [&](const MapType&)       -> const RangeType& { throw DereferenceError(describe(), type(), ValueType::Map); },
                [&](const RangeType& r)   -> const RangeType& { return r; },
                [&](const NullType&)      -> const RangeType& { throw DereferenceError(describe(), type(), ValueType::Null); },
                [&](const UndefinedType&) -> const RangeType& { throw DereferenceError(describe(), type(), ValueType::Undefined); },
            }, m_value);
        }
        
        bool Value::null() const {
            return type() == ValueType::Null;
        }
        
        bool Value::undefined() const {
            return type() == ValueType::Undefined;
        }
        
        const std::vector<std::string> Value::asStringList() const {
            const ArrayType& array = arrayValue();
            std::vector<std::string> result;
            result.reserve(array.size());
            
            for (const auto& entry : array) {
                result.push_back(entry.convertTo(ValueType::String).stringValue());
            }
            
            return result;
        }
        
        const std::vector<std::string> Value::asStringSet() const {
            const ArrayType& array = arrayValue();
            kdl::vector_set<std::string> result(array.size());

            for (const auto& entry : array) {
                result.insert(entry.convertTo(ValueType::String).stringValue());
            }

            return result.release_data();
        }

        size_t Value::length() const {
            return std::visit(kdl::overload{
                [](const BooleanType&)   -> size_t { return 1u; },
                [](const StringType& s)  -> size_t { return s.length(); },
                [](const NumberType&)    -> size_t { return 1u; },
                [](const ArrayType& a)   -> size_t { return a.size(); },
                [](const MapType& m)     -> size_t { return m.size(); },
                [](const RangeType& r)   -> size_t { return r.size(); },
                [](const NullType&)      -> size_t { return 0u; },
                [](const UndefinedType&) -> size_t { return 0u; },
            }, m_value);
        }
        
        bool Value::convertibleTo(const ValueType toType) const {
            return std::visit(kdl::overload{
                [&](const BooleanType&) {
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
                },
                [&](const StringType& s) {
                    switch (toType) {
                        case ValueType::Boolean:
                        case ValueType::String:
                            return true;
                        case ValueType::Number: {
                            if (kdl::str_is_blank(s)) {
                                return true;
                            }
                            const char* begin = s.c_str();
                            char* end;
                            const NumberType value = std::strtod(begin, &end);
                            if (value == 0.0 && end == begin) {
                                return false;
                            }
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
                },
                [&](const NumberType&) {
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
                },
                [&](const ArrayType&) {
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
                },
                [&](const MapType&) {
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
                },
                [&](const RangeType&) {
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
                },
                [&](const NullType&) {
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
                },
                [&](const UndefinedType&) {
                    switch (toType) {
                        case ValueType::Undefined:
                            return true;
                        case ValueType::Boolean:
                        case ValueType::Number:
                        case ValueType::String:
                        case ValueType::Array:
                        case ValueType::Map:
                        case ValueType::Range:
                        case ValueType::Null:
                            break;
                    }

                    return false;
                },
            }, m_value);
        }
        
        Value Value::convertTo(const ValueType toType) const {
            return std::visit(kdl::overload{
                [&](const BooleanType& b) -> Value {
                    switch (toType) {
                        case ValueType::Boolean:
                            return *this;
                        case ValueType::String:
                            return Value(b ? "true" : "false", m_line, m_column);
                        case ValueType::Number:
                            return Value(b ? 1.0 : 0.0, m_line, m_column);
                        case ValueType::Array:
                        case ValueType::Map:
                        case ValueType::Range:
                        case ValueType::Undefined:
                        case ValueType::Null:
                            break;
                    }

                    throw ConversionError(describe(), type(), toType);
                },
                [&](const StringType& s) -> Value {
                    switch (toType) {
                        case ValueType::Boolean:
                            return Value(!kdl::cs::str_is_equal(s, "false") && !s.empty(), m_line, m_column);
                        case ValueType::String:
                            return *this;
                        case ValueType::Number: {
                            if (kdl::str_is_blank(s)) {
                                return Value(0.0, m_line, m_column);
                            }
                            const char* begin = s.c_str();
                            char* end;
                            const NumberType value = std::strtod(begin, &end);
                            if (value == 0.0 && end == begin) {
                                throw ConversionError(describe(), type(), toType);
                            }
                            return Value(value, m_line, m_column);
                        }
                        case ValueType::Array:
                        case ValueType::Map:
                        case ValueType::Range:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }

                    throw ConversionError(describe(), type(), toType);
                },
                [&](const NumberType& n) -> Value {
                    switch (toType) {
                        case ValueType::Boolean:
                            return Value(n != 0.0, m_line, m_column);
                        case ValueType::String:
                            return Value(describe(), m_line, m_column);
                        case ValueType::Number:
                            return *this;
                        case ValueType::Array:
                        case ValueType::Map:
                        case ValueType::Range:
                        case ValueType::Null:
                        case ValueType::Undefined:
                            break;
                    }

                    throw ConversionError(describe(), type(), toType);
                },
                [&](const ArrayType&) -> Value {
                    switch (toType) {
                        case ValueType::Array:
                            return *this;
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
                },
                [&](const MapType&) -> Value {
                    switch (toType) {
                        case ValueType::Map:
                            return *this;
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
                },
                [&](const RangeType&) -> Value {
                    switch (toType) {
                        case ValueType::Range:
                            return *this;
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
                },
                [&](const NullType&) -> Value {
                    switch (toType) {
                        case ValueType::Boolean:
                            return Value(false, m_line, m_column);
                        case ValueType::Null:
                            return *this;
                        case ValueType::Number:
                            return Value(0.0, m_line, m_column);
                        case ValueType::String:
                            return Value("", m_line, m_column);
                        case ValueType::Array:
                            return Value(ArrayType(0), m_line, m_column);
                        case ValueType::Map:
                            return Value(MapType(), m_line, m_column);
                        case ValueType::Range:
                        case ValueType::Undefined:
                            break;
                    }

                    throw ConversionError(describe(), type(), toType);
                },
                [&](const UndefinedType&) -> Value {
                    switch (toType) {
                        case ValueType::Undefined:
                            return *this;
                        case ValueType::Boolean:
                        case ValueType::Number:
                        case ValueType::String:
                        case ValueType::Array:
                        case ValueType::Map:
                        case ValueType::Range:
                        case ValueType::Null:
                            break;
                    }

                    throw ConversionError(describe(), type(), toType);
                },
            }, m_value);
        }

        std::string Value::asString(const bool multiline) const {
            std::stringstream str;
            appendToStream(str, multiline);
            return str.str();
        }
        
        void Value::appendToStream(std::ostream& str, const bool multiline, const std::string& indent) const {
            std::visit(kdl::overload{
                [&](const BooleanType& b) {
                    str << (b ? "true" : "false");
                },
                [&](const StringType& s) {
                    // Unescaping happens in IO::ELParser::parseLiteral
                    str << "\"" << kdl::str_escape(s, "\\\"") << "\"";
                },
                [&](const NumberType& n) {
                    static constexpr auto RoundingThreshold = 0.00001;
                    if (std::abs(n - std::round(n)) < RoundingThreshold) {
                        str.precision(0);
                        str.setf(std::ios::fixed);
                    } else {
                        str.precision(17);
                        str.unsetf(std::ios::fixed);
                    }
                    str << n;
                },
                [&](const ArrayType& a) {
                    if (a.empty()) {
                        str << "[]";
                    } else {
                        const std::string childIndent = multiline ? indent + "\t" : "";
                        str << "[";
                        if (multiline) {
                            str << "\n";
                        } else {
                            str << " ";
                        }
                        for (size_t i = 0; i < a.size(); ++i) {
                            str << childIndent;
                            a[i].appendToStream(str, multiline, childIndent);
                            if (i < a.size() - 1) {
                                str << ",";
                                if (!multiline) {
                                    str << " ";
                                }
                            }
                            if (multiline) {
                                str << "\n";
                            }
                        }
                        if (multiline) {
                            str << indent;
                        } else {
                            str << " ";
                        }
                        str << "]";
                    }
                },
                [&](const MapType& m) {
                    if (m.empty()) {
                        str << "{}";
                    } else {
                        const std::string childIndent = multiline ? indent + "\t" : "";
                        str << "{";
                        if (multiline) {
                            str << "\n";
                        } else {
                            str << " ";
                        }

                        size_t i = 0;
                        for (const auto& entry : m) {
                            str << childIndent << "\"" << entry.first << "\"" << ": ";
                            entry.second.appendToStream(str, multiline, childIndent);
                            if (i++ < m.size() - 1) {
                                str << ",";
                                if (!multiline) {
                                    str << " ";
                                }
                            }
                            if (multiline) {
                                str << "\n";
                            }
                        }
                        if (multiline) {
                            str << indent;
                        } else {
                            str << " ";
                        }
                        str << "}";
                    }
                },
                [&](const RangeType& r) {
                    str << "[";
                    for (size_t i = 0; i < r.size(); ++i) {
                        str << r[i];
                        if (i < r.size() - 1) {
                            str << ", ";
                        }
                    }
                    str << "]";
                },
                [&](const NullType&) {
                    str << "null";
                },
                [&](const UndefinedType&) {
                    str << "undefined";
                },
            }, m_value);
        }

        static  size_t computeIndex(const long index, const size_t indexableSize) {
            const long size  = static_cast<long>(indexableSize);
            if ((index >= 0 && index <   size) ||
                (index <  0 && index >= -size )) {
                return static_cast<size_t>((size + index % size) % size);
            } else {
                return static_cast<size_t>(size);
            }
        }

        static size_t computeIndex(const Value& indexValue, const size_t indexableSize) {
            return computeIndex(static_cast<long>(indexValue.convertTo(ValueType::Number).numberValue()), indexableSize);
        }

        static void computeIndexArray(const Value& indexValue, const size_t indexableSize, std::vector<size_t>& result) {
            switch (indexValue.type()) {
                case ValueType::Array: {
                    const ArrayType& indexArray = indexValue.arrayValue();
                    result.reserve(result.size() + indexArray.size());
                    for (size_t i = 0; i < indexArray.size(); ++i) {
                        computeIndexArray(indexArray[i], indexableSize, result);
                    }
                    break;
                }
                case ValueType::Range: {
                    const RangeType& range = indexValue.rangeValue();
                    result.reserve(result.size() + range.size());
                    for (size_t i = 0; i < range.size(); ++i) {
                        result.push_back(computeIndex(range[i], indexableSize));
                    }
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

        static std::vector<size_t> computeIndexArray(const Value& indexValue, const size_t indexableSize) {
            std::vector<size_t> result;
            computeIndexArray(indexValue, indexableSize, result);
            return result;
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
                            const std::vector<size_t> indices = computeIndexArray(indexValue, length());
                            for (size_t i = 0; i < indices.size(); ++i) {
                                const size_t index = indices[i];
                                if (index >= length()) {
                                    return false;
                                }
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
                            const std::vector<size_t> indices = computeIndexArray(indexValue, length());
                            for (size_t i = 0; i < indices.size(); ++i) {
                                const size_t index = indices[i];
                                if (index >= length()) {
                                    return false;
                                }
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
                            const std::string& key = indexValue.stringValue();
                            const auto it = map.find(key);
                            return it != std::end(map);
                        }
                        case ValueType::Array: {
                            const MapType& map = mapValue();
                            const ArrayType& keys = indexValue.arrayValue();
                            for (size_t i = 0; i < keys.size(); ++i) {
                                const Value& keyValue = keys[i];
                                if (keyValue.type() != ValueType::String) {
                                    throw ConversionError(keyValue.describe(), keyValue.type(), ValueType::String);
                                }
                                const std::string& key = keyValue.stringValue();
                                const auto it = map.find(key);
                                if (it == std::end(map)) {
                                    return false;
                                }
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
        
        bool Value::contains(const std::string& key) const {
            const MapType& map = mapValue();
            const auto it = map.find(key);
            return it != std::end(map);
        }
        
        std::vector<std::string> Value::keys() const {
            return kdl::map_keys(mapValue());
        }

        Value Value::operator[](const Value& indexValue) const {
            switch (type()) {
                case ValueType::String:
                    switch (indexValue.type()) {
                        case ValueType::Boolean:
                        case ValueType::Number: {
                            const StringType& str = stringValue();
                            const size_t index = computeIndex(indexValue, str.length());
                            std::stringstream result;
                            if (index < str.length()) {
                                result << str[index];
                            }
                            return Value(result.str(), m_line, m_column);
                        }
                        case ValueType::Array:
                        case ValueType::Range: {
                            const StringType& str = stringValue();
                            const std::vector<size_t> indices = computeIndexArray(indexValue, str.length());
                            std::stringstream result;
                            for (size_t i = 0; i < indices.size(); ++i) {
                                const size_t index = indices[i];
                                if (index < str.length()) {
                                    result << str[index];
                                }
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
                            if (index >= array.size()) {
                                throw IndexOutOfBoundsError(*this, indexValue, index);
                            }
                            return array[index];
                        }
                        case ValueType::Array:
                        case ValueType::Range: {
                            const ArrayType& array = arrayValue();
                            const std::vector<size_t> indices = computeIndexArray(indexValue, array.size());
                            ArrayType result;
                            result.reserve(indices.size());
                            for (size_t i = 0; i < indices.size(); ++i) {
                                const size_t index = indices[i];
                                if (index >= array.size()) {
                                    throw IndexOutOfBoundsError(*this, indexValue, index);
                                }
                                result.push_back(array[index]);
                            }
                            return Value(std::move(result), m_line, m_column);
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
                            const std::string& key = indexValue.stringValue();
                            const MapType::const_iterator it = map.find(key);
                            if (it == std::end(map)) {
                                return Value(UndefinedType::Value);
                            }
                            return it->second;
                        }
                        case ValueType::Array: {
                            const MapType& map = mapValue();
                            const ArrayType& keys = indexValue.arrayValue();
                            MapType result;
                            for (size_t i = 0; i < keys.size(); ++i) {
                                const Value& keyValue = keys[i];
                                if (keyValue.type() != ValueType::String) {
                                    throw ConversionError(keyValue.describe(), keyValue.type(), ValueType::String);
                                }
                                const std::string& key = keyValue.stringValue();
                                const MapType::const_iterator it = map.find(key);
                                if (it != std::end(map)) {
                                    result.insert(std::make_pair(key, it->second));
                                }
                            }
                            return Value(std::move(result), m_line, m_column);
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
                    std::stringstream result;
                    if (index < str.length()) {
                        result << str[index];
                    }
                    return Value(result.str());
                }
                case ValueType::Array: {
                    const ArrayType& array = arrayValue();
                    if (index >= array.size()) {
                        throw IndexOutOfBoundsError(*this, index);
                    }
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
        
        Value Value::operator[](const std::string& key) const {
            return this->operator[](key.c_str());
        }
        
        Value Value::operator[](const char* key) const {
            switch (type()) {
                case ValueType::Map: {
                    const MapType& map = mapValue();
                    const auto it = map.find(key);
                    if (it == std::end(map)) {
                        return Value(NullType::Value);
                    } else {
                        return it->second;
                    }
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
    
        std::ostream& operator<<(std::ostream& stream, const Value& value) {
            value.appendToStream(stream);
            return stream;
        }
    
        Value operator+(const Value& v) {
            switch (v.type()) {
                case ValueType::Boolean:
                case ValueType::Number:
                    return Value(v.convertTo(ValueType::Number).numberValue());
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }
            throw EvaluationError("Cannot apply unary plus to value '" + v.describe() + "' of type '" + v.typeName());
        }

        Value operator-(const Value& v) {
            switch (v.type()) {
                case ValueType::Boolean:
                case ValueType::Number:
                    return Value(-v.convertTo(ValueType::Number).numberValue());
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }
            throw EvaluationError("Cannot negate value '" + v.describe() + "' of type '" + v.typeName());
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
                            return Value(kdl::vec_concat(lhs.arrayValue(), rhs.arrayValue()));
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
                            return Value(kdl::map_union(lhs.mapValue(), rhs.mapValue()));
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
    
        Value operator!(const Value& v) {
            switch (v.type()) {
                case ValueType::Boolean:
                    return Value(!v.booleanValue());
                case ValueType::Number:
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }
            throw ConversionError(v.describe(), v.type(), ValueType::Boolean);
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
                    if (rhs.type() == ValueType::Null) {
                        return 0;
                    } else {
                        return -1;
                    }
                case ValueType::Undefined:
                    if (rhs.type() == ValueType::Undefined) {
                        return 0;
                    } else {
                        return -1;
                    }
                case ValueType::Array:
                    switch (rhs.type()) {
                        case ValueType::Array:
                            return kdl::col_lexicographical_compare(lhs.arrayValue(), rhs.arrayValue());
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
                            return kdl::map_lexicographical_compare(lhs.mapValue(), rhs.mapValue());
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
                            return kdl::col_lexicographical_compare(lhs.rangeValue(), rhs.rangeValue());
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
            if (lhsValue == rhsValue) {
                return 0;
            } else if (lhsValue) {
                return 1;
            } else {
                return -1;
            }
        }

        int compareAsNumbers(const Value& lhs, const Value& rhs) {
            const NumberType diff = lhs.convertTo(ValueType::Number).numberValue() - rhs.convertTo(ValueType::Number).numberValue();
            if (diff < 0.0) {
                return -1;
            } else if (diff > 0.0) {
                return 1;
            } else {
                return 0;
            }
        }


        Value operator~(const Value& v) {
            switch (v.type()) {
                case ValueType::Number:
                    return Value(~v.integerValue());
                case ValueType::Boolean:
                case ValueType::String:
                case ValueType::Array:
                case ValueType::Map:
                case ValueType::Range:
                case ValueType::Null:
                case ValueType::Undefined:
                    break;
            }
            throw ConversionError(v.describe(), v.type(), ValueType::Boolean);
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
