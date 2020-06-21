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

#include "Expression.h"

#include "Ensure.h"
#include "EL/EvaluationContext.h"

#include <sstream>
#include <string>

namespace TrenchBroom {
    namespace EL {
        Expression::Expression(ExpressionBase* expression) :
        m_expression(expression) {
            ensure(m_expression.get() != nullptr, "expression is null");
        }

        bool Expression::optimize() {
            ExpressionBase* optimized = m_expression->optimize();
            if (optimized != nullptr && optimized != m_expression.get()) {
                m_expression.reset(optimized);
                return true;
            } else {
                return false;
            }
        }

        Value Expression::evaluate(const EvaluationContext& context) const {
            return m_expression->evaluate(context);
        }

        ExpressionBase* Expression::clone() const {
            return m_expression->clone();
        }

        size_t Expression::line() const {
            return m_expression->m_line;
        }

        size_t Expression::column() const {
            return m_expression->m_column;
        }

        std::string Expression::asString() const {
            return m_expression->asString();
        }

        std::ostream& operator<<(std::ostream& stream, const Expression& expression) {
            return stream << *(expression.m_expression.get());
        }

        bool ExpressionBase::replaceExpression(std::unique_ptr<ExpressionBase>& oldExpression, ExpressionBase* newExpression) {
            if (newExpression != nullptr && newExpression != oldExpression.get()) {
                oldExpression.reset(newExpression);
            }
            return newExpression != nullptr;
        }

        ExpressionBase::ExpressionBase(const size_t line, const size_t column) : m_line(line), m_column(column) {}
        ExpressionBase::~ExpressionBase() {}

        ExpressionBase* ExpressionBase::reorderByPrecedence() {
            return doReorderByPrecedence();
        }

        ExpressionBase* ExpressionBase::reorderByPrecedence(BinaryOperator* parent) {
            return doReorderByPrecedence(parent);
        }

        ExpressionBase* ExpressionBase::clone() const {
            return doClone();
        }

        ExpressionBase* ExpressionBase::optimize() {
            return doOptimize();
        }

        Value ExpressionBase::evaluate(const EvaluationContext& context) const {
            return doEvaluate(context);
        }

        std::string ExpressionBase::asString() const {
            std::stringstream result;
            appendToStream(result);
            return result.str();
        }

        void ExpressionBase::appendToStream(std::ostream& str) const {
            doAppendToStream(str);
        }

        std::ostream& operator<<(std::ostream& stream, const ExpressionBase& expression) {
            expression.appendToStream(stream);
            return stream;
        }

        ExpressionBase* ExpressionBase::doReorderByPrecedence() {
            return this;
        }

        ExpressionBase* ExpressionBase::doReorderByPrecedence(BinaryOperator* parent) {
            return parent;
        }

