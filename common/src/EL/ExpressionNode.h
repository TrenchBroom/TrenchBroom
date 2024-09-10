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

#pragma once

#include "EL/EL_Forward.h"
#include "EL/Value.h"
#include "FileLocation.h"

#include <iosfwd>
#include <memory>
#include <optional>
#include <string>

namespace TrenchBroom::EL
{
class Expression;

class LiteralExpression;
class VariableExpression;

class ArrayExpression;
class MapExpression;

class UnaryExpression;
class BinaryExpression;
class SubscriptExpression;
class SwitchExpression;

class EvaluationTrace;

class ExpressionNode
{
private:
  std::shared_ptr<Expression> m_expression;
  std::optional<FileLocation> m_location;

  explicit ExpressionNode(
    std::unique_ptr<Expression> expression,
    std::optional<FileLocation> location = std::nullopt);

public:
  explicit ExpressionNode(
    LiteralExpression expression, std::optional<FileLocation> location = std::nullopt);
  explicit ExpressionNode(
    VariableExpression expression, std::optional<FileLocation> location = std::nullopt);
  explicit ExpressionNode(
    ArrayExpression expression, std::optional<FileLocation> location = std::nullopt);
  explicit ExpressionNode(
    MapExpression expression, std::optional<FileLocation> location = std::nullopt);
  explicit ExpressionNode(
    UnaryExpression expression, std::optional<FileLocation> location = std::nullopt);
  explicit ExpressionNode(
    BinaryExpression expression, std::optional<FileLocation> location = std::nullopt);
  explicit ExpressionNode(
    SubscriptExpression expression, std::optional<FileLocation> location = std::nullopt);
  explicit ExpressionNode(
    SwitchExpression expression, std::optional<FileLocation> location = std::nullopt);

  Value evaluate(
    const EvaluationContext& context, EvaluationTrace* trace = nullptr) const;
  Value evaluate(const EvaluationContext& context, EvaluationTrace& trace) const;
  ExpressionNode optimize() const;

  const std::optional<FileLocation>& location() const;

  std::string asString() const;

  friend bool operator==(const ExpressionNode& lhs, const ExpressionNode& rhs);
  friend bool operator!=(const ExpressionNode& lhs, const ExpressionNode& rhs);
  friend std::ostream& operator<<(std::ostream& str, const ExpressionNode& exp);

private:
  void rebalanceByPrecedence();
  size_t precedence() const;
};

} // namespace TrenchBroom::EL
