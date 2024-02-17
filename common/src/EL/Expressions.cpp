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

#include "Expressions.h"

#include "EL/ELExceptions.h"
#include "EL/EvaluationContext.h"
#include "Ensure.h"
#include "Macros.h"

#include "kdl/collection_utils.h"
#include "kdl/map_utils.h"
#include "kdl/vector_utils.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>

namespace TrenchBroom
{
namespace EL
{
ExpressionImpl::~ExpressionImpl() = default;

size_t ExpressionImpl::precedence() const
{
  return 13u;
}

bool ExpressionImpl::operator!=(const ExpressionImpl& rhs) const
{
  return !(*this == rhs);
}

bool ExpressionImpl::operator==(const LiteralExpression&) const
{
  return false;
}

bool ExpressionImpl::operator==(const VariableExpression&) const
{
  return false;
}

bool ExpressionImpl::operator==(const ArrayExpression&) const
{
  return false;
}

bool ExpressionImpl::operator==(const MapExpression&) const
{
  return false;
}

bool ExpressionImpl::operator==(const UnaryExpression&) const
{
  return false;
}

bool ExpressionImpl::operator==(const BinaryExpression&) const
{
  return false;
}

bool ExpressionImpl::operator==(const SubscriptExpression&) const
{
  return false;
}

bool ExpressionImpl::operator==(const SwitchExpression&) const
{
  return false;
}

std::ostream& operator<<(std::ostream& lhs, const ExpressionImpl& rhs)
{
  rhs.appendToStream(lhs);
  return lhs;
}

LiteralExpression::LiteralExpression(Value value)
  : m_value{std::move(value)}
{
}

Value LiteralExpression::evaluate(const EvaluationContext&) const
{
  return m_value;
}

std::unique_ptr<ExpressionImpl> LiteralExpression::optimize() const
{
  return std::make_unique<LiteralExpression>(m_value);
}

bool LiteralExpression::operator==(const ExpressionImpl& rhs) const
{
  return rhs == *this;
}

bool LiteralExpression::operator==(const LiteralExpression& rhs) const
{
  return m_value == rhs.m_value;
}

void LiteralExpression::appendToStream(std::ostream& str) const
{
  str << m_value;
}

VariableExpression::VariableExpression(std::string variableName)
  : m_variableName{std::move(variableName)}
{
}

Value VariableExpression::evaluate(const EvaluationContext& context) const
{
  return context.variableValue(m_variableName);
}

std::unique_ptr<ExpressionImpl> VariableExpression::optimize() const
{
  return std::make_unique<VariableExpression>(m_variableName);
}

bool VariableExpression::operator==(const ExpressionImpl& rhs) const
{
  return rhs == *this;
}

bool VariableExpression::operator==(const VariableExpression& rhs) const
{
  return m_variableName == rhs.m_variableName;
}

void VariableExpression::appendToStream(std::ostream& str) const
{
  str << m_variableName;
}

ArrayExpression::ArrayExpression(std::vector<Expression> elements)
  : m_elements{std::move(elements)}
{
}

Value ArrayExpression::evaluate(const EvaluationContext& context) const
{
  auto array = ArrayType{};
  array.reserve(m_elements.size());
  for (const auto& element : m_elements)
  {
    auto value = element.evaluate(context);
    if (value.hasType(ValueType::Range))
    {
      const auto& range = value.rangeValue();
      if (!range.empty())
      {
        array.reserve(array.size() + range.size() - 1u);
        for (size_t i = 0u; i < range.size(); ++i)
        {
          array.emplace_back(range[i], value.expression());
        }
      }
    }
    else
    {
      array.push_back(std::move(value));
    }
  }

  return Value{std::move(array)};
}

std::unique_ptr<ExpressionImpl> ArrayExpression::optimize() const
{
  auto optimizedExpressions = kdl::vec_transform(
    m_elements, [](const auto& expression) { return expression.optimize(); });

  auto values = ArrayType{};
  values.reserve(m_elements.size());

  const auto evaluationContext = EvaluationContext{};
  for (const auto& expression : optimizedExpressions)
  {
    if (auto value = expression.evaluate(evaluationContext); value != Value::Undefined)
    {
      values.push_back(std::move(value));
    }
    else
    {
      return std::make_unique<ArrayExpression>(std::move(optimizedExpressions));
    }
  }

  return std::make_unique<LiteralExpression>(Value{std::move(values)});
}

bool ArrayExpression::operator==(const ExpressionImpl& rhs) const
{
  return rhs == *this;
}

bool ArrayExpression::operator==(const ArrayExpression& rhs) const
{
  return m_elements == rhs.m_elements;
}

void ArrayExpression::appendToStream(std::ostream& str) const
{
  str << "[ ";
  size_t i = 0u;
  for (const auto& expression : m_elements)
  {
    str << expression;
    if (i < m_elements.size() - 1u)
    {
      str << ", ";
    }
    ++i;
  }
  str << " ]";
}

MapExpression::MapExpression(std::map<std::string, Expression> elements)
  : m_elements{std::move(elements)}
{
}

Value MapExpression::evaluate(const EvaluationContext& context) const
{
  auto map = MapType{};
  for (const auto& [key, expression] : m_elements)
  {
    map.emplace(key, expression.evaluate(context));
  }

  return Value{std::move(map)};
}

std::unique_ptr<ExpressionImpl> MapExpression::optimize() const
{
  auto optimizedExpressions = std::map<std::string, Expression>{};
  for (const auto& [key, expression] : m_elements)
  {
    optimizedExpressions.emplace(key, expression.optimize());
  }

  auto values = MapType{};

  const auto evaluationContext = EvaluationContext{};
  for (const auto& [key, expression] : optimizedExpressions)
  {
    if (auto value = expression.evaluate(evaluationContext); value != Value::Undefined)
    {
      values.emplace(key, std::move(value));
    }
    else
    {
      return std::make_unique<MapExpression>(std::move(optimizedExpressions));
    }
  }

  return std::make_unique<LiteralExpression>(Value{std::move(values)});
}

bool MapExpression::operator==(const ExpressionImpl& rhs) const
{
  return rhs == *this;
}

bool MapExpression::operator==(const MapExpression& rhs) const
{
  return m_elements == rhs.m_elements;
}

void MapExpression::appendToStream(std::ostream& str) const
{
  str << "{ ";
  size_t i = 0u;
  for (const auto& [key, expression] : m_elements)
  {
    str << "\"" << key << "\": " << expression;
    if (i < m_elements.size() - 1u)
    {
      str << ", ";
    }
    ++i;
  }
  str << " }";
}

UnaryExpression::UnaryExpression(const UnaryOperator i_operator, Expression operand)
  : m_operator{i_operator}
  , m_operand{std::move(operand)}
{
}

static Value evaluateUnaryPlus(const Value& v)
{
  switch (v.type())
  {
  case ValueType::Boolean:
  case ValueType::Number:
    return Value{v.convertTo(ValueType::Number).numberValue()};
  case ValueType::String:
    if (const auto result = v.tryConvertTo(ValueType::Number))
    {
      return Value{result->numberValue()};
    }
  case ValueType::Array:
  case ValueType::Map:
  case ValueType::Range:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }
  throw EvaluationError{
    "Cannot apply unary plus to value '" + v.describe() + "' of type '" + v.typeName()};
}

static Value evaluateUnaryMinus(const Value& v)
{
  switch (v.type())
  {
  case ValueType::Boolean:
  case ValueType::Number:
    return Value{-v.convertTo(ValueType::Number).numberValue()};
  case ValueType::String:
    if (const auto result = v.tryConvertTo(ValueType::Number))
    {
      return Value{-result->numberValue()};
    }
  case ValueType::Array:
  case ValueType::Map:
  case ValueType::Range:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }
  throw EvaluationError{
    "Cannot apply unary minus to value '" + v.describe() + "' of type '" + v.typeName()};
}

static Value evaluateLogicalNegation(const Value& v)
{
  switch (v.type())
  {
  case ValueType::Boolean:
    return Value{!v.booleanValue()};
  case ValueType::Number:
  case ValueType::String:
  case ValueType::Array:
  case ValueType::Map:
  case ValueType::Range:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }
  throw EvaluationError{
    "Cannot apply logical negation to value '" + v.describe() + "' of type '"
    + v.typeName()};
}

static Value evaluateBitwiseNegation(const Value& v)
{
  switch (v.type())
  {
  case ValueType::Number:
    return Value{~v.integerValue()};
  case ValueType::String:
    if (const auto result = v.tryConvertTo(ValueType::Number))
    {
      return Value{~result->integerValue()};
    }
  case ValueType::Boolean:
  case ValueType::Array:
  case ValueType::Map:
  case ValueType::Range:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }
  throw EvaluationError{
    "Cannot apply bitwise negation to value '" + v.describe() + "' of type '"
    + v.typeName()};
}

static Value evaluateUnaryExpression(const UnaryOperator& operator_, const Value& operand)
{
  if (operand == Value::Undefined)
  {
    return Value::Undefined;
  }

  switch (operator_)
  {
  case UnaryOperator::Plus:
    return evaluateUnaryPlus(operand);
  case UnaryOperator::Minus:
    return evaluateUnaryMinus(operand);
  case UnaryOperator::LogicalNegation:
    return evaluateLogicalNegation(operand);
  case UnaryOperator::BitwiseNegation:
    return evaluateBitwiseNegation(operand);
  case UnaryOperator::Group:
    return Value{operand};
    switchDefault();
  }
}

Value UnaryExpression::evaluate(const EvaluationContext& context) const
{
  return evaluateUnaryExpression(m_operator, m_operand.evaluate(context));
}

std::unique_ptr<ExpressionImpl> UnaryExpression::optimize() const
{
  auto optimizedOperand = m_operand.optimize();
  if (auto value = evaluateUnaryExpression(
        m_operator, optimizedOperand.evaluate(EvaluationContext{}));
      value != Value::Undefined)
  {
    return std::make_unique<LiteralExpression>(std::move(value));
  }

  return std::make_unique<UnaryExpression>(m_operator, std::move(optimizedOperand));
}

bool UnaryExpression::operator==(const ExpressionImpl& rhs) const
{
  return rhs == *this;
}

bool UnaryExpression::operator==(const UnaryExpression& rhs) const
{
  return m_operator == rhs.m_operator && m_operand == rhs.m_operand;
}

void UnaryExpression::appendToStream(std::ostream& str) const
{
  switch (m_operator)
  {
  case UnaryOperator::Plus:
    str << "+" << m_operand;
    break;
  case UnaryOperator::Minus:
    str << "-" << m_operand;
    break;
  case UnaryOperator::LogicalNegation:
    str << "!" << m_operand;
    break;
  case UnaryOperator::BitwiseNegation:
    str << "~" << m_operand;
    break;
  case UnaryOperator::Group:
    str << "( " << m_operand << " )";
    break;
    switchDefault();
  }
}

BinaryExpression::BinaryExpression(
  const BinaryOperator i_operator, Expression leftOperand, Expression rightOperand)
  : m_operator{i_operator}
  , m_leftOperand{std::move(leftOperand)}
  , m_rightOperand{std::move(rightOperand)}
{
}

Expression BinaryExpression::createAutoRangeWithRightOperand(
  Expression rightOperand, const size_t line, const size_t column)
{
  auto leftOperand = Expression{
    VariableExpression{SubscriptExpression::AutoRangeParameterName()}, line, column};
  return Expression{
    BinaryExpression{
      BinaryOperator::Range, std::move(leftOperand), std::move(rightOperand)},
    line,
    column};
}

Expression BinaryExpression::createAutoRangeWithLeftOperand(
  Expression leftOperand, const size_t line, const size_t column)
{
  auto rightOperand = Expression{
    VariableExpression{SubscriptExpression::AutoRangeParameterName()}, line, column};
  return Expression{
    BinaryExpression{
      BinaryOperator::Range, std::move(leftOperand), std::move(rightOperand)},
    line,
    column};
}

template <typename Eval>
static std::optional<Value> tryEvaluateAlgebraicOperator(
  const Value& lhs, const Value& rhs, const Eval& eval)
{
  if (lhs.hasType(ValueType::Undefined) || rhs.hasType(ValueType::Undefined))
  {
    return Value::Undefined;
  }

  if (
    lhs.hasType(ValueType::Boolean, ValueType::Number)
    && rhs.hasType(ValueType::Boolean, ValueType::Number))
  {
    return Value{
      eval(lhs.convertTo(ValueType::Number), rhs.convertTo(ValueType::Number))};
  }

  if (
    lhs.hasType(ValueType::Boolean, ValueType::Number) && rhs.hasType(ValueType::String))
  {
    if (const auto rhsAsNumber = rhs.tryConvertTo(ValueType::Number))
    {
      return Value{eval(lhs.convertTo(ValueType::Number), *rhsAsNumber)};
    }
  }

  if (
    lhs.hasType(ValueType::String) && rhs.hasType(ValueType::Boolean, ValueType::Number))
  {
    if (const auto lhsAsNumber = lhs.tryConvertTo(ValueType::Number))
    {
      return Value{eval(*lhsAsNumber, rhs.convertTo(ValueType::Number))};
    }
  }

  return std::nullopt;
}

static Value evaluateAddition(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.numberValue() + rhsNumber.numberValue();
      }))
  {
    return *result;
  }

  if (lhs.hasType(ValueType::String) && rhs.hasType(ValueType::String))
  {
    return Value{
      lhs.convertTo(ValueType::String).stringValue()
      + rhs.convertTo(ValueType::String).stringValue()};
  }

  if (lhs.hasType(ValueType::Array) && rhs.hasType(ValueType::Array))
  {
    return Value{kdl::vec_concat(lhs.arrayValue(), rhs.arrayValue())};
  }

  if (lhs.hasType(ValueType::Map) && rhs.hasType(ValueType::Map))
  {
    return Value{kdl::map_union(lhs.mapValue(), rhs.mapValue())};
  }

  throw EvaluationError{
    "Cannot add '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + " to '"
    + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'"};
}

