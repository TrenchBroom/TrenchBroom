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

#ifndef EL_h
#define EL_h

#include "Exceptions.h"
#include "Macros.h"
#include "StringUtils.h"
#include "SharedPointer.h"

#include <cstdlib>
#include <map>
#include <vector>

namespace TrenchBroom {
    namespace EL {
        class Value;
        
        typedef bool BooleanType;
        typedef String StringType;
        typedef double NumberType;
        typedef std::vector<Value> ArrayType;
        typedef std::map<String, Value> MapType;
        
        typedef enum {
            Type_Boolean,
            Type_String,
            Type_Number,
            Type_Array,
            Type_Map,
            Type_Null
        } ValueType;
        
        String typeName(ValueType type);
        
        class ELException : public ExceptionStream<ELException> {
        public:
            ELException() throw() {}
            ELException(const String& str) throw() : ExceptionStream(str) {}
            ~ELException() throw() {}
        };
        
        class ConversionError : public ELException {
        public:
            ConversionError(const String& value, const ValueType from, const ValueType to) throw() :
            ELException("Cannot convert value '" + value + "' of type '" + typeName(from) + "' to type '" + typeName(to) + "'") {}
        };
        
        class ValueError : public ELException {
        public:
            ValueError(const String& value, const ValueType from, const ValueType to) throw() :
            ELException("Cannot dereference value '" + value + "' of type '" + typeName(from) + "' as type '" + typeName(to) + "'") {}
        };
        
        class EvaluationError : public ELException {
        public:
            EvaluationError(const String& msg) throw() :
            ELException(msg) {}
        };
        
        class EvaluationContext {
        public:
            Value variableValue(const String& name) const;
        };
        
        class ValueHolder {
        public:
            virtual ~ValueHolder();
            
            virtual ValueType type() const = 0;
            virtual String description() const = 0;
            
            virtual const BooleanType& boolValue()   const;
            virtual const StringType&  stringValue() const;
            virtual const NumberType&  numberValue() const;
            virtual const ArrayType&   arrayValue()  const;
            virtual const MapType&     mapValue()    const;

            virtual ValueHolder* convertTo(ValueType toType) const = 0;
            
            virtual ValueHolder* clone() const = 0;
            
            virtual void appendToStream(std::ostream& str) const = 0;
        protected:
            String asString() const;
        };
        
        class BooleanValueHolder : public ValueHolder {
        private:
            BooleanType m_value;
        public:
            BooleanValueHolder(const BooleanType& value);
            ValueType type() const;
            String description() const;
            const BooleanType& boolValue() const;
            ValueHolder* convertTo(const ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str) const;
        };
        
        class StringValueHolder : public ValueHolder {
        private:
            StringType m_value;
        public:
            StringValueHolder(const StringType& value);
            ValueType type() const;
            String description() const;
            const StringType& stringValue() const;
            ValueHolder* convertTo(const ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str) const;
        };
        
        class NumberValueHolder : public ValueHolder {
        private:
            NumberType m_value;
        public:
            NumberValueHolder(const NumberType& value);
            ValueType type() const;
            String description() const;
            const NumberType& numberValue() const;
            ValueHolder* convertTo(const ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str) const;
        };
        
        class ArrayValueHolder : public ValueHolder {
        private:
            ArrayType m_value;
        public:
            ArrayValueHolder(const ArrayType& value);
            ValueType type() const;
            String description() const;
            const ArrayType& arrayValue() const;
            ValueHolder* convertTo(const ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str) const;
        };

        class MapValueHolder : public ValueHolder {
        private:
            MapType m_value;
        public:
            MapValueHolder(const MapType& value);
            ValueType type() const;
            String description() const;
            const MapType& mapValue() const;
            ValueHolder* convertTo(const ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str) const;
        };

        class NullValueHolder : public ValueHolder {
        public:
            ValueType type() const;
            String description() const;
            ValueHolder* convertTo(const ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str) const;
        };
        
        class Value {
        public:
            static const Value Null;
        private:
            typedef std::tr1::shared_ptr<ValueHolder> ValuePtr;
            ValuePtr m_value;
        private:
            Value(ValueHolder* holder);
        public:
            Value(const BooleanType& value);
            Value(const StringType& value);
            Value(const char* value);
            Value(const NumberType& value);
            Value(int value);
            Value(long value);
            Value(size_t value);
            Value(const ArrayType& value);
            Value(const MapType& value);
            Value();
            
