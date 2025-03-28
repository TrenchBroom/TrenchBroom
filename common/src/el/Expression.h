/*
 Copyright (C) 2021 Kristian Duske

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

#pragma once

#include "FileLocation.h"
#include "el/EL_Forward.h"
#include "el/Value.h"

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace tb::el
{

struct LiteralExpression;
struct VariableExpression;
struct ArrayExpression;
struct MapExpression;
struct UnaryExpression;
struct BinaryExpression;
struct SubscriptExpression;
struct SwitchExpression;

using Expression = std::variant<
  LiteralExpression,
  VariableExpression,
  ArrayExpression,
  MapExpression,
  UnaryExpression,
  BinaryExpression,
  SubscriptExpression,
  SwitchExpression>;

std::ostream& operator<<(std::ostream& lhs, const Expression& rhs);

template <typename Visitor, typename Enable = void>
struct VisitorResultType
{
  using type = std::invoke_result_t<Visitor, const LiteralExpression&>;
};

template <typename Visitor>
struct VisitorResultType<
  Visitor,
  typename std::enable_if_t<std::is_invocable_v<
    Visitor,
    const Visitor&,
    const LiteralExpression&,
    const ExpressionNode&>>>
{
  using type = std::invoke_result_t<
    Visitor,
    const Visitor&,
    const LiteralExpression&,
    const ExpressionNode&>;
};

template <typename Visitor>
struct VisitorResultType<
  Visitor,
  typename std::enable_if_t<
    std::is_invocable_v<Visitor, const Visitor&, const LiteralExpression&>>>
{
  using type = std::invoke_result_t<Visitor, const Visitor&, const LiteralExpression&>;
};

template <typename Visitor>
struct VisitorResultType<
  Visitor,
  typename std::enable_if_t<
    std::is_invocable_v<Visitor, const LiteralExpression&, const ExpressionNode&>>>
{
  using type =
    std::invoke_result_t<Visitor, const LiteralExpression&, const ExpressionNode&>;
};

template <typename Visitor>
using VisitorResultType_t = typename VisitorResultType<Visitor>::type;


class ExpressionNode
{
private:
  std::shared_ptr<Expression> m_expression;
  std::optional<FileLocation> m_location;

  explicit ExpressionNode(
    std::shared_ptr<Expression> expression,
    std::optional<FileLocation> location = std::nullopt);

public:
  explicit ExpressionNode(
    Expression&& expression, std::optional<FileLocation> location = std::nullopt);

  bool isLiteral() const;

  template <typename Visitor>
  VisitorResultType_t<Visitor> accept(const Visitor& visitor) const;

  Value evaluate(EvaluationContext& context) const;
  Value tryEvaluate(EvaluationContext& context) const;

  ExpressionNode optimize(EvaluationContext& context) const;

  const std::optional<FileLocation>& location() const;

  std::string asString() const;

  friend bool operator==(const ExpressionNode& lhs, const ExpressionNode& rhs);
  friend bool operator!=(const ExpressionNode& lhs, const ExpressionNode& rhs);
  friend std::ostream& operator<<(std::ostream& str, const ExpressionNode& exp);

private:
  void rebalanceByPrecedence();
};

struct LiteralExpression
{
  Value value;
};

bool operator==(const LiteralExpression& lhs, const LiteralExpression& rhs);
bool operator!=(const LiteralExpression& lhs, const LiteralExpression& rhs);

std::ostream& operator<<(std::ostream& lhs, const LiteralExpression& rhs);


struct VariableExpression
{
  std::string variableName;
};

bool operator==(const VariableExpression& lhs, const VariableExpression& rhs);
bool operator!=(const VariableExpression& lhs, const VariableExpression& rhs);

std::ostream& operator<<(std::ostream& lhs, const VariableExpression& rhs);


struct ArrayExpression
{
  std::vector<ExpressionNode> elements;
};

bool operator==(const ArrayExpression& lhs, const ArrayExpression& rhs);
bool operator!=(const ArrayExpression& lhs, const ArrayExpression& rhs);

std::ostream& operator<<(std::ostream& lhs, const ArrayExpression& rhs);


struct MapExpression
{
  std::map<std::string, ExpressionNode> elements;
};

bool operator==(const MapExpression& lhs, const MapExpression& rhs);
bool operator!=(const MapExpression& lhs, const MapExpression& rhs);

std::ostream& operator<<(std::ostream& lhs, const MapExpression& rhs);


enum class UnaryOperation
{
  Plus,
  Minus,
  LogicalNegation,
  BitwiseNegation,
  Group,
  LeftBoundedRange,
  RightBoundedRange,
};

struct UnaryExpression
{
  UnaryOperation operation;
  ExpressionNode operand;
};

bool operator==(const UnaryExpression& lhs, const UnaryExpression& rhs);
bool operator!=(const UnaryExpression& lhs, const UnaryExpression& rhs);

std::ostream& operator<<(std::ostream& lhs, const UnaryExpression& rhs);


enum class BinaryOperation
{
  Addition,
  Subtraction,
  Multiplication,
  Division,
  Modulus,
  LogicalAnd,
  LogicalOr,
  BitwiseAnd,
  BitwiseXOr,
  BitwiseOr,
  BitwiseShiftLeft,
  BitwiseShiftRight,
  Less,
  LessOrEqual,
  Greater,
  GreaterOrEqual,
  Equal,
  NotEqual,
  BoundedRange,
  Case,
};

struct BinaryExpression
{
  BinaryOperation operation;
  ExpressionNode leftOperand;
  ExpressionNode rightOperand;
};

bool operator==(const BinaryExpression& lhs, const BinaryExpression& rhs);
bool operator!=(const BinaryExpression& lhs, const BinaryExpression& rhs);

std::ostream& operator<<(std::ostream& lhs, const BinaryExpression& rhs);


struct SubscriptExpression
{
  ExpressionNode leftOperand;
  ExpressionNode rightOperand;
};

bool operator==(const SubscriptExpression& lhs, const SubscriptExpression& rhs);
bool operator!=(const SubscriptExpression& lhs, const SubscriptExpression& rhs);

std::ostream& operator<<(std::ostream& lhs, const SubscriptExpression& rhs);


struct SwitchExpression
{
  std::vector<ExpressionNode> cases;
};

bool operator==(const SwitchExpression& lhs, const SwitchExpression& rhs);
bool operator!=(const SwitchExpression& lhs, const SwitchExpression& rhs);

std::ostream& operator<<(std::ostream& lhs, const SwitchExpression& rhs);

template <typename Visitor>
VisitorResultType_t<Visitor> ExpressionNode::accept(const Visitor& visitor) const
{
  return std::visit(
    [&](const auto& x) {
      if constexpr (
        std::is_invocable_v<Visitor, const Visitor&, decltype(x), const ExpressionNode&>)
      {
        return visitor(visitor, x, *this);
      }
      else if constexpr (std::is_invocable_v<Visitor, const Visitor&, decltype(x)>)
      {
        return visitor(visitor, x);
      }
      else if constexpr (std::is_invocable_v<Visitor, decltype(x), const ExpressionNode&>)
      {
        return visitor(x, *this);
      }
      else
      {
        return visitor(x);
      }
    },
    *m_expression);
}

} // namespace tb::el