static Value evaluateSubtraction(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.numberValue() - rhsNumber.numberValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{
    "Cannot subtract '" + rhs.describe() + "' of type '" + typeName(rhs.type())
    + " from '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + "'"};
}

static Value evaluateMultiplication(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.numberValue() * rhsNumber.numberValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{
    "Cannot multiply '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + " by '"
    + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'"};
}

static Value evaluateDivision(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.numberValue() / rhsNumber.numberValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{
    "Cannot divide '" + lhs.describe() + "' of type '" + typeName(lhs.type()) + " by '"
    + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'"};
}

static Value evaluateModulus(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return std::fmod(lhsNumber.numberValue(), rhsNumber.numberValue());
      }))
  {
    return *result;
  }

  throw EvaluationError{
    "Cannot apply operator % to '" + lhs.describe() + "' of type '" + typeName(lhs.type())
    + " and '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'"};
}

template <typename EvaluateLhs, typename EvaluateRhs>
static Value evaluateLogicalAnd(
  const EvaluateLhs& evaluateLhs, const EvaluateRhs& evaluateRhs)
{
  const auto lhs = evaluateLhs();
  auto rhs = std::make_optional<Value>();

  if (lhs.hasType(ValueType::Undefined))
  {
    return Value::Undefined;
  }

  if (lhs.hasType(ValueType::Boolean, ValueType::Null))
  {
    const auto lhsValue = lhs.convertTo(ValueType::Boolean).booleanValue();
    if (!lhsValue)
    {
      return Value{false};
    }

    rhs = evaluateRhs();
    if (rhs->hasType(ValueType::Boolean, ValueType::Null))
    {
      return Value{rhs->convertTo(ValueType::Boolean).booleanValue()};
    }
  }

  if (!rhs)
  {
    rhs = evaluateRhs();
  }

  if (rhs->hasType(ValueType::Undefined))
  {
    return Value::Undefined;
  }

  throw EvaluationError{
    "Cannot apply operator && to '" + lhs.describe() + "' of type '"
    + typeName(lhs.type()) + " and '" + rhs->describe() + "' of type '"
    + typeName(rhs->type()) + "'"};
}

