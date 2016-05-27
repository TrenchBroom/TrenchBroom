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

        const Value& EvaluationContext::variableValue(const String& name) const {
            VariableTable::const_iterator it = m_variables.find(name);
            if (it == m_variables.end())
                throw EvaluationError("Unknown variable '" + name + "'");
            return it->second;
        }

        void EvaluationContext::defineVariable(const String& name, const Value& value) {
            m_variables.insert(std::make_pair(name, value));
        }

        ValueHolder::~ValueHolder() {}
        
        const BooleanType& ValueHolder::boolValue()   const { throw ValueError(description(), type(), Type_Boolean); }
        const StringType&  ValueHolder::stringValue() const { throw ValueError(description(), type(), Type_String); }
        const NumberType&  ValueHolder::numberValue() const { throw ValueError(description(), type(), Type_Number); }
        const ArrayType&   ValueHolder::arrayValue()  const { throw ValueError(description(), type(), Type_Array); }
        const MapType&     ValueHolder::mapValue()    const { throw ValueError(description(), type(), Type_Map); }
        
        String ValueHolder::asString() const {
            StringStream str;
            str.precision(17);
            appendToStream(str);
            return str.str();
        }
        
        BooleanValueHolder::BooleanValueHolder(const BooleanType& value) : m_value(value) {}
        ValueType BooleanValueHolder::type() const { return Type_Boolean; }
        String BooleanValueHolder::description() const { return asString(); }
        const BooleanType& BooleanValueHolder::boolValue() const { return m_value; }
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
                case Type_Null:
                    throw ConversionError(description(), type(), toType);
            }
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
                case Type_Null:
                    throw ConversionError(description(), type(), toType);
            }
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
                case Type_Null:
                    throw ConversionError(description(), type(), toType);
            }
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
                case Type_Boolean:
                case Type_String:
                case Type_Number:
                case Type_Map:
                case Type_Null:
                    throw ConversionError(description(), type(), toType);
                case Type_Array:
                    return new ArrayValueHolder(m_value);
            }
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
                case Type_Boolean:
                case Type_String:
                case Type_Number:
                case Type_Array:
                case Type_Null:
                    throw ConversionError(description(), type(), toType);
                case Type_Map:
                    return new MapValueHolder(m_value);
            }
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
        
        
        ValueType NullValueHolder::type() const { return Type_Null; }
        String NullValueHolder::description() const { return "null"; }
        size_t NullValueHolder::length() const { return 0; }
        ValueHolder* NullValueHolder::convertTo(const ValueType toType) const { throw ConversionError(description(), type(), toType); }
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
        
        const NumberType& Value::numberValue() const {
            return m_value->numberValue();
        }
        
        const ArrayType& Value::arrayValue() const {
            return m_value->arrayValue();
        }
        
        const MapType& Value::mapValue() const {
            return m_value->mapValue();
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
                        case Type_Array: {
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
                        case Type_Array: {
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
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_Boolean:
                case Type_Number:
                case Type_Null:
                    break;
            }
            
            throw IndexError(*this, indexValue);
        }
        
        Value::IndexList Value::computeIndexArray(const Value& indexValue, const size_t indexableSize) const {
            assert(indexValue.type() == Type_Array);
            IndexList result;
            const ArrayType& indexArray = indexValue.arrayValue();
            result.reserve(indexArray.size());
            for (size_t i = 0; i < indexArray.size(); ++i)
                result.push_back(computeIndex(indexArray[i], indexableSize));
            return result;
        }

        size_t Value::computeIndex(const Value& indexValue, const size_t indexableSize) const {
            const long value = static_cast<long>(indexValue.convertTo(Type_Number).numberValue());
            const long size  = static_cast<long>(indexableSize);
            if ((value >= 0 && value <   size) ||
                (value <  0 && value >= -size ))
                return static_cast<size_t>((size + value % size) % size);
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
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_Array:
                case Type_Map:
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
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_String:
                case Type_Array:
                case Type_Map:
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
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_String:
                case Type_Array:
                case Type_Map:
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
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_String:
                case Type_Array:
                case Type_Map:
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
                        case Type_Null:
                            break;
                    }
                    break;
                case Type_String:
                case Type_Array:
                case Type_Map:
                case Type_Null:
                    break;
            }
            
            throw EvaluationError("Cannot subtract value '" + rhs.description() + "' of type '" + typeName(rhs.type()) + " from value '" + lhs.description() + "' of type '" + typeName(lhs.type()) + "'");
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
            }
            throw EvaluationError("Cannot compare value '" + lhs.description() + "' of type '" + typeName(lhs.type()) + " to value '" + rhs.description() + "' of type '" + typeName(rhs.type()) + "'");
        }

        Expression::InternalEvaluationContext::InternalEvaluationContext(const EvaluationContext& context) :
        m_context(context) {}
        
        const Value& Expression::InternalEvaluationContext::variableValue(const String& name) const {
            VariableTable::const_iterator it = m_variables.find(name);
            if (it == m_variables.end())
                return m_context.variableValue(name);
            assert(!it->second.empty());
            return it->second.back();
        }
        
        void Expression::InternalEvaluationContext::pushVariable(const String& name, const Value& value) {
            VariableTable::iterator it = m_variables.find(name);
            if (it == m_variables.end())
                m_variables.insert(std::make_pair(name, ValueStack(1, value)));
            else
                it->second.push_back(value);
        }
        
        void Expression::InternalEvaluationContext::popVariable(const String& name) {
            VariableTable::iterator it = m_variables.find(name);
            if (it == m_variables.end())
                throw EvaluationError("Unknown variable '" + name + "'");
            assert(!it->second.empty());
            it->second.pop_back();
            if (it->second.empty())
                m_variables.erase(it);
        }

        Expression::Expression() {}
        Expression::~Expression() {}
        
        bool Expression::range() const {
            return doIsRange();
        }

        Expression* Expression::reorderByPrecedence() {
            return doReorderByPrecedence();
        }
        
        Expression* Expression::reorderByPrecedence(BinaryOperator* parent) {
            return doReorderByPrecedence(parent);
        }

        Expression* Expression::clone() const {
            return doClone();
        }

        Value Expression::evaluate(const EvaluationContext& context) const {
            InternalEvaluationContext internalContext(context);
            return evaluate(internalContext);
        }

        Value Expression::evaluate(InternalEvaluationContext& context) const {
            return doEvaluate(context);
        }

        bool Expression::doIsRange() const {
            return false;
        }

        Expression* Expression::doReorderByPrecedence() {
            return this;
        }
        
        Expression* Expression::doReorderByPrecedence(BinaryOperator* parent) {
            return parent;
        }

        LiteralExpression::LiteralExpression(const Value& value) :
        m_value(value) {}
        
        Expression* LiteralExpression::create(const Value& value) {
            return new LiteralExpression(value);
        }

        Expression* LiteralExpression::doClone() const {
            return new LiteralExpression(m_value);
        }

        Value LiteralExpression::doEvaluate(InternalEvaluationContext& context) const {
            return m_value;
        }

        VariableExpression::VariableExpression(const String& variableName) :
        m_variableName(variableName) {}

        Expression* VariableExpression::create(const String& variableName) {
            return new VariableExpression(variableName);
        }
        
        Expression* VariableExpression::doClone() const {
            return new VariableExpression(m_variableName);
        }

        Value VariableExpression::doEvaluate(InternalEvaluationContext& context) const {
            return context.variableValue(m_variableName);
        }

        ArrayLiteralExpression::ArrayLiteralExpression(const Expression::List& elements) :
        m_elements(elements) {}
        
        Expression* ArrayLiteralExpression::create(const Expression::List& elements) {
            return new ArrayLiteralExpression(elements);
        }
        
        ArrayLiteralExpression::~ArrayLiteralExpression() {
            ListUtils::clearAndDelete(m_elements);
        }

        Expression* ArrayLiteralExpression::doClone() const {
            Expression::List clones;
            Expression::List::const_iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                const Expression* element = *it;
                clones.push_back(element->clone());
            }
            
            return new ArrayLiteralExpression(clones);
        }
        
        Value ArrayLiteralExpression::doEvaluate(InternalEvaluationContext& context) const {
            ArrayType array;
            Expression::List::const_iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                const Expression* element = *it;
                if (element->range())
                    VectorUtils::append(array, element->evaluate(context).arrayValue());
                else
                    array.push_back(element->evaluate(context));
            }
            
            return Value(array);
        }

        MapLiteralExpression::MapLiteralExpression(const Expression::Map& elements) :
        m_elements(elements) {}
        
        Expression* MapLiteralExpression::create(const Expression::Map& elements) {
            return new MapLiteralExpression(elements);
        }
        
        MapLiteralExpression::~MapLiteralExpression() {
            MapUtils::clearAndDelete(m_elements);
        }

        Expression* MapLiteralExpression::doClone() const {
            Expression::Map clones;
            Expression::Map::const_iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                const String& key = it->first;
                const Expression* value = it->second;
                clones.insert(std::make_pair(key, value->clone()));
            }
            
            return new MapLiteralExpression(clones);
        }

        Value MapLiteralExpression::doEvaluate(InternalEvaluationContext& context) const {
            MapType map;
            Expression::Map::const_iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                const String& key = it->first;
                const Expression* value = it->second;
                map.insert(std::make_pair(key, value->evaluate(context)));
            }
            
            return Value(map);
        }

        UnaryOperator::UnaryOperator(Expression* operand) :
        m_operand(operand) {
            assert(m_operand != NULL);
        }

        UnaryOperator::~UnaryOperator() {
            delete m_operand;
        }

        UnaryPlusOperator::UnaryPlusOperator(Expression* operand) :
        UnaryOperator(operand) {}

        Expression* UnaryPlusOperator::create(Expression* operand) {
            return new UnaryPlusOperator(operand);
        }

        Expression* UnaryPlusOperator::doClone() const {
            return new UnaryPlusOperator(m_operand->clone());
        }

        Value UnaryPlusOperator::doEvaluate(InternalEvaluationContext& context) const {
            return +m_operand->evaluate(context);
        }

        UnaryMinusOperator::UnaryMinusOperator(Expression* operand) :
        UnaryOperator(operand) {}
        
        Expression* UnaryMinusOperator::create(Expression* operand) {
            return new UnaryMinusOperator(operand);
        }

        Expression* UnaryMinusOperator::doClone() const {
            return new UnaryMinusOperator(m_operand->clone());
        }

        Value UnaryMinusOperator::doEvaluate(InternalEvaluationContext& context) const {
            return -m_operand->evaluate(context);
        }
        
        GroupingOperator::GroupingOperator(Expression* operand) :
        UnaryOperator(operand) {}

        Expression* GroupingOperator::create(Expression* operand) {
            return new GroupingOperator(operand);
        }

        Expression* GroupingOperator::doClone() const {
            return new GroupingOperator(m_operand->clone());
        }
        
        Value GroupingOperator::doEvaluate(InternalEvaluationContext& context) const {
            return m_operand->evaluate(context);
        }

        SubscriptOperator::SubscriptOperator(Expression* indexableOperand, Expression* indexOperand) :
        m_indexableOperand(indexableOperand),
        m_indexOperand(indexOperand) {
            assert(m_indexableOperand != NULL);
            assert(m_indexOperand != NULL);
        }
        
        SubscriptOperator::~SubscriptOperator() {
            delete m_indexableOperand;
            delete m_indexOperand;
        }

        Expression* SubscriptOperator::create(Expression* indexableOperand, Expression* indexOperand) {
            return (new SubscriptOperator(indexableOperand, indexOperand))->reorderByPrecedence();
        }
        
        Expression* SubscriptOperator::doClone() const {
            return new SubscriptOperator(m_indexableOperand->clone(), m_indexOperand->clone());
        }
        
        Value SubscriptOperator::doEvaluate(InternalEvaluationContext& context) const {
            const Value indexableValue = m_indexableOperand->evaluate(context);
            context.pushVariable(RangeOperator::AutoRangeParameterName(), Value(indexableValue.length()-1));
            const Value indexValue = m_indexOperand->evaluate(context);
            context.popVariable(RangeOperator::AutoRangeParameterName());
            return indexableValue[indexValue];
        }

        BinaryOperator::BinaryOperator(Expression* leftOperand, Expression* rightOperand) :
        m_leftOperand(leftOperand),
        m_rightOperand(rightOperand) {
            assert(m_leftOperand != NULL);
            assert(m_rightOperand != NULL);
        }

        BinaryOperator::~BinaryOperator() {
            delete m_leftOperand;
            delete m_rightOperand;
        }

        Expression* BinaryOperator::doReorderByPrecedence() {
            Expression* result = m_leftOperand->reorderByPrecedence(this);
            if (result == this)
                result = m_rightOperand->reorderByPrecedence(this);
            return result;
        }
        
        Expression* BinaryOperator::doReorderByPrecedence(BinaryOperator* parent) {
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
        
        struct BinaryOperator::Traits {
            size_t precedence;
            bool associative;
            bool commutative;
            
            Traits(const size_t i_precedence, const bool i_associative, const bool i_commutative) :
            precedence(i_precedence),
            associative(i_associative),
            commutative(i_commutative) {}
        };
        
        const BinaryOperator::Traits& BinaryOperator::traits() const {
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

        AdditionOperator::AdditionOperator(Expression* leftOperand, Expression* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}

        Expression* AdditionOperator::create(Expression* leftOperand, Expression* rightOperand) {
            return (new AdditionOperator(leftOperand, rightOperand))->reorderByPrecedence();
        }

        Expression* AdditionOperator::doClone() const {
            return new AdditionOperator(m_leftOperand->clone(), m_rightOperand->clone());
        }
        
        Value AdditionOperator::doEvaluate(InternalEvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return leftValue + rightValue;
        }

        const BinaryOperator::Traits& AdditionOperator::doGetTraits() const {
            static const Traits traits(0, true, true);
            return traits;
        }
        
        SubtractionOperator::SubtractionOperator(Expression* leftOperand, Expression* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}

        Expression* SubtractionOperator::create(Expression* leftOperand, Expression* rightOperand) {
            return (new SubtractionOperator(leftOperand, rightOperand))->reorderByPrecedence();
        }

        Expression* SubtractionOperator::doClone() const {
            return new SubtractionOperator(m_leftOperand->clone(), m_rightOperand->clone());
        }
        
        Value SubtractionOperator::doEvaluate(InternalEvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return leftValue - rightValue;
        }
        
        const BinaryOperator::Traits& SubtractionOperator::doGetTraits() const {
            static const Traits traits(0, false, false);
            return traits;
        }
        
        MultiplicationOperator::MultiplicationOperator(Expression* leftOperand, Expression* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}
        
        Expression* MultiplicationOperator::create(Expression* leftOperand, Expression* rightOperand) {
            return (new MultiplicationOperator(leftOperand, rightOperand))->reorderByPrecedence();
        }

        Expression* MultiplicationOperator::doClone() const {
            return new MultiplicationOperator(m_leftOperand->clone(), m_rightOperand->clone());
        }
        
        Value MultiplicationOperator::doEvaluate(InternalEvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return leftValue * rightValue;
        }
        
        const BinaryOperator::Traits& MultiplicationOperator::doGetTraits() const {
            static const Traits traits(1, true, true);
            return traits;
        }

        DivisionOperator::DivisionOperator(Expression* leftOperand, Expression* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}
        
        Expression* DivisionOperator::create(Expression* leftOperand, Expression* rightOperand) {
            return (new DivisionOperator(leftOperand, rightOperand))->reorderByPrecedence();
        }

        Expression* DivisionOperator::doClone() const {
            return new DivisionOperator(m_leftOperand->clone(), m_rightOperand->clone());
        }
        
        Value DivisionOperator::doEvaluate(InternalEvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return leftValue / rightValue;
        }
        
        const BinaryOperator::Traits& DivisionOperator::doGetTraits() const {
            static const Traits traits(1, false, false);
            return traits;
        }

        ModulusOperator::ModulusOperator(Expression* leftOperand, Expression* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}
        
        Expression* ModulusOperator::create(Expression* leftOperand, Expression* rightOperand) {
            return (new ModulusOperator(leftOperand, rightOperand))->reorderByPrecedence();
        }

        Expression* ModulusOperator::doClone() const {
            return new ModulusOperator(m_leftOperand->clone(), m_rightOperand->clone());
        }
        
        Value ModulusOperator::doEvaluate(InternalEvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return leftValue % rightValue;
        }
        
        const BinaryOperator::Traits& ModulusOperator::doGetTraits() const {
            static const Traits traits(1, false, false);
            return traits;
        }
        
        const String& RangeOperator::AutoRangeParameterName() {
            static const String Name = "__AutoRangeParameter";
            return Name;
        }

        RangeOperator::RangeOperator(Expression* leftOperand, Expression* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}
        
        Expression* RangeOperator::create(Expression* leftOperand, Expression* rightOperand) {
            return (new RangeOperator(leftOperand, rightOperand))->reorderByPrecedence();
        }
        
        Expression* RangeOperator::createAutoRangeWithLeftOperand(Expression* leftOperand) {
            return create(leftOperand, VariableExpression::create(AutoRangeParameterName()));
        }
        
        Expression* RangeOperator::createAutoRangeWithRightOperand(Expression* rightOperand) {
            return create(VariableExpression::create(AutoRangeParameterName()), rightOperand);
        }

        bool RangeOperator::doIsRange() const {
            return true;
        }

        Expression* RangeOperator::doClone() const {
            return new RangeOperator(m_leftOperand->clone(), m_rightOperand->clone());
        }
        
        Value RangeOperator::doEvaluate(InternalEvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);

            const long from = static_cast<long>(leftValue.convertTo(Type_Number).numberValue());
            const long to = static_cast<long>(rightValue.convertTo(Type_Number).numberValue());
            
            ArrayType array;
            if (from <= to) {
                array.reserve(static_cast<size_t>(to - from + 1));
                for (long i = from; i <= to; ++i) {
                    assert(array.capacity() > array.size());
                    array.push_back(Value(static_cast<double>(i)));
                }
            } else if (to < from) {
                array.reserve(static_cast<size_t>(from - to + 1));
                for (long i = from; i >= to; --i) {
                    assert(array.capacity() > array.size());
                    array.push_back(Value(static_cast<double>(i)));
                }
            }
            assert(array.capacity() == array.size());

            return Value(array);
        }
        
        const BinaryOperator::Traits& RangeOperator::doGetTraits() const {
            static const Traits traits(0, false, false);
            return traits;
        }
    }
}
