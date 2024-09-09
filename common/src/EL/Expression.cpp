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

#include "EL/EvaluationContext.h"
#include "EL/Expressions.h"
#include "Macros.h"

#include <sstream>

namespace TrenchBroom::EL
{

Expression::Expression(
  std::unique_ptr<ExpressionImpl> expression, std::optional<FileLocation> location)
  : m_expression{std::move(expression)}
  , m_location{std::move(location)}
{
}

Expression::Expression(LiteralExpression expression, std::optional<FileLocation> location)
  : m_expression{std::make_shared<LiteralExpression>(std::move(expression))}
  , m_location{std::move(location)}
{
}

Expression::Expression(
  VariableExpression expression, std::optional<FileLocation> location)
  : m_expression{std::make_shared<VariableExpression>(std::move(expression))}
  , m_location{std::move(location)}
{
}

Expression::Expression(ArrayExpression expression, std::optional<FileLocation> location)
  : m_expression{std::make_shared<ArrayExpression>(std::move(expression))}
  , m_location{std::move(location)}
{
}

Expression::Expression(MapExpression expression, std::optional<FileLocation> location)
  : m_expression{std::make_shared<MapExpression>(std::move(expression))}
  , m_location{std::move(location)}
{
}

Expression::Expression(UnaryExpression expression, std::optional<FileLocation> location)
  : m_expression{std::make_shared<UnaryExpression>(std::move(expression))}
  , m_location{std::move(location)}
{
}

Expression::Expression(BinaryExpression expression, std::optional<FileLocation> location)
  : m_expression{std::make_shared<BinaryExpression>(std::move(expression))}
  , m_location{std::move(location)}
{
  rebalanceByPrecedence();
}

Expression::Expression(
  SubscriptExpression expression, std::optional<FileLocation> location)
  : m_expression{std::make_shared<SubscriptExpression>(std::move(expression))}
  , m_location{std::move(location)}
{
}

Expression::Expression(SwitchExpression expression, std::optional<FileLocation> location)
  : m_expression{std::make_shared<SwitchExpression>(std::move(expression))}
  , m_location{std::move(location)}
{
}

Value Expression::evaluate(const EvaluationContext& context) const
{
  return Value{m_expression->evaluate(context), *this};
}

Expression Expression::optimize() const
{
  return Expression{m_expression->optimize(), m_location};
}

const std::optional<FileLocation>& Expression::location() const
{
  return m_location;
}

std::string Expression::asString() const
{
  auto str = std::stringstream{};
  str << *this;
  return str.str();
}

bool operator==(const Expression& lhs, const Expression& rhs)
{
  return *lhs.m_expression == *rhs.m_expression;
}

bool operator!=(const Expression& lhs, const Expression& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const Expression& rhs)
{
  lhs << *rhs.m_expression;
  return lhs;
}

void Expression::rebalanceByPrecedence()
{
  /*
   * The expression tree has a similar invariant to a heap: For any given node, its
   * precedence must be less than or equal to the precedences of its children. This
   * guarantees that evaluating the tree in a depth first traversal yields correct results
   * because the nodes with the highest precedence are evaluated before the nodes with
   * lower precedence.
   */

  assert(dynamic_cast<BinaryExpression*>(m_expression.get()) != nullptr);

  const auto parentPrecedence =
    dynamic_cast<BinaryExpression*>(m_expression.get())->precedence();
  const auto leftPrecedence =
    dynamic_cast<BinaryExpression*>(m_expression.get())->m_leftOperand.precedence();
  const auto rightPrecedence =
    dynamic_cast<BinaryExpression*>(m_expression.get())->m_rightOperand.precedence();

  if (parentPrecedence > std::min(leftPrecedence, rightPrecedence))
  {
    if (leftPrecedence < rightPrecedence)
    {
      // push this operator into the right subtree, rotating the right node up, and
      // rebalancing the right subtree again
      auto leftExpression =
        std::move(dynamic_cast<BinaryExpression*>(m_expression.get())->m_leftOperand);

      assert(
        dynamic_cast<BinaryExpression*>(leftExpression.m_expression.get()) != nullptr);
      dynamic_cast<BinaryExpression*>(m_expression.get())->m_leftOperand =
        std::move(dynamic_cast<BinaryExpression*>(leftExpression.m_expression.get())
                    ->m_rightOperand);
      dynamic_cast<BinaryExpression*>(leftExpression.m_expression.get())->m_rightOperand =
        std::move(*this);
      *this = std::move(leftExpression);

      dynamic_cast<BinaryExpression*>(m_expression.get())
        ->m_rightOperand.rebalanceByPrecedence();
    }
    else
    {
      // push this operator into the left subtree, rotating the left node up, and
      // rebalancing the left subtree again
      auto rightExpression =
        std::move(dynamic_cast<BinaryExpression*>(m_expression.get())->m_rightOperand);

      assert(
        dynamic_cast<BinaryExpression*>(rightExpression.m_expression.get()) != nullptr);
      dynamic_cast<BinaryExpression*>(m_expression.get())->m_rightOperand =
        std::move(dynamic_cast<BinaryExpression*>(rightExpression.m_expression.get())
                    ->m_leftOperand);
      dynamic_cast<BinaryExpression*>(rightExpression.m_expression.get())->m_leftOperand =
        std::move(*this);
      *this = std::move(rightExpression);

      dynamic_cast<BinaryExpression*>(m_expression.get())
        ->m_leftOperand.rebalanceByPrecedence();
    }
  }
}

size_t Expression::precedence() const
{
  return m_expression->precedence();
}

} // namespace TrenchBroom::EL
