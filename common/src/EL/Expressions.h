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

#include "EL/EL_Forward.h"
#include "EL/Value.h"

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
namespace EL {
class ExpressionImpl {
public:
  virtual ~ExpressionImpl();

  virtual Value evaluate(const EvaluationContext& context) const = 0;
  virtual std::unique_ptr<ExpressionImpl> optimize() const = 0;

  virtual size_t precedence() const;

  virtual bool operator==(const ExpressionImpl& rhs) const = 0;
  bool operator!=(const ExpressionImpl& rhs) const;

  virtual bool operator==(const LiteralExpression& rhs) const;
  virtual bool operator==(const VariableExpression& rhs) const;
  virtual bool operator==(const ArrayExpression& rhs) const;
  virtual bool operator==(const MapExpression& rhs) const;
  virtual bool operator==(const UnaryExpression& rhs) const;
  virtual bool operator==(const BinaryExpression& rhs) const;
  virtual bool operator==(const SubscriptExpression& rhs) const;
  virtual bool operator==(const SwitchExpression& rhs) const;

  friend std::ostream& operator<<(std::ostream& lhs, const ExpressionImpl& rhs);

private:
  virtual void appendToStream(std::ostream& str) const = 0;
};

class LiteralExpression : public ExpressionImpl {
private:
  Value m_value;

public:
  LiteralExpression(Value value);

  Value evaluate(const EvaluationContext& context) const override;
  std::unique_ptr<ExpressionImpl> optimize() const override;

  bool operator==(const ExpressionImpl& rhs) const override;
  bool operator==(const LiteralExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

class VariableExpression : public ExpressionImpl {
private:
  std::string m_variableName;

public:
  VariableExpression(std::string variableName);

  Value evaluate(const EvaluationContext& context) const override;
  std::unique_ptr<ExpressionImpl> optimize() const override;

  bool operator==(const ExpressionImpl& rhs) const override;
  bool operator==(const VariableExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

class ArrayExpression : public ExpressionImpl {
private:
  std::vector<Expression> m_elements;

public:
  ArrayExpression(std::vector<Expression> elements);

  Value evaluate(const EvaluationContext& context) const override;
  std::unique_ptr<ExpressionImpl> optimize() const override;

  bool operator==(const ExpressionImpl& rhs) const override;
  bool operator==(const ArrayExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

class MapExpression : public ExpressionImpl {
private:
  std::map<std::string, Expression> m_elements;

public:
  MapExpression(std::map<std::string, Expression> elements);

  Value evaluate(const EvaluationContext& context) const override;
  std::unique_ptr<ExpressionImpl> optimize() const override;

  bool operator==(const ExpressionImpl& rhs) const override;
  bool operator==(const MapExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

enum class UnaryOperator {
  Plus,
  Minus,
  LogicalNegation,
  BitwiseNegation,
  Group
};

class UnaryExpression : public ExpressionImpl {
private:
  UnaryOperator m_operator;
  Expression m_operand;

public:
  UnaryExpression(UnaryOperator i_operator, Expression operand);

  Value evaluate(const EvaluationContext& context) const override;
  std::unique_ptr<ExpressionImpl> optimize() const override;

  bool operator==(const ExpressionImpl& rhs) const override;
  bool operator==(const UnaryExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

enum class BinaryOperator {
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
  Range,
  Case,
};

class BinaryExpression : public ExpressionImpl {
public:
  friend class Expression;

private:
  BinaryOperator m_operator;
  Expression m_leftOperand;
  Expression m_rightOperand;

public:
  BinaryExpression(BinaryOperator i_operator, Expression leftOperand, Expression rightOperand);
  static Expression createAutoRangeWithRightOperand(
    Expression rightOperand, size_t line, size_t column);
  static Expression createAutoRangeWithLeftOperand(
    Expression leftOperand, size_t line, size_t column);

  Value evaluate(const EvaluationContext& context) const override;
  std::unique_ptr<ExpressionImpl> optimize() const override;

  size_t precedence() const override;

  bool operator==(const ExpressionImpl& rhs) const override;
  bool operator==(const BinaryExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

class SubscriptExpression : public ExpressionImpl {
public:
  static const std::string& AutoRangeParameterName();

private:
  Expression m_leftOperand;
  Expression m_rightOperand;

public:
  SubscriptExpression(Expression leftOperand, Expression rightOperand);

  Value evaluate(const EvaluationContext& context) const override;
  std::unique_ptr<ExpressionImpl> optimize() const override;

  bool operator==(const ExpressionImpl& rhs) const override;
  bool operator==(const SubscriptExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

class SwitchExpression : public ExpressionImpl {
private:
  std::vector<Expression> m_cases;

public:
  SwitchExpression(std::vector<Expression> cases);

  Value evaluate(const EvaluationContext& context) const override;
  std::unique_ptr<ExpressionImpl> optimize() const override;

  bool operator==(const ExpressionImpl& rhs) const override;
  bool operator==(const SwitchExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};
} // namespace EL
} // namespace TrenchBroom
