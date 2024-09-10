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
#include "EL/ExpressionNode.h"
#include "EL/Value.h"

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom::EL
{

class LiteralExpression;
class VariableExpression;
class ArrayExpression;
class MapExpression;
class UnaryExpression;
class BinaryExpression;
class SubscriptExpression;
class SwitchExpression;

class Expression
{
public:
  virtual ~Expression();

  virtual Value evaluate(
    const EvaluationContext& context, EvaluationTrace* trace = nullptr) const = 0;
  virtual std::unique_ptr<Expression> optimize() const = 0;

  virtual size_t precedence() const;

  virtual bool operator==(const Expression& rhs) const = 0;
  bool operator!=(const Expression& rhs) const;

  virtual bool operator==(const LiteralExpression& rhs) const;
  virtual bool operator==(const VariableExpression& rhs) const;
  virtual bool operator==(const ArrayExpression& rhs) const;
  virtual bool operator==(const MapExpression& rhs) const;
  virtual bool operator==(const UnaryExpression& rhs) const;
  virtual bool operator==(const BinaryExpression& rhs) const;
  virtual bool operator==(const SubscriptExpression& rhs) const;
  virtual bool operator==(const SwitchExpression& rhs) const;

  friend std::ostream& operator<<(std::ostream& lhs, const Expression& rhs);

private:
  virtual void appendToStream(std::ostream& str) const = 0;
};

class LiteralExpression : public Expression
{
private:
  Value m_value;

public:
  explicit LiteralExpression(Value value);

  Value evaluate(const EvaluationContext& context, EvaluationTrace* trace) const override;
  std::unique_ptr<Expression> optimize() const override;

  bool operator==(const Expression& rhs) const override;
  bool operator==(const LiteralExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

class VariableExpression : public Expression
{
private:
  std::string m_variableName;

public:
  explicit VariableExpression(std::string variableName);

  Value evaluate(const EvaluationContext& context, EvaluationTrace* trace) const override;
  std::unique_ptr<Expression> optimize() const override;

  bool operator==(const Expression& rhs) const override;
  bool operator==(const VariableExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

class ArrayExpression : public Expression
{
private:
  std::vector<ExpressionNode> m_elements;

public:
  explicit ArrayExpression(std::vector<ExpressionNode> elements);

  Value evaluate(const EvaluationContext& context, EvaluationTrace* trace) const override;
  std::unique_ptr<Expression> optimize() const override;

  bool operator==(const Expression& rhs) const override;
  bool operator==(const ArrayExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

class MapExpression : public Expression
{
private:
  std::map<std::string, ExpressionNode> m_elements;

public:
  explicit MapExpression(std::map<std::string, ExpressionNode> elements);

  Value evaluate(const EvaluationContext& context, EvaluationTrace* trace) const override;
  std::unique_ptr<Expression> optimize() const override;

  bool operator==(const Expression& rhs) const override;
  bool operator==(const MapExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

enum class UnaryOperator
{
  Plus,
  Minus,
  LogicalNegation,
  BitwiseNegation,
  Group
};

class UnaryExpression : public Expression
{
private:
  UnaryOperator m_operator;
  ExpressionNode m_operand;

public:
  UnaryExpression(UnaryOperator i_operator, ExpressionNode operand);

  Value evaluate(const EvaluationContext& context, EvaluationTrace* trace) const override;
  std::unique_ptr<Expression> optimize() const override;

  bool operator==(const Expression& rhs) const override;
  bool operator==(const UnaryExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

enum class BinaryOperator
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
  Range,
  Case,
};

class BinaryExpression : public Expression
{
public:
  friend class ExpressionNode;

private:
  BinaryOperator m_operator;
  ExpressionNode m_leftOperand;
  ExpressionNode m_rightOperand;

public:
  BinaryExpression(
    BinaryOperator i_operator, ExpressionNode leftOperand, ExpressionNode rightOperand);
  static ExpressionNode createAutoRangeWithRightOperand(
    ExpressionNode rightOperand, FileLocation location);
  static ExpressionNode createAutoRangeWithLeftOperand(
    ExpressionNode leftOperand, FileLocation location);

  Value evaluate(const EvaluationContext& context, EvaluationTrace* trace) const override;
  std::unique_ptr<Expression> optimize() const override;

  size_t precedence() const override;

  bool operator==(const Expression& rhs) const override;
  bool operator==(const BinaryExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

class SubscriptExpression : public Expression
{
public:
  static const std::string& AutoRangeParameterName();

private:
  ExpressionNode m_leftOperand;
  ExpressionNode m_rightOperand;

public:
  SubscriptExpression(ExpressionNode leftOperand, ExpressionNode rightOperand);

  Value evaluate(const EvaluationContext& context, EvaluationTrace* trace) const override;
  std::unique_ptr<Expression> optimize() const override;

  bool operator==(const Expression& rhs) const override;
  bool operator==(const SubscriptExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

class SwitchExpression : public Expression
{
private:
  std::vector<ExpressionNode> m_cases;

public:
  explicit SwitchExpression(std::vector<ExpressionNode> cases);

  Value evaluate(const EvaluationContext& context, EvaluationTrace* trace) const override;
  std::unique_ptr<Expression> optimize() const override;

  bool operator==(const Expression& rhs) const override;
  bool operator==(const SwitchExpression& rhs) const override;

private:
  void appendToStream(std::ostream& str) const override;
};

} // namespace TrenchBroom::EL
