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
        typedef std::vector<long> RangeType;
        
        typedef enum {
            Type_Boolean,
            Type_String,
            Type_Number,
            Type_Array,
            Type_Map,
            Type_Range,
            Type_Null
        } ValueType;
        
        String typeName(ValueType type);
        ValueType typeForName(const String& type);
        
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
        
        class DereferenceError : public ELException {
        public:
            DereferenceError(const String& value, const ValueType from, const ValueType to) throw();
        };
        
        class EvaluationError : public ELException {
        public:
            EvaluationError(const String& msg) throw();
        };
        
        class IndexError : public EvaluationError {
        public:
            IndexError(const Value& indexableValue, const Value& indexValue) throw();
            IndexError(const Value& indexableValue, size_t index) throw();
            IndexError(const Value& indexableValue, const String& key) throw();
        };
        
        class IndexOutOfBoundsError : public IndexError {
        public:
            IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, size_t outOfBoundsIndex) throw();
            IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, const String& outOfBoundsIndex) throw();
            IndexOutOfBoundsError(const Value& indexableValue, size_t index) throw();
            IndexOutOfBoundsError(const Value& indexableValue, const String& key) throw();
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
            virtual const RangeType&   rangeValue()   const;

            virtual size_t length() const = 0;
            virtual ValueHolder* convertTo(ValueType toType) const = 0;
            
            virtual ValueHolder* clone() const = 0;
            
            virtual void appendToStream(std::ostream& str, const String& indent) const = 0;
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
            void appendToStream(std::ostream& str, const String& indent) const;
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
            void appendToStream(std::ostream& str, const String& indent) const;
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
            void appendToStream(std::ostream& str, const String& indent) const;
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
            void appendToStream(std::ostream& str, const String& indent) const;
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
            void appendToStream(std::ostream& str, const String& indent) const;
        };
        
        class RangeValueHolder : public ValueHolder {
        private:
            RangeType m_value;
        public:
            RangeValueHolder(const RangeType& value);
            ValueType type() const;
            String description() const;
            const RangeType& rangeValue() const;
            size_t length() const;
            ValueHolder* convertTo(const ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str, const String& indent) const;
        };

        class NullValueHolder : public ValueHolder {
        public:
            ValueType type() const;
            String description() const;
            const StringType& stringValue() const;
            const BooleanType& booleanValue() const;
            const NumberType& numberValue() const;
            const ArrayType& arrayValue() const;
            const MapType& mapValue() const;
            const RangeType& rangeValue() const;
            size_t length() const;
            ValueHolder* convertTo(const ValueType toType) const;
            ValueHolder* clone() const;
            void appendToStream(std::ostream& str, const String& indent) const;
        };
        
        class Value {
        public:
            static const Value Null;
            typedef std::set<Value> Set;
        private:
            typedef std::vector<size_t> IndexList;
            typedef std::tr1::shared_ptr<ValueHolder> ValuePtr;
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
            
            Value(const MapType& value, size_t line, size_t column);
            explicit Value(const MapType& value);

            Value(const RangeType& value, size_t line, size_t column);
            explicit Value(const RangeType& value);

            Value(const Value& other, size_t line, size_t column);
            Value(const Value& other);

            Value();
            
            ValueType type() const;
            String typeName() const;
            String description() const;
            
            size_t line() const;
            size_t column() const;
            
            const StringType& stringValue() const;
            const BooleanType& booleanValue() const;
            const NumberType& numberValue() const;
            const ArrayType& arrayValue() const;
            const MapType& mapValue() const;
            const RangeType& rangeValue() const;
            bool null() const;
            
            const StringList asStringList() const;
            const StringSet asStringSet() const;
            
            size_t length() const;
            Value convertTo(ValueType toType) const;

            void appendToStream(std::ostream& str, const String& indent = "") const;
            friend std::ostream& operator<<(std::ostream& stream, const Value& value);
            
            bool contains(const Value& indexValue) const;
            bool contains(size_t index) const;
            bool contains(const String& key) const;
            StringSet keys() const;
            
            Value operator[](const Value& indexValue) const;
            Value operator[](size_t index) const;
            Value operator[](const String& key) const;
        private:
            IndexList computeIndexArray(const Value& indexValue, size_t indexableSize) const;
            void computeIndexArray(const Value& indexValue, size_t indexableSize, IndexList& result) const;
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
            size_t m_offset;
            size_t m_length;
        public:
            Expression(ExpressionBase* expression, size_t offset, size_t length);

            size_t offset() const;
            size_t length() const;
            
            void optimize();
            Value evaluate(const EvaluationContext& context) const;
        };
        
        class BinaryOperator;

        struct ExpressionPosition {
            size_t line;
            size_t column;
            size_t offset;
            size_t length;
            
            ExpressionPosition(size_t i_line, size_t i_column, size_t i_offset, size_t i_length);
        };
        
        class ExpressionBase {
        public:
            typedef std::auto_ptr<ExpressionBase> Ptr;
            typedef std::list<ExpressionBase*> List;
            typedef std::map<String, ExpressionBase*> Map;
        protected:
            ExpressionPosition m_position;
        protected:
            static void replaceExpression(ExpressionBase*& oldExpression, ExpressionBase* newExpression);
        public:
            ExpressionBase(const ExpressionPosition& position);
            virtual ~ExpressionBase();

            const ExpressionPosition& position() const;
            
            ExpressionBase* reorderByPrecedence();
            ExpressionBase* reorderByPrecedence(BinaryOperator* parent);
            
            ExpressionBase* clone() const;
            ExpressionBase* optimize();
            Value evaluate(InternalEvaluationContext& context) const;
        private:
            virtual ExpressionBase* doReorderByPrecedence();
            virtual ExpressionBase* doReorderByPrecedence(BinaryOperator* parent);
            virtual ExpressionBase* doClone() const = 0;
            virtual ExpressionBase* doOptimize() = 0;
            virtual Value doEvaluate(InternalEvaluationContext& context) const = 0;
            
            deleteCopyAndAssignment(ExpressionBase)
        };
        
        class LiteralExpression : public ExpressionBase {
        private:
            Value m_value;
        private:
            LiteralExpression(const Value& value, const ExpressionPosition& position);
        public:
            static ExpressionBase* create(const Value& value, const ExpressionPosition& position);
        private:
            ExpressionBase* doClone() const;
            ExpressionBase* doOptimize();
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(LiteralExpression)
        };
        
        class VariableExpression : public ExpressionBase {
        private:
            String m_variableName;
        private:
            VariableExpression(const String& variableName, const ExpressionPosition& position);
        public:
            static ExpressionBase* create(const String& variableName, const ExpressionPosition& position);
        private:
            ExpressionBase* doClone() const;
            ExpressionBase* doOptimize();
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(VariableExpression)
        };
        
        class ArrayExpression : public ExpressionBase {
        private:
            ExpressionBase::List m_elements;
        private:
            ArrayExpression(const ExpressionBase::List& elements, const ExpressionPosition& position);
        public:
            static ExpressionBase* create(const ExpressionBase::List& elements, const ExpressionPosition& position);
            ~ArrayExpression();
        private:
            ExpressionBase* doClone() const;
            ExpressionBase* doOptimize();
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(ArrayExpression)
        };
        
        class MapExpression : public ExpressionBase {
        private:
            ExpressionBase::Map m_elements;
        private:
            MapExpression(const ExpressionBase::Map& elements, const ExpressionPosition& position);
        public:
            static ExpressionBase* create(const ExpressionBase::Map& elements, const ExpressionPosition& position);
            ~MapExpression();
        private:
            ExpressionBase* doClone() const;
            ExpressionBase* doOptimize();
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(MapExpression)
        };
        
        class UnaryOperator : public ExpressionBase {
        protected:
            ExpressionBase* m_operand;
        protected:
            UnaryOperator(ExpressionBase* operand, const ExpressionPosition& position);
        public:
            virtual ~UnaryOperator();
        private:
            ExpressionBase* doOptimize();
            deleteCopyAndAssignment(UnaryOperator)
        };

        class UnaryPlusOperator : public UnaryOperator {
        private:
            UnaryPlusOperator(ExpressionBase* operand, const ExpressionPosition& position);
        public:
            static ExpressionBase* create(ExpressionBase* operand, const ExpressionPosition& position);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(UnaryPlusOperator)
        };
        
        class UnaryMinusOperator : public UnaryOperator {
        private:
            UnaryMinusOperator(ExpressionBase* operand, const ExpressionPosition& position);
        public:
            static ExpressionBase* create(ExpressionBase* operand, const ExpressionPosition& position);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(UnaryMinusOperator)
        };
        
        class NegationOperator : public UnaryOperator {
        private:
            NegationOperator(ExpressionBase* operand, const ExpressionPosition& position);
        public:
            static ExpressionBase* create(ExpressionBase* operand, const ExpressionPosition& position);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            
            deleteCopyAndAssignment(NegationOperator)
        };
        
        class GroupingOperator : public UnaryOperator {
        private:
            GroupingOperator(ExpressionBase* operand, const ExpressionPosition& position);
        public:
            static ExpressionBase* create(ExpressionBase* operand, const ExpressionPosition& position);
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
            SubscriptOperator(ExpressionBase* indexableOperand, ExpressionBase* indexOperand, const ExpressionPosition& position);
        public:
            ~SubscriptOperator();
        public:
            static ExpressionBase* create(ExpressionBase* indexableOperand, ExpressionBase* indexOperand, const ExpressionPosition& position);
        private:
            ExpressionBase* doClone() const;
            ExpressionBase* doOptimize();
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
        private:
            ExpressionBase* doOptimize();
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
            static ExpressionBase* createAutoRangeWithRightOperand(ExpressionBase* rightOperand, const ExpressionPosition& position);
        private:
            ExpressionBase* doClone() const;
            Value doEvaluate(InternalEvaluationContext& context) const;
            Traits doGetTraits() const;
            
            deleteCopyAndAssignment(RangeOperator)
        };
    }
}

#endif /* EL_h */
