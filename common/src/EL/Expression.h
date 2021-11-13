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
#include "Macros.h"

#include <iosfwd>
#include <memory>
#include <string>

namespace TrenchBroom {
namespace EL {
class ExpressionImpl;

class LiteralExpression;
class VariableExpression;

class ArrayExpression;
class MapExpression;

class UnaryExpression;
class BinaryExpression;
class SubscriptExpression;
class SwitchExpression;

class Expression {
private:
  std::shared_ptr<ExpressionImpl> m_expression;
  size_t m_line;
  size_t m_column;

  Expression(std::unique_ptr<ExpressionImpl> expression, size_t line, size_t column);

public:
  Expression(LiteralExpression expression, size_t line, size_t column);
  Expression(VariableExpression expression, size_t line, size_t column);
  Expression(ArrayExpression expression, size_t line, size_t column);
  Expression(MapExpression expression, size_t line, size_t column);
  Expression(UnaryExpression expression, size_t line, size_t column);
  Expression(BinaryExpression expression, size_t line, size_t column);
  Expression(SubscriptExpression expression, size_t line, size_t column);
  Expression(SwitchExpression expression, size_t line, size_t column);

  Value evaluate(const EvaluationContext& context) const;
  Expression optimize() const;

  size_t line() const;
  size_t column() const;

  std::string asString() const;

  friend bool operator==(const Expression& lhs, const Expression& rhs);
  friend bool operator!=(const Expression& lhs, const Expression& rhs);
  friend std::ostream& operator<<(std::ostream& str, const Expression& exp);

private:
  void rebalanceByPrecedence();
  size_t precedence() const;
};
} // namespace EL
} // namespace TrenchBroom
