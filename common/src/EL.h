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
#include <list>
#include <map>
#include <memory>
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
            ELException() throw();
            ELException(const String& str) throw();
            ~ELException() throw();
        };
        
        class ConversionError : public ELException {
        public:
            ConversionError(const String& value, const ValueType from, const ValueType to) throw();
        };
        
        class ValueError : public ELException {
        public:
            ValueError(const String& value, const ValueType from, const ValueType to) throw();
        };
        
        class EvaluationError : public ELException {
        public:
            EvaluationError(const String& msg) throw();
        };
        
        class IndexError : public EvaluationError {
        public:
            IndexError(const Value& indexableValue, const Value& indexValue) throw();
        };
        
        class IndexOutOfBoundsError : public IndexError {
        public:
            IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, size_t outOfBoundsIndex) throw();
            IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, const String& outOfBoundsIndex) throw();
        };
        
        class EvaluationContext {
        private:
            typedef std::map<String, Value> VariableTable;
            VariableTable m_variables;
        public:
            Value variableValue(const String& name) const;
            
            void defineVariable(const String& name, const Value& value);
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
            typedef std::vector<size_t> IndexList;
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
            String typeName() const;
            String description() const;
            
            const StringType& stringValue() const;
            const NumberType& numberValue() const;
            const ArrayType& arrayValue() const;
            const MapType& mapValue() const;

            Value convertTo(ValueType toType) const;

            void appendToStream(std::ostream& str) const;
            friend std::ostream& operator<<(std::ostream& stream, const Value& value);
            
            Value operator[](const Value& indexValue) const;
        private:
            IndexList computeIndexArray(const Value& indexValue, size_t indexableSize) const;
            size_t computeIndex(const Value& indexValue, size_t indexableSize) const;
        public:
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
        
        class BinaryOperator;
        
        class Expression {
        public:
            typedef std::auto_ptr<Expression> Ptr;
            typedef std::list<Expression*> List;
            typedef std::map<String, Expression*> Map;
        public:
            Expression();
            virtual ~Expression();

            Expression* reorderByPrecedence();
            Expression* reorderByPrecedence(BinaryOperator* parent);
            
            Expression* clone() const;
            Value evaluate(const EvaluationContext& context) const;
        private:
            virtual Expression* doReorderByPrecedence();
            virtual Expression* doReorderByPrecedence(BinaryOperator* parent);
            virtual Expression* doClone() const = 0;
            virtual Value doEvaluate(const EvaluationContext& context) const = 0;
            
            deleteCopyAndAssignment(Expression)
        };
        
        class LiteralExpression : public Expression {
        private:
            Value m_value;
        private:
            LiteralExpression(const Value& value);
        public:
            static Expression* create(const Value& value);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(LiteralExpression)
        };
        
        class VariableExpression : public Expression {
        private:
            String m_variableName;
        private:
            VariableExpression(const String& variableName);
        public:
            static Expression* create(const String& variableName);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(VariableExpression)
        };
        
        class ArrayLiteralExpression : public Expression {
        private:
            Expression::List m_elements;
        private:
            ArrayLiteralExpression(const Expression::List& elements);
        public:
            static Expression* create(const Expression::List& elements);
            ~ArrayLiteralExpression();
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(ArrayLiteralExpression)
        };
        
        class MapLiteralExpression : public Expression {
        private:
            Expression::Map m_elements;
        private:
            MapLiteralExpression(const Expression::Map& elements);
        public:
            static Expression* create(const Expression::Map& elements);
            ~MapLiteralExpression();
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(MapLiteralExpression)
        };
        
        class UnaryOperator : public Expression {
        protected:
            Expression* m_operand;
        protected:
            UnaryOperator(Expression* operand);
        public:
            virtual ~UnaryOperator();
        private:
            deleteCopyAndAssignment(UnaryOperator)
        };

        class UnaryPlusOperator : public UnaryOperator {
        private:
            UnaryPlusOperator(Expression* operand);
        public:
            static Expression* create(Expression* operand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(UnaryPlusOperator)
        };
        
        class UnaryMinusOperator : public UnaryOperator {
        private:
            UnaryMinusOperator(Expression* operand);
        public:
            static Expression* create(Expression* operand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(UnaryMinusOperator)
        };
        
        class GroupingOperator : public UnaryOperator {
        private:
            GroupingOperator(Expression* operand);
        public:
            static Expression* create(Expression* operand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(GroupingOperator)
        };
        
        class SubscriptOperator : public Expression {
        private:
            Expression* m_indexableOperand;
            Expression* m_indexOperand;
        private:
            SubscriptOperator(Expression* indexableOperand, Expression* indexOperand);
        public:
            ~SubscriptOperator();
        public:
            static Expression* create(Expression* indexableOperand, Expression* indexOperand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            
            deleteCopyAndAssignment(SubscriptOperator)
        };
        
        class BinaryOperator : public Expression {
        protected:
            Expression* m_leftOperand;
            Expression* m_rightOperand;
        protected:
            BinaryOperator(Expression* leftOperand, Expression* rightOperand);
        public:
            virtual ~BinaryOperator();
        private:
            Expression* doReorderByPrecedence();
            Expression* doReorderByPrecedence(BinaryOperator* parent);
            BinaryOperator* rotateLeftUp(BinaryOperator* leftOperand);
            BinaryOperator* rotateRightUp(BinaryOperator* rightOperand);
        protected:
            struct Traits;
        private:
            const Traits& traits() const;
            virtual const Traits& doGetTraits() const = 0;
        public:
            size_t precedence() const;
            bool associative() const;
            bool commutative() const;
        private:
            
            deleteCopyAndAssignment(BinaryOperator)
        };
        
        class AdditionOperator : public BinaryOperator {
        private:
            AdditionOperator(Expression* leftOperand, Expression* rightOperand);
        public:
            static Expression* create(Expression* leftOperand, Expression* rightOperand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            const Traits& doGetTraits() const;
            
            deleteCopyAndAssignment(AdditionOperator)
        };
        
        class SubtractionOperator : public BinaryOperator {
        private:
            SubtractionOperator(Expression* leftOperand, Expression* rightOperand);
        public:
            static Expression* create(Expression* leftOperand, Expression* rightOperand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            const Traits& doGetTraits() const;
            
            deleteCopyAndAssignment(SubtractionOperator)
        };
        
        class MultiplicationOperator : public BinaryOperator {
        private:
            MultiplicationOperator(Expression* leftOperand, Expression* rightOperand);
        public:
            static Expression* create(Expression* leftOperand, Expression* rightOperand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            const Traits& doGetTraits() const;
            
            deleteCopyAndAssignment(MultiplicationOperator)
        };
        
        class DivisionOperator : public BinaryOperator {
        private:
            DivisionOperator(Expression* leftOperand, Expression* rightOperand);
        public:
            static Expression* create(Expression* leftOperand, Expression* rightOperand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            const Traits& doGetTraits() const;
            
            deleteCopyAndAssignment(DivisionOperator)
        };
        
        class ModulusOperator : public BinaryOperator {
        private:
            ModulusOperator(Expression* leftOperand, Expression* rightOperand);
        public:
            static Expression* create(Expression* leftOperand, Expression* rightOperand);
        private:
            Expression* doClone() const;
            Value doEvaluate(const EvaluationContext& context) const;
            const Traits& doGetTraits() const;
            
            deleteCopyAndAssignment(ModulusOperator)
        };
    }
}

#endif /* EL_h */
