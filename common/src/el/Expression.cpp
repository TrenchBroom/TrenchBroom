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

#include "Expression.h"

#include "Value.h"
#include "el/ELExceptions.h"
#include "el/EvaluationContext.h"
#include "el/EvaluationTrace.h"

#include "kdl/map_utils.h"
#include "kdl/overload.h"
#include "kdl/range_to.h"
#include "kdl/range_to_vector.h"
#include "kdl/vector_utils.h"

#include <fmt/format.h>

#include <ranges>
#include <sstream>

namespace tb::el
{
namespace
{

template <typename Evaluator>
Value evaluate(
  const Evaluator&, const LiteralExpression& expression, const EvaluationContext&)
{
  return expression.value;
}

template <typename Evaluator>
Value evaluate(
  const Evaluator&,
  const VariableExpression& expression,
  const EvaluationContext& context)
{
  return context.variableValue(expression.variableName);
}

template <typename Evaluator>
Value evaluate(
  const Evaluator& evaluator, const ArrayExpression& expression, const EvaluationContext&)
{
  auto array = ArrayType{};
  array.reserve(expression.elements.size());

  for (const auto& element : expression.elements)
  {
    auto value = element.accept(evaluator);
    if (value.hasType(ValueType::Range))
    {
      const auto& range = std::get<BoundedRange>(value.rangeValue());
      array.reserve(array.size() + range.length());
      range.forEach([&](const auto& i) { array.emplace_back(i); });
    }
    else
    {
      array.push_back(std::move(value));
    }
  }

  return Value{std::move(array)};
}

template <typename Evaluator>
Value evaluate(
  const Evaluator& evaluator, const MapExpression& expression, const EvaluationContext&)
{
  auto map = MapType{};
  for (const auto& [key, element] : expression.elements)
  {
    map.emplace(key, element.accept(evaluator));
  }

  return Value{std::move(map)};
}

Value evaluateUnaryPlus(const Value& v)
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
  throw EvaluationError{fmt::format(
    "Cannot apply unary plus to value '{}' of type '{}'", v.describe(), v.typeName())};
}

Value evaluateUnaryMinus(const Value& v)
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
  throw EvaluationError{fmt::format(
    "Cannot apply unary minus to value '{}' of type '{}'", v.describe(), v.typeName())};
}

Value evaluateLogicalNegation(const Value& v)
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
  throw EvaluationError{fmt::format(
    "Cannot apply logical negation to value '{}' of type '{}'",
    v.describe(),
    v.typeName())};
}

Value evaluateBitwiseNegation(const Value& v)
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
  throw EvaluationError{fmt::format(
    "Cannot apply bitwise negation to value '{}' of type '{}'",
    v.describe(),
    v.typeName())};
}

Value evaluateLeftBoundedRange(const Value& v)
{
  const auto first = static_cast<long>(v.convertTo(ValueType::Number).numberValue());
  return Value{LeftBoundedRange{first}};
}

Value evaluateRightBoundedRange(const Value& v)
{
  const auto last = static_cast<long>(v.convertTo(ValueType::Number).numberValue());
  return Value{RightBoundedRange{last}};
}

Value evaluateUnaryExpression(const UnaryOperation& operator_, const Value& operand)
{
  if (operand == Value::Undefined)
  {
    return Value::Undefined;
  }

  switch (operator_)
  {
  case UnaryOperation::Plus:
    return evaluateUnaryPlus(operand);
  case UnaryOperation::Minus:
    return evaluateUnaryMinus(operand);
  case UnaryOperation::LogicalNegation:
    return evaluateLogicalNegation(operand);
  case UnaryOperation::BitwiseNegation:
    return evaluateBitwiseNegation(operand);
  case UnaryOperation::Group:
    return Value{operand};
  case UnaryOperation::LeftBoundedRange:
    return evaluateLeftBoundedRange(operand);
  case UnaryOperation::RightBoundedRange:
    return evaluateRightBoundedRange(operand);
    switchDefault();
  }
}

template <typename Evaluator>
Value evaluate(
  const Evaluator& evaluator, const UnaryExpression& expression, const EvaluationContext&)
{
  return evaluateUnaryExpression(
    expression.operation, expression.operand.accept(evaluator));
}

template <typename Eval>
std::optional<Value> tryEvaluateAlgebraicOperator(
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

Value evaluateAddition(const Value& lhs, const Value& rhs)
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

  throw EvaluationError{fmt::format(
    "Cannot add '{}' of type '{}' to '{}' of type '{}'",
    lhs.describe(),
    typeName(lhs.type()),
    rhs.describe(),
    typeName(rhs.type()))};
}