        LiteralExpression::LiteralExpression(const Value& value, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_value(value, line, column) {}

        ExpressionBase* LiteralExpression::create(const Value& value, const size_t line, const size_t column) {
            return new LiteralExpression(value, line, column);
        }

        ExpressionBase* LiteralExpression::doClone() const {
            return new LiteralExpression(m_value, m_line, m_column);
        }

        ExpressionBase* LiteralExpression::doOptimize() {
            return this;
        }

        Value LiteralExpression::doEvaluate(const EvaluationContext& /* context */) const {
            return m_value;
        }

        void LiteralExpression::doAppendToStream(std::ostream& str) const {
            m_value.appendToStream(str, false);
        }

        VariableExpression::VariableExpression(const std::string& variableName, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_variableName(variableName) {}

        ExpressionBase* VariableExpression::create(const std::string& variableName, const size_t line, const size_t column) {
            return new VariableExpression(variableName, line, column);
        }

        ExpressionBase* VariableExpression::doClone() const {
            return new VariableExpression(m_variableName, m_line, m_column);
        }

        ExpressionBase* VariableExpression::doOptimize() {
            return nullptr;
        }

        Value VariableExpression::doEvaluate(const EvaluationContext& context) const {
            return context.variableValue(m_variableName);
        }

        void VariableExpression::doAppendToStream(std::ostream& str) const {
            str << m_variableName;
        }

        ArrayExpression::ArrayExpression(ExpressionBase::List&& elements, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_elements(std::move(elements)) {}

        ExpressionBase* ArrayExpression::create(ExpressionBase::List&& elements, const size_t line, const size_t column) {
            return new ArrayExpression(std::move(elements), line, column);
        }

        ArrayExpression::~ArrayExpression() = default;

        ExpressionBase* ArrayExpression::doClone() const {
            ExpressionBase::List clones;
            for (const auto& element : m_elements) {
                clones.emplace_back(element->clone());
            }

            return new ArrayExpression(std::move(clones), m_line, m_column);
        }

        ExpressionBase* ArrayExpression::doOptimize() {
            bool allOptimized = true;

            for (auto& expression : m_elements) {
                allOptimized &= replaceExpression(expression, expression->optimize());
            }

            if (allOptimized) {
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);
            }

            return nullptr;
        }

        Value ArrayExpression::doEvaluate(const EvaluationContext& context) const {
            ArrayType array;
            for (const auto& element : m_elements) {
                const Value value = element->evaluate(context);
                if (value.type() == ValueType::Range) {
                    const RangeType& range = value.rangeValue();
                    array.reserve(array.size() + range.size());
                    for (size_t i = 0; i < range.size(); ++i)
                        array.push_back(Value(range[i], value.line(), value.column()));
                } else {
                    array.push_back(value);
                }
            }

            return Value(array, m_line, m_column);
        }

        void ArrayExpression::doAppendToStream(std::ostream& str) const {
            str << "[ ";

            size_t i = 0;
            for (const auto& expression : m_elements) {
                str << *expression;
                if (i < m_elements.size() - 1)
                    str << ", ";
                ++i;
            }

            str << " ]";
        }

        MapExpression::MapExpression(ExpressionBase::Map&& elements, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_elements(std::move(elements)) {}

        ExpressionBase* MapExpression::create(ExpressionBase::Map&& elements, const size_t line, const size_t column) {
            return new MapExpression(std::move(elements), line, column);
        }

        MapExpression::~MapExpression() = default;

        ExpressionBase* MapExpression::doClone() const {
            ExpressionBase::Map clones;
            for (const auto& entry : m_elements) {
                const std::string& key = entry.first;
                const auto& value = entry.second;
                clones.insert(std::make_pair(key, value->clone()));
            }

            return new MapExpression(std::move(clones), m_line, m_column);
        }


        ExpressionBase* MapExpression::doOptimize() {
            bool allOptimized = true;

            for (auto& entry : m_elements) {
                auto& expression = entry.second;
                allOptimized &= replaceExpression(expression, expression->optimize());
            }

            if (allOptimized) {
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);
            } else {
                return nullptr;
            }
        }

        Value MapExpression::doEvaluate(const EvaluationContext& context) const {
            MapType map;
            for (const auto& entry : m_elements) {
                const std::string& key = entry.first;
                const auto& expression = entry.second;
                map.insert(std::make_pair(key, expression->evaluate(context)));
            }

            return Value(map, m_line, m_column);
        }

        void MapExpression::doAppendToStream(std::ostream& str) const {
            str << "{ ";
            size_t i = 0;
            for (const auto& entry : m_elements) {
                const std::string& key = entry.first;
                const auto& value = entry.second;

                str << "\"" << key << "\": " << *value;
                if (i < m_elements.size() - 1)
                    str << ", ";
                ++i;
            }
            str << " }";
        }

        UnaryOperator::UnaryOperator(ExpressionBase* operand, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_operand(operand) {
            ensure(m_operand != nullptr, "operand is null");
        }

        UnaryOperator::~UnaryOperator() = default;

        ExpressionBase* UnaryOperator::doOptimize() {
            if (replaceExpression(m_operand, m_operand->optimize())) {
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);
            }

            return nullptr;
        }

        UnaryPlusOperator::UnaryPlusOperator(ExpressionBase* operand, const size_t line, const size_t column) :
        UnaryOperator(operand, line, column) {}

        ExpressionBase* UnaryPlusOperator::create(ExpressionBase* operand, const size_t line, const size_t column) {
            return new UnaryPlusOperator(operand, line, column);
        }

        ExpressionBase* UnaryPlusOperator::doClone() const {
            return new UnaryPlusOperator(m_operand->clone(), m_line, m_column);
        }

        Value UnaryPlusOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(+m_operand->evaluate(context), m_line, m_column);
        }

        void UnaryPlusOperator::doAppendToStream(std::ostream& str) const {
            str << "+" << *m_operand;
        }

        UnaryMinusOperator::UnaryMinusOperator(ExpressionBase* operand, const size_t line, const size_t column) :
        UnaryOperator(operand, line, column) {}

        ExpressionBase* UnaryMinusOperator::create(ExpressionBase* operand, const size_t line, const size_t column) {
            return new UnaryMinusOperator(operand, line, column);
        }

        ExpressionBase* UnaryMinusOperator::doClone() const {
            return new UnaryMinusOperator(m_operand->clone(), m_line, m_column);
        }

        Value UnaryMinusOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(-m_operand->evaluate(context), m_line, m_column);
        }

