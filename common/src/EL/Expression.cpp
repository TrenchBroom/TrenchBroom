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

#include "Expression.h"

#include "CollectionUtils.h"
#include "EL/EvaluationContext.h"

namespace TrenchBroom {
    namespace EL {
        Expression::Expression(ExpressionBase* expression) :
        m_expression(expression) {
            assert(m_expression.get() != NULL);
        }
        
        bool Expression::optimize() {
            ExpressionBase* optimized = m_expression->optimize();
            if (optimized != NULL && optimized != m_expression.get()) {
                m_expression.reset(optimized);
                return true;
            }
            return false;
        }
        
        Value Expression::evaluate(const EvaluationContext& context) const {
            return m_expression->evaluate(context);
        }
        
        void ExpressionBase::replaceExpression(ExpressionBase*& oldExpression, ExpressionBase* newExpression) {
            if (newExpression != NULL && newExpression != oldExpression) {
                delete oldExpression;
                oldExpression = newExpression;
            }
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
        
        Value LiteralExpression::doEvaluate(const EvaluationContext& context) const {
            return m_value;
        }
        
        VariableExpression::VariableExpression(const String& variableName, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_variableName(variableName) {}
        
        ExpressionBase* VariableExpression::create(const String& variableName, const size_t line, const size_t column) {
            return new VariableExpression(variableName, line, column);
        }
        
        ExpressionBase* VariableExpression::doClone() const {
            return new VariableExpression(m_variableName, m_line, m_column);
        }
        
        ExpressionBase* VariableExpression::doOptimize() {
            return NULL;
        }
        
        Value VariableExpression::doEvaluate(const EvaluationContext& context) const {
            return context.variableValue(m_variableName);
        }
        
        ArrayExpression::ArrayExpression(const ExpressionBase::List& elements, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_elements(elements) {}
        
        ExpressionBase* ArrayExpression::create(const ExpressionBase::List& elements, const size_t line, const size_t column) {
            return new ArrayExpression(elements, line, column);
        }
        
        ArrayExpression::~ArrayExpression() {
            ListUtils::clearAndDelete(m_elements);
        }
        
        ExpressionBase* ArrayExpression::doClone() const {
            ExpressionBase::List clones;
            ExpressionBase::List::const_iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                const ExpressionBase* element = *it;
                clones.push_back(element->clone());
            }
            
            return new ArrayExpression(clones, m_line, m_column);
        }
        
        ExpressionBase* ArrayExpression::doOptimize() {
            bool allOptimized = true;
            
            ExpressionBase::List::iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                ExpressionBase*& expression = *it;
                ExpressionBase* optimized = expression->optimize();
                replaceExpression(expression, optimized);
                allOptimized &= optimized != NULL;
            }
            
            if (allOptimized)
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);
            
            return NULL;
        }
        
        Value ArrayExpression::doEvaluate(const EvaluationContext& context) const {
            ArrayType array;
            ExpressionBase::List::const_iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                const ExpressionBase* element = *it;
                const Value value = element->evaluate(context);
                if (value.type() == Type_Range) {
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
        
        MapExpression::MapExpression(const ExpressionBase::Map& elements, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_elements(elements) {}
        
        ExpressionBase* MapExpression::create(const ExpressionBase::Map& elements, const size_t line, const size_t column) {
            return new MapExpression(elements, line, column);
        }
        
        MapExpression::~MapExpression() {
            MapUtils::clearAndDelete(m_elements);
        }
        
        ExpressionBase* MapExpression::doClone() const {
            ExpressionBase::Map clones;
            ExpressionBase::Map::const_iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                const String& key = it->first;
                const ExpressionBase* value = it->second;
                clones.insert(std::make_pair(key, value->clone()));
            }
            
            return new MapExpression(clones, m_line, m_column);
        }
        
        
        ExpressionBase* MapExpression::doOptimize() {
            bool allOptimized = true;
            
            ExpressionBase::Map::iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                ExpressionBase*& expression = it->second;
                ExpressionBase* optimized = expression->optimize();
                replaceExpression(expression, optimized);
                allOptimized &= optimized != NULL;
            }
            
            if (allOptimized)
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);
            
            return NULL;
        }
        
        Value MapExpression::doEvaluate(const EvaluationContext& context) const {
            MapType map;
            ExpressionBase::Map::const_iterator it, end;
            for (it = m_elements.begin(), end = m_elements.end(); it != end; ++it) {
                const String& key = it->first;
                const ExpressionBase* expression = it->second;
                map.insert(std::make_pair(key, expression->evaluate(context)));
            }
            
            return Value(map, m_line, m_column);
        }
        
