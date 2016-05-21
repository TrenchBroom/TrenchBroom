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

#include <cassert>

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
                str << it->first << ":";
                it->second.appendToStream(str);
                if (i++ < m_value.size() - 1)
                    str << ",";
            }
            str << "}";
        }
        
        
        ValueType NullValueHolder::type() const { return Type_Null; }
        String NullValueHolder::description() const { return "null"; }
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

        Value::Value(const Value& other) :
        m_value(other.m_value->clone()) {}
        
        Value::~Value() {
            delete m_value;
        }
        
        Value& Value::operator=(Value other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }
        
        void swap(Value& lhs, Value& rhs) {
            using std::swap;
            swap(lhs.m_value, rhs.m_value);
        }

        ValueType Value::type() const {
            return m_value->type();
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

        Value Value::convertTo(const ValueType toType) const {
            return Value(m_value->convertTo(toType));
        }

        void Value::appendToStream(std::ostream& str) const {
            m_value->appendToStream(str);
        }
        
        std::ostream& operator<<(std::ostream& stream, const Value& value) {
            value.appendToStream(stream);
            return stream;
        }

        const Value& Value::operator[](const Value& indexValue) const {
            switch (type()) {
                case Type_Array:
                    if (indexValue.type() == Type_Number ||
                        indexValue.type() == Type_Boolean) {
                        const ArrayType& array = arrayValue();
                        const long value = static_cast<long>(indexValue.convertTo(Type_Number).numberValue());
                        const long size  = static_cast<long>(array.size());
                        if ((value >= 0 && value <   size) ||
                            (value <  0 && value >= -size )) {
                            const long index = (size + value % size) % size;
                            return array[static_cast<size_t>(index)];
                        }
                    }
                    break;
                case Type_Map:
                    if (indexValue.type() == Type_String) {
                        const MapType& map = mapValue();
                        const String key = indexValue.convertTo(Type_String).stringValue();
                        const MapType::const_iterator it = map.find(key);
                        if (it == map.end())
                            return Null;
                        return it->second;
                    }
                    break;
                case Type_Boolean:
                case Type_String:
                case Type_Number:
                case Type_Null:
                    break;
            }
            
            throw EvaluationError("Cannot index value '" + description() + "' of type '" + typeName(type()) + " with index '" + indexValue.description() + "' of type '" + typeName(indexValue.type()) + "'");
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
                    throw EvaluationError("Cannot apply unary plus to value '" + description() + "' of type '" + typeName(type()));
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
                    throw EvaluationError("Cannot negate value '" + description() + "' of type '" + typeName(type()));
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
                            return Value(lhs.convertTo(Type_String).stringValue() + rhs.stringValue());
                        case Type_Array:
                        case Type_Map:
                            break;
                        case Type_Null:
                            return Value::Null;
                    }
                    break;
                case Type_String:
                    switch (rhs.type()) {
                        case Type_Boolean:
                        case Type_Number:
                        case Type_String:
                            return Value(lhs.convertTo(Type_String).stringValue() + rhs.convertTo(Type_String).stringValue());
                        case Type_Array:
                        case Type_Map:
                            break;
                        case Type_Null:
                            return Value::Null;
                    }
                    break;
                case Type_Array:
                case Type_Map:
                    break;
                case Type_Null:
                    return Value::Null;
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
                            return Value(lhs.convertTo(Type_Number).numberValue() + rhs.convertTo(Type_Number).numberValue());
                        case Type_String:
                        case Type_Array:
                        case Type_Map:
                            break;
                        case Type_Null:
                            return Value::Null;
                    }
                    break;
                case Type_String:
                case Type_Array:
                case Type_Map:
                    break;
                case Type_Null:
                    return Value::Null;
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
                case Type_Map:
                    break;
            }
            throw EvaluationError("Cannot compare value '" + lhs.description() + "' of type '" + typeName(lhs.type()) + " to value '" + rhs.description() + "' of type '" + typeName(rhs.type()) + "'");
        }

        Expression::~Expression() {}
        
        Value Expression::evaluate(const EvaluationContext& context) const {
            return doEvaluate(context);
        }

        UnaryOperator::UnaryOperator(const Expression* operand) :
        m_operand(operand) {
            assert(m_operand != NULL);
        }

        UnaryOperator::~UnaryOperator() {
            delete m_operand;
        }

        UnaryPlusOperator::UnaryPlusOperator(const Expression* operand) :
        UnaryOperator(operand) {}

        Value UnaryPlusOperator::doEvaluate(const EvaluationContext& context) const {
            const Value operandValue = m_operand->evaluate(context);
            return operandValue.convertTo(Type_Number);
        }

        UnaryMinusOperator::UnaryMinusOperator(const Expression* operand) :
        UnaryOperator(operand) {}
        
        Value UnaryMinusOperator::doEvaluate(const EvaluationContext& context) const {
            const Value operandValue = m_operand->evaluate(context);
            return -operandValue.convertTo(Type_Number).numberValue();
        }
        
        BinaryOperator::BinaryOperator(const Expression* leftOperand, const Expression* rightOperand) :
        m_leftOperand(leftOperand),
        m_rightOperand(rightOperand) {
            assert(m_leftOperand != NULL);
            assert(m_rightOperand != NULL);
        }

        BinaryOperator::~BinaryOperator() {
            delete m_leftOperand;
            delete m_rightOperand;
        }

        SubscriptOperator::SubscriptOperator(const Expression* leftOperand, const Expression* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}

        Value SubscriptOperator::doEvaluate(const EvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return leftValue[rightValue];
        }

        AdditionOperator::AdditionOperator(const Expression* leftOperand, const Expression* rightOperand) :
        BinaryOperator(leftOperand, rightOperand) {}

        Value AdditionOperator::doEvaluate(const EvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return leftValue + rightValue;
        }
    }
}