template <typename EvaluateLhs, typename EvaluateRhs>
static Value evaluateLogicalOr(
  const EvaluateLhs& evaluateLhs, const EvaluateRhs& evaluateRhs)
{
  const auto lhs = evaluateLhs();
  auto rhs = std::make_optional<Value>();

  if (lhs.hasType(ValueType::Undefined))
  {
    return Value::Undefined;
  }

  if (lhs.hasType(ValueType::Boolean, ValueType::Null))
  {
    const auto lhsValue = lhs.convertTo(ValueType::Boolean).booleanValue();
    if (lhsValue)
    {
      return Value{true};
    }

    rhs = evaluateRhs();
    if (rhs->hasType(ValueType::Boolean, ValueType::Null))
    {
      return Value{rhs->convertTo(ValueType::Boolean).booleanValue()};
    }
  }

  if (!rhs)
  {
    rhs = evaluateRhs();
  }

  if (rhs->hasType(ValueType::Undefined))
  {
    return Value::Undefined;
  }

  throw EvaluationError{
    "Cannot apply operator || to '" + lhs.describe() + "' of type '"
    + typeName(lhs.type()) + " and '" + rhs->describe() + "' of type '"
    + typeName(rhs->type()) + "'"};
}

template <typename Eval>
static std::optional<Value> tryEvaluateBitwiseOperator(
  const Value& lhs, const Value& rhs, const Eval& eval)
{
  if (lhs.hasType(ValueType::Undefined) || rhs.hasType(ValueType::Undefined))
  {
    return Value::Undefined;
  }

  if (lhs.convertibleTo(ValueType::Number) && rhs.convertibleTo(ValueType::Number))
  {
    return Value{
      eval(lhs.convertTo(ValueType::Number), rhs.convertTo(ValueType::Number))};
  }

  return std::nullopt;
}