Value evaluateSubtraction(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.numberValue() - rhsNumber.numberValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{fmt::format(
    "Cannot subtract '{}' of type '{}' from '{}' of type '{}'",
    rhs.describe(),
    typeName(rhs.type()),
    lhs.describe(),
    typeName(lhs.type()))};
}

Value evaluateMultiplication(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.numberValue() * rhsNumber.numberValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{fmt::format(
    "Cannot multiply '{}' of type '{}' by '{}' of type '{}'",
    lhs.describe(),
    typeName(lhs.type()),
    rhs.describe(),
    typeName(rhs.type()))};
}

Value evaluateDivision(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.numberValue() / rhsNumber.numberValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{fmt::format(
    "Cannot divide '{}' of type '{}' by '{}' of type '{}'",
    lhs.describe(),
    typeName(lhs.type()),
    rhs.describe(),
    typeName(rhs.type()))};
}

Value evaluateModulus(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return std::fmod(lhsNumber.numberValue(), rhsNumber.numberValue());
      }))
  {
    return *result;
  }

  throw EvaluationError{fmt::format(
    "Cannot take '{}' of type '{}' modulo '{}' of type '{}'",
    lhs.describe(),
    typeName(lhs.type()),
    rhs.describe(),
    typeName(rhs.type()))};
}

template <typename EvaluateLhs, typename EvaluateRhs>
Value evaluateLogicalAnd(const EvaluateLhs& evaluateLhs, const EvaluateRhs& evaluateRhs)
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

  throw EvaluationError{fmt::format(
    "Cannot apply operator && '{}' of type '{}' to '{}' of type '{}'",
    lhs.describe(),
    typeName(lhs.type()),
    rhs->describe(),
    typeName(rhs->type()))};
}

template <typename EvaluateLhs, typename EvaluateRhs>
Value evaluateLogicalOr(const EvaluateLhs& evaluateLhs, const EvaluateRhs& evaluateRhs)
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

  throw EvaluationError{fmt::format(
    "Cannot apply operator || '{}' of type '{}' to '{}' of type '{}'",
    lhs.describe(),
    typeName(lhs.type()),
    rhs->describe(),
    typeName(rhs->type()))};
}

template <typename Eval>
std::optional<Value> tryEvaluateBitwiseOperator(
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

Value evaluateBitwiseAnd(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() & rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{fmt::format(
    "Cannot apply operator & '{}' of type '{}' to '{}' of type '{}'",
    lhs.describe(),
    typeName(lhs.type()),
    rhs.describe(),
    typeName(rhs.type()))};
}

Value evaluateBitwiseXOr(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() ^ rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{fmt::format(
    "Cannot apply operator ^ '{}' of type '{}' to '{}' of type '{}'",
    lhs.describe(),
    typeName(lhs.type()),
    rhs.describe(),
    typeName(rhs.type()))};
}

Value evaluateBitwiseOr(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() | rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{fmt::format(
    "Cannot apply operator | '{}' of type '{}' to '{}' of type '{}'",
    lhs.describe(),
    typeName(lhs.type()),
    rhs.describe(),
    typeName(rhs.type()))};
}

Value evaluateBitwiseShiftLeft(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() << rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{fmt::format(
    "Cannot apply operator << '{}' of type '{}' to '{}' of type '{}'",
    lhs.describe(),
    typeName(lhs.type()),
    rhs.describe(),
    typeName(rhs.type()))};
}

Value evaluateBitwiseShiftRight(const Value& lhs, const Value& rhs)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() >> rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{fmt::format(
    "Cannot apply operator >> '{}' of type '{}' to '{}' of type '{}'",
    lhs.describe(),
    typeName(lhs.type()),
    rhs.describe(),
    typeName(rhs.type()))};
}

int compareAsBooleans(const Value& lhs, const Value& rhs)
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

int compareAsNumbers(const Value& lhs, const Value& rhs)
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

int evaluateCompare(const Value& lhs, const Value& rhs)
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
      case ValueType::Null:
      case ValueType::Undefined:
        return 1;
      case ValueType::Boolean:
      case ValueType::Number:
      case ValueType::String:
      case ValueType::Array:
      case ValueType::Map:
      case ValueType::Range:
        break;
      }
      break;
    }

    throw EvaluationError{fmt::format(
      "Cannot compare '{}' of type '{}' to '{}' of type '{}'",
      lhs.describe(),
      typeName(lhs.type()),
      rhs.describe(),
      typeName(rhs.type()))};
  }
  catch (const ConversionError& c)
  {
    throw EvaluationError{fmt::format(
      "Cannot apply compare '{}' of type '{}' to '{}' of type '{}': {}",
      lhs.describe(),
      typeName(lhs.type()),
      rhs.describe(),
      typeName(rhs.type()),
      c.what())};
  }
}

