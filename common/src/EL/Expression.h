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
#include "EL/EL_Forward.h"
#include "EL/Value.h"

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace EL {
        class Expression {
        private:
            std::shared_ptr<ExpressionBase> m_expression;
        public:
            // intentionally allows implicit conversions
            Expression(ExpressionBase* expression);

            bool optimize();
            Value evaluate(const EvaluationContext& context) const;
            ExpressionBase* clone() const;

            size_t line() const;
            size_t column() const;
            std::string asString() const;
            friend std::ostream& operator<<(std::ostream& stream, const Expression& expression);
        };

        class BinaryOperator;

        class ExpressionBase {
        public:
            using Ptr = std::unique_ptr<ExpressionBase>;
            using List = std::vector<Ptr>;
            using Map = std::map<std::string, Ptr>;

            friend class Expression;
        protected:
            size_t m_line;
            size_t m_column;
        protected:
            static bool replaceExpression(Ptr& oldExpression, ExpressionBase* newExpression);
        public:
            ExpressionBase(size_t line, size_t column);
            virtual ~ExpressionBase();

            ExpressionBase* reorderByPrecedence();
            ExpressionBase* reorderByPrecedence(BinaryOperator* parent);

            ExpressionBase* clone() const;
            ExpressionBase* optimize();
            Value evaluate(const EvaluationContext& context) const;

            std::string asString() const;
            void appendToStream(std::ostream& str) const;
            friend std::ostream& operator<<(std::ostream& stream, const ExpressionBase& expression);
        private:
            virtual ExpressionBase* doReorderByPrecedence();
            virtual ExpressionBase* doReorderByPrecedence(BinaryOperator* parent);
            virtual ExpressionBase* doClone() const = 0;
            virtual ExpressionBase* doOptimize() = 0;
            virtual Value doEvaluate(const EvaluationContext& context) const = 0;
            virtual void doAppendToStream(std::ostream& str) const = 0;

            deleteCopyAndMove(ExpressionBase)
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

            deleteCopyAndMove(LiteralExpression)
        };

        class VariableExpression : public ExpressionBase {
        private:
            std::string m_variableName;
        private:
            VariableExpression(const std::string& variableName, size_t line, size_t column);
        public:
            static ExpressionBase* create(const std::string& variableName, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            ExpressionBase* doOptimize() override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;

            deleteCopyAndMove(VariableExpression)
        };

        class ArrayExpression : public ExpressionBase {
        private:
            ExpressionBase::List m_elements;
        private:
            ArrayExpression(ExpressionBase::List&& elements, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase::List&& elements, size_t line, size_t column);
            ~ArrayExpression() override;
        private:
            ExpressionBase* doClone() const override;
            ExpressionBase* doOptimize() override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;

            deleteCopyAndMove(ArrayExpression)
        };

        class MapExpression : public ExpressionBase {
        private:
            ExpressionBase::Map m_elements;
        private:
            MapExpression(ExpressionBase::Map&& elements, size_t line, size_t column);
        public:
            static ExpressionBase* create(ExpressionBase::Map&& elements, size_t line, size_t column);
            ~MapExpression() override;
        private:
            ExpressionBase* doClone() const override;
            ExpressionBase* doOptimize() override;
            Value doEvaluate(const EvaluationContext& context) const override;
            void doAppendToStream(std::ostream& str) const override;

            deleteCopyAndMove(MapExpression)
        };

        class UnaryOperator : public ExpressionBase {
        protected:
            ExpressionBase::Ptr m_operand;
        protected:
            UnaryOperator(ExpressionBase* operand, size_t line, size_t column);
        public:
            virtual ~UnaryOperator() override;
        private:
            ExpressionBase* doOptimize() override;
            deleteCopyAndMove(UnaryOperator)
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

            deleteCopyAndMove(UnaryPlusOperator)
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

            deleteCopyAndMove(UnaryMinusOperator)
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

            deleteCopyAndMove(LogicalNegationOperator)
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

            deleteCopyAndMove(BitwiseNegationOperator)
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

            deleteCopyAndMove(GroupingOperator)
        };

        class SubscriptOperator : public ExpressionBase {
        private:
            ExpressionBase::Ptr m_indexableOperand;
            ExpressionBase::Ptr m_indexOperand;
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

            deleteCopyAndMove(SubscriptOperator)
        };

        class BinaryOperator : public ExpressionBase {
        protected:
            ExpressionBase::Ptr m_leftOperand;
            ExpressionBase::Ptr m_rightOperand;
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

            deleteCopyAndMove(BinaryOperator)
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

            deleteCopyAndMove(AdditionOperator)
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

            deleteCopyAndMove(SubtractionOperator)
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

            deleteCopyAndMove(MultiplicationOperator)
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

            deleteCopyAndMove(DivisionOperator)
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

            deleteCopyAndMove(ModulusOperator)
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

            deleteCopyAndMove(LogicalAndOperator)
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

            deleteCopyAndMove(LogicalOrOperator)
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

            deleteCopyAndMove(BitwiseAndOperator)
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

            deleteCopyAndMove(BitwiseXorOperator)
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

            deleteCopyAndMove(BitwiseOrOperator)
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

            deleteCopyAndMove(BitwiseShiftLeftOperator)
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

            deleteCopyAndMove(BitwiseShiftRightOperator)
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

            deleteCopyAndMove(ComparisonOperator)
        };

        class RangeOperator : public BinaryOperator {
        public:
            static const std::string& AutoRangeParameterName();
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

            deleteCopyAndMove(RangeOperator)
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

            deleteCopyAndMove(CaseOperator)
        };

        class SwitchOperator : public ExpressionBase {
        private:
            ExpressionBase::List m_cases;
        private:
            SwitchOperator(ExpressionBase::List&& cases, size_t line, size_t column);
        public:
            ~SwitchOperator() override;
        public:
            static ExpressionBase* create(ExpressionBase::List&& cases, size_t line, size_t column);
        private:
            ExpressionBase* doClone() const override;
            ExpressionBase* doOptimize() override;
            void doAppendToStream(std::ostream& str) const override;
            Value doEvaluate(const EvaluationContext& context) const override;

            deleteCopyAndMove(SwitchOperator)
        };
    }
}

#endif /* Expression_h */