static Value evaluateBitwiseAnd(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() & rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{
    "Cannot apply operator & to '" + lhs.describe() + "' of type '" + typeName(lhs.type())
    + " and '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'"};
}

static Value evaluateBitwiseXOr(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() ^ rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{
    "Cannot apply operator ^ to '" + lhs.describe() + "' of type '" + typeName(lhs.type())
    + " and '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'"};
}

static Value evaluateBitwiseOr(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() | rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{
    "Cannot apply operator | to '" + lhs.describe() + "' of type '" + typeName(lhs.type())
    + " and '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'"};
}

static Value evaluateBitwiseShiftLeft(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() << rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{
    "Cannot apply operator << to '" + lhs.describe() + "' of type '"
    + typeName(lhs.type()) + " and '" + rhs.describe() + "' of type '"
    + typeName(rhs.type()) + "'"};
}

static Value evaluateBitwiseShiftRight(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() >> rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{
    "Cannot apply operator >> to '" + lhs.describe() + "' of type '"
    + typeName(lhs.type()) + " and '" + rhs.describe() + "' of type '"
    + typeName(rhs.type()) + "'"};
}

static int compareAsBooleans(const Value& lhs, const Value& rhs)
{
  const bool lhsValue = lhs.convertTo(ValueType::Boolean).booleanValue();
  const bool rhsValue = rhs.convertTo(ValueType::Boolean).booleanValue();
  if (lhsValue == rhsValue)
  {
    return 0;
  }
  else if (lhsValue)
  {
    return 1;
  }
  else
  {
    return -1;
  }
}