Value evaluateBoundedRange(const Value& lhs, const Value& rhs)
{
  if (lhs.hasType(ValueType::Undefined) || rhs.hasType(ValueType::Undefined))
  {
    return Value::Undefined;
  }

  const auto from = static_cast<long>(lhs.convertTo(ValueType::Number).numberValue());
  const auto to = static_cast<long>(rhs.convertTo(ValueType::Number).numberValue());

  return Value{BoundedRange{from, to}};
}

template <typename EvaluateLhs, typename EvaluateRhs>
Value evaluateCase(const EvaluateLhs& evaluateLhs, const EvaluateRhs& evaluateRhs)
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
Value evaluateBinaryExpression(
  const BinaryOperation operator_,
  const EvalualateLhs& evaluateLhs,
  const EvaluateRhs& evaluateRhs)
{
  switch (operator_)
  {
  case BinaryOperation::Addition:
    return evaluateAddition(evaluateLhs(), evaluateRhs());
  case BinaryOperation::Subtraction:
    return evaluateSubtraction(evaluateLhs(), evaluateRhs());
  case BinaryOperation::Multiplication:
    return evaluateMultiplication(evaluateLhs(), evaluateRhs());
  case BinaryOperation::Division:
    return evaluateDivision(evaluateLhs(), evaluateRhs());
  case BinaryOperation::Modulus:
    return evaluateModulus(evaluateLhs(), evaluateRhs());
  case BinaryOperation::LogicalAnd:
    return evaluateLogicalAnd(evaluateLhs, evaluateRhs);
  case BinaryOperation::LogicalOr:
    return evaluateLogicalOr(evaluateLhs, evaluateRhs);
  case BinaryOperation::BitwiseAnd:
    return evaluateBitwiseAnd(evaluateLhs(), evaluateRhs());
  case BinaryOperation::BitwiseXOr:
    return evaluateBitwiseXOr(evaluateLhs(), evaluateRhs());
  case BinaryOperation::BitwiseOr:
    return evaluateBitwiseOr(evaluateLhs(), evaluateRhs());
  case BinaryOperation::BitwiseShiftLeft:
    return evaluateBitwiseShiftLeft(evaluateLhs(), evaluateRhs());
  case BinaryOperation::BitwiseShiftRight:
    return evaluateBitwiseShiftRight(evaluateLhs(), evaluateRhs());
  case BinaryOperation::Less:
    return Value{evaluateCompare(evaluateLhs(), evaluateRhs()) < 0};
  case BinaryOperation::LessOrEqual:
    return Value{evaluateCompare(evaluateLhs(), evaluateRhs()) <= 0};
  case BinaryOperation::Greater:
    return Value{evaluateCompare(evaluateLhs(), evaluateRhs()) > 0};
  case BinaryOperation::GreaterOrEqual:
    return Value{evaluateCompare(evaluateLhs(), evaluateRhs()) >= 0};
  case BinaryOperation::Equal:
    return Value{evaluateCompare(evaluateLhs(), evaluateRhs()) == 0};
  case BinaryOperation::NotEqual:
    return Value{evaluateCompare(evaluateLhs(), evaluateRhs()) != 0};
  case BinaryOperation::BoundedRange:
    return Value{evaluateBoundedRange(evaluateLhs(), evaluateRhs())};
  case BinaryOperation::Case:
    return evaluateCase(evaluateLhs, evaluateRhs);
    switchDefault();
  };
}

template <typename Evaluator>
Value evaluate(
  const Evaluator& evaluator,
  const BinaryExpression& expression,
  const EvaluationContext&)
{
  return evaluateBinaryExpression(
    expression.operation,
    [&] { return expression.leftOperand.accept(evaluator); },
    [&] { return expression.rightOperand.accept(evaluator); });
}

size_t computeIndex(const long index, const size_t indexableSize)
{
  const auto size = static_cast<long>(indexableSize);
  return (index >= 0 && index < size) || (index < 0 && index >= -size)
           ? static_cast<size_t>((size + index % size) % size)
           : static_cast<size_t>(size);
}

size_t computeIndex(const Value& indexValue, const size_t indexableSize)
{
  return computeIndex(
    static_cast<long>(indexValue.convertTo(ValueType::Number).numberValue()),
    indexableSize);
}

void computeIndexArray(
  const LeftBoundedRange& range, const size_t indexableSize, std::vector<size_t>& result)
{
  result.reserve(result.size() + range.length(indexableSize));
  range.forEach(
    [&](const auto i) { result.push_back(computeIndex(i, indexableSize)); },
    indexableSize);
}

