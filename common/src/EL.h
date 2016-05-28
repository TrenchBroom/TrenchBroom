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
        
        class ValueHolder {
        public:
            virtual ~ValueHolder();
            
            virtual ValueType type() const = 0;
            virtual String description() const = 0;
            
            virtual const BooleanType& booleanValue() const;
            virtual const StringType&  stringValue()  const;
            virtual const NumberType&  numberValue()  const;
            virtual const ArrayType&   arrayValue()   const;
            virtual const MapType&     mapValue()     const;

            virtual size_t length() const = 0;
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
            const BooleanType& booleanValue() const;
            size_t length() const;
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
            size_t length() const;
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
            size_t length() const;
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
            size_t length() const;
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
            size_t length() const;
            ValueHolder* convertTo(const ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str) const;
        };

        class NullValueHolder : public ValueHolder {
        public:
            ValueType type() const;
            String description() const;
            size_t length() const;
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
            const BooleanType& booleanValue() const;
            const NumberType& numberValue() const;
            const ArrayType& arrayValue() const;
            const MapType& mapValue() const;

            size_t length() const;
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

            explicit operator bool() const;
            Value operator!() const;
            
            friend bool operator==(const Value& lhs, const Value& rhs);
            friend bool operator!=(const Value& lhs, const Value& rhs);
            friend bool operator<(const Value& lhs, const Value& rhs);
            friend bool operator<=(const Value& lhs, const Value& rhs);
            friend bool operator>(const Value& lhs, const Value& rhs);
            friend bool operator>=(const Value& lhs, const Value& rhs);
        private:
            friend int compare(const Value& lhs, const Value& rhs);
        };
        
        class EvaluationContext {
        private:
            typedef std::map<String, Value> VariableTable;
            VariableTable m_variables;
        public:
            const Value& variableValue(const String& name) const;
            void defineVariable(const String& name, const Value& value);
        };
        
        class InternalEvaluationContext {
        private:
            typedef std::list<Value> ValueStack;
            typedef std::map<String, ValueStack> VariableTable;
            VariableTable m_variables;
            
            const EvaluationContext& m_context;
        public:
            InternalEvaluationContext(const EvaluationContext& context);
            
            const Value& variableValue(const String& name) const;
            
            void pushVariable(const String& name, const Value& value);
            void popVariable(const String& name);
        };

        class ExpressionBase;
        
        class Expression {
        private:
            typedef std::tr1::shared_ptr<ExpressionBase> ExpressionPtr;
            ExpressionPtr m_expression;
        public:
            Expression(ExpressionBase* expression);

            Value evaluate(const EvaluationContext& context) const;
        };
        
        class BinaryOperator;

        class ExpressionBase {
        public:
            typedef std::auto_ptr<ExpressionBase> Ptr;
            typedef std::list<ExpressionBase*> List;
            typedef std::map<String, ExpressionBase*> Map;
        protected:
        public:
            ExpressionBase();
            virtual ~ExpressionBase();

            bool range() const;
            
            ExpressionBase* reorderByPrecedence();
            ExpressionBase* reorderByPrecedence(BinaryOperator* parent);
            
            ExpressionBase* clone() const;
            Value evaluate(InternalEvaluationContext& context) const;
        private:
            virtual bool doIsRange() const;
            virtual ExpressionBase* doReorderByPrecedence();
            virtual ExpressionBase* doReorderByPrecedence(BinaryOperator* parent);
            virtual ExpressionBase* doClone() const = 0;
            virtual Value doEvaluate(InternalEvaluationContext& context) const = 0;
            
            deleteCopyAndAssignment(ExpressionBase)
        };
        
        class LiteralExpression : public ExpressionBase {
        private:
            Value m_value;
        private:
            LiteralExpression(const Value& value);
        public:
            static ExpressionBase* create(const Value& value);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(LiteralExpression)
        };
        
        class VariableExpression : public ExpressionBase {
        private:
            String m_variableName;
        private:
            VariableExpression(const String& variableName);
        public:
            static ExpressionBase* create(const String& variableName);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(VariableExpression)
        };
        
        class ArrayLiteralExpression : public ExpressionBase {
        private:
            ExpressionBase::List m_elements;
        private:
            ArrayLiteralExpression(const ExpressionBase::List& elements);
        public:
            static ExpressionBase* create(const ExpressionBase::List& elements);
            ~ArrayLiteralExpression();
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(ArrayLiteralExpression)
        };
        
        class MapLiteralExpression : public ExpressionBase {
        private:
            ExpressionBase::Map m_elements;
        private:
            MapLiteralExpression(const ExpressionBase::Map& elements);
        public:
            static ExpressionBase* create(const ExpressionBase::Map& elements);
            ~MapLiteralExpression();
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(MapLiteralExpression)
        };
        
        class UnaryOperator : public ExpressionBase {
        protected:
            ExpressionBase* m_operand;
        protected:
            UnaryOperator(ExpressionBase* operand);
        public:
            virtual ~UnaryOperator();
        private:
            deleteCopyAndAssignment(UnaryOperator)
        };

        class UnaryPlusOperator : public UnaryOperator {
        private:
            UnaryPlusOperator(ExpressionBase* operand);
        public:
            static ExpressionBase* create(ExpressionBase* operand);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(UnaryPlusOperator)
        };
        
        class UnaryMinusOperator : public UnaryOperator {
        private:
            UnaryMinusOperator(ExpressionBase* operand);
        public:
            static ExpressionBase* create(ExpressionBase* operand);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(UnaryMinusOperator)
        };
        
        class NegationOperator : public UnaryOperator {
        private:
            NegationOperator(ExpressionBase* operand);
        public:
            static ExpressionBase* create(ExpressionBase* operand);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(NegationOperator)
        };
        
        class GroupingOperator : public UnaryOperator {
        private:
            GroupingOperator(ExpressionBase* operand);
        public:
            static ExpressionBase* create(ExpressionBase* operand);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(GroupingOperator)
        };
        
        class SubscriptOperator : public ExpressionBase {
        private:
            ExpressionBase* m_indexableOperand;
            ExpressionBase* m_indexOperand;
        private:
            SubscriptOperator(ExpressionBase* indexableOperand, ExpressionBase* indexOperand);
        public:
            ~SubscriptOperator();
        public:
            static ExpressionBase* create(ExpressionBase* indexableOperand, ExpressionBase* indexOperand);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(SubscriptOperator)
        };
        
        class BinaryOperator : public ExpressionBase {
        protected:
            ExpressionBase* m_leftOperand;
            ExpressionBase* m_rightOperand;
        protected:
            BinaryOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        public:
            virtual ~BinaryOperator();
        private:
            ExpressionBase* doReorderByPrecedence();
            ExpressionBase* doReorderByPrecedence(BinaryOperator* parent);
            BinaryOperator* rotateLeftUp(BinaryOperator* leftOperand);
            BinaryOperator* rotateRightUp(BinaryOperator* rightOperand);
        protected:
            struct Traits;
        private:
            Traits traits() const;
            virtual Traits doGetTraits() const = 0;
        public:
            size_t precedence() const;
            bool associative() const;
            bool commutative() const;
        private:
            
            deleteCopyAndAssignment(BinaryOperator)
        };
        
        class AdditionOperator : public BinaryOperator {
        private:
            AdditionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            Traits doGetTraits() const;
            
            deleteCopyAndAssignment(AdditionOperator)
        };
        
        class SubtractionOperator : public BinaryOperator {
        private:
            SubtractionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            Traits doGetTraits() const;
            
            deleteCopyAndAssignment(SubtractionOperator)
        };
        
        class MultiplicationOperator : public BinaryOperator {
        private:
            MultiplicationOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            Traits doGetTraits() const;
            
            deleteCopyAndAssignment(MultiplicationOperator)
        };
        
        class DivisionOperator : public BinaryOperator {
        private:
            DivisionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            Traits doGetTraits() const;
            
            deleteCopyAndAssignment(DivisionOperator)
        };
        
        class ModulusOperator : public BinaryOperator {
        private:
            ModulusOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            Traits doGetTraits() const;
            
            deleteCopyAndAssignment(ModulusOperator)
        };
        
        class ConjunctionOperator : public BinaryOperator {
        private:
            ConjunctionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        private:
            bool doIsRange() const;
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            Traits doGetTraits() const;
            
            deleteCopyAndAssignment(ConjunctionOperator)
        };
        
        class DisjunctionOperator : public BinaryOperator {
        private:
            DisjunctionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        private:
            bool doIsRange() const;
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            Traits doGetTraits() const;
            
            deleteCopyAndAssignment(DisjunctionOperator)
        };
        
        class ComparisonOperator : public BinaryOperator {
        private:
            typedef enum {
                Op_Less,
                Op_LessOrEqual,
                Op_Equal,
                Op_Inequal,
                Op_GreaterOrEqual,
                Op_Greater
            } Op;
            Op m_op;
        private:
            ComparisonOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, Op op);
        public:
            static ExpressionBase* createLess(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
            static ExpressionBase* createLessOrEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
            static ExpressionBase* createEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
            static ExpressionBase* createInequal(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
            static ExpressionBase* createGreaterOrEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
            static ExpressionBase* createGreater(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        private:
            bool doIsRange() const;
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            Traits doGetTraits() const;
            
            deleteCopyAndAssignment(ComparisonOperator)
        };
        
        class RangeOperator : public BinaryOperator {
        public:
            static const String& AutoRangeParameterName();
        private:
            RangeOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand);
            static ExpressionBase* createAutoRangeWithLeftOperand(ExpressionBase* leftOperand);
            static ExpressionBase* createAutoRangeWithRightOperand(ExpressionBase* rightOperand);
        private:
            bool doIsRange() const;
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            Traits doGetTraits() const;
            
            deleteCopyAndAssignment(RangeOperator)
        };
    }
}

#endif /* EL_h */
