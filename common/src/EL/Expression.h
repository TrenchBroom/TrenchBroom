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
class ExpressionImpl;

class LiteralExpression;
class VariableExpression;

class ArrayExpression;
class MapExpression;

class UnaryExpression;
class BinaryExpression;
class SubscriptExpression;
class SwitchExpression;

class EvaluationTrace;

class Expression
{
private:
  std::shared_ptr<ExpressionImpl> m_expression;
  std::optional<FileLocation> m_location;

  explicit Expression(
    std::unique_ptr<ExpressionImpl> expression,
    std::optional<FileLocation> location = std::nullopt);

public:
  explicit Expression(
    LiteralExpression expression, std::optional<FileLocation> location = std::nullopt);
  explicit Expression(
    VariableExpression expression, std::optional<FileLocation> location = std::nullopt);
  explicit Expression(
    ArrayExpression expression, std::optional<FileLocation> location = std::nullopt);
  explicit Expression(
    MapExpression expression, std::optional<FileLocation> location = std::nullopt);
  explicit Expression(
    UnaryExpression expression, std::optional<FileLocation> location = std::nullopt);
  explicit Expression(
    BinaryExpression expression, std::optional<FileLocation> location = std::nullopt);
  explicit Expression(
    SubscriptExpression expression, std::optional<FileLocation> location = std::nullopt);
  explicit Expression(
    SwitchExpression expression, std::optional<FileLocation> location = std::nullopt);

  Value evaluate(
    const EvaluationContext& context, EvaluationTrace* trace = nullptr) const;
  Value evaluate(const EvaluationContext& context, EvaluationTrace& trace) const;
  Expression optimize() const;

  const std::optional<FileLocation>& location() const;

  std::string asString() const;

  friend bool operator==(const Expression& lhs, const Expression& rhs);
  friend bool operator!=(const Expression& lhs, const Expression& rhs);
  friend std::ostream& operator<<(std::ostream& str, const Expression& exp);

private:
  void rebalanceByPrecedence();
  size_t precedence() const;
};

} // namespace TrenchBroom::EL