void computeIndexArray(
  const RightBoundedRange& range, const size_t indexableSize, std::vector<size_t>& result)
{
  result.reserve(result.size() + range.length(indexableSize));
  range.forEach(
    [&](const auto i) { result.push_back(computeIndex(i, indexableSize)); },
    indexableSize);
}

void computeIndexArray(
  const BoundedRange& range, const size_t indexableSize, std::vector<size_t>& result)
{
  result.reserve(result.size() + range.length());
  range.forEach([&](const auto i) { result.push_back(computeIndex(i, indexableSize)); });
}

void computeIndexArray(
  const Value& indexValue, const size_t indexableSize, std::vector<size_t>& result)
{
  switch (indexValue.type())
  {
  case ValueType::Array: {
    const ArrayType& indexArray = indexValue.arrayValue();
    result.reserve(result.size() + indexArray.size());
    for (size_t i = 0; i < indexArray.size(); ++i)
    {
      computeIndexArray(indexArray[i], indexableSize, result);
    }
    break;
  }
  case ValueType::Range: {
    std::visit(
      [&](const auto& x) { computeIndexArray(x, indexableSize, result); },
      indexValue.rangeValue());
    break;
  }
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::String:
  case ValueType::Map:
  case ValueType::Null:
  case ValueType::Undefined:
    result.push_back(computeIndex(indexValue, indexableSize));
    break;
  }
}

std::vector<size_t> computeIndexArray(const Value& indexValue, const size_t indexableSize)
{
  auto result = std::vector<size_t>{};
  computeIndexArray(indexValue, indexableSize, result);
  return result;
}

Value evaluateSubscript(const Value& lhs, const Value& rhs)
{
  switch (lhs.type())
  {
  case ValueType::String:
    switch (rhs.type())
    {
    case ValueType::Boolean:
    case ValueType::Number: {
      const auto& str = lhs.stringValue();
      const auto index = computeIndex(rhs, str.length());
      auto result = std::stringstream{};
      if (index < str.length())
      {
        result << str[index];
      }
      return Value{result.str()};
    }
    case ValueType::Array:
    case ValueType::Range: {
      const auto& str = lhs.stringValue();
      const auto indices = computeIndexArray(rhs, str.length());
      auto result = std::stringstream{};
      for (size_t i = 0; i < indices.size(); ++i)
      {
        const auto index = indices[i];
        if (index < str.length())
        {
          result << str[index];
        }
      }
      return Value{result.str()};
    }
    case ValueType::String:
    case ValueType::Map:
    case ValueType::Null:
    case ValueType::Undefined:
      break;
    }
    break;
  case ValueType::Array:
    switch (rhs.type())
    {
    case ValueType::Boolean:
    case ValueType::Number: {
      const auto& array = lhs.arrayValue();
      const auto index = computeIndex(rhs, array.size());
      if (index >= array.size())
      {
        throw IndexOutOfBoundsError{lhs, rhs, index};
      }
      return array[index];
    }
    case ValueType::Array:
    case ValueType::Range: {
      const auto& array = lhs.arrayValue();
      const auto indices = computeIndexArray(rhs, array.size());
      auto result = ArrayType{};
      result.reserve(indices.size());
      for (size_t i = 0; i < indices.size(); ++i)
      {
        const auto index = indices[i];
        if (index >= array.size())
        {
          throw IndexOutOfBoundsError{lhs, rhs, index};
        }
        result.push_back(array[index]);
      }
      return Value{std::move(result)};
    }
    case ValueType::String:
    case ValueType::Map:
    case ValueType::Null:
    case ValueType::Undefined:
      break;
    }
    break;
  case ValueType::Map:
    switch (rhs.type())
    {
    case ValueType::String: {
      const auto& map = lhs.mapValue();
      const auto& key = rhs.stringValue();
      const auto it = map.find(key);
      if (it == std::end(map))
      {
        return Value{UndefinedType::Value};
      }
      return it->second;
    }
    case ValueType::Array: {
      const auto& map = lhs.mapValue();
      const auto& keys = rhs.arrayValue();
      auto result = MapType{};
      for (size_t i = 0; i < keys.size(); ++i)
      {
        const auto& keyValue = keys[i];
        if (keyValue.type() != ValueType::String)
        {
          throw ConversionError{keyValue.describe(), keyValue.type(), ValueType::String};
        }
        const auto& key = keyValue.stringValue();
        const auto it = map.find(key);
        if (it != std::end(map))
        {
          result.insert(std::pair{key, it->second});
        }
      }
      return Value{std::move(result)};
    }
    case ValueType::Boolean:
    case ValueType::Number:
    case ValueType::Map:
    case ValueType::Range:
    case ValueType::Null:
    case ValueType::Undefined:
      break;
    }
    break;
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::Range:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }

  throw IndexError{lhs, rhs};
}

