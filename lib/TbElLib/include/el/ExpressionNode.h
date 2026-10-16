/*
 Copyright (C) 2026 Kristian Duske

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

#include "Forward.h"
#include "base/FileLocation.h"

#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <variant>

namespace tb::el
{

struct LiteralExpression;
struct VariableExpression;
struct ArrayExpression;
struct MapExpression;
struct UnaryExpression;
struct BinaryExpression;
struct SubscriptExpression;
struct DotExpression;
struct CallExpression;
struct SwitchExpression;

using Expression = std::variant<
  LiteralExpression,
  VariableExpression,
  ArrayExpression,
  MapExpression,
  UnaryExpression,
  BinaryExpression,
  SubscriptExpression,
  DotExpression,
  CallExpression,
  SwitchExpression>;

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
  friend std::ostream& operator<<(std::ostream& str, const ExpressionNode& exp);

  friend class Value;

private:
  void rebalanceByPrecedence();
};

} // namespace tb::el
