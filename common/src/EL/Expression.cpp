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
#include "Macros.h"
#include "EL/EvaluationContext.h"

#include <kdl/overload.h>

#include <algorithm>
#include <sstream>
#include <string>

namespace TrenchBroom {
    namespace EL {
        Expression::Expression(LiteralExpression expression, const size_t line, const size_t column) :
        m_expression(std::make_unique<ExpressionVariant>(std::move(expression))),
        m_line(line),
        m_column(column) {}
        
        Expression::Expression(VariableExpression expression, const size_t line, const size_t column) :
        m_expression(std::make_unique<ExpressionVariant>(std::move(expression))),
        m_line(line),
        m_column(column) {}
        
        Expression::Expression(ArrayExpression expression, const size_t line, const size_t column) :
        m_expression(std::make_unique<ExpressionVariant>(std::move(expression))),
        m_line(line),
        m_column(column) {}
        
        Expression::Expression(MapExpression expression, const size_t line, const size_t column) :
        m_expression(std::make_unique<ExpressionVariant>(std::move(expression))),
        m_line(line),
        m_column(column) {}
        
        Expression::Expression(UnaryExpression expression, const size_t line, const size_t column) :
        m_expression(std::make_unique<ExpressionVariant>(std::move(expression))),
        m_line(line),
        m_column(column) {}
        
        Expression::Expression(BinaryExpression expression, const size_t line, const size_t column) :
        m_expression(std::make_unique<ExpressionVariant>(std::move(expression))),
        m_line(line),
        m_column(column) {
            rebalanceByPrecedence();
        }
        
        Expression::Expression(SubscriptExpression expression, const size_t line, const size_t column) :
        m_expression(std::make_unique<ExpressionVariant>(std::move(expression))),
        m_line(line),
        m_column(column) {}
        
        Expression::Expression(SwitchExpression expression, const size_t line, const size_t column) :
        m_expression(std::make_unique<ExpressionVariant>(std::move(expression))),
        m_line(line),
        m_column(column) {}
        
        Expression::Expression(const Expression& other) :
        m_expression(std::make_unique<ExpressionVariant>(*other.m_expression)),
        m_line(other.m_line),
        m_column(other.m_column) {}
        
        Expression::Expression(Expression&& other) noexcept = default;
        
        Expression& Expression::operator=(const Expression& other) {
            if (&other != this) {
                m_expression = std::make_unique<ExpressionVariant>(*other.m_expression);
                m_line = other.m_line;
                m_column = other.m_column;
            }
            
            return *this;
        }

        Expression& Expression::operator=(Expression&& other) noexcept {
            if (&other != this) {
                m_expression = std::move(other.m_expression);
                m_line = other.m_line;
                m_column = other.m_column;
            }

            return *this;
        }

        Expression::~Expression() = default;

        Value Expression::evaluate(const EvaluationContext& context) const {
            return std::visit([&](const auto& e) { return e.evaluate(context); }, *m_expression);
        }

        bool Expression::optimize() {
            auto replacement = std::visit(kdl::overload(
                [](LiteralExpression& e) -> std::optional<LiteralExpression> { return e; },
                [](VariableExpression&)  -> std::optional<LiteralExpression> { return std::nullopt; },
                [](auto& e)              -> std::optional<LiteralExpression> { return e.optimize(); }
            ), *m_expression);
            
            if (replacement) {
                *m_expression = std::move(*replacement);
                return true;
            } else {
                return false;
            }
        }

        size_t Expression::line() const {
            return m_line;
        }
        
        size_t Expression::column() const {
            return m_column;
        }

        std::string Expression::asString() const {
            std::stringstream str;
            str << *this;
            return str.str();
        }

        std::ostream& operator<<(std::ostream& str, const Expression& exp) {
            std::visit([&](const auto& e) { str << e; }, *exp.m_expression);
            return str;
        }
        