template <typename Evaluator>
Value evaluate(
  const Evaluator& evaluator,
  const SubscriptExpression& expression,
  const EvaluationContext&)
{
  return evaluateSubscript(
    expression.leftOperand.accept(evaluator), expression.rightOperand.accept(evaluator));
}

template <typename Evaluator>
Value evaluate(
  const Evaluator& evaluator,
  const SwitchExpression& expression,
  const EvaluationContext&)
{
  for (const auto& case_ : expression.cases)
  {
    if (auto result = case_.accept(evaluator); result != Value::Undefined)
    {
      return result;
    }
  }
  return Value::Undefined;
}


Expression optimize(const LiteralExpression& expression)
{
  return LiteralExpression{expression.value};
}

Expression optimize(const VariableExpression& expression)
{
  return VariableExpression{expression.variableName};
}

Expression optimize(const ArrayExpression& expression)
{
  auto optimizedExpressions =
    expression.elements
    | std::views::transform([](const auto& x) { return x.optimize(); });

  const auto isLiteral = std::ranges::all_of(
    optimizedExpressions, [](const auto& x) { return x.isLiteral(); });

  if (isLiteral)
  {
    const auto evaluationContext = EvaluationContext{};
    return LiteralExpression{Value{
      optimizedExpressions | std::views::transform([&](const auto& x) {
        return x.evaluate(evaluationContext);
      })
      | kdl::to_vector}};
  }

  return ArrayExpression{std::move(optimizedExpressions) | kdl::to_vector};
}

Expression optimize(const MapExpression& expression)
{
  auto optimizedExpressions =
    expression.elements | std::views::transform([](const auto& entry) {
      return std::pair{entry.first, entry.second.optimize()};
    });

  const auto isLiteral = std::ranges::all_of(
    optimizedExpressions, [](const auto& entry) { return entry.second.isLiteral(); });

  if (isLiteral)
  {
    const auto evaluationContext = EvaluationContext{};
    return LiteralExpression{Value{
      optimizedExpressions | std::views::transform([&](const auto& entry) {
        return std::pair{entry.first, entry.second.evaluate(evaluationContext)};
      })
      | kdl::to<MapType>()}};
  }

  return MapExpression{
    std::move(optimizedExpressions) | kdl::to<std::map<std::string, ExpressionNode>>()};
}

Expression optimize(const UnaryExpression& expression)
{
  const auto evaluationContext = EvaluationContext{};
  auto optimizedOperand = expression.operand.optimize();

  if (optimizedOperand.isLiteral())
  {
    return LiteralExpression{evaluateUnaryExpression(
      expression.operation, optimizedOperand.evaluate(evaluationContext))};
  }

  return UnaryExpression{expression.operation, std::move(optimizedOperand)};
}

Expression optimize(const BinaryExpression& expression)
{
  auto optimizedLeftOperand = std::optional<ExpressionNode>{};
  auto optimizedRightOperand = std::optional<ExpressionNode>{};

  const auto evaluationContext = EvaluationContext{};

  const auto evaluateLeftOperand = [&] {
    optimizedLeftOperand = expression.leftOperand.optimize();
    return optimizedLeftOperand->evaluate(evaluationContext);
  };

  const auto evaluateRightOperand = [&] {
    optimizedRightOperand = expression.rightOperand.optimize();
    return optimizedRightOperand->evaluate(evaluationContext);
  };

  auto value = evaluateBinaryExpression(
    expression.operation, evaluateLeftOperand, evaluateRightOperand);

  const auto isLiteral =
    (!optimizedLeftOperand || optimizedLeftOperand->isLiteral())
    && (!optimizedRightOperand || optimizedRightOperand->isLiteral());

  if (isLiteral)
  {
    return LiteralExpression{std::move(value)};
  }

  return BinaryExpression{
    expression.operation,
    std::move(optimizedLeftOperand).value_or(expression.leftOperand.optimize()),
    std::move(optimizedRightOperand).value_or(expression.rightOperand.optimize())};
}

Expression optimize(const SubscriptExpression& expression)
{
  auto optimizedLeftOperand = expression.leftOperand.optimize();
  auto optimizedRightOperand = expression.rightOperand.optimize();

  const auto isLiteral =
    optimizedLeftOperand.isLiteral() && optimizedRightOperand.isLiteral();

  if (isLiteral)
  {
    const auto evaluationContext = EvaluationContext{};
    const auto leftValue = optimizedLeftOperand.evaluate(evaluationContext);
    const auto rightValue = optimizedRightOperand.evaluate(evaluationContext);
    return LiteralExpression{evaluateSubscript(leftValue, rightValue)};
  }

  return SubscriptExpression{
    std::move(optimizedLeftOperand), std::move(optimizedRightOperand)};
}