        UnaryOperator::UnaryOperator(ExpressionBase* operand, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_operand(operand) {
            assert(m_operand != NULL);
        }
        
        UnaryOperator::~UnaryOperator() {
            delete m_operand;
        }
        
        ExpressionBase* UnaryOperator::doOptimize() {
            ExpressionBase* optimized = m_operand->optimize();
            replaceExpression(m_operand, optimized);
            
            if (optimized != NULL)
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);
            
            return NULL;
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
        
        NegationOperator::NegationOperator(ExpressionBase* operand, const size_t line, const size_t column) :
        UnaryOperator(operand, line, column) {}
        
        ExpressionBase* NegationOperator::create(ExpressionBase* operand, const size_t line, const size_t column) {
            return new NegationOperator(operand, line, column);
        }
        
        ExpressionBase* NegationOperator::doClone() const  {
            return new NegationOperator(m_operand->clone(), m_line, m_column);
        }
        
        Value NegationOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(!m_operand->evaluate(context), m_line, m_column);
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
        
        SubscriptOperator::SubscriptOperator(ExpressionBase* indexableOperand, ExpressionBase* indexOperand, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_indexableOperand(indexableOperand),
        m_indexOperand(indexOperand) {
            assert(m_indexableOperand != NULL);
            assert(m_indexOperand != NULL);
        }
        
        SubscriptOperator::~SubscriptOperator() {
            delete m_indexableOperand;
            delete m_indexOperand;
        }
        
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
            
            if (indexableOptimized != NULL && indexOptimized != NULL)
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);
            
            return NULL;
        }
        
        Value SubscriptOperator::doEvaluate(const EvaluationContext& context) const {
            const Value indexableValue = m_indexableOperand->evaluate(context);
            
            EvaluationStack stack(context);
            stack.declareVariable(RangeOperator::AutoRangeParameterName(), Value(indexableValue.length()-1, m_line, m_column));
            const Value indexValue = m_indexOperand->evaluate(stack);
            
            return indexableValue[indexValue];
        }
        
        BinaryOperator::BinaryOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        ExpressionBase(line, column),
        m_leftOperand(leftOperand),
        m_rightOperand(rightOperand) {
            assert(m_leftOperand != NULL);
            assert(m_rightOperand != NULL);
        }
        
        BinaryOperator::~BinaryOperator() {
            delete m_leftOperand;
            delete m_rightOperand;
        }
        
        ExpressionBase* BinaryOperator::doReorderByPrecedence() {
            ExpressionBase* result = m_leftOperand->reorderByPrecedence(this);
            if (result == this)
                result = m_rightOperand->reorderByPrecedence(this);
            return result;
        }
        
        ExpressionBase* BinaryOperator::doReorderByPrecedence(BinaryOperator* parent) {
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
        
        ExpressionBase* BinaryOperator::doOptimize() {
            ExpressionBase* leftOptimized = m_leftOperand->optimize();
            ExpressionBase* rightOptimized = m_rightOperand->optimize();
            
            replaceExpression(m_leftOperand, leftOptimized);
            replaceExpression(m_rightOperand, rightOptimized);
            
            if (leftOptimized != NULL && rightOptimized != NULL)
                return LiteralExpression::create(evaluate(EvaluationContext()), m_line, m_column);
            
            return NULL;
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
        
        BinaryOperator::Traits AdditionOperator::doGetTraits() const {
            return Traits(5, true, true);
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
        
        BinaryOperator::Traits SubtractionOperator::doGetTraits() const {
            return Traits(5, false, false);
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
        
        BinaryOperator::Traits MultiplicationOperator::doGetTraits() const {
            return Traits(6, true, true);
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
        
        BinaryOperator::Traits DivisionOperator::doGetTraits() const {
            return Traits(6, false, false);
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
        
        BinaryOperator::Traits ModulusOperator::doGetTraits() const {
            return Traits(6, false, false);
        }
        
        const String& RangeOperator::AutoRangeParameterName() {
            static const String Name = "__AutoRangeParameter";
            return Name;
        }
        
        ConjunctionOperator::ConjunctionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}
        
        ExpressionBase* ConjunctionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new ConjunctionOperator(leftOperand, rightOperand, line, column);
        }
        
        ExpressionBase* ConjunctionOperator::doClone() const {
            return new ConjunctionOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }
        
        Value ConjunctionOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(m_leftOperand->evaluate(context) && m_rightOperand->evaluate(context), m_line, m_column);
        }
        
        BinaryOperator::Traits ConjunctionOperator::doGetTraits() const {
            return Traits(3, true, true);
        }
        
        DisjunctionOperator::DisjunctionOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column) {}
        
        ExpressionBase* DisjunctionOperator::create(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new DisjunctionOperator(leftOperand, rightOperand, line, column);
        }
        
        ExpressionBase* DisjunctionOperator::doClone() const {
            return new DisjunctionOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }
        
        Value DisjunctionOperator::doEvaluate(const EvaluationContext& context) const {
            return Value(m_leftOperand->evaluate(context) || m_rightOperand->evaluate(context), m_line, m_column);
        }
        
        BinaryOperator::Traits DisjunctionOperator::doGetTraits() const {
            return Traits(2, true, true);
        }
        
        ComparisonOperator::ComparisonOperator(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const Op op, const size_t line, const size_t column) :
        BinaryOperator(leftOperand, rightOperand, line, column),
        m_op(op) {}
        
        ExpressionBase* ComparisonOperator::createLess(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_Less, line, column);
        }
        
        ExpressionBase* ComparisonOperator::createLessOrEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_LessOrEqual, line, column);
        }
        