        void Expression::rebalanceByPrecedence() {
            /*
             * The expression tree has a similar invariant to a heap: For any given node, its precedence must be less
             * than or equal to the precedences of its children. This guarantees that the tree evaluating the tree in a
             * depth first traversal yields correct results because the nodes with the highest precedence are evaluated
             * before the nodes with lower precedence.
             */
        
            assert(std::holds_alternative<BinaryExpression>(*m_expression));
        
            const auto parentPrecedence = std::get<BinaryExpression>(*m_expression).precedence();
            const auto leftPrecedence = std::get<BinaryExpression>(*m_expression).m_leftOperand.precedence();
            const auto rightPrecedence = std::get<BinaryExpression>(*m_expression).m_rightOperand.precedence();
            
            if (parentPrecedence > std::min(leftPrecedence, rightPrecedence)) {
                if (leftPrecedence < rightPrecedence) {
                    // push this operator into the right subtree, rotating the right node up, and rebalancing the right subtree again
                    Expression leftExpression = std::move(std::get<BinaryExpression>(*m_expression).m_leftOperand);
                    
                    assert(std::holds_alternative<BinaryExpression>(*leftExpression.m_expression));
                    std::get<BinaryExpression>(*m_expression).m_leftOperand = std::move(std::get<BinaryExpression>(*leftExpression.m_expression).m_rightOperand);
                    std::get<BinaryExpression>(*leftExpression.m_expression).m_rightOperand = std::move(*this);
                    *this = std::move(leftExpression);
                    
                    std::get<BinaryExpression>(*m_expression).m_rightOperand.rebalanceByPrecedence();
                } else {
                    // push this operator into the left subtree, rotating the left node up, and rebalancing the left subtree again
                    Expression rightExpression = std::move(std::get<BinaryExpression>(*m_expression).m_rightOperand);
                    
                    assert(std::holds_alternative<BinaryExpression>(*rightExpression.m_expression));
                    std::get<BinaryExpression>(*m_expression).m_rightOperand = std::move(std::get<BinaryExpression>(*rightExpression.m_expression).m_leftOperand);
                    std::get<BinaryExpression>(*rightExpression.m_expression).m_leftOperand = std::move(*this);
                    *this = std::move(rightExpression);
                    
                    std::get<BinaryExpression>(*m_expression).m_leftOperand.rebalanceByPrecedence();
                }
            }
        }
        
        size_t Expression::precedence() const {
            return std::visit(kdl::overload(
                [](const BinaryExpression& exp) -> size_t {
                    return exp.precedence();
                },
                [](const auto&) -> size_t {
                    return 13u;
                }
            ), *m_expression);
        }

        LiteralExpression::LiteralExpression(Value value) :
        m_value(std::move(value)) {}
        
        const Value& LiteralExpression::evaluate(const EvaluationContext&) const {
            return m_value;
        }
        
        std::ostream& operator<<(std::ostream& str, const LiteralExpression& exp) {
            str << exp.m_value;
            return str;
        }

        VariableExpression::VariableExpression(std::string variableName) :
        m_variableName(std::move(variableName)) {}
        
        Value VariableExpression::evaluate(const EvaluationContext& context) const {
            return context.variableValue(m_variableName);
        }
        
        std::ostream& operator<<(std::ostream& str, const VariableExpression& exp) {
            str << exp.m_variableName;
            return str;
        }

        ArrayExpression::ArrayExpression(std::vector<Expression> elements) :
        m_elements(std::move(elements)) {}
        
        Value ArrayExpression::evaluate(const EvaluationContext& context) const {
            ArrayType array;
            array.reserve(m_elements.size());
            for (const auto& element : m_elements) {
                auto value = element.evaluate(context);
                if (value.type() == ValueType::Range) {
                    const auto& range = value.rangeValue();
                    if (!range.empty()) {
                        array.reserve(array.size() + range.size() - 1u);
                        for (size_t i = 0u; i < range.size(); ++i) {
                            array.emplace_back(range[i], value.line(), value.column());
                        }
                    }
                } else {
                    array.push_back(std::move(value));
                }
            }
            
            return Value(std::move(array));
        }
        
        std::optional<LiteralExpression> ArrayExpression::optimize() {
            bool allOptimized = true;
            for (auto& expression : m_elements) {
                allOptimized &= expression.optimize();
            }
            
            if (allOptimized) {
                return LiteralExpression(evaluate(EvaluationContext()));
            } else {
                return std::nullopt;
            }
        }

        std::ostream& operator<<(std::ostream& str, const ArrayExpression& exp) {
            str << "[ ";
            size_t i = 0u;
            for (const auto& expression : exp.m_elements) {
                str << expression;
                if (i < exp.m_elements.size() - 1u) {
                    str << ", ";
                }
                ++i;
            }
            str << " ]";
            return str;
        }
        
        MapExpression::MapExpression(std::map<std::string, Expression> elements) :
        m_elements(std::move(elements)) {}

        Value MapExpression::evaluate(const EvaluationContext& context) const {
            MapType map;
            for (const auto& [key, expression] : m_elements) {
                map.insert(std::make_pair(key, expression.evaluate(context)));
            }

            return Value(std::move(map));
        }
        
