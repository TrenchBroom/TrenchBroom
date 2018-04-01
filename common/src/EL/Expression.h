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

#ifndef Expression_h
#define Expression_h

#include "Macros.h"
#include "SharedPointer.h"
#include "EL/Value.h"

#include <list>
#include <memory>

namespace TrenchBroom {
    namespace EL {
        class EvaluationContext;
        class ExpressionBase;
        
        class Expression {
        private:
            typedef std::shared_ptr<ExpressionBase> ExpressionPtr;
            ExpressionPtr m_expression;
        public:
            Expression(ExpressionBase* expression);
            
            bool optimize();
            Value evaluate(const EvaluationContext& context) const;
            ExpressionBase* clone() const;
            
            size_t line() const;
            size_t column() const;
            String asString() const;
            friend std::ostream& operator<<(std::ostream& stream, const Expression& expression);
        };
        
        class BinaryOperator;
        
        class ExpressionBase {
        public:
            typedef std::unique_ptr<ExpressionBase> Ptr;
            typedef std::list<ExpressionBase*> List;
            typedef std::map<String, ExpressionBase*> Map;
            
            friend class Expression;
        protected:
            size_t m_line;
            size_t m_column;
        protected:
            static void replaceExpression(ExpressionBase*& oldExpression, ExpressionBase* newExpression);
        public:
            ExpressionBase(size_t line, size_t column);
            virtual ~ExpressionBase();
            
            ExpressionBase* reorderByPrecedence();
            ExpressionBase* reorderByPrecedence(BinaryOperator* parent);
            
            ExpressionBase* clone() const;
            ExpressionBase* optimize();
            Value evaluate(const EvaluationContext& context) const;
            
            String asString() const;
            void appendToStream(std::ostream& str) const;
            friend std::ostream& operator<<(std::ostream& stream, const ExpressionBase& expression);
        private:
            virtual ExpressionBase* doReorderByPrecedence();
            virtual ExpressionBase* doReorderByPrecedence(BinaryOperator* parent);
            virtual ExpressionBase* doClone() const = 0;
            virtual ExpressionBase* doOptimize() = 0;
            virtual Value doEvaluate(const EvaluationContext& context) const = 0;
            virtual void doAppendToStream(std::ostream& str) const = 0;
            
            deleteCopyAndAssignment(ExpressionBase)
        };
        
        class LiteralExpression : public ExpressionBase {
        private:
            Value m_value;
        private:
            LiteralExpression(const Value& value, size_t line, size_t column);
        public:
            static ExpressionBase* create(const Value& value, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            ExpressionBase* doOptimize() override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            
            deleteCopyAndAssignment(LiteralExpression)
        };
        
        class VariableExpression : public ExpressionBase {
        private:
            String m_variableName;
        private:
            VariableExpression(const String& variableName, size_t line, size_t column);
        public:
            static ExpressionBase* create(const String& variableName, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            ExpressionBase* doOptimize() override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            
            deleteCopyAndAssignment(VariableExpression)
        };
        
        class ArrayExpression : public ExpressionBase {
        private:
            ExpressionBase::List m_elements;
        private:
            ArrayExpression(const ExpressionBase::List& elements, size_t line, size_t column);
        public:
            static ExpressionBase* create(const ExpressionBase::List& elements, size_t line, size_t column);
            ~ArrayExpression() override;
        private:
            ExpressionBase* doClone() const override;
            ExpressionBase* doOptimize() override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            
            deleteCopyAndAssignment(ArrayExpression)
        };
        
        class MapExpression : public ExpressionBase {
        private:
            ExpressionBase::Map m_elements;
        private:
            MapExpression(const ExpressionBase::Map& elements, size_t line, size_t column);
        public:
            static ExpressionBase* create(const ExpressionBase::Map& elements, size_t line, size_t column);
            ~MapExpression() override;
        private:
            ExpressionBase* doClone() const override;
            ExpressionBase* doOptimize() override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            
            deleteCopyAndAssignment(MapExpression)
        };
        
        class UnaryOperator : public ExpressionBase {
        protected:
            ExpressionBase* m_operand;
        protected:
            UnaryOperator(ExpressionBase* operand, size_t line, size_t column);
        public:
            virtual ~UnaryOperator() override;
        private:
            ExpressionBase* doOptimize() override;
            deleteCopyAndAssignment(UnaryOperator)
        };
        
        class UnaryPlusOperator : public UnaryOperator {
        private:
            UnaryPlusOperator(ExpressionBase* operand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* operand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            
            deleteCopyAndAssignment(UnaryPlusOperator)
        };
        
        class UnaryMinusOperator : public UnaryOperator {
        private:
            UnaryMinusOperator(ExpressionBase* operand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* operand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            
            deleteCopyAndAssignment(UnaryMinusOperator)
        };
        
        class LogicalNegationOperator : public UnaryOperator {
        private:
            LogicalNegationOperator(ExpressionBase* operand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* operand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            
            deleteCopyAndAssignment(LogicalNegationOperator)
        };
        