Expression optimize(const SwitchExpression& expression)
{
  if (expression.cases.empty())
  {
    return LiteralExpression{Value::Undefined};
  }

  const auto evaluationContext = EvaluationContext{};

  auto firstOptimizedExpression = expression.cases.front().optimize();
  if (firstOptimizedExpression.isLiteral())
  {
    return LiteralExpression{firstOptimizedExpression.evaluate(evaluationContext)};
  }

  auto optimizedExpressions =
    std::vector<ExpressionNode>{std::move(firstOptimizedExpression)};
  for (size_t i = 1u; i < expression.cases.size(); ++i)
  {
    optimizedExpressions.push_back(expression.cases[i].optimize());
  }

  return SwitchExpression{std::move(optimizedExpressions)};
}

Expression optimizeExpression(const Expression& expression)
{
  return std::visit([](const auto& x) { return optimize(x); }, expression);
}


size_t precedence(const BinaryOperation operation)
{
  switch (operation)
  {
  case BinaryOperation::Multiplication:
  case BinaryOperation::Division:
  case BinaryOperation::Modulus:
    return 12;
  case BinaryOperation::Addition:
  case BinaryOperation::Subtraction:
    return 11;
  case BinaryOperation::BitwiseShiftLeft:
  case BinaryOperation::BitwiseShiftRight:
    return 10;
  case BinaryOperation::Less:
  case BinaryOperation::LessOrEqual:
  case BinaryOperation::Greater:
  case BinaryOperation::GreaterOrEqual:
    return 9;
  case BinaryOperation::Equal:
  case BinaryOperation::NotEqual:
    return 8;
  case BinaryOperation::BitwiseAnd:
    return 7;
  case BinaryOperation::BitwiseXOr:
    return 6;
  case BinaryOperation::BitwiseOr:
    return 5;
  case BinaryOperation::LogicalAnd:
    return 4;
  case BinaryOperation::LogicalOr:
    return 3;
  case BinaryOperation::BoundedRange:
    return 2;
  case BinaryOperation::Case:
    return 1;
    switchDefault();
  };
}

size_t precedence(const Expression& expression)
{
  return std::visit(
    kdl::overload(
      [](const BinaryExpression& binaryExpression) {
        return precedence(binaryExpression.operation);
      },
      [](const auto&) { return size_t(13); }),
    expression);
}

} // namespace

std::ostream& operator<<(std::ostream& lhs, const Expression& rhs)
{
  std::visit([&](const auto& x) { lhs << x; }, rhs);
  return lhs;
}

ExpressionNode::ExpressionNode(
  std::shared_ptr<Expression> expression, std::optional<FileLocation> location)
  : m_expression{std::move(expression)}
  , m_location{std::move(location)}
{
}

ExpressionNode::ExpressionNode(
  Expression&& expression, std::optional<FileLocation> location)
  : m_expression{std::make_shared<Expression>(std::move(expression))}
  , m_location{std::move(location)}
{
  rebalanceByPrecedence();
}

bool ExpressionNode::isLiteral() const
{
  return std::holds_alternative<LiteralExpression>(*m_expression);
}

Value ExpressionNode::evaluate(
  const EvaluationContext& context, EvaluationTrace* trace) const
{
  return accept(
    [&](const auto& evaluator, const auto& expression, const auto& containingNode) {
      auto value = el::evaluate(evaluator, expression, context);
      if (trace)
      {
        trace->addTrace(value, containingNode);
      }
      return value;
    });
}

Value ExpressionNode::evaluate(
  const EvaluationContext& context, EvaluationTrace& trace) const
{
  return evaluate(context, &trace);
}

Value ExpressionNode::tryEvaluate(
  const EvaluationContext& context, EvaluationTrace* trace) const
{
  return accept(
    [&](const auto& evaluator, const auto& expression, const auto& containingNode) {
      try
      {
        auto value = el::evaluate(evaluator, expression, context);
        if (trace)
        {
          trace->addTrace(value, containingNode);
        }
        return value;
      }
      catch (const EvaluationError&)
      {
        return Value::Undefined;
      }
    });
}

Value ExpressionNode::tryEvaluate(
  const EvaluationContext& context, EvaluationTrace& trace) const
{
  return tryEvaluate(context, &trace);
}

ExpressionNode ExpressionNode::optimize() const
{
  return ExpressionNode{
    std::make_shared<Expression>(optimizeExpression(*m_expression)), m_location};
}