        void UnaryMinusOperator::doAppendToStream(std::ostream& str) const {
            str << "-" << *m_operand;
        }

        LogicalNegationOperator::LogicalNegationOperator(ExpressionBase* operand, const size_t line, const size_t column) :
        UnaryOperator(operand, line, column) {}

        ExpressionBase* LogicalNegationOperator::create(ExpressionBase* operand, const size_t line, const size_t column) {
            return new LogicalNegationOperator(operand, line, column);
        }

        ExpressionBase* LogicalNegationOperator::doClone() const  {
            return new LogicalNegationOperator(m_operand->clone(), m_line, m_column);
        }

        Value LogicalNegationOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(!m_operand->evaluate(context), m_line, m_column);
        }

        void LogicalNegationOperator::doAppendToStream(std::ostream& str) const {
            str << "!" << *m_operand;
        }

        BitwiseNegationOperator::BitwiseNegationOperator(ExpressionBase* operand, const size_t line, const size_t column) :
        UnaryOperator(operand, line, column) {}

        ExpressionBase* BitwiseNegationOperator::create(ExpressionBase* operand, const size_t line, const size_t column) {
            return new BitwiseNegationOperator(operand, line, column);
        }

        ExpressionBase* BitwiseNegationOperator::doClone() const {
            return new BitwiseNegationOperator(m_operand->clone(), m_line, m_column);
        }