        class BitwiseNegationOperator : public UnaryOperator {
        private:
            BitwiseNegationOperator(ExpressionBase* operand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* operand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            
            deleteCopyAndAssignment(BitwiseNegationOperator)
        };
        
        class GroupingOperator : public UnaryOperator {
        private:
            GroupingOperator(ExpressionBase* operand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* operand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            
            deleteCopyAndAssignment(GroupingOperator)
        };
        
        class SubscriptOperator : public ExpressionBase {
        private:
            ExpressionBase* m_indexableOperand;
            ExpressionBase* m_indexOperand;
        private:
            SubscriptOperator(ExpressionBase* indexableOperand, ExpressionBase* indexOperand, size_t line, size_t column);
        public:
            ~SubscriptOperator() override;
        public:
            static ExpressionBase* create(ExpressionBase* indexableOperand, ExpressionBase* indexOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            ExpressionBase* doOptimize() override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            
            deleteCopyAndAssignment(SubscriptOperator)
        };
        
        class BinaryOperator : public ExpressionBase {
        protected:
            ExpressionBase* m_leftOperand;
            ExpressionBase* m_rightOperand;
        protected:
            BinaryOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        public:
            virtual ~BinaryOperator() override;
        private:
            ExpressionBase* doReorderByPrecedence() override;
            ExpressionBase* doReorderByPrecedence(BinaryOperator* parent) override;
            BinaryOperator* rotateLeftUp(BinaryOperator* leftOperand);
            BinaryOperator* rotateRightUp(BinaryOperator* rightOperand);
        private:
            ExpressionBase* doOptimize() override;
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
            AdditionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(AdditionOperator)
        };
        
        class SubtractionOperator : public BinaryOperator {
        private:
            SubtractionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(SubtractionOperator)
        };
        
        class MultiplicationOperator : public BinaryOperator {
        private:
            MultiplicationOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(MultiplicationOperator)
        };
        
        class DivisionOperator : public BinaryOperator {
        private:
            DivisionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(DivisionOperator)
        };
        
        class ModulusOperator : public BinaryOperator {
        private:
            ModulusOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(ModulusOperator)
        };
        
        class LogicalAndOperator : public BinaryOperator {
        private:
            LogicalAndOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(LogicalAndOperator)
        };
        
        class LogicalOrOperator : public BinaryOperator {
        private:
            LogicalOrOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(LogicalOrOperator)
        };
        
        class BitwiseAndOperator : public BinaryOperator {
        private:
            BitwiseAndOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(BitwiseAndOperator)
        };
        
        class BitwiseXorOperator : public BinaryOperator {
        private:
            BitwiseXorOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(BitwiseXorOperator)
        };
        
        class BitwiseOrOperator : public BinaryOperator {
        private:
            BitwiseOrOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(BitwiseOrOperator)
        };
        
        class BitwiseShiftLeftOperator : public BinaryOperator {
        private:
            BitwiseShiftLeftOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(BitwiseShiftLeftOperator)
        };
        
        class BitwiseShiftRightOperator : public BinaryOperator {
        private:
            BitwiseShiftRightOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(BitwiseShiftRightOperator)
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
            ComparisonOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, Op op, size_t line, size_t column);
        public:
            static ExpressionBase* createLess(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
            static ExpressionBase* createLessOrEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
            static ExpressionBase* createEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
            static ExpressionBase* createInequal(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
            static ExpressionBase* createGreaterOrEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
            static ExpressionBase* createGreater(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(ComparisonOperator)
        };
        
        class RangeOperator : public BinaryOperator {
        public:
            static const String& AutoRangeParameterName();
        private:
            RangeOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, size_t line, size_t column);
            static ExpressionBase* createAutoRangeWithLeftOperand(ExpressionBase* leftOperand, size_t line, size_t column);
            static ExpressionBase* createAutoRangeWithRightOperand(ExpressionBase* rightOperand, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(RangeOperator)
        };

        class CaseOperator : public BinaryOperator {
        private:
            CaseOperator(ExpressionBase* premise, ExpressionBase* conclusion, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase* premise, ExpressionBase* conclusion, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;
            Traits doGetTraits() const override;
            
            deleteCopyAndAssignment(CaseOperator)
        };
        
        class SwitchOperator : public ExpressionBase {
        private:
            ExpressionBase::List m_cases;
        private:
            SwitchOperator(const ExpressionBase::List& cases, size_t line, size_t column);
        public:
            ~SwitchOperator() override;
        public:
            static ExpressionBase* create(const ExpressionBase::List& cases, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            ExpressionBase* doOptimize() override;
            void doAppendToStream(std::ostream& str) const override;
            Value doEvaluate(const EvaluationContext& context) const override;
            
            deleteCopyAndAssignment(SwitchOperator)
        };
    }
}

#endif /* Expression_h */
