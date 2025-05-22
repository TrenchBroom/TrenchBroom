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
  EvaluationContext&,
  const Evaluator&,
  const LiteralExpression& expression,
  const ExpressionNode&)
{
  return expression.value;
}

template <typename Evaluator>
Value evaluate(
  EvaluationContext& context,
  const Evaluator&,
  const VariableExpression& expression,
  const ExpressionNode&)
{
  return context.variableValue(expression.variableName);
}

template <typename Evaluator>
Value evaluate(
  EvaluationContext& context,
  const Evaluator& evaluator,
  const ArrayExpression& expression,
  const ExpressionNode&)
{
  auto array = ArrayType{};
  array.reserve(expression.elements.size());

  for (const auto& element : expression.elements)
  {
    auto value = element.accept(evaluator);
    if (value.hasType(ValueType::Range))
    {
      const auto& range = std::get<BoundedRange>(value.rangeValue(context));
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
  EvaluationContext&,
  const Evaluator& evaluator,
  const MapExpression& expression,
  const ExpressionNode&)
{
  auto map = MapType{};
  for (const auto& [key, element] : expression.elements)
  {
    map.emplace(key, element.accept(evaluator));
  }

  return Value{std::move(map)};
}

Value evaluateUnaryPlus(
  EvaluationContext& context, const Value& v, const ExpressionNode& expressionNode)
{
  switch (v.type())
  {
  case ValueType::Boolean:
  case ValueType::Number:
    return Value{v.convertTo(context, ValueType::Number).numberValue(context)};
  case ValueType::String:
    if (const auto result = v.tryConvertTo(context, ValueType::Number))
    {
      return Value{result->numberValue(context)};
    }
  case ValueType::Array:
  case ValueType::Map:
  case ValueType::Range:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }
  throw EvaluationError{expressionNode, "Invalid type {}"};
}

Value evaluateUnaryMinus(
  EvaluationContext& context, const Value& v, const ExpressionNode& expressionNode)
{
  switch (v.type())
  {
  case ValueType::Boolean:
  case ValueType::Number:
    return Value{-v.convertTo(context, ValueType::Number).numberValue(context)};
  case ValueType::String:
    if (const auto result = v.tryConvertTo(context, ValueType::Number))
    {
      return Value{-result->numberValue(context)};
    }
  case ValueType::Array:
  case ValueType::Map:
  case ValueType::Range:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }
  throw EvaluationError{expressionNode, "Invalid type {}"};
}

Value evaluateLogicalNegation(
  EvaluationContext& context, const Value& v, const ExpressionNode& expressionNode)
{
  switch (v.type())
  {
  case ValueType::Boolean:
    return Value{!v.booleanValue(context)};
  case ValueType::Number:
  case ValueType::String:
  case ValueType::Array:
  case ValueType::Map:
  case ValueType::Range:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }
  throw EvaluationError{expressionNode, "Invalid type {}"};
}

Value evaluateBitwiseNegation(
  EvaluationContext& context, const Value& v, const ExpressionNode& expressionNode)
{
  switch (v.type())
  {
  case ValueType::Number:
    return Value{~v.integerValue(context)};
  case ValueType::String:
    if (const auto result = v.tryConvertTo(context, ValueType::Number))
    {
      return Value{~result->integerValue(context)};
    }
  case ValueType::Boolean:
  case ValueType::Array:
  case ValueType::Map:
  case ValueType::Range:
  case ValueType::Null:
  case ValueType::Undefined:
    break;
  }
  throw EvaluationError{expressionNode, "Invalid type {}"};
}

Value evaluateLeftBoundedRange(EvaluationContext& context, const Value& v)
{
  const auto first =
    static_cast<long>(v.convertTo(context, ValueType::Number).numberValue(context));
  return context.trace(Value{LeftBoundedRange{first}}, v);
}

Value evaluateRightBoundedRange(EvaluationContext& context, const Value& v)
{
  const auto last =
    static_cast<long>(v.convertTo(context, ValueType::Number).numberValue(context));
  return context.trace(Value{RightBoundedRange{last}}, v);
}

Value evaluateUnaryExpression(
  EvaluationContext& context,
  const UnaryOperation& operator_,
  const Value& operand,
  const ExpressionNode& expressionNode)
{
  if (operand == Value::Undefined)
  {
    return Value::Undefined;
  }

  switch (operator_)
  {
  case UnaryOperation::Plus:
    return evaluateUnaryPlus(context, operand, expressionNode);
  case UnaryOperation::Minus:
    return evaluateUnaryMinus(context, operand, expressionNode);
  case UnaryOperation::LogicalNegation:
    return evaluateLogicalNegation(context, operand, expressionNode);
  case UnaryOperation::BitwiseNegation:
    return evaluateBitwiseNegation(context, operand, expressionNode);
  case UnaryOperation::Group:
    return context.trace(Value{operand}, operand);
  case UnaryOperation::LeftBoundedRange:
    return evaluateLeftBoundedRange(context, operand);
  case UnaryOperation::RightBoundedRange:
    return evaluateRightBoundedRange(context, operand);
    switchDefault();
  }
}

template <typename Evaluator>
Value evaluate(
  EvaluationContext& context,
  const Evaluator& evaluator,
  const UnaryExpression& expression,
  const ExpressionNode& expressionNode)
{
  return evaluateUnaryExpression(
    context, expression.operation, expression.operand.accept(evaluator), expressionNode);
}

template <typename Eval>
std::optional<Value> tryEvaluateAlgebraicOperator(
  EvaluationContext& context, const Value& lhs, const Value& rhs, const Eval& eval)
{
  if (lhs.hasType(ValueType::Undefined) || rhs.hasType(ValueType::Undefined))
  {
    return Value::Undefined;
  }

  if (
    lhs.hasType(ValueType::Boolean, ValueType::Number)
    && rhs.hasType(ValueType::Boolean, ValueType::Number))
  {
    return Value{eval(
      lhs.convertTo(context, ValueType::Number),
      rhs.convertTo(context, ValueType::Number))};
  }

  if (
    lhs.hasType(ValueType::Boolean, ValueType::Number) && rhs.hasType(ValueType::String))
  {
    if (const auto rhsAsNumber = rhs.tryConvertTo(context, ValueType::Number))
    {
      return Value{eval(lhs.convertTo(context, ValueType::Number), *rhsAsNumber)};
    }
  }

  if (
    lhs.hasType(ValueType::String) && rhs.hasType(ValueType::Boolean, ValueType::Number))
  {
    if (const auto lhsAsNumber = lhs.tryConvertTo(context, ValueType::Number))
    {
      return Value{eval(*lhsAsNumber, rhs.convertTo(context, ValueType::Number))};
    }
  }

  return std::nullopt;
}

Value evaluateAddition(
  EvaluationContext& context,
  const Value& lhs,
  const Value& rhs,
  const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      context, lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.numberValue(context) + rhsNumber.numberValue(context);
      }))
  {
    return *result;
  }

  if (lhs.hasType(ValueType::String) && rhs.hasType(ValueType::String))
  {
    return Value{
      lhs.convertTo(context, ValueType::String).stringValue(context)
      + rhs.convertTo(context, ValueType::String).stringValue(context)};
  }

  if (lhs.hasType(ValueType::Array) && rhs.hasType(ValueType::Array))
  {
    return Value{kdl::vec_concat(lhs.arrayValue(context), rhs.arrayValue(context))};
  }

  if (lhs.hasType(ValueType::Map) && rhs.hasType(ValueType::Map))
  {
    return Value{kdl::map_union(lhs.mapValue(context), rhs.mapValue(context))};
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateSubtraction(
  EvaluationContext& context,
  const Value& lhs,
  const Value& rhs,
  const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      context, lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.numberValue(context) - rhsNumber.numberValue(context);
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateMultiplication(
  EvaluationContext& context,
  const Value& lhs,
  const Value& rhs,
  const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      context, lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.numberValue(context) * rhsNumber.numberValue(context);
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateDivision(
  EvaluationContext& context,
  const Value& lhs,
  const Value& rhs,
  const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      context, lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.numberValue(context) / rhsNumber.numberValue(context);
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateModulus(
  EvaluationContext& context,
  const Value& lhs,
  const Value& rhs,
  const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      context, lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return std::fmod(lhsNumber.numberValue(context), rhsNumber.numberValue(context));
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

template <typename EvaluateLhs, typename EvaluateRhs>
Value evaluateLogicalAnd(
  EvaluationContext& context,
  const EvaluateLhs& evaluateLhs,
  const EvaluateRhs& evaluateRhs,
  const ExpressionNode& expressionNode)
{
  const auto lhs = evaluateLhs();
  auto rhs = std::make_optional<Value>();

  if (lhs.hasType(ValueType::Undefined))
  {
    return Value::Undefined;
  }

  if (lhs.hasType(ValueType::Boolean, ValueType::Null))
  {
    const auto lhsValue =
      lhs.convertTo(context, ValueType::Boolean).booleanValue(context);
    if (!lhsValue)
    {
      return Value{false};
    }

    rhs = evaluateRhs();
    if (rhs->hasType(ValueType::Boolean, ValueType::Null))
    {
      return Value{rhs->convertTo(context, ValueType::Boolean).booleanValue(context)};
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
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs->typeName())};
}

template <typename EvaluateLhs, typename EvaluateRhs>
Value evaluateLogicalOr(
  EvaluationContext& context,
  const EvaluateLhs& evaluateLhs,
  const EvaluateRhs& evaluateRhs,
  const ExpressionNode& expressionNode)
{
  const auto lhs = evaluateLhs();
  auto rhs = std::make_optional<Value>();

  if (lhs.hasType(ValueType::Undefined))
  {
    return Value::Undefined;
  }

  if (lhs.hasType(ValueType::Boolean, ValueType::Null))
  {
    const auto lhsValue =
      lhs.convertTo(context, ValueType::Boolean).booleanValue(context);
    if (lhsValue)
    {
      return Value{true};
    }

    rhs = evaluateRhs();
    if (rhs->hasType(ValueType::Boolean, ValueType::Null))
    {
      return Value{rhs->convertTo(context, ValueType::Boolean).booleanValue(context)};
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
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs->typeName())};
}

template <typename Eval>
std::optional<Value> tryEvaluateBitwiseOperator(
  EvaluationContext& context, const Value& lhs, const Value& rhs, const Eval& eval)
{
  if (lhs.hasType(ValueType::Undefined) || rhs.hasType(ValueType::Undefined))
  {
    return Value::Undefined;
  }

  if (lhs.convertibleTo(ValueType::Number) && rhs.convertibleTo(ValueType::Number))
  {
    return Value{eval(
      lhs.convertTo(context, ValueType::Number),
      rhs.convertTo(context, ValueType::Number))};
  }

  return std::nullopt;
}

Value evaluateBitwiseAnd(
  EvaluationContext& context,
  const Value& lhs,
  const Value& rhs,
  const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      context, lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue(context) & rhsNumber.integerValue(context);
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateBitwiseXOr(
  EvaluationContext& context,
  const Value& lhs,
  const Value& rhs,
  const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      context, lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue(context) ^ rhsNumber.integerValue(context);
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateBitwiseOr(
  EvaluationContext& context,
  const Value& lhs,
  const Value& rhs,
  const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      context, lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue(context) | rhsNumber.integerValue(context);
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateBitwiseShiftLeft(
  EvaluationContext& context,
  const Value& lhs,
  const Value& rhs,
  const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      context, lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue(context) << rhsNumber.integerValue(context);
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateBitwiseShiftRight(
  EvaluationContext& context,
  const Value& lhs,
  const Value& rhs,
  const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      context, lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue(context) >> rhsNumber.integerValue(context);
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

int compareAsBooleans(EvaluationContext& context, const Value& lhs, const Value& rhs)
{
  const bool lhsValue = lhs.convertTo(context, ValueType::Boolean).booleanValue(context);
  const bool rhsValue = rhs.convertTo(context, ValueType::Boolean).booleanValue(context);
  return lhsValue == rhsValue ? 0 : lhsValue ? 1 : -1;
}

int compareAsNumbers(EvaluationContext& context, const Value& lhs, const Value& rhs)
{
  const auto diff = lhs.convertTo(context, ValueType::Number).numberValue(context)
                    - rhs.convertTo(context, ValueType::Number).numberValue(context);
  return diff < 0.0 ? -1 : diff > 0.0 ? 1 : 0;
}

int evaluateCompare(
  EvaluationContext& context,
  const Value& lhs,
  const Value& rhs,
  const ExpressionNode& expressionNode)
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
        return compareAsBooleans(context, lhs, rhs);
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
        return compareAsBooleans(context, lhs, rhs);
      case ValueType::Number:
      case ValueType::String:
        return compareAsNumbers(context, lhs, rhs);
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
        return compareAsBooleans(context, lhs, rhs);
      case ValueType::Number:
        return compareAsNumbers(context, lhs, rhs);
      case ValueType::String:
        return lhs.stringValue(context).compare(
          rhs.convertTo(context, ValueType::String).stringValue(context));
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
      return rhs.hasType(ValueType::Null) ? 0 : -1;
    case ValueType::Undefined:
      return rhs.hasType(ValueType::Undefined) ? 0 : -1;
    case ValueType::Array:
      switch (rhs.type())
      {
      case ValueType::Array:
        return kdl::col_lexicographical_compare(
          lhs.arrayValue(context),
          rhs.arrayValue(context),
          [&](const auto& l, const auto& r) {
            return evaluateCompare(context, l, r, expressionNode) < 0;
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
          lhs.mapValue(context),
          rhs.mapValue(context),
          [&](const auto& l, const auto& r) {
            return evaluateCompare(context, l, r, expressionNode) < 0;
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

    throw EvaluationError{
      expressionNode,
      fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
  }
  catch (const ConversionError& c)
  {
    throw EvaluationError{expressionNode, c.what()};
  }
}

Value evaluateBoundedRange(EvaluationContext& context, const Value& lhs, const Value& rhs)
{
  if (lhs.hasType(ValueType::Undefined) || rhs.hasType(ValueType::Undefined))
  {
    return Value::Undefined;
  }

  const auto from =
    static_cast<long>(lhs.convertTo(context, ValueType::Number).numberValue(context));
  const auto to =
    static_cast<long>(rhs.convertTo(context, ValueType::Number).numberValue(context));

  return Value{BoundedRange{from, to}};
}

template <typename EvaluateLhs, typename EvaluateRhs>
Value evaluateCoalesce(
  EvaluationContext& context,
  const EvaluateLhs& evaluateLhs,
  const EvaluateRhs& evaluateRhs)
{
  const auto lhs = evaluateLhs();
  const auto type = lhs.type();

  if (type == ValueType::Undefined || type == ValueType::Null || (type == ValueType::String && !lhs.convertTo(context, ValueType::Boolean).booleanValue(context)))
  {
    return evaluateRhs();
  }

  return lhs;
}

template <typename EvaluateLhs, typename EvaluateRhs>
Value evaluateCase(
  EvaluationContext& context,
  const EvaluateLhs& evaluateLhs,
  const EvaluateRhs& evaluateRhs)
{
  const auto lhs = evaluateLhs();

  if (
    lhs.type() != ValueType::Undefined
    && lhs.convertTo(context, ValueType::Boolean).booleanValue(context))
  {
    return evaluateRhs();
  }

  return Value::Undefined;
}

template <typename EvalualateLhs, typename EvaluateRhs>
Value evaluateBinaryExpression(
  EvaluationContext& context,
  const BinaryOperation operator_,
  const EvalualateLhs& evaluateLhs,
  const EvaluateRhs& evaluateRhs,
  const ExpressionNode& expressionNode)
{
  switch (operator_)
  {
  case BinaryOperation::Addition:
    return evaluateAddition(context, evaluateLhs(), evaluateRhs(), expressionNode);
  case BinaryOperation::Subtraction:
    return evaluateSubtraction(context, evaluateLhs(), evaluateRhs(), expressionNode);
  case BinaryOperation::Multiplication:
    return evaluateMultiplication(context, evaluateLhs(), evaluateRhs(), expressionNode);
  case BinaryOperation::Division:
    return evaluateDivision(context, evaluateLhs(), evaluateRhs(), expressionNode);
  case BinaryOperation::Modulus:
    return evaluateModulus(context, evaluateLhs(), evaluateRhs(), expressionNode);
  case BinaryOperation::LogicalAnd:
    return evaluateLogicalAnd(context, evaluateLhs, evaluateRhs, expressionNode);
  case BinaryOperation::LogicalOr:
    return evaluateLogicalOr(context, evaluateLhs, evaluateRhs, expressionNode);
  case BinaryOperation::BitwiseAnd:
    return evaluateBitwiseAnd(context, evaluateLhs(), evaluateRhs(), expressionNode);
  case BinaryOperation::BitwiseXOr:
    return evaluateBitwiseXOr(context, evaluateLhs(), evaluateRhs(), expressionNode);
  case BinaryOperation::BitwiseOr:
    return evaluateBitwiseOr(context, evaluateLhs(), evaluateRhs(), expressionNode);
  case BinaryOperation::BitwiseShiftLeft:
    return evaluateBitwiseShiftLeft(
      context, evaluateLhs(), evaluateRhs(), expressionNode);
  case BinaryOperation::BitwiseShiftRight:
    return evaluateBitwiseShiftRight(
      context, evaluateLhs(), evaluateRhs(), expressionNode);
  case BinaryOperation::Less:
    return Value{
      evaluateCompare(context, evaluateLhs(), evaluateRhs(), expressionNode) < 0};
  case BinaryOperation::LessOrEqual:
    return Value{
      evaluateCompare(context, evaluateLhs(), evaluateRhs(), expressionNode) <= 0};
  case BinaryOperation::Greater:
    return Value{
      evaluateCompare(context, evaluateLhs(), evaluateRhs(), expressionNode) > 0};
  case BinaryOperation::GreaterOrEqual:
    return Value{
      evaluateCompare(context, evaluateLhs(), evaluateRhs(), expressionNode) >= 0};
  case BinaryOperation::Equal:
    return Value{
      evaluateCompare(context, evaluateLhs(), evaluateRhs(), expressionNode) == 0};
  case BinaryOperation::NotEqual:
    return Value{
      evaluateCompare(context, evaluateLhs(), evaluateRhs(), expressionNode) != 0};
  case BinaryOperation::BoundedRange:
    return Value{evaluateBoundedRange(context, evaluateLhs(), evaluateRhs())};
  case BinaryOperation::Coalesce:
    return evaluateCoalesce(context, evaluateLhs, evaluateRhs);
  case BinaryOperation::Case:
    return evaluateCase(context, evaluateLhs, evaluateRhs);
    switchDefault();
  };
}

template <typename Evaluator>
Value evaluate(
  EvaluationContext& context,
  const Evaluator& evaluator,
  const BinaryExpression& expression,
  const ExpressionNode& expressionNode)
{
  return evaluateBinaryExpression(
    context,
    expression.operation,
    [&] { return expression.leftOperand.accept(evaluator); },
    [&] { return expression.rightOperand.accept(evaluator); },
    expressionNode);
}

size_t computeIndex(const long index, const size_t indexableSize)
{
  const auto size = static_cast<long>(indexableSize);
  return (index >= 0 && index < size) || (index < 0 && index >= -size)
           ? static_cast<size_t>((size + index % size) % size)
           : static_cast<size_t>(size);
}

size_t computeIndex(
  EvaluationContext& context, const Value& indexValue, const size_t indexableSize)
{
  return computeIndex(
    static_cast<long>(
      indexValue.convertTo(context, ValueType::Number).numberValue(context)),
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
  EvaluationContext& context,
  const Value& indexValue,
  const size_t indexableSize,
  std::vector<size_t>& result)
{
  switch (indexValue.type())
  {
  case ValueType::Array: {
    const auto& indexArray = indexValue.arrayValue(context);
    result.reserve(result.size() + indexArray.size());
    for (size_t i = 0; i < indexArray.size(); ++i)
    {
      computeIndexArray(context, indexArray[i], indexableSize, result);
    }
    break;
  }
  case ValueType::Range: {
    std::visit(
      [&](const auto& x) { computeIndexArray(x, indexableSize, result); },
      indexValue.rangeValue(context));
    break;
  }
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::String:
  case ValueType::Map:
  case ValueType::Null:
  case ValueType::Undefined:
    result.push_back(computeIndex(context, indexValue, indexableSize));
    break;
  }
}

std::vector<size_t> computeIndexArray(
  EvaluationContext& context, const Value& indexValue, const size_t indexableSize)
{
  auto result = std::vector<size_t>{};
  computeIndexArray(context, indexValue, indexableSize, result);
  return result;
}

Value evaluateSubscript(
  EvaluationContext& context,
  const Value& lhs,
  const Value& rhs,
  const ExpressionNode& expressionNode)
{
  switch (lhs.type())
  {
  case ValueType::String:
    switch (rhs.type())
    {
    case ValueType::Boolean:
    case ValueType::Number: {
      const auto& str = lhs.stringValue(context);
      const auto index = computeIndex(context, rhs, str.length());
      auto result = std::stringstream{};
      if (index < str.length())
      {
        result << str[index];
      }
      return Value{result.str()};
    }
    case ValueType::Array:
    case ValueType::Range: {
      const auto& str = lhs.stringValue(context);
      const auto indices = computeIndexArray(context, rhs, str.length());
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
      const auto& array = lhs.arrayValue(context);
      const auto index = computeIndex(context, rhs, array.size());
      if (index >= array.size())
      {
        throw IndexOutOfBoundsError{expressionNode, lhs, index};
      }
      return array[index];
    }
    case ValueType::Array:
    case ValueType::Range: {
      const auto& array = lhs.arrayValue(context);
      const auto indices = computeIndexArray(context, rhs, array.size());
      auto result = ArrayType{};
      result.reserve(indices.size());
      for (size_t i = 0; i < indices.size(); ++i)
      {
        const auto index = indices[i];
        if (index >= array.size())
        {
          throw IndexOutOfBoundsError{expressionNode, lhs, index};
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
      const auto& map = lhs.mapValue(context);
      const auto& key = rhs.stringValue(context);
      const auto it = map.find(key);
      if (it == std::end(map))
      {
        return Value{UndefinedType::Value};
      }
      return it->second;
    }
    case ValueType::Array: {
      const auto& map = lhs.mapValue(context);
      const auto& keys = rhs.arrayValue(context);
      auto result = MapType{};
      for (size_t i = 0; i < keys.size(); ++i)
      {
        const auto& keyValue = keys[i];
        if (keyValue.type() != ValueType::String)
        {
          throw ConversionError{
            context.location(keyValue),
            keyValue.describe(),
            keyValue.type(),
            ValueType::String};
        }
        const auto& key = keyValue.stringValue(context);
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

  throw IndexError{expressionNode, lhs, rhs};
}

template <typename Evaluator>
Value evaluate(
  EvaluationContext& context,
  const Evaluator& evaluator,
  const SubscriptExpression& expression,
  const ExpressionNode& expressionNode)
{
  return evaluateSubscript(
    context,
    expression.leftOperand.accept(evaluator),
    expression.rightOperand.accept(evaluator),
    expressionNode);
}

template <typename Evaluator>
Value evaluate(
  EvaluationContext&,
  const Evaluator& evaluator,
  const SwitchExpression& expression,
  const ExpressionNode&)
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


Expression optimize(
  EvaluationContext&, const LiteralExpression& expression, const ExpressionNode&)
{
  return LiteralExpression{expression.value};
}

Expression optimize(
  EvaluationContext&, const VariableExpression& expression, const ExpressionNode&)
{
  return VariableExpression{expression.variableName};
}

Expression optimize(
  EvaluationContext& context, const ArrayExpression& expression, const ExpressionNode&)
{
  auto optimizedExpressions =
    expression.elements
    | std::views::transform([&](const auto& x) { return x.optimize(context); });

  const auto isLiteral = std::ranges::all_of(
    optimizedExpressions, [](const auto& x) { return x.isLiteral(); });

  if (isLiteral)
  {
    return LiteralExpression{Value{
      optimizedExpressions
      | std::views::transform([&](const auto& x) { return x.evaluate(context); })
      | kdl::to_vector}};
  }

  return ArrayExpression{std::move(optimizedExpressions) | kdl::to_vector};
}

Expression optimize(
  EvaluationContext& context, const MapExpression& expression, const ExpressionNode&)
{
  auto optimizedExpressions =
    expression.elements | std::views::transform([&](const auto& entry) {
      return std::pair{entry.first, entry.second.optimize(context)};
    });

  const auto isLiteral = std::ranges::all_of(
    optimizedExpressions, [](const auto& entry) { return entry.second.isLiteral(); });

  if (isLiteral)
  {
    return LiteralExpression{Value{
      optimizedExpressions | std::views::transform([&](const auto& entry) {
        return std::pair{entry.first, entry.second.evaluate(context)};
      })
      | kdl::to<MapType>()}};
  }

  return MapExpression{
    std::move(optimizedExpressions) | kdl::to<std::map<std::string, ExpressionNode>>()};
}

Expression optimize(
  EvaluationContext& context,
  const UnaryExpression& expression,
  const ExpressionNode& expressionNode)
{
  auto optimizedOperand = expression.operand.optimize(context);

  if (optimizedOperand.isLiteral())
  {
    return LiteralExpression{evaluateUnaryExpression(
      context, expression.operation, optimizedOperand.evaluate(context), expressionNode)};
  }

  return UnaryExpression{expression.operation, std::move(optimizedOperand)};
}

Expression optimize(
  EvaluationContext& context,
  const BinaryExpression& expression,
  const ExpressionNode& expressionNode)
{
  auto optimizedLeftOperand = std::optional<ExpressionNode>{};
  auto optimizedRightOperand = std::optional<ExpressionNode>{};

  const auto evaluateLeftOperand = [&] {
    optimizedLeftOperand = expression.leftOperand.optimize(context);
    return optimizedLeftOperand->evaluate(context);
  };

  const auto evaluateRightOperand = [&] {
    optimizedRightOperand = expression.rightOperand.optimize(context);
    return optimizedRightOperand->evaluate(context);
  };

  auto value = evaluateBinaryExpression(
    context,
    expression.operation,
    evaluateLeftOperand,
    evaluateRightOperand,
    expressionNode);

  const auto isLiteral =
    (!optimizedLeftOperand || optimizedLeftOperand->isLiteral())
    && (!optimizedRightOperand || optimizedRightOperand->isLiteral());

  if (isLiteral)
  {
    return LiteralExpression{std::move(value)};
  }

  return BinaryExpression{
    expression.operation,
    std::move(optimizedLeftOperand).value_or(expression.leftOperand.optimize(context)),
    std::move(optimizedRightOperand).value_or(expression.rightOperand.optimize(context))};
}

Expression optimize(
  EvaluationContext& context,
  const SubscriptExpression& expression,
  const ExpressionNode& expressionNode)
{
  auto optimizedLeftOperand = expression.leftOperand.optimize(context);
  auto optimizedRightOperand = expression.rightOperand.optimize(context);

  const auto isLiteral =
    optimizedLeftOperand.isLiteral() && optimizedRightOperand.isLiteral();

  if (isLiteral)
  {
    const auto leftValue = optimizedLeftOperand.evaluate(context);
    const auto rightValue = optimizedRightOperand.evaluate(context);
    return LiteralExpression{
      evaluateSubscript(context, leftValue, rightValue, expressionNode)};
  }

  return SubscriptExpression{
    std::move(optimizedLeftOperand), std::move(optimizedRightOperand)};
}

Expression optimize(
  EvaluationContext& context, const SwitchExpression& expression, const ExpressionNode&)
{
  if (expression.cases.empty())
  {
    return LiteralExpression{Value::Undefined};
  }

  auto firstOptimizedExpression = expression.cases.front().optimize(context);
  if (firstOptimizedExpression.isLiteral())
  {
    return LiteralExpression{firstOptimizedExpression.evaluate(context)};
  }

  auto optimizedExpressions =
    std::vector<ExpressionNode>{std::move(firstOptimizedExpression)};
  for (size_t i = 1u; i < expression.cases.size(); ++i)
  {
    optimizedExpressions.push_back(expression.cases[i].optimize(context));
  }

  return SwitchExpression{std::move(optimizedExpressions)};
}

Expression optimizeExpression(
  EvaluationContext& context,
  const Expression& expression,
  const ExpressionNode& expressionNode)
{
  return std::visit(
    [&](const auto& x) { return optimize(context, x, expressionNode); }, expression);
}


size_t precedence(const BinaryOperation operation)
{
  switch (operation)
  {
  case BinaryOperation::Multiplication:
  case BinaryOperation::Division:
  case BinaryOperation::Modulus:
    return 13;
  case BinaryOperation::Addition:
  case BinaryOperation::Subtraction:
    return 12;
  case BinaryOperation::BitwiseShiftLeft:
  case BinaryOperation::BitwiseShiftRight:
    return 11;
  case BinaryOperation::Less:
  case BinaryOperation::LessOrEqual:
  case BinaryOperation::Greater:
  case BinaryOperation::GreaterOrEqual:
    return 10;
  case BinaryOperation::Equal:
  case BinaryOperation::NotEqual:
    return 9;
  case BinaryOperation::BitwiseAnd:
    return 8;
  case BinaryOperation::BitwiseXOr:
    return 7;
  case BinaryOperation::BitwiseOr:
    return 6;
  case BinaryOperation::LogicalAnd:
    return 5;
  case BinaryOperation::LogicalOr:
    return 4;
  case BinaryOperation::Coalesce:
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

Value ExpressionNode::evaluate(EvaluationContext& context) const
{
  return accept(
    [&](const auto& evaluator, const auto& expression, const auto& containingNode) {
      return context.trace(
        el::evaluate(context, evaluator, expression, containingNode), containingNode);
    });
}

Value ExpressionNode::tryEvaluate(EvaluationContext& context) const
{
  return accept(
    [&](const auto& evaluator, const auto& expression, const auto& containingNode) {
      try
      {
        return context.trace(
          el::evaluate(context, evaluator, expression, containingNode), containingNode);
      }
      catch (const EvaluationError&)
      {
        return Value::Undefined;
      }
    });
}

ExpressionNode ExpressionNode::optimize(EvaluationContext& context) const
{
  return ExpressionNode{
    std::make_shared<Expression>(optimizeExpression(context, *m_expression, *this)),
    m_location};
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
  case BinaryOperation::Coalesce:
    lhs << rhs.leftOperand << "??" << rhs.rightOperand;
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