const std::optional<FileLocation>& ExpressionNode::location() const
{
  return m_location;
}

std::string ExpressionNode::asString() const
{
  auto str = std::stringstream{};
  str << *this;
  return str.str();
}

bool operator==(const ExpressionNode& lhs, const ExpressionNode& rhs)
{
  return *lhs.m_expression == *rhs.m_expression;
}

bool operator!=(const ExpressionNode& lhs, const ExpressionNode& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const ExpressionNode& rhs)
{
  lhs << *rhs.m_expression;
  return lhs;
}

void ExpressionNode::rebalanceByPrecedence()
{
  /*
   * The expression tree has a similar invariant to a heap: For any given node, its
   * precedence must be less than or equal to the precedences of its children. This
   * guarantees that evaluating the tree in a depth first traversal yields correct results
   * because the nodes with the highest precedence are evaluated before the nodes with
   * lower precedence.
   */

  std::visit(
    kdl::overload(
      [&](BinaryExpression& binaryExpression) {
        const auto myPrecedence = precedence(*m_expression);
        const auto leftPrecedence =
          precedence(*binaryExpression.leftOperand.m_expression);
        const auto rightPrecedence =
          precedence(*binaryExpression.rightOperand.m_expression);

        if (myPrecedence > std::min(leftPrecedence, rightPrecedence))
        {
          if (leftPrecedence < rightPrecedence)
          {
            // push this operator into the right subtree, rotating the right node up, and
            // rebalancing the right subtree again
            auto leftExpressionNode = std::move(binaryExpression.leftOperand);
            auto& leftBinaryExpression =
              std::get<BinaryExpression>(*leftExpressionNode.m_expression);

            binaryExpression.leftOperand = std::move(leftBinaryExpression.rightOperand);
            leftBinaryExpression.rightOperand = std::move(*this);
            *this = std::move(leftExpressionNode);

            binaryExpression.rightOperand.rebalanceByPrecedence();
          }
          else
          {
            // push this operator into the left subtree, rotating the left node up, and
            // rebalancing the left subtree again
            auto rightExpressionNode = std::move(binaryExpression.rightOperand);
            auto& rightBinaryExpression =
              std::get<BinaryExpression>(*rightExpressionNode.m_expression);

            binaryExpression.rightOperand = std::move(rightBinaryExpression.leftOperand);
            rightBinaryExpression.leftOperand = std::move(*this);
            *this = std::move(rightExpressionNode);

            binaryExpression.leftOperand.rebalanceByPrecedence();
          }
        }
      },
      [](auto&) {}),
    *m_expression);
}


bool operator==(const LiteralExpression& lhs, const LiteralExpression& rhs)
{
  return lhs.value == rhs.value;
}

bool operator!=(const LiteralExpression& lhs, const LiteralExpression& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const LiteralExpression& rhs)
{
  return lhs << rhs.value;
}


bool operator==(const VariableExpression& lhs, const VariableExpression& rhs)
{
  return lhs.variableName == rhs.variableName;
}

bool operator!=(const VariableExpression& lhs, const VariableExpression& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const VariableExpression& rhs)
{
  return lhs << rhs.variableName;
}


bool operator==(const ArrayExpression& lhs, const ArrayExpression& rhs)
{
  return lhs.elements == rhs.elements;
}

bool operator!=(const ArrayExpression& lhs, const ArrayExpression& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const ArrayExpression& rhs)
{
  lhs << "[ ";
  for (size_t i = 0; i < rhs.elements.size(); ++i)
  {
    lhs << rhs.elements[i];
    if (i < rhs.elements.size() - 1)
    {
      lhs << ", ";
    }
  }
  lhs << " ]";

  return lhs;
}


bool operator==(const MapExpression& lhs, const MapExpression& rhs)
{
  return lhs.elements == rhs.elements;
}

bool operator!=(const MapExpression& lhs, const MapExpression& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const MapExpression& rhs)
{
  lhs << "{ ";
  size_t i = 0u;
  for (const auto& [key, expression] : rhs.elements)
  {
    lhs << "\"" << key << "\": " << expression;
    if (i < rhs.elements.size() - 1u)
    {
      lhs << ", ";
    }
    ++i;
  }
  lhs << " }";

  return lhs;
}


bool operator==(const UnaryExpression& lhs, const UnaryExpression& rhs)
{
  return lhs.operation == rhs.operation && lhs.operand == rhs.operand;
}