static int compareAsNumbers(const Value& lhs, const Value& rhs)
{
  const NumberType diff = lhs.convertTo(ValueType::Number).numberValue()
                          - rhs.convertTo(ValueType::Number).numberValue();
  if (diff < 0.0)
  {
    return -1;
  }
  else if (diff > 0.0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

static int evaluateCompare(const Value& lhs, const Value& rhs)
{
  try
  {
    switch (lhs.type())
    {
    case ValueType::Boolean:
      switch (rhs.type())
      {
      case ValueType::Boolean:
      case ValueType::Number:
      case ValueType::String:
        return compareAsBooleans(lhs, rhs);
      case ValueType::Null:
      case ValueType::Undefined:
        return 1;
      case ValueType::Array:
      case ValueType::Map:
      case ValueType::Range:
        break;
      }
      break;
    case ValueType::Number:
      switch (rhs.type())
      {
      case ValueType::Boolean:
        return compareAsBooleans(lhs, rhs);
      case ValueType::Number:
      case ValueType::String:
        return compareAsNumbers(lhs, rhs);
      case ValueType::Null:
      case ValueType::Undefined:
        return 1;
      case ValueType::Array:
      case ValueType::Map:
      case ValueType::Range:
        break;
      }
      break;
    case ValueType::String:
      switch (rhs.type())
      {
      case ValueType::Boolean:
        return compareAsBooleans(lhs, rhs);
      case ValueType::Number:
        return compareAsNumbers(lhs, rhs);
      case ValueType::String:
        return lhs.stringValue().compare(rhs.convertTo(ValueType::String).stringValue());
      case ValueType::Null:
      case ValueType::Undefined:
        return 1;
      case ValueType::Array:
      case ValueType::Map:
      case ValueType::Range:
        break;
      }
      break;
    case ValueType::Null:
      if (rhs.hasType(ValueType::Null))
      {
        return 0;
      }
      else
      {
        return -1;
      }
    case ValueType::Undefined:
      if (rhs.hasType(ValueType::Undefined))
      {
        return 0;
      }
      else
      {
        return -1;
      }
    case ValueType::Array:
      switch (rhs.type())
      {
      case ValueType::Array:
        return kdl::col_lexicographical_compare(
          lhs.arrayValue(), rhs.arrayValue(), [](const auto& l, const auto& r) {
            return evaluateCompare(l, r) < 0;
          });
      case ValueType::Null:
      case ValueType::Undefined:
        return 1;
      case ValueType::Boolean:
      case ValueType::Number:
      case ValueType::String:
      case ValueType::Map:
      case ValueType::Range:
        break;
      }
      break;
    case ValueType::Map:
      switch (rhs.type())
      {
      case ValueType::Map:
        return kdl::map_lexicographical_compare(
          lhs.mapValue(), rhs.mapValue(), [](const auto& l, const auto& r) {
            return evaluateCompare(l, r) < 0;
          });
      case ValueType::Null:
      case ValueType::Undefined:
        return 1;
      case ValueType::Boolean:
      case ValueType::Number:
      case ValueType::String:
      case ValueType::Array:
      case ValueType::Range:
        break;
      }
      break;
    case ValueType::Range:
      switch (rhs.type())
      {
      case ValueType::Range:
        return kdl::col_lexicographical_compare(lhs.rangeValue(), rhs.rangeValue());
      case ValueType::Null:
      case ValueType::Undefined:
        return 1;
      case ValueType::Boolean:
      case ValueType::Number:
      case ValueType::String:
      case ValueType::Array:
      case ValueType::Map:
        break;
      }
      break;
    }
    throw EvaluationError{
      "Cannot compare value '" + lhs.describe() + "' of type '" + typeName(lhs.type())
      + " to value '" + rhs.describe() + "' of type '" + typeName(rhs.type()) + "'"};
  }
  catch (const ConversionError& c)
  {
    throw EvaluationError{
      "Cannot compare value '" + lhs.describe() + "' of type '" + typeName(lhs.type())
      + " to value '" + rhs.describe() + "' of type '" + typeName(rhs.type())
      + "': " + c.what()};
  }
}

static Value evaluateRange(const Value& lhs, const Value& rhs)
{
  if (lhs.hasType(ValueType::Undefined) || rhs.hasType(ValueType::Undefined))
  {
    return Value::Undefined;
  }

  const auto from = static_cast<long>(lhs.convertTo(ValueType::Number).numberValue());
  const auto to = static_cast<long>(rhs.convertTo(ValueType::Number).numberValue());

  auto range = RangeType{};
  if (from <= to)
  {
    range.reserve(static_cast<size_t>(to - from + 1));
    for (long i = from; i <= to; ++i)
    {
      assert(range.capacity() > range.size());
      range.push_back(i);
    }
  }
  else if (to < from)
  {
    range.reserve(static_cast<size_t>(from - to + 1));
    for (long i = from; i >= to; --i)
    {
      assert(range.capacity() > range.size());
      range.push_back(i);
    }
  }
  assert(range.capacity() == range.size());

  return Value{range};
}

template <typename EvaluateLhs, typename EvaluateRhs>
static Value evaluateCase(const EvaluateLhs& evaluateLhs, const EvaluateRhs& evaluateRhs)
{
  const auto lhs = evaluateLhs();

  if (
    lhs.type() != ValueType::Undefined
    && lhs.convertTo(ValueType::Boolean).booleanValue())
  {
    return evaluateRhs();
  }

  return Value::Undefined;
}

template <typename EvalualateLhs, typename EvaluateRhs>
static Value evaluateBinaryExpression(
  const BinaryOperator operator_,
  const EvalualateLhs& evaluateLhs,
  const EvaluateRhs& evaluateRhs)
{
  switch (operator_)
  {
  case BinaryOperator::Addition:
    return evaluateAddition(evaluateLhs(), evaluateRhs());
  case BinaryOperator::Subtraction:
    return evaluateSubtraction(evaluateLhs(), evaluateRhs());
  case BinaryOperator::Multiplication:
    return evaluateMultiplication(evaluateLhs(), evaluateRhs());
  case BinaryOperator::Division:
    return evaluateDivision(evaluateLhs(), evaluateRhs());
  case BinaryOperator::Modulus:
    return evaluateModulus(evaluateLhs(), evaluateRhs());
  case BinaryOperator::LogicalAnd:
    return evaluateLogicalAnd(evaluateLhs, evaluateRhs);
  case BinaryOperator::LogicalOr:
    return evaluateLogicalOr(evaluateLhs, evaluateRhs);
  case BinaryOperator::BitwiseAnd:
    return evaluateBitwiseAnd(evaluateLhs(), evaluateRhs());
  case BinaryOperator::BitwiseXOr:
    return evaluateBitwiseXOr(evaluateLhs(), evaluateRhs());
  case BinaryOperator::BitwiseOr:
    return evaluateBitwiseOr(evaluateLhs(), evaluateRhs());
  case BinaryOperator::BitwiseShiftLeft:
    return evaluateBitwiseShiftLeft(evaluateLhs(), evaluateRhs());
  case BinaryOperator::BitwiseShiftRight:
    return evaluateBitwiseShiftRight(evaluateLhs(), evaluateRhs());
  case BinaryOperator::Less:
    return Value{evaluateCompare(evaluateLhs(), evaluateRhs()) < 0};
  case BinaryOperator::LessOrEqual:
    return Value{evaluateCompare(evaluateLhs(), evaluateRhs()) <= 0};
  case BinaryOperator::Greater:
    return Value{evaluateCompare(evaluateLhs(), evaluateRhs()) > 0};
  case BinaryOperator::GreaterOrEqual:
    return Value{evaluateCompare(evaluateLhs(), evaluateRhs()) >= 0};
  case BinaryOperator::Equal:
    return Value{evaluateCompare(evaluateLhs(), evaluateRhs()) == 0};
  case BinaryOperator::NotEqual:
    return Value{evaluateCompare(evaluateLhs(), evaluateRhs()) != 0};
  case BinaryOperator::Range:
    return Value{evaluateRange(evaluateLhs(), evaluateRhs())};
  case BinaryOperator::Case:
    return evaluateCase(evaluateLhs, evaluateRhs);
    switchDefault();
  };
}

Value BinaryExpression::evaluate(const EvaluationContext& context) const
{
  return evaluateBinaryExpression(
    m_operator,
    [&] { return m_leftOperand.evaluate(context); },
    [&] { return m_rightOperand.evaluate(context); });
}

std::unique_ptr<ExpressionImpl> BinaryExpression::optimize() const
{
  auto optimizedLeftOperand = std::optional<Expression>{};
  auto optimizedRightOperand = std::optional<Expression>{};

  const auto evaluationContext = EvaluationContext{};

  const auto evaluateLeftOperand = [&] {
    optimizedLeftOperand = m_leftOperand.optimize();
    return optimizedLeftOperand->evaluate(evaluationContext);
  };

  const auto evaluateRightOperand = [&] {
    optimizedRightOperand = m_rightOperand.optimize();
    return optimizedRightOperand->evaluate(evaluationContext);
  };

  if (auto value =
        evaluateBinaryExpression(m_operator, evaluateLeftOperand, evaluateRightOperand);
      value != Value::Undefined)
  {
    return std::make_unique<LiteralExpression>(std::move(value));
  }

  return std::make_unique<BinaryExpression>(
    m_operator,
    std::move(optimizedLeftOperand).value_or(m_leftOperand.optimize()),
    std::move(optimizedRightOperand).value_or(m_rightOperand.optimize()));
}

size_t BinaryExpression::precedence() const
{
  switch (m_operator)
  {
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

bool BinaryExpression::operator==(const ExpressionImpl& rhs) const
{
  return rhs == *this;
}

bool BinaryExpression::operator==(const BinaryExpression& rhs) const
{
  return m_operator == rhs.m_operator && m_leftOperand == rhs.m_leftOperand
         && m_rightOperand == rhs.m_rightOperand;
}

void BinaryExpression::appendToStream(std::ostream& str) const
{
  switch (m_operator)
  {
  case BinaryOperator::Addition:
    str << m_leftOperand << " + " << m_rightOperand;
    break;
  case BinaryOperator::Subtraction:
    str << m_leftOperand << " - " << m_rightOperand;
    break;
  case BinaryOperator::Multiplication:
    str << m_leftOperand << " * " << m_rightOperand;
    break;
  case BinaryOperator::Division:
    str << m_leftOperand << " / " << m_rightOperand;
    break;
  case BinaryOperator::Modulus:
    str << m_leftOperand << " % " << m_rightOperand;
    break;
  case BinaryOperator::LogicalAnd:
    str << m_leftOperand << " && " << m_rightOperand;
    break;
  case BinaryOperator::LogicalOr:
    str << m_leftOperand << " || " << m_rightOperand;
    break;
  case BinaryOperator::BitwiseAnd:
    str << m_leftOperand << " & " << m_rightOperand;
    break;
  case BinaryOperator::BitwiseXOr:
    str << m_leftOperand << " ^ " << m_rightOperand;
    break;
  case BinaryOperator::BitwiseOr:
    str << m_leftOperand << " | " << m_rightOperand;
    break;
  case BinaryOperator::BitwiseShiftLeft:
    str << m_leftOperand << " << " << m_rightOperand;
    break;
  case BinaryOperator::BitwiseShiftRight:
    str << m_leftOperand << " >> " << m_rightOperand;
    break;
  case BinaryOperator::Less:
    str << m_leftOperand << " < " << m_rightOperand;
    break;
  case BinaryOperator::LessOrEqual:
    str << m_leftOperand << " <= " << m_rightOperand;
    break;
  case BinaryOperator::Greater:
    str << m_leftOperand << " > " << m_rightOperand;
    break;
  case BinaryOperator::GreaterOrEqual:
    str << m_leftOperand << " >= " << m_rightOperand;
    break;
  case BinaryOperator::Equal:
    str << m_leftOperand << " == " << m_rightOperand;
    break;
  case BinaryOperator::NotEqual:
    str << m_leftOperand << " != " << m_rightOperand;
    break;
  case BinaryOperator::Range:
    str << m_leftOperand << ".." << m_rightOperand;
    break;
  case BinaryOperator::Case:
    str << m_leftOperand << " -> " << m_rightOperand;
    break;
    switchDefault();
  };
}

const std::string& SubscriptExpression::AutoRangeParameterName()
{
  static const std::string Name = "__AutoRangeParameter";
  return Name;
}

SubscriptExpression::SubscriptExpression(Expression leftOperand, Expression rightOperand)
  : m_leftOperand{std::move(leftOperand)}
  , m_rightOperand{std::move(rightOperand)}
{
}

Value SubscriptExpression::evaluate(const EvaluationContext& context) const
{
  const auto leftValue = m_leftOperand.evaluate(context);

  auto stack = EvaluationStack{context};
  stack.declareVariable(AutoRangeParameterName(), Value(leftValue.length() - 1u));

  const auto rightValue = m_rightOperand.evaluate(stack);
  return leftValue[rightValue];
}

std::unique_ptr<ExpressionImpl> SubscriptExpression::optimize() const
{
  auto optimizedLeftOperand = m_leftOperand.optimize();
  auto optimizedRightOperand = m_rightOperand.optimize();

  auto evaluationContext = EvaluationContext{};
  if (auto leftValue = optimizedLeftOperand.evaluate(evaluationContext);
      leftValue != Value::Undefined)
  {
    auto stack = EvaluationStack{evaluationContext};
    stack.declareVariable(AutoRangeParameterName(), Value(leftValue.length() - 1u));

    if (auto rightValue = optimizedRightOperand.evaluate(stack);
        rightValue != Value::Undefined)
    {
      if (auto value = leftValue[rightValue]; value != Value::Undefined)
      {
        return std::make_unique<LiteralExpression>(std::move(value));
      }
    }
  }

  return std::make_unique<SubscriptExpression>(
    std::move(optimizedLeftOperand), std::move(optimizedRightOperand));
}

bool SubscriptExpression::operator==(const ExpressionImpl& rhs) const
{
  return rhs == *this;
}

bool SubscriptExpression::operator==(const SubscriptExpression& rhs) const
{
  return m_leftOperand == rhs.m_leftOperand && m_rightOperand == rhs.m_rightOperand;
}

void SubscriptExpression::appendToStream(std::ostream& str) const
{
  str << m_leftOperand << "[" << m_rightOperand << "]";
}

SwitchExpression::SwitchExpression(std::vector<Expression> cases)
  : m_cases{std::move(cases)}
{
}

Value SwitchExpression::evaluate(const EvaluationContext& context) const
{
  for (const auto& case_ : m_cases)
  {
    if (auto result = case_.evaluate(context); result != Value::Undefined)
    {
      return result;
    }
  }
  return Value::Undefined;
}

std::unique_ptr<ExpressionImpl> SwitchExpression::optimize() const
{
  if (m_cases.empty())
  {
    return std::make_unique<SwitchExpression>(m_cases);
  }

  auto optimizedExpressions = kdl::vec_transform(
    m_cases, [](const auto& expression) { return expression.optimize(); });
  if (auto firstValue = optimizedExpressions.front().evaluate(EvaluationContext{});
      firstValue != Value::Undefined)
  {
    return std::make_unique<LiteralExpression>(std::move(firstValue));
  }

  return std::make_unique<SwitchExpression>(std::move(optimizedExpressions));
}

bool SwitchExpression::operator==(const ExpressionImpl& rhs) const
{
  return rhs == *this;
}

bool SwitchExpression::operator==(const SwitchExpression& rhs) const
{
  return m_cases == rhs.m_cases;
}

void SwitchExpression::appendToStream(std::ostream& str) const
{
  str << "{{ ";
  size_t i = 0u;
  for (const auto& case_ : m_cases)
  {
    str << case_;
    if (i < m_cases.size() - 1u)
    {
      str << ", ";
    }
    ++i;
  }
  str << " }}";
}
} // namespace EL
} // namespace TrenchBroom