        std::optional<LiteralExpression> MapExpression::optimize() {
            bool allOptimized = true;
            
            for (auto& [key, expression] : m_elements) {
                allOptimized &= expression.optimize();
            }
            
            if (allOptimized) {
                return LiteralExpression(evaluate(EvaluationContext()));
            } else {
                return std::nullopt;
            }
        }

        std::ostream& operator<<(std::ostream& str, const MapExpression& exp) {
            str << "{ ";
            size_t i = 0u;
            for (const auto& [key, expression] : exp.m_elements) {
                str << "\"" << key << "\": " << expression;
                if (i < exp.m_elements.size() - 1u) {
                    str << ", ";
                }
                ++i;
            }
            str << " }";
            return str;
        }

        UnaryExpression::UnaryExpression(UnaryOperator i_operator, Expression operand) :
        m_operator(i_operator),
        m_operand(std::move(operand)) {}

        Value UnaryExpression::evaluate(const EvaluationContext& context) const {
            switch (m_operator) {
                case UnaryOperator::Plus:
                    return Value(+m_operand.evaluate(context));
                case UnaryOperator::Minus:
                    return Value(-m_operand.evaluate(context));
                case UnaryOperator::LogicalNegation:
                    return Value(!m_operand.evaluate(context));
                case UnaryOperator::BitwiseNegation:
                    return Value(~m_operand.evaluate(context));
                case UnaryOperator::Group:
                    return Value(m_operand.evaluate(context));
                switchDefault();
            }
        }
        
        std::optional<LiteralExpression> UnaryExpression::optimize() {
            if (m_operand.optimize()) {
                return LiteralExpression(evaluate(EvaluationContext()));
            } else {
                return std::nullopt;
            }
        }

        std::ostream& operator<<(std::ostream& str, const UnaryExpression& exp) {
            switch (exp.m_operator) {
                case UnaryOperator::Plus:
                    str << "+" << exp.m_operand;
                    break;
                case UnaryOperator::Minus:
                    str << "-" << exp.m_operand;
                    break;
                case UnaryOperator::LogicalNegation:
                    str << "!" << exp.m_operand;
                    break;
                case UnaryOperator::BitwiseNegation:
                    str << "~" << exp.m_operand;
                    break;
                case UnaryOperator::Group:
                    str << "( " << exp.m_operand << " )";
                    break;
                switchDefault();
            }
            return str;
        }

        BinaryExpression::BinaryExpression(BinaryOperator i_operator, Expression leftOperand, Expression rightOperand) :
        m_operator(i_operator),
        m_leftOperand(std::move(leftOperand)),
        m_rightOperand(std::move(rightOperand)) {}

        Expression BinaryExpression::createAutoRangeWithRightOperand(Expression rightOperand, const size_t line, const size_t column) {
            auto leftOperand = Expression(VariableExpression(SubscriptExpression::AutoRangeParameterName()), line, column);
            return EL::Expression(BinaryExpression(BinaryOperator::Range, std::move(leftOperand), std::move(rightOperand)), line, column);
        }
        
        Expression BinaryExpression::createAutoRangeWithLeftOperand(Expression leftOperand, const size_t line, const size_t column) {
            auto rightOperand = Expression(VariableExpression(SubscriptExpression::AutoRangeParameterName()), line, column);
            return EL::Expression(BinaryExpression(BinaryOperator::Range, std::move(leftOperand), std::move(rightOperand)), line, column);
        }