bool operator!=(const UnaryExpression& lhs, const UnaryExpression& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const UnaryExpression& rhs)
{
  switch (rhs.operation)
  {
  case UnaryOperation::Plus:
    lhs << "+" << rhs.operand;
    break;
  case UnaryOperation::Minus:
    lhs << "-" << rhs.operand;
    break;
  case UnaryOperation::LogicalNegation:
    lhs << "!" << rhs.operand;
    break;
  case UnaryOperation::BitwiseNegation:
    lhs << "~" << rhs.operand;
    break;
  case UnaryOperation::Group:
    lhs << "( " << rhs.operand << " )";
    break;
  case UnaryOperation::LeftBoundedRange:
    lhs << rhs.operand << "..";
    break;
  case UnaryOperation::RightBoundedRange:
    lhs << ".." << rhs.operand;
    break;
    switchDefault();
  }

  return lhs;
}


bool operator==(const BinaryExpression& lhs, const BinaryExpression& rhs)
{
  return lhs.operation == rhs.operation && lhs.leftOperand == rhs.leftOperand
         && lhs.rightOperand == rhs.rightOperand;
}

bool operator!=(const BinaryExpression& lhs, const BinaryExpression& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const BinaryExpression& rhs)
{
  switch (rhs.operation)
  {
  case BinaryOperation::Addition:
    lhs << rhs.leftOperand << " + " << rhs.rightOperand;
    break;
  case BinaryOperation::Subtraction:
    lhs << rhs.leftOperand << " - " << rhs.rightOperand;
    break;
  case BinaryOperation::Multiplication:
    lhs << rhs.leftOperand << " * " << rhs.rightOperand;
    break;
  case BinaryOperation::Division:
    lhs << rhs.leftOperand << " / " << rhs.rightOperand;
    break;
  case BinaryOperation::Modulus:
    lhs << rhs.leftOperand << " % " << rhs.rightOperand;
    break;
  case BinaryOperation::LogicalAnd:
    lhs << rhs.leftOperand << " && " << rhs.rightOperand;
    break;
  case BinaryOperation::LogicalOr:
    lhs << rhs.leftOperand << " || " << rhs.rightOperand;
    break;
  case BinaryOperation::BitwiseAnd:
    lhs << rhs.leftOperand << " & " << rhs.rightOperand;
    break;
  case BinaryOperation::BitwiseXOr:
    lhs << rhs.leftOperand << " ^ " << rhs.rightOperand;
    break;
  case BinaryOperation::BitwiseOr:
    lhs << rhs.leftOperand << " | " << rhs.rightOperand;
    break;
  case BinaryOperation::BitwiseShiftLeft:
    lhs << rhs.leftOperand << " << " << rhs.rightOperand;
    break;
  case BinaryOperation::BitwiseShiftRight:
    lhs << rhs.leftOperand << " >> " << rhs.rightOperand;
    break;
  case BinaryOperation::Less:
    lhs << rhs.leftOperand << " < " << rhs.rightOperand;
    break;
  case BinaryOperation::LessOrEqual:
    lhs << rhs.leftOperand << " <= " << rhs.rightOperand;
    break;
  case BinaryOperation::Greater:
    lhs << rhs.leftOperand << " > " << rhs.rightOperand;
    break;
  case BinaryOperation::GreaterOrEqual:
    lhs << rhs.leftOperand << " >= " << rhs.rightOperand;
    break;
  case BinaryOperation::Equal:
    lhs << rhs.leftOperand << " == " << rhs.rightOperand;
    break;
  case BinaryOperation::NotEqual:
    lhs << rhs.leftOperand << " != " << rhs.rightOperand;
    break;
  case BinaryOperation::BoundedRange:
    lhs << rhs.leftOperand << ".." << rhs.rightOperand;
    break;
  case BinaryOperation::Case:
    lhs << rhs.leftOperand << " -> " << rhs.rightOperand;
    break;
    switchDefault();
  };

  return lhs;
}


bool operator==(const SubscriptExpression& lhs, const SubscriptExpression& rhs)
{
  return lhs.leftOperand == rhs.leftOperand && lhs.rightOperand == rhs.rightOperand;
}

bool operator!=(const SubscriptExpression& lhs, const SubscriptExpression& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const SubscriptExpression& rhs)
{
  return lhs << rhs.leftOperand << "[" << rhs.rightOperand << "]";
}


bool operator==(const SwitchExpression& lhs, const SwitchExpression& rhs)
{
  return lhs.cases == rhs.cases;
}

bool operator!=(const SwitchExpression& lhs, const SwitchExpression& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const SwitchExpression& rhs)
{
  lhs << "{{ ";
  size_t i = 0u;
  for (const auto& case_ : rhs.cases)
  {
    lhs << case_;
    if (i < rhs.cases.size() - 1u)
    {
      lhs << ", ";
    }
    ++i;
  }
  lhs << " }}";

  return lhs;
}

} // namespace tb::el