        ExpressionBase* ComparisonOperator::createEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_Equal, line, column);
        }
        
        ExpressionBase* ComparisonOperator::createInequal(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_Inequal, line, column);
        }
        
        ExpressionBase* ComparisonOperator::createGreaterOrEqual(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_GreaterOrEqual, line, column);
        }
        
        ExpressionBase* ComparisonOperator::createGreater(ExpressionBase* leftOperand, ExpressionBase* rightOperand, const size_t line, const size_t column) {
            return new ComparisonOperator(leftOperand, rightOperand, Op_Greater, line, column);
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
        
        BinaryOperator::Traits ComparisonOperator::doGetTraits() const {
            switch (m_op) {
                case Op_Less:
                case Op_LessOrEqual:
                case Op_Greater:
                case Op_GreaterOrEqual:
                    return Traits(4, false, false);
                case Op_Equal:
                case Op_Inequal:
                    return Traits(4, true, false);
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
            
            const long from = static_cast<long>(leftValue.convertTo(Type_Number).numberValue());
            const long to = static_cast<long>(rightValue.convertTo(Type_Number).numberValue());
            
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
        
        BinaryOperator::Traits RangeOperator::doGetTraits() const {
            return Traits(1, false, false);
        }

        CaseOperator::CaseOperator(ExpressionBase* premise, ExpressionBase* conclusion, size_t line, size_t column) :
        BinaryOperator(premise, conclusion, line, column) {}

        ExpressionBase* CaseOperator::create(ExpressionBase* premise, ExpressionBase* conclusion, size_t line, size_t column) {
            return new CaseOperator(premise, conclusion, line, column);
        }

        ExpressionBase* CaseOperator::doClone() const {
            return new CaseOperator(m_leftOperand->clone(), m_rightOperand->clone(), m_line, m_column);
        }
        
        Value CaseOperator::doEvaluate(const EvaluationContext& context) const {
            const Value premise = m_leftOperand->evaluate(context);
            if (premise.convertTo(Type_Boolean))
                return m_rightOperand->evaluate(context);
            return Value::Undefined;
        }
        
        BinaryOperator::Traits CaseOperator::doGetTraits() const {
            return Traits(0, false, false);
        }

        SwitchOperator::SwitchOperator(const ExpressionBase::List& cases, size_t line, size_t column) :
        ExpressionBase(line, column),
        m_cases(cases) {}

        SwitchOperator::~SwitchOperator() {
            ListUtils::clearAndDelete(m_cases);
        }
        
        ExpressionBase* SwitchOperator::create(const ExpressionBase::List& cases, size_t line, size_t column) {
            return new SwitchOperator(cases, line, column);
        }

        ExpressionBase* SwitchOperator::doOptimize() {
            ExpressionBase::List::const_iterator it, end;
            for (it = m_cases.begin(), end = m_cases.end(); it != end; ++it) {
                ExpressionBase* case_ = *it;
                ExpressionBase* optimized = case_->optimize();
                
                if (optimized != NULL && optimized != case_) {
                    const Value result = optimized->evaluate(EvaluationContext());
                    if (!result.undefined())
                        return LiteralExpression::create(result, m_line, m_column);
                    
                    delete case_;
                    case_ = optimized;
                }
            }
            
            return NULL;
        }

        ExpressionBase* SwitchOperator::doClone() const {
            ExpressionBase::List caseClones;
            ExpressionBase::List::const_iterator it, end;
            for (it = m_cases.begin(), end = m_cases.end(); it != end; ++it)
                caseClones.push_back((*it)->clone());
            return new SwitchOperator(caseClones, m_line, m_column);
        }
        
        Value SwitchOperator::doEvaluate(const EvaluationContext& context) const {
            ExpressionBase::List::const_iterator it, end;
            for (it = m_cases.begin(), end = m_cases.end(); it != end; ++it) {
                const ExpressionBase* case_ = *it;
                const Value result = case_->evaluate(context);
                if (!result.undefined())
                    return result;
            }
            return Value::Undefined;
        }
    }
}