        Value BinaryExpression::evaluate(const EvaluationContext& context) const {
            switch (m_operator) {
                case BinaryOperator::Addition:
                    return Value(m_leftOperand.evaluate(context) + m_rightOperand.evaluate(context));
                case BinaryOperator::Subtraction:
                    return Value(m_leftOperand.evaluate(context) - m_rightOperand.evaluate(context));
                case BinaryOperator::Multiplication:
                    return Value(m_leftOperand.evaluate(context) * m_rightOperand.evaluate(context));
                case BinaryOperator::Division:
                    return Value(m_leftOperand.evaluate(context) / m_rightOperand.evaluate(context));
                case BinaryOperator::Modulus:
                    return Value(m_leftOperand.evaluate(context) % m_rightOperand.evaluate(context));
                case BinaryOperator::LogicalAnd:
                    return Value(m_leftOperand.evaluate(context) && m_rightOperand.evaluate(context));
                case BinaryOperator::LogicalOr:
                    return Value(m_leftOperand.evaluate(context) || m_rightOperand.evaluate(context));
                case BinaryOperator::BitwiseAnd:
                    return Value(m_leftOperand.evaluate(context) & m_rightOperand.evaluate(context));
                case BinaryOperator::BitwiseXOr:
                    return Value(m_leftOperand.evaluate(context) ^ m_rightOperand.evaluate(context));
                case BinaryOperator::BitwiseOr:
                    return Value(m_leftOperand.evaluate(context) | m_rightOperand.evaluate(context));
                case BinaryOperator::BitwiseShiftLeft:
                    return Value(m_leftOperand.evaluate(context) << m_rightOperand.evaluate(context));
                case BinaryOperator::BitwiseShiftRight:
                    return Value(m_leftOperand.evaluate(context) >> m_rightOperand.evaluate(context));
                case BinaryOperator::Less:
                    return Value(m_leftOperand.evaluate(context) < m_rightOperand.evaluate(context));
                case BinaryOperator::LessOrEqual:
                    return Value(m_leftOperand.evaluate(context) <= m_rightOperand.evaluate(context));
                case BinaryOperator::Greater:
                    return Value(m_leftOperand.evaluate(context) > m_rightOperand.evaluate(context));
                case BinaryOperator::GreaterOrEqual:
                    return Value(m_leftOperand.evaluate(context) >= m_rightOperand.evaluate(context));
                case BinaryOperator::Equal:
                    return Value(m_leftOperand.evaluate(context) == m_rightOperand.evaluate(context));
                case BinaryOperator::NotEqual:
                    return Value(m_leftOperand.evaluate(context) != m_rightOperand.evaluate(context));
                case BinaryOperator::Range: {
                    const auto leftValue = m_leftOperand.evaluate(context);
                    const auto rightValue = m_rightOperand.evaluate(context);
                    
                    const auto from = static_cast<long>(leftValue.convertTo(ValueType::Number).numberValue());
                    const auto to = static_cast<long>(rightValue.convertTo(ValueType::Number).numberValue());
                    
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

                    return Value(std::move(range));
                }
                case BinaryOperator::Case: {
                    const auto leftValue = m_leftOperand.evaluate(context);
                    if (leftValue.convertTo(ValueType::Boolean)) {
                        return m_rightOperand.evaluate(context);
                    } else {
                        return Value::Undefined;
                    }
                }
                switchDefault();
            };
        }
        
        std::optional<LiteralExpression> BinaryExpression::optimize() {
            const auto leftOptimized = m_leftOperand.optimize();
            const auto rightOptimized = m_rightOperand.optimize();
            if (leftOptimized && rightOptimized) {
                return LiteralExpression(evaluate(EvaluationContext()));
            } else {
                return std::nullopt;
            }
        }

        size_t BinaryExpression::precedence() const {
            switch (m_operator) {
                case BinaryOperator::Multiplication:
                case BinaryOperator::Division:
                case BinaryOperator::Modulus:
                    return 12;
                case BinaryOperator::Addition:
                case BinaryOperator::Subtraction:
                    return 11;
                case BinaryOperator::BitwiseShiftLeft:
                case BinaryOperator::BitwiseShiftRight:
                    return 10;
                case BinaryOperator::Less:
                case BinaryOperator::LessOrEqual:
                case BinaryOperator::Greater:
                case BinaryOperator::GreaterOrEqual:
                    return 9;
                case BinaryOperator::Equal:
                case BinaryOperator::NotEqual:
                    return 8;
                case BinaryOperator::BitwiseAnd:
                    return 7;
                case BinaryOperator::BitwiseXOr:
                    return 6;
                case BinaryOperator::BitwiseOr:
                    return 5;
                case BinaryOperator::LogicalAnd:
                    return 4;
                case BinaryOperator::LogicalOr:
                    return 3;
                case BinaryOperator::Range:
                    return 2;
                case BinaryOperator::Case:
                    return 1;
                switchDefault();
            };
        }