            ValueType type() const;
            String description() const;
            
            const StringType& stringValue() const;
            const NumberType& numberValue() const;
            const ArrayType& arrayValue() const;
            const MapType& mapValue() const;

            Value convertTo(ValueType toType) const;

            void appendToStream(std::ostream& str) const;
            friend std::ostream& operator<<(std::ostream& stream, const Value& value);
            
            const Value& operator[](const Value& indexValue) const;
            
            Value operator+() const;
            Value operator-() const;
            
            friend Value operator+(const Value& lhs, const Value& rhs);
            friend Value operator-(const Value& lhs, const Value& rhs);
            friend Value operator*(const Value& lhs, const Value& rhs);
            friend Value operator/(const Value& lhs, const Value& rhs);
            friend Value operator%(const Value& lhs, const Value& rhs);
            
            friend bool operator==(const Value& lhs, const Value& rhs);
            friend bool operator!=(const Value& lhs, const Value& rhs);
            friend bool operator<(const Value& lhs, const Value& rhs);
            friend bool operator<=(const Value& lhs, const Value& rhs);
            friend bool operator>(const Value& lhs, const Value& rhs);
            friend bool operator>=(const Value& lhs, const Value& rhs);
        private:
            friend int compare(const Value& lhs, const Value& rhs);
        };
        
        class Expression {
        public:
            Expression();
            virtual ~Expression();
            
            Expression* clone() const;
            Value evaluate(const EvaluationContext& context) const;
        private:
            virtual Expression* doClone() const = 0;
            virtual Value doEvaluate(const EvaluationContext& context) const = 0;
            
            deleteCopyAndAssignment(Expression)
        };
        
        class LiteralExpression : public Expression {
        private:
            Value m_value;
        public:
            LiteralExpression(const Value& value);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(LiteralExpression)
        };
        
        class VariableExpression : public Expression {
        private:
            String m_variableName;
        public:
            VariableExpression(const String& variableName);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(VariableExpression)
        };
        
        class UnaryOperator : public Expression {
        protected:
            const Expression* m_operand;
        protected:
            UnaryOperator(const Expression* operand);
        public:
            virtual ~UnaryOperator();
            
            deleteCopyAndAssignment(UnaryOperator)
        };

        class UnaryPlusOperator : public UnaryOperator {
        public:
            UnaryPlusOperator(const Expression* operand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(UnaryPlusOperator)
        };
        
        class UnaryMinusOperator : public UnaryOperator {
        public:
            UnaryMinusOperator(const Expression* operand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(UnaryMinusOperator)
        };
        
        class BinaryOperator : public Expression {
        protected:
            const Expression* m_leftOperand;
            const Expression* m_rightOperand;
        protected:
            BinaryOperator(const Expression* leftOperand, const Expression* rightOperand);
        public:
            virtual ~BinaryOperator();
            
            deleteCopyAndAssignment(BinaryOperator)
        };
        
        class SubscriptOperator : public BinaryOperator {
        public:
            SubscriptOperator(const Expression* leftOperand, const Expression* rightOperand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(SubscriptOperator)
        };
        
        class AdditionOperator : public BinaryOperator {
        public:
            AdditionOperator(const Expression* leftOperand, const Expression* rightOperand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(AdditionOperator)
        };
        
        class SubtractionOperator : public BinaryOperator {
        public:
            SubtractionOperator(const Expression* leftOperand, const Expression* rightOperand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(SubtractionOperator)
        };
        
        class MultiplicationOperator : public BinaryOperator {
        public:
            MultiplicationOperator(const Expression* leftOperand, const Expression* rightOperand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(MultiplicationOperator)
        };
        
        class DivisionOperator : public BinaryOperator {
        public:
            DivisionOperator(const Expression* leftOperand, const Expression* rightOperand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(DivisionOperator)
        };
        
        class ModulusOperator : public BinaryOperator {
        public:
            ModulusOperator(const Expression* leftOperand, const Expression* rightOperand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(ModulusOperator)
        };
    }
}

#endif /* EL_h */