        Value BitwiseNegationOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(~m_operand->evaluate(context), m_line, m_column);
        }

        void BitwiseNegationOperator::doAppendToStream(std::ostream& str) const {
            str << "~" << *m_operand;
        }

        GroupingOperator::GroupingOperator(ExpressionBase* operand, const size_t line, const size_t column) :
        UnaryOperator(operand, line, column) {}

        ExpressionBase* GroupingOperator::create(ExpressionBase* operand, const size_t line, const size_t column) {
            return new GroupingOperator(operand, line, column);
        }

        ExpressionBase* GroupingOperator::doClone() const {
            return new GroupingOperator(m_operand->clone(), m_line, m_column);
        }

        Value GroupingOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(m_operand->evaluate(context), m_line, m_column);
        }

        void GroupingOperator::doAppendToStream(std::ostream& str) const {
            str << "( " << *m_operand << " )";
        }

        SubscriptOperator::SubscriptOperator(ExpressionBase* indexableOperand, ExpressionBase* indexOperand, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_indexableOperand(indexableOperand),
        m_indexOperand(indexOperand) {
            ensure(m_indexableOperand != nullptr, "indexableOperand is null");
            ensure(m_indexOperand != nullptr, "indexOperand is null");
        }

        SubscriptOperator::~SubscriptOperator() = default;

        ExpressionBase* SubscriptOperator::create(ExpressionBase* indexableOperand, ExpressionBase* indexOperand, const size_t line, const size_t column) {
            return (new SubscriptOperator(indexableOperand, indexOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* SubscriptOperator::doClone() const {
            return new SubscriptOperator(m_indexableOperand->clone(), m_indexOperand->clone(), m_line, m_column);
        }

        ExpressionBase* SubscriptOperator::doOptimize() {
            ExpressionBase* indexableOptimized = m_indexableOperand->optimize();
            ExpressionBase* indexOptimized = m_indexOperand->optimize();

            replaceExpression(m_indexableOperand, indexableOptimized);
            replaceExpression(m_indexOperand, indexOptimized);

            if (indexableOptimized != nullptr && indexOptimized != nullptr)
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);

            return nullptr;
        }

        Value SubscriptOperator::doEvaluate(const EvaluationContext& context) const {
            const Value indexableValue = m_indexableOperand->evaluate(context);

            EvaluationStack stack(context);
            stack.declareVariable(RangeOperator::AutoRangeParameterName(), Value(indexableValue.length()-1, m_line, m_column));
            const Value indexValue = m_indexOperand->evaluate(stack);

            return indexableValue[indexValue];
        }

        void SubscriptOperator::doAppendToStream(std::ostream& str) const {
            str << *m_indexableOperand << "[" << *m_indexOperand << "]";
        }

        BinaryOperator::BinaryOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_leftOperand(leftOperand),
        m_rightOperand(rightOperand) {
            ensure(m_leftOperand != nullptr, "leftOperand is null");
            ensure(m_rightOperand != nullptr, "rightOperand is null");
        }

        BinaryOperator::~BinaryOperator() = default;

        ExpressionBase* BinaryOperator::doReorderByPrecedence() {
            ExpressionBase* result = m_leftOperand->reorderByPrecedence(this);
            if (result == this) {
                result = m_rightOperand->reorderByPrecedence(this);
            }
            return result;
        }

        ExpressionBase* BinaryOperator::doReorderByPrecedence(BinaryOperator* parent) {
            assert(parent->m_leftOperand.get() == this || parent->m_rightOperand.get() == this);
            if (parent->m_leftOperand.get() == this && precedence() < parent->precedence()) {
                return parent->rotateLeftUp(this);
            }
            if (parent->m_rightOperand.get() == this && precedence() < parent->precedence()) {
                return parent->rotateRightUp(this);
            }
            return parent;
        }


        BinaryOperator* BinaryOperator::rotateLeftUp(BinaryOperator* leftOperand) {
            assert(m_leftOperand.get() == leftOperand);

            m_leftOperand.release();
            m_leftOperand = std::move(leftOperand->m_rightOperand);
            leftOperand->m_rightOperand.reset(this);

            return leftOperand;
        }

        BinaryOperator* BinaryOperator::rotateRightUp(BinaryOperator* rightOperand) {
            assert(m_rightOperand.get() == rightOperand);

            m_rightOperand.release();
            m_rightOperand = std::move(rightOperand->m_leftOperand);
            rightOperand->m_leftOperand.reset(this);

            return rightOperand;
        }

        ExpressionBase* BinaryOperator::doOptimize() {
            ExpressionBase* leftOptimized = m_leftOperand->optimize();
            ExpressionBase* rightOptimized = m_rightOperand->optimize();

            replaceExpression(m_leftOperand, leftOptimized);
            replaceExpression(m_rightOperand, rightOptimized);

            if (leftOptimized != nullptr && rightOptimized != nullptr)
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);

            return nullptr;
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

        BinaryOperator::Traits BinaryOperator::traits() const {
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

        AdditionOperator::AdditionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* AdditionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new AdditionOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* AdditionOperator::doClone() const {
            return new AdditionOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }

        Value AdditionOperator::doEvaluate(const EvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return Value(leftValue + rightValue, m_line, m_column);
        }

        void AdditionOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand << " + " << *m_rightOperand;
        }

        BinaryOperator::Traits AdditionOperator::doGetTraits() const {
            return Traits(10, true, true);
        }

        SubtractionOperator::SubtractionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* SubtractionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new SubtractionOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* SubtractionOperator::doClone() const {
            return new SubtractionOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }

        Value SubtractionOperator::doEvaluate(const EvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return Value(leftValue - rightValue, m_line, m_column);
        }

        void SubtractionOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand << " - " << *m_rightOperand;
        }

        BinaryOperator::Traits SubtractionOperator::doGetTraits() const {
            return Traits(10, false, false);
        }

        MultiplicationOperator::MultiplicationOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* MultiplicationOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new MultiplicationOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* MultiplicationOperator::doClone() const {
            return new MultiplicationOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }

        Value MultiplicationOperator::doEvaluate(const EvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return Value(leftValue * rightValue, m_line, m_column);
        }

        void MultiplicationOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand << " * " << *m_rightOperand;
        }

        BinaryOperator::Traits MultiplicationOperator::doGetTraits() const {
            return Traits(11, true, true);
        }

        DivisionOperator::DivisionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* DivisionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new DivisionOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* DivisionOperator::doClone() const {
            return new DivisionOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }

        Value DivisionOperator::doEvaluate(const EvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return Value(leftValue / rightValue, m_line, m_column);
        }

        void DivisionOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand << " / " << *m_rightOperand;
        }

        BinaryOperator::Traits DivisionOperator::doGetTraits() const {
            return Traits(11, false, false);
        }

        ModulusOperator::ModulusOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* ModulusOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new ModulusOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* ModulusOperator::doClone() const {
            return new ModulusOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }

        Value ModulusOperator::doEvaluate(const EvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);
            return Value(leftValue % rightValue, m_line, m_column);
        }

        void ModulusOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand << " % " << *m_rightOperand;
        }

        BinaryOperator::Traits ModulusOperator::doGetTraits() const {
            return Traits(11, false, false);
        }

        const std::string& RangeOperator::AutoRangeParameterName() {
            static const std::string Name = "__AutoRangeParameter";
            return Name;
        }

        LogicalAndOperator::LogicalAndOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* LogicalAndOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new LogicalAndOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* LogicalAndOperator::doClone() const {
            return new LogicalAndOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }

        Value LogicalAndOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(m_leftOperand->evaluate(context) && m_rightOperand->evaluate(context), m_line, m_column);
        }

        void LogicalAndOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand << " && " << *m_rightOperand;
        }

        BinaryOperator::Traits LogicalAndOperator::doGetTraits() const {
            return Traits(3, true, true);
        }

        LogicalOrOperator::LogicalOrOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* LogicalOrOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new LogicalOrOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* LogicalOrOperator::doClone() const {
            return new LogicalOrOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }

        Value LogicalOrOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(m_leftOperand->evaluate(context) || m_rightOperand->evaluate(context), m_line, m_column);
        }

        void LogicalOrOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand << " || " << *m_rightOperand;
        }

        BinaryOperator::Traits LogicalOrOperator::doGetTraits() const {
            return Traits(2, true, true);
        }

        BitwiseAndOperator::BitwiseAndOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* BitwiseAndOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new BitwiseAndOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* BitwiseAndOperator::doClone() const {
            return new BitwiseAndOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }

        Value BitwiseAndOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(m_leftOperand->evaluate(context) & m_rightOperand->evaluate(context), m_line, m_column);
        }

        void BitwiseAndOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand << " & " << *m_rightOperand;
        }

        BinaryOperator::Traits BitwiseAndOperator::doGetTraits() const {
            return Traits(6, true, true);
        }

        BitwiseXorOperator::BitwiseXorOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* BitwiseXorOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new BitwiseXorOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* BitwiseXorOperator::doClone() const {
            return new BitwiseXorOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }

        Value BitwiseXorOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(m_leftOperand->evaluate(context) ^ m_rightOperand->evaluate(context), m_line, m_column);
        }

        void BitwiseXorOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand << " ^ " << *m_rightOperand;
        }

        BinaryOperator::Traits BitwiseXorOperator::doGetTraits() const {
            return Traits(5, true, true);
        }

        BitwiseOrOperator::BitwiseOrOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* BitwiseOrOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new BitwiseOrOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* BitwiseOrOperator::doClone() const {
            return new BitwiseOrOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }

        Value BitwiseOrOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(m_leftOperand->evaluate(context) | m_rightOperand->evaluate(context), m_line, m_column);
        }

        void BitwiseOrOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand << " | " << *m_rightOperand;
        }

        BinaryOperator::Traits BitwiseOrOperator::doGetTraits() const {
            return Traits(4, true, true);
        }

        BitwiseShiftLeftOperator::BitwiseShiftLeftOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* BitwiseShiftLeftOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new BitwiseShiftLeftOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* BitwiseShiftLeftOperator::doClone() const {
            return new BitwiseShiftLeftOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }

        Value BitwiseShiftLeftOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(m_leftOperand->evaluate(context) << m_rightOperand->evaluate(context), m_line, m_column);
        }

        void BitwiseShiftLeftOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand << " << " << *m_rightOperand;
        }

        BinaryOperator::Traits BitwiseShiftLeftOperator::doGetTraits() const {
            return Traits(9, true, true);
        }

        BitwiseShiftRightOperator::BitwiseShiftRightOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* BitwiseShiftRightOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new BitwiseShiftRightOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* BitwiseShiftRightOperator::doClone() const {
            return new BitwiseShiftRightOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }

        Value BitwiseShiftRightOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(m_leftOperand->evaluate(context) >> m_rightOperand->evaluate(context), m_line, m_column);
        }

        void BitwiseShiftRightOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand << " >> " << *m_rightOperand;
        }

        BinaryOperator::Traits BitwiseShiftRightOperator::doGetTraits() const {
            return Traits(9, true, true);
        }

        ComparisonOperator::ComparisonOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const Op op, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column),
        m_op(op) {}

        ExpressionBase* ComparisonOperator::createLess(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new ComparisonOperator(leftOperand, rightOperand, Op_Less, line, column))->reorderByPrecedence();
        }

        ExpressionBase* ComparisonOperator::createLessOrEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new ComparisonOperator(leftOperand, rightOperand, Op_LessOrEqual, line, column))->reorderByPrecedence();
        }

        ExpressionBase* ComparisonOperator::createEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new ComparisonOperator(leftOperand, rightOperand, Op_Equal, line, column))->reorderByPrecedence();
        }

        ExpressionBase* ComparisonOperator::createInequal(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new ComparisonOperator(leftOperand, rightOperand, Op_Inequal, line, column))->reorderByPrecedence();
        }

        ExpressionBase* ComparisonOperator::createGreaterOrEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new ComparisonOperator(leftOperand, rightOperand, Op_GreaterOrEqual, line, column))->reorderByPrecedence();
        }

        ExpressionBase* ComparisonOperator::createGreater(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new ComparisonOperator(leftOperand, rightOperand, Op_Greater, line, column))->reorderByPrecedence();
        }

        ExpressionBase* ComparisonOperator::doClone() const {
            return new ComparisonOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_op, m_line, m_column);
        }

        Value ComparisonOperator::doEvaluate(const EvaluationContext& context) const {
            switch (m_op) {
                case Op_Less:
                    return Value(m_leftOperand->evaluate(context) < m_rightOperand->evaluate(context), m_line, m_column);
                case Op_LessOrEqual:
                    return Value(m_leftOperand->evaluate(context) <= m_rightOperand->evaluate(context), m_line, m_column);
                case Op_Equal:
                    return Value(m_leftOperand->evaluate(context) == m_rightOperand->evaluate(context), m_line, m_column);
                case Op_Inequal:
                    return Value(m_leftOperand->evaluate(context) != m_rightOperand->evaluate(context), m_line, m_column);
                case Op_GreaterOrEqual:
                    return Value(m_leftOperand->evaluate(context) >= m_rightOperand->evaluate(context), m_line, m_column);
                case Op_Greater:
                    return Value(m_leftOperand->evaluate(context) > m_rightOperand->evaluate(context), m_line, m_column);
                    switchDefault()
            }
        }

        void ComparisonOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand;
            switch (m_op) {
                case Op_Less:
                    str << " < ";
                    break;
                case Op_LessOrEqual:
                    str << " <= ";
                    break;
                case Op_Equal:
                    str << " == ";
                    break;
                case Op_Inequal:
                    str << " != ";
                    break;
                case Op_GreaterOrEqual:
                    str << " >= ";
                    break;
                case Op_Greater:
                    str << " > ";
                    break;
                switchDefault()
            }
            str << *m_rightOperand;
        }

        BinaryOperator::Traits ComparisonOperator::doGetTraits() const {
            switch (m_op) {
                case Op_Less:
                case Op_LessOrEqual:
                case Op_Greater:
                case Op_GreaterOrEqual:
                    return Traits(8, false, false);
                case Op_Equal:
                case Op_Inequal:
                    return Traits(7, true, false);
                    switchDefault()
            }
        }

        RangeOperator::RangeOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}

        ExpressionBase* RangeOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return (new RangeOperator(leftOperand, rightOperand, line, column))->reorderByPrecedence();
        }

        ExpressionBase* RangeOperator::createAutoRangeWithLeftOperand(ExpressionBase* leftOperand, const size_t line, const size_t column) {
            return create(leftOperand, VariableExpression::create(AutoRangeParameterName(), line, column), line, column);
        }

        ExpressionBase* RangeOperator::createAutoRangeWithRightOperand(ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return create(VariableExpression::create(AutoRangeParameterName(), line, column), rightOperand, line, column);
        }

        ExpressionBase* RangeOperator::doClone() const {
            return new RangeOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }

        Value RangeOperator::doEvaluate(const EvaluationContext& context) const {
            const Value leftValue = m_leftOperand->evaluate(context);
            const Value rightValue = m_rightOperand->evaluate(context);

            const long from = static_cast<long>(leftValue.convertTo(ValueType::Number).numberValue());
            const long to = static_cast<long>(rightValue.convertTo(ValueType::Number).numberValue());

            RangeType range;
            if (from <= to) {
                range.reserve(static_cast<size_t>(to - from + 1));
                for (long i = from; i <= to; ++i) {
                    assert(range.capacity() > range.size());
                    range.push_back(i);
                }
            } else if (to < from) {
                range.reserve(static_cast<size_t>(from - to + 1));
                for (long i = from; i >= to; --i) {
                    assert(range.capacity() > range.size());
                    range.push_back(i);
                }
            }
            assert(range.capacity() == range.size());

            return Value(range, m_line, m_column);
        }

        void RangeOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand << ".." << *m_rightOperand;
        }

        BinaryOperator::Traits RangeOperator::doGetTraits() const {
            return Traits(1, false, false);
        }

        CaseOperator::CaseOperator(ExpressionBase* premise, ExpressionBase* conclusion, size_t line, size_t column) :
        BinaryOperator(premise, conclusion, line, column) {}

        ExpressionBase* CaseOperator::create(ExpressionBase* premise, ExpressionBase* conclusion, size_t line, size_t column) {
            return (new CaseOperator(premise, conclusion, line, column))->reorderByPrecedence();
        }

        ExpressionBase* CaseOperator::doClone() const {
            return new CaseOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }

        Value CaseOperator::doEvaluate(const EvaluationContext& context) const {
            const Value premise = m_leftOperand->evaluate(context);
            if (premise.convertTo(ValueType::Boolean))
                return m_rightOperand->evaluate(context);
            return Value::Undefined;
        }

        void CaseOperator::doAppendToStream(std::ostream& str) const {
            str << *m_leftOperand << " -> " << *m_rightOperand;
        }

        BinaryOperator::Traits CaseOperator::doGetTraits() const {
            return Traits(0, false, false);
        }

        SwitchOperator::SwitchOperator(ExpressionBase::List&& cases, size_t line, size_t column) :
        ExpressionBase(line, column),
        m_cases(std::move(cases)) {}

        SwitchOperator::~SwitchOperator() = default;

        ExpressionBase* SwitchOperator::create(ExpressionBase::List&& cases, size_t line, size_t column) {
            return new SwitchOperator(std::move(cases), line, column);
        }

        ExpressionBase* SwitchOperator::doOptimize() {
            for (auto& case_ : m_cases) {
                ExpressionBase* optimized = case_->optimize();

                if (optimized != nullptr && optimized != case_.get()) {
                    const Value result = optimized->evaluate(EvaluationContext());
                    if (!result.undefined()) {
                        return LiteralExpression::create(result, m_line, m_column);
                    }

                    case_.reset(optimized);
                }
            }

            return nullptr;
        }

        ExpressionBase* SwitchOperator::doClone() const {
            ExpressionBase::List caseClones;
            for (const auto& case_ : m_cases) {
                caseClones.emplace_back(case_->clone());
            }
            return new SwitchOperator(std::move(caseClones), m_line, m_column);
        }

        Value SwitchOperator::doEvaluate(const EvaluationContext& context) const {
            for (const auto& case_ : m_cases) {
                const Value result = case_->evaluate(context);
                if (!result.undefined())
                    return result;
            }
            return Value::Undefined;
        }

        void SwitchOperator::doAppendToStream(std::ostream& str) const {
            str << "{{ ";
            size_t i = 0;
            for (const auto& expression : m_cases) {
                str << *expression;
                if (i < m_cases.size() - 1)
                    str << ", ";
                ++i;
            }
            str << " }}";
        }
    }
}