        std::ostream& operator<<(std::ostream& str, const BinaryExpression& exp) {
            switch (exp.m_operator) {
                case BinaryOperator::Addition:
                    str << exp.m_leftOperand << " + " << exp.m_rightOperand;
                    break;
                case BinaryOperator::Subtraction:
                    str << exp.m_leftOperand << " - " << exp.m_rightOperand;
                    break;
                case BinaryOperator::Multiplication:
                    str << exp.m_leftOperand << " * " << exp.m_rightOperand;
                    break;
                case BinaryOperator::Division:
                    str << exp.m_leftOperand << " / " << exp.m_rightOperand;
                    break;
                case BinaryOperator::Modulus:
                    str << exp.m_leftOperand << " % " << exp.m_rightOperand;
                    break;
                case BinaryOperator::LogicalAnd:
                    str << exp.m_leftOperand << " && " << exp.m_rightOperand;
                    break;
                case BinaryOperator::LogicalOr:
                    str << exp.m_leftOperand << " || " << exp.m_rightOperand;
                    break;
                case BinaryOperator::BitwiseAnd:
                    str << exp.m_leftOperand << " & " << exp.m_rightOperand;
                    break;
                case BinaryOperator::BitwiseXOr:
                    str << exp.m_leftOperand << " ^ " << exp.m_rightOperand;
                    break;
                case BinaryOperator::BitwiseOr:
                    str << exp.m_leftOperand << " | " << exp.m_rightOperand;
                    break;
                case BinaryOperator::BitwiseShiftLeft:
                    str << exp.m_leftOperand << " << " << exp.m_rightOperand;
                    break;
                case BinaryOperator::BitwiseShiftRight:
                    str << exp.m_leftOperand << " >> " << exp.m_rightOperand;
                    break;
                case BinaryOperator::Less:
                    str << exp.m_leftOperand << " < " << exp.m_rightOperand;
                    break;
                case BinaryOperator::LessOrEqual:
                    str << exp.m_leftOperand << " <= " << exp.m_rightOperand;
                    break;
                case BinaryOperator::Greater:
                    str << exp.m_leftOperand << " > " << exp.m_rightOperand;
                    break;
                case BinaryOperator::GreaterOrEqual:
                    str << exp.m_leftOperand << " >= " << exp.m_rightOperand;
                    break;
                case BinaryOperator::Equal:
                    str << exp.m_leftOperand << " == " << exp.m_rightOperand;
                    break;
                case BinaryOperator::NotEqual:
                    str << exp.m_leftOperand << " != " << exp.m_rightOperand;
                    break;
                case BinaryOperator::Range:
                    str << exp.m_leftOperand << ".." << exp.m_rightOperand;
                    break;
                case BinaryOperator::Case:
                    str << exp.m_leftOperand << " -> " << exp.m_rightOperand;
                    break;
                switchDefault();
            };
            return str;
        }

        const std::string& SubscriptExpression::AutoRangeParameterName() {
            static const std::string Name = "__AutoRangeParameter";
            return Name;
        }

        SubscriptExpression::SubscriptExpression(Expression leftOperand, Expression rightOperand) :
        m_leftOperand(std::move(leftOperand)),
        m_rightOperand(std::move(rightOperand)) {}
        
        Value SubscriptExpression::evaluate(const EvaluationContext& context) const {
            const auto leftValue = m_leftOperand.evaluate(context);
            
            EvaluationStack stack(context);
            stack.declareVariable(AutoRangeParameterName(), Value(leftValue.length() - 1u));
            
            const auto rightValue = m_rightOperand.evaluate(stack);
            return leftValue[rightValue];
        }
        
        std::optional<LiteralExpression> SubscriptExpression::optimize() {
            if (m_leftOperand.optimize() && m_rightOperand.optimize()) {
                return LiteralExpression(evaluate(EvaluationContext()));
            } else {
                return std::nullopt;
            }
        }

        std::ostream& operator<<(std::ostream& str, const SubscriptExpression& exp) {
            str << exp.m_leftOperand << "[" << exp.m_rightOperand << "]";
            return str;
        }

        SwitchExpression::SwitchExpression(std::vector<Expression> cases) :
        m_cases(std::move(cases)) {}

        Value SwitchExpression::evaluate(const EvaluationContext& context) const {
            for (const auto& case_ : m_cases) {
                Value result = case_.evaluate(context);
                if (!result.undefined()) {
                    return result;
                }
            }
            return Value::Undefined;
        }
        
        std::optional<LiteralExpression> SwitchExpression::optimize() {
            bool allOptimized = true;
            
            for (auto& case_ : m_cases) {
                allOptimized &= case_.optimize();
                if (allOptimized) {
                    auto result = case_.evaluate(EvaluationContext());
                    if (!result.undefined()) {
                        return LiteralExpression(std::move(result));
                    }
                }
            }
            
            return std::nullopt;
        }

        std::ostream& operator<<(std::ostream& str, const SwitchExpression& exp) {
            str << "{{ ";
            size_t i = 0u;
            for (const auto& case_ : exp.m_cases) {
                str << case_;
                if (i < exp.m_cases.size() - 1u) {
                    str << ", ";
                }
                ++i;
            }
            str << " }}";
            return str;
        }
    }
}
