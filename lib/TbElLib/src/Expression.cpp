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

#include "el/Expression.h"

#include "el/EvaluationContext.h"
#include "el/Exceptions.h"
#include "el/Value.h"

#include "kd/flat_map.h"
#include "kd/map_utils.h"
#include "kd/overload.h"
#include "kd/ranges/concat_view.h"
#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"
#include "kd/string_compare.h"
#include "kd/unpack.h"

#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <functional>
#include <ranges>
#include <span>
#include <sstream>
#include <string_view>

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
  EvaluationContext&,
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
      const auto range = std::get<BoundedRange>(value.rangeValue());
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

Value evaluateUnaryPlus(const Value& v, const ExpressionNode& expressionNode)
{
  switch (v.type())
  {
  case ValueType::Boolean:
  case ValueType::Number:
    return Value{v.convertTo(ValueType::Number).numberValue()};
  case ValueType::Vec3:
    return Value{+v.vec3Value()};
  case ValueType::String:
    if (const auto result = v.tryConvertTo(ValueType::Number))
    {
      return Value{result->numberValue()};
    }
    [[fallthrough]];
  case ValueType::Array:
    [[fallthrough]];
  case ValueType::Map:
    [[fallthrough]];
  case ValueType::Range:
    [[fallthrough]];
  case ValueType::BBox:
    [[fallthrough]];
  case ValueType::Null:
    [[fallthrough]];
  case ValueType::Undefined:
    break;
  }
  throw EvaluationError{expressionNode, fmt::format("Invalid type {}", v.typeName())};
}

Value evaluateUnaryMinus(const Value& v, const ExpressionNode& expressionNode)
{
  switch (v.type())
  {
  case ValueType::Boolean:
    [[fallthrough]];
  case ValueType::Number:
    return Value{-v.convertTo(ValueType::Number).numberValue()};
  case ValueType::Vec3:
    return Value{-v.vec3Value()};
  case ValueType::String:
    if (const auto result = v.tryConvertTo(ValueType::Number))
    {
      return Value{-result->numberValue()};
    }
    [[fallthrough]];
  case ValueType::Array:
    [[fallthrough]];
  case ValueType::Map:
    [[fallthrough]];
  case ValueType::Range:
    [[fallthrough]];
  case ValueType::BBox:
    [[fallthrough]];
  case ValueType::Null:
    [[fallthrough]];
  case ValueType::Undefined:
    break;
  }
  throw EvaluationError{expressionNode, fmt::format("Invalid type {}", v.typeName())};
}

Value evaluateLogicalNegation(const Value& v, const ExpressionNode& expressionNode)
{
  switch (v.type())
  {
  case ValueType::Boolean:
    return Value{!v.booleanValue()};
  case ValueType::Number:
    [[fallthrough]];
  case ValueType::String:
    [[fallthrough]];
  case ValueType::Array:
    [[fallthrough]];
  case ValueType::Map:
    [[fallthrough]];
  case ValueType::Range:
    [[fallthrough]];
  case ValueType::Vec3:
    [[fallthrough]];
  case ValueType::BBox:
    [[fallthrough]];
  case ValueType::Null:
    [[fallthrough]];
  case ValueType::Undefined:
    break;
  }
  throw EvaluationError{expressionNode, fmt::format("Invalid type {}", v.typeName())};
}

Value evaluateBitwiseNegation(const Value& v, const ExpressionNode& expressionNode)
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
    [[fallthrough]];
  case ValueType::Boolean:
    [[fallthrough]];
  case ValueType::Array:
    [[fallthrough]];
  case ValueType::Map:
    [[fallthrough]];
  case ValueType::Range:
    [[fallthrough]];
  case ValueType::Vec3:
    [[fallthrough]];
  case ValueType::BBox:
    [[fallthrough]];
  case ValueType::Null:
    [[fallthrough]];
  case ValueType::Undefined:
    break;
  }
  throw EvaluationError{expressionNode, fmt::format("Invalid type {}", v.typeName())};
}

Value evaluateLeftBoundedRange(const Value& v)
{
  const auto first = static_cast<long>(v.convertTo(ValueType::Number).numberValue());
  return Value{LeftBoundedRange{first}}.producedBy(v);
}

Value evaluateRightBoundedRange(const Value& v)
{
  const auto last = static_cast<long>(v.convertTo(ValueType::Number).numberValue());
  return Value{RightBoundedRange{last}}.producedBy(v);
}

Value evaluateUnaryExpression(
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
    return evaluateUnaryPlus(operand, expressionNode);
  case UnaryOperation::Minus:
    return evaluateUnaryMinus(operand, expressionNode);
  case UnaryOperation::LogicalNegation:
    return evaluateLogicalNegation(operand, expressionNode);
  case UnaryOperation::BitwiseNegation:
    return evaluateBitwiseNegation(operand, expressionNode);
  case UnaryOperation::Group:
    return operand;
  case UnaryOperation::LeftBoundedRange:
    return evaluateLeftBoundedRange(operand);
  case UnaryOperation::RightBoundedRange:
    return evaluateRightBoundedRange(operand);
    switchDefault();
  }
}

template <typename Evaluator>
Value evaluate(
  EvaluationContext&,
  const Evaluator& evaluator,
  const UnaryExpression& expression,
  const ExpressionNode& expressionNode)
{
  return evaluateUnaryExpression(
    expression.operation, expression.operand.accept(evaluator), expressionNode);
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

std::optional<Vec3Type> tryConvertToVec3(const Value& value)
{
  const auto vec3 = value.tryConvertTo(ValueType::Vec3);
  return vec3 ? std::optional{vec3->vec3Value()} : std::nullopt;
}

std::optional<NumberType> tryConvertToScalar(const Value& value)
{
  if (value.hasType(ValueType::Boolean, ValueType::Number, ValueType::String))
  {
    if (const auto number = value.tryConvertTo(ValueType::Number))
    {
      return number->numberValue();
    }
  }

  return std::nullopt;
}

// a string is only converted to a vector if the other operand is a vector
template <typename Eval>
std::optional<Value> tryEvaluateVec3Operator(
  const Value& lhs, const Value& rhs, const Eval& eval)
{
  if (lhs.hasType(ValueType::Vec3) || rhs.hasType(ValueType::Vec3))
  {
    const auto lhsVec3 = tryConvertToVec3(lhs);
    const auto rhsVec3 = tryConvertToVec3(rhs);
    if (lhsVec3 && rhsVec3)
    {
      return Value{eval(*lhsVec3, *rhsVec3)};
    }
  }

  return std::nullopt;
}

template <typename Eval>
std::optional<Value> tryEvaluateScalarVec3Operator(
  const Value& lhs, const Value& rhs, const Eval& eval)
{
  if (lhs.hasType(ValueType::Vec3))
  {
    if (const auto rhsScalar = tryConvertToScalar(rhs))
    {
      return Value{eval(lhs.vec3Value(), *rhsScalar)};
    }
  }

  if (rhs.hasType(ValueType::Vec3))
  {
    if (const auto lhsScalar = tryConvertToScalar(lhs))
    {
      return Value{eval(*lhsScalar, rhs.vec3Value())};
    }
  }

  return std::nullopt;
}

Value evaluateAddition(
  const Value& lhs, const Value& rhs, const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
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
    return Value{
      kdl::views::concat(lhs.arrayValue(), rhs.arrayValue())
      | kdl::ranges::to<std::vector>()};
  }

  if (lhs.hasType(ValueType::Map) && rhs.hasType(ValueType::Map))
  {
    return Value{kdl::map_union(lhs.mapValue(), rhs.mapValue())};
  }

  if (
    const auto result =
      tryEvaluateVec3Operator(lhs, rhs, [](const auto& lhsVec3, const auto& rhsVec3) {
        return lhsVec3 + rhsVec3;
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateSubtraction(
  const Value& lhs, const Value& rhs, const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.numberValue() - rhsNumber.numberValue();
      }))
  {
    return *result;
  }

  if (
    const auto result =
      tryEvaluateVec3Operator(lhs, rhs, [](const auto& lhsVec3, const auto& rhsVec3) {
        return lhsVec3 - rhsVec3;
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateMultiplication(
  const Value& lhs, const Value& rhs, const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.numberValue() * rhsNumber.numberValue();
      }))
  {
    return *result;
  }

  const auto multiply = [](const auto& lhsValue, const auto& rhsValue) {
    return lhsValue * rhsValue;
  };

  if (const auto result = tryEvaluateVec3Operator(lhs, rhs, multiply))
  {
    return *result;
  }

  if (const auto result = tryEvaluateScalarVec3Operator(lhs, rhs, multiply))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateDivision(
  const Value& lhs, const Value& rhs, const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.numberValue() / rhsNumber.numberValue();
      }))
  {
    return *result;
  }

  const auto divide = [](const auto& lhsValue, const auto& rhsValue) {
    return lhsValue / rhsValue;
  };

  if (const auto result = tryEvaluateVec3Operator(lhs, rhs, divide))
  {
    return *result;
  }

  if (const auto result = tryEvaluateScalarVec3Operator(lhs, rhs, divide))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateModulus(
  const Value& lhs, const Value& rhs, const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateAlgebraicOperator(
      lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return std::fmod(lhsNumber.numberValue(), rhsNumber.numberValue());
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
  const EvaluateLhs& evaluateLhs,
  const EvaluateRhs& evaluateRhs,
  const ExpressionNode& expressionNode)
{
  const auto lhs = evaluateLhs();
  auto rhs = std::optional<Value>{};

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
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs->typeName())};
}

template <typename EvaluateLhs, typename EvaluateRhs>
Value evaluateLogicalOr(
  const EvaluateLhs& evaluateLhs,
  const EvaluateRhs& evaluateRhs,
  const ExpressionNode& expressionNode)
{
  const auto lhs = evaluateLhs();
  auto rhs = std::optional<Value>{};

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
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs->typeName())};
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

Value evaluateBitwiseAnd(
  const Value& lhs, const Value& rhs, const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() & rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateBitwiseXOr(
  const Value& lhs, const Value& rhs, const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() ^ rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateBitwiseOr(
  const Value& lhs, const Value& rhs, const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() | rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateBitwiseShiftLeft(
  const Value& lhs, const Value& rhs, const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() << rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

Value evaluateBitwiseShiftRight(
  const Value& lhs, const Value& rhs, const ExpressionNode& expressionNode)
{
  if (
    const auto result = tryEvaluateBitwiseOperator(
      lhs, rhs, [&](const auto& lhsNumber, const auto& rhsNumber) {
        return lhsNumber.integerValue() >> rhsNumber.integerValue();
      }))
  {
    return *result;
  }

  throw EvaluationError{
    expressionNode,
    fmt::format("Invalid operand types {} and {}", lhs.typeName(), rhs.typeName())};
}

int compareAsBooleans(const Value& lhs, const Value& rhs)
{
  const bool lhsValue = lhs.convertTo(ValueType::Boolean).booleanValue();
  const bool rhsValue = rhs.convertTo(ValueType::Boolean).booleanValue();
  return lhsValue == rhsValue ? 0 : lhsValue ? 1 : -1;
}

int compareAsNumbers(const Value& lhs, const Value& rhs)
{
  const auto diff = lhs.convertTo(ValueType::Number).numberValue()
                    - rhs.convertTo(ValueType::Number).numberValue();
  return diff < 0.0 ? -1 : diff > 0.0 ? 1 : 0;
}

int compareAsVec3s(const Value& lhs, const Value& rhs)
{
  const auto ordering = lhs.convertTo(ValueType::Vec3).vec3Value()
                        <=> rhs.convertTo(ValueType::Vec3).vec3Value();
  return ordering < 0 ? -1 : ordering > 0 ? 1 : 0;
}

int compareAsBBoxes(const Value& lhs, const Value& rhs)
{
  const auto& lhsBBox = lhs.bboxValue();
  const auto& rhsBBox = rhs.bboxValue();
  if (const auto ordering = lhsBBox.min <=> rhsBBox.min; ordering != 0)
  {
    return ordering < 0 ? -1 : 1;
  }
  const auto ordering = lhsBBox.max <=> rhsBBox.max;
  return ordering < 0 ? -1 : ordering > 0 ? 1 : 0;
}

int evaluateCompare(
  const Value& lhs, const Value& rhs, const ExpressionNode& expressionNode)
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
      case ValueType::Vec3:
      case ValueType::BBox:
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
      case ValueType::Vec3:
      case ValueType::BBox:
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
      case ValueType::Vec3:
        return compareAsVec3s(lhs, rhs);
      case ValueType::Null:
      case ValueType::Undefined:
        return 1;
      case ValueType::Array:
      case ValueType::Map:
      case ValueType::Range:
      case ValueType::BBox:
        break;
      }
      break;
    case ValueType::Null:
      // undefined sorts below everything else, null included
      return rhs.hasType(ValueType::Null)        ? 0
             : rhs.hasType(ValueType::Undefined) ? 1
                                                 : -1;
    case ValueType::Undefined:
      return rhs.hasType(ValueType::Undefined) ? 0 : -1;
    case ValueType::Array:
      switch (rhs.type())
      {
      case ValueType::Array:
        return kdl::col_lexicographical_compare(
          lhs.arrayValue(), rhs.arrayValue(), [&](const auto& l, const auto& r) {
            return evaluateCompare(l, r, expressionNode) < 0;
          });
      case ValueType::Null:
      case ValueType::Undefined:
        return 1;
      case ValueType::Boolean:
      case ValueType::Number:
      case ValueType::String:
      case ValueType::Map:
      case ValueType::Range:
      case ValueType::Vec3:
      case ValueType::BBox:
        break;
      }
      break;
    case ValueType::Map:
      switch (rhs.type())
      {
      case ValueType::Map:
        return kdl::map_lexicographical_compare(
          lhs.mapValue(), rhs.mapValue(), [&](const auto& l, const auto& r) {
            return evaluateCompare(l, r, expressionNode) < 0;
          });
      case ValueType::Null:
      case ValueType::Undefined:
        return 1;
      case ValueType::Boolean:
      case ValueType::Number:
      case ValueType::String:
      case ValueType::Array:
      case ValueType::Range:
      case ValueType::Vec3:
      case ValueType::BBox:
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
      case ValueType::Vec3:
      case ValueType::BBox:
        break;
      }
      break;
    case ValueType::Vec3:
      switch (rhs.type())
      {
      case ValueType::Vec3:
      case ValueType::String:
        return compareAsVec3s(lhs, rhs);
      case ValueType::Null:
      case ValueType::Undefined:
        return 1;
      case ValueType::Boolean:
      case ValueType::Number:
      case ValueType::Array:
      case ValueType::Map:
      case ValueType::Range:
      case ValueType::BBox:
        break;
      }
      break;
    case ValueType::BBox:
      switch (rhs.type())
      {
      case ValueType::BBox:
        return compareAsBBoxes(lhs, rhs);
      case ValueType::Null:
      case ValueType::Undefined:
        return 1;
      case ValueType::Boolean:
      case ValueType::Number:
      case ValueType::String:
      case ValueType::Array:
      case ValueType::Map:
      case ValueType::Range:
      case ValueType::Vec3:
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

Value evaluateCall(
  EvaluationContext& context,
  const std::string& name,
  std::span<const Value> arguments,
  const ExpressionNode& expressionNode);

template <typename EvalualateLhs, typename EvaluateRhs>
Value evaluateBinaryExpression(
  EvaluationContext& context,
  const BinaryOperation& operator_,
  const EvalualateLhs& evaluateLhs,
  const EvaluateRhs& evaluateRhs,
  const ExpressionNode& expressionNode)
{
  return std::visit(
    kdl::overload(
      [&](const binop::Addition&) {
        return evaluateAddition(evaluateLhs(), evaluateRhs(), expressionNode);
      },
      [&](const binop::Subtraction&) {
        return evaluateSubtraction(evaluateLhs(), evaluateRhs(), expressionNode);
      },
      [&](const binop::Multiplication&) {
        return evaluateMultiplication(evaluateLhs(), evaluateRhs(), expressionNode);
      },
      [&](const binop::Division&) {
        return evaluateDivision(evaluateLhs(), evaluateRhs(), expressionNode);
      },
      [&](const binop::Modulus&) {
        return evaluateModulus(evaluateLhs(), evaluateRhs(), expressionNode);
      },
      [&](const binop::LogicalAnd&) {
        return evaluateLogicalAnd(evaluateLhs, evaluateRhs, expressionNode);
      },
      [&](const binop::LogicalOr&) {
        return evaluateLogicalOr(evaluateLhs, evaluateRhs, expressionNode);
      },
      [&](const binop::BitwiseAnd&) {
        return evaluateBitwiseAnd(evaluateLhs(), evaluateRhs(), expressionNode);
      },
      [&](const binop::BitwiseXOr&) {
        return evaluateBitwiseXOr(evaluateLhs(), evaluateRhs(), expressionNode);
      },
      [&](const binop::BitwiseOr&) {
        return evaluateBitwiseOr(evaluateLhs(), evaluateRhs(), expressionNode);
      },
      [&](const binop::BitwiseShiftLeft&) {
        return evaluateBitwiseShiftLeft(evaluateLhs(), evaluateRhs(), expressionNode);
      },
      [&](const binop::BitwiseShiftRight&) {
        return evaluateBitwiseShiftRight(evaluateLhs(), evaluateRhs(), expressionNode);
      },
      [&](const binop::Less&) {
        return Value{evaluateCompare(evaluateLhs(), evaluateRhs(), expressionNode) < 0};
      },
      [&](const binop::LessOrEqual&) {
        return Value{evaluateCompare(evaluateLhs(), evaluateRhs(), expressionNode) <= 0};
      },
      [&](const binop::Greater&) {
        return Value{evaluateCompare(evaluateLhs(), evaluateRhs(), expressionNode) > 0};
      },
      [&](const binop::GreaterOrEqual&) {
        return Value{evaluateCompare(evaluateLhs(), evaluateRhs(), expressionNode) >= 0};
      },
      [&](const binop::Equal&) {
        return Value{evaluateCompare(evaluateLhs(), evaluateRhs(), expressionNode) == 0};
      },
      [&](const binop::NotEqual&) {
        return Value{evaluateCompare(evaluateLhs(), evaluateRhs(), expressionNode) != 0};
      },
      [&](const binop::BoundedRange&) {
        return Value{evaluateBoundedRange(evaluateLhs(), evaluateRhs())};
      },
      [&](const binop::Case&) { return evaluateCase(evaluateLhs, evaluateRhs); },
      [&](const binop::InfixCall& infixCall) {
        const auto arguments = std::array{evaluateLhs(), evaluateRhs()};
        return evaluateCall(context, infixCall.functionName, arguments, expressionNode);
      }),
    operator_);
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

std::optional<size_t> computeIndex(const long index, const size_t indexableSize)
{
  const auto size = static_cast<long>(indexableSize);
  return (index >= 0 && index < size) || (index < 0 && index >= -size)
           ? std::optional{static_cast<size_t>((size + index % size) % size)}
           : std::nullopt;
}

long toIndex(const Value& indexValue)
{
  return static_cast<long>(indexValue.convertTo(ValueType::Number).numberValue());
}

void computeIndexArray(
  const LeftBoundedRange& range, const size_t indexableSize, std::vector<long>& result)
{
  result.reserve(result.size() + range.length(indexableSize));
  range.forEach([&](const auto i) { result.push_back(i); }, indexableSize);
}

void computeIndexArray(
  const RightBoundedRange& range, const size_t indexableSize, std::vector<long>& result)
{
  result.reserve(result.size() + range.length(indexableSize));
  range.forEach([&](const auto i) { result.push_back(i); }, indexableSize);
}

void computeIndexArray(const BoundedRange& range, const size_t, std::vector<long>& result)
{
  result.reserve(result.size() + range.length());
  range.forEach([&](const auto i) { result.push_back(i); });
}

void computeIndexArray(
  const Value& indexValue, const size_t indexableSize, std::vector<long>& result)
{
  switch (indexValue.type())
  {
  case ValueType::Array: {
    const auto& indexArray = indexValue.arrayValue();
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
  case ValueType::Vec3:
  case ValueType::BBox:
  case ValueType::Null:
  case ValueType::Undefined:
    result.push_back(toIndex(indexValue));
    break;
  }
}

std::vector<long> computeIndexArray(const Value& indexValue, const size_t indexableSize)
{
  auto result = std::vector<long>{};
  computeIndexArray(indexValue, indexableSize, result);
  return result;
}

Value evaluateSubscript(
  const Value& lhs, const Value& rhs, const ExpressionNode& expressionNode)
{
  switch (lhs.type())
  {
  case ValueType::String:
    switch (rhs.type())
    {
    case ValueType::Boolean:
    case ValueType::Number: {
      const auto& str = lhs.stringValue();
      const auto index = computeIndex(toIndex(rhs), str.length());
      auto result = std::stringstream{};
      if (index)
      {
        result << str[*index];
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
        if (const auto index = computeIndex(indices[i], str.length()))
        {
          result << str[*index];
        }
      }
      return Value{result.str()};
    }
    case ValueType::String:
    case ValueType::Map:
    case ValueType::Vec3:
    case ValueType::BBox:
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
      const auto index = toIndex(rhs);
      const auto arrayIndex = computeIndex(index, array.size());
      if (!arrayIndex)
      {
        throw IndexOutOfBoundsError{expressionNode, lhs, index};
      }
      return array[*arrayIndex];
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
        const auto arrayIndex = computeIndex(index, array.size());
        if (!arrayIndex)
        {
          throw IndexOutOfBoundsError{expressionNode, lhs, index};
        }
        result.push_back(array[*arrayIndex]);
      }
      return Value{std::move(result)};
    }
    case ValueType::String:
    case ValueType::Map:
    case ValueType::Vec3:
    case ValueType::BBox:
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
          throw ConversionError{
            keyValue.location(), keyValue.describe(), keyValue.type(), ValueType::String};
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
    case ValueType::Vec3:
    case ValueType::BBox:
    case ValueType::Null:
    case ValueType::Undefined:
      break;
    }
    break;
  case ValueType::Vec3:
    switch (rhs.type())
    {
    case ValueType::Boolean:
    case ValueType::Number: {
      const auto& v = lhs.vec3Value();
      const auto index = toIndex(rhs);
      const auto componentIndex = computeIndex(index, 3u);
      if (!componentIndex)
      {
        throw IndexOutOfBoundsError{expressionNode, lhs, index};
      }
      return Value{v[*componentIndex]};
    }
    case ValueType::String: {
      const auto& v = lhs.vec3Value();
      const auto& key = rhs.stringValue();
      if (key == "x")
      {
        return Value{v.x()};
      }
      if (key == "y")
      {
        return Value{v.y()};
      }
      if (key == "z")
      {
        return Value{v.z()};
      }
      return Value::Undefined;
    }
    case ValueType::Array:
    case ValueType::Map:
    case ValueType::Range:
    case ValueType::Vec3:
    case ValueType::BBox:
    case ValueType::Null:
    case ValueType::Undefined:
      break;
    }
    break;
  case ValueType::BBox:
    switch (rhs.type())
    {
    case ValueType::String: {
      const auto& b = lhs.bboxValue();
      const auto& key = rhs.stringValue();
      if (key == "min")
      {
        return Value{b.min};
      }
      if (key == "max")
      {
        return Value{b.max};
      }
      return Value::Undefined;
    }
    case ValueType::Boolean:
    case ValueType::Number:
    case ValueType::Array:
    case ValueType::Map:
    case ValueType::Range:
    case ValueType::Vec3:
    case ValueType::BBox:
    case ValueType::Null:
    case ValueType::Undefined:
      break;
    }
    break;
  case ValueType::Undefined:
    return Value::Undefined;
  case ValueType::Boolean:
  case ValueType::Number:
  case ValueType::Range:
  case ValueType::Null:
    break;
  }

  throw IndexError{expressionNode, lhs, rhs};
}

template <typename Evaluator>
Value evaluate(
  EvaluationContext&,
  const Evaluator& evaluator,
  const SubscriptExpression& expression,
  const ExpressionNode& expressionNode)
{
  return evaluateSubscript(
    expression.leftOperand.accept(evaluator),
    expression.rightOperand.accept(evaluator),
    expressionNode);
}

template <typename Evaluator>
Value evaluate(
  EvaluationContext&,
  const Evaluator& evaluator,
  const DotExpression& expression,
  const ExpressionNode& expressionNode)
{
  return evaluateSubscript(
    expression.operand.accept(evaluator), Value{expression.fieldName}, expressionNode);
}

// whether the pattern contains a wildcard that isn't escaped
bool isGlobPattern(const std::string& pattern)
{
  for (size_t i = 0u; i < pattern.size(); ++i)
  {
    if (pattern[i] == '\\')
    {
      ++i;
    }
    else if (pattern[i] == '?' || pattern[i] == '*' || pattern[i] == '%')
    {
      return true;
    }
  }
  return false;
}

// a trailing backslash that doesn't escape anything is escaped so that it doesn't escape
// the appended wildcard
std::string makeSubstringPattern(const std::string& pattern)
{
  const auto trailingBackslashes = std::ranges::distance(
    pattern | std::views::reverse
    | std::views::take_while([](const auto c) { return c == '\\'; }));
  return "*" + pattern + (trailingBackslashes % 2 == 1 ? "\\" : "") + "*";
}

bool matchesGlob(const Value& lhs, const std::string_view pattern)
{
  if (lhs.hasType(ValueType::String))
  {
    return kdl::ci::str_matches_glob(lhs.stringValue(), pattern);
  }

  // array elements and map values that aren't strings never match
  const auto matches = [&](const Value& value) {
    return value.hasType(ValueType::String)
           && kdl::ci::str_matches_glob(value.stringValue(), pattern);
  };

  if (lhs.hasType(ValueType::Array))
  {
    return std::ranges::any_of(lhs.arrayValue(), matches);
  }

  if (lhs.hasType(ValueType::Map))
  {
    return std::ranges::any_of(
      lhs.mapValue(), kdl::unpack([&](const auto& key, const auto& value) {
        return kdl::ci::str_matches_glob(key, pattern) || matches(value);
      }));
  }

  return false;
}

Value evaluateLike(const Value& lhs, const Value& rhs)
{
  if (lhs.hasType(ValueType::Undefined) || rhs.hasType(ValueType::Undefined))
  {
    return Value{false};
  }

  if (!rhs.hasType(ValueType::String))
  {
    return Value{false};
  }

  // a pattern with no glob metacharacters at all matches as a substring
  const auto& pattern = rhs.stringValue();
  return Value{
    isGlobPattern(pattern) ? matchesGlob(lhs, pattern)
                           : matchesGlob(lhs, makeSubstringPattern(pattern))};
}

// like `==`, but values that can't be compared are not equal instead of an error
bool evaluateEqual(
  const Value& lhs, const Value& rhs, const ExpressionNode& expressionNode)
{
  try
  {
    return evaluateCompare(lhs, rhs, expressionNode) == 0;
  }
  catch (const EvaluationError&)
  {
    return false;
  }
}

Value evaluateContains(
  const Value& lhs, const Value& rhs, const ExpressionNode& expressionNode)
{
  if (lhs.hasType(ValueType::Undefined) || rhs.hasType(ValueType::Undefined))
  {
    return Value{false};
  }

  switch (lhs.type())
  {
  case ValueType::Array: {
    const auto& array = lhs.arrayValue();
    return Value{std::ranges::any_of(array, [&](const auto& element) {
      return evaluateEqual(element, rhs, expressionNode);
    })};
  }
  case ValueType::Map:
    return Value{rhs.hasType(ValueType::String) && lhs.contains(rhs.stringValue())};
  case ValueType::BBox:
    if (rhs.hasType(ValueType::Vec3))
    {
      return Value{lhs.bboxValue().contains(rhs.vec3Value())};
    }
    if (rhs.hasType(ValueType::BBox))
    {
      return Value{lhs.bboxValue().contains(rhs.bboxValue())};
    }
    return Value{false};
  case ValueType::Boolean:
  case ValueType::String:
  case ValueType::Number:
  case ValueType::Range:
  case ValueType::Vec3:
  case ValueType::Null:
  case ValueType::Undefined:
    return Value{false};
    switchDefault();
  }
}

struct BuiltinFunction
{
  size_t arity;
  std::function<Value(EvaluationContext&, std::span<const Value>, const ExpressionNode&)>
    evaluate;
};

bool hasUndefinedArgument(const std::span<const Value> arguments)
{
  return std::ranges::any_of(arguments, [](const auto& argument) {
    return argument.hasType(ValueType::Undefined);
  });
}

const auto builtinFunctions = kdl::flat_map<std::string, BuiltinFunction>{
  {"vec",
   {3,
    [](auto&, const auto& arguments, const auto&) {
      if (hasUndefinedArgument(arguments))
      {
        return Value::Undefined;
      }
      return Value{Vec3Type{
        arguments[0].convertTo(ValueType::Number).numberValue(),
        arguments[1].convertTo(ValueType::Number).numberValue(),
        arguments[2].convertTo(ValueType::Number).numberValue(),
      }};
    }}},
  {"bbox",
   {2,
    [](auto&, const auto& arguments, const auto&) {
      if (hasUndefinedArgument(arguments))
      {
        return Value::Undefined;
      }
      const auto a = arguments[0].convertTo(ValueType::Vec3).vec3Value();
      const auto b = arguments[1].convertTo(ValueType::Vec3).vec3Value();
      return Value{BBoxType{vm::min(a, b), vm::max(a, b)}};
    }}},
  {"distanceTo",
   {2,
    [](auto&, const auto& arguments, const auto&) {
      if (hasUndefinedArgument(arguments))
      {
        return Value::Undefined;
      }
      const auto a = arguments[0].convertTo(ValueType::Vec3).vec3Value();
      const auto b = arguments[1].convertTo(ValueType::Vec3).vec3Value();
      return Value{vm::distance(a, b)};
    }}},
  {"intersects",
   {2,
    [](auto&, const auto& arguments, const auto&) {
      if (hasUndefinedArgument(arguments))
      {
        return Value::Undefined;
      }
      const auto& a = arguments[0].bboxValue();
      const auto& b = arguments[1].bboxValue();
      return Value{a.intersects(b)};
    }}},
  {"like",
   {2,
    [](auto&, const auto& arguments, const auto&) {
      return evaluateLike(arguments[0], arguments[1]);
    }}},
  {"contains",
   {2,
    [](auto&, const auto& arguments, const auto& expressionNode) {
      return evaluateContains(arguments[0], arguments[1], expressionNode);
    }}},
  {"in",
   {2,
    [](auto&, const auto& arguments, const auto& expressionNode) {
      return evaluateContains(arguments[1], arguments[0], expressionNode);
    }}},
  {"is",
   {2,
    [](auto&, const auto& arguments, const auto& expressionNode) {
      return Value{evaluateCompare(arguments[0], arguments[1], expressionNode) == 0};
    }}},
};

Value evaluateCall(
  EvaluationContext& context,
  const std::string& name,
  const std::span<const Value> arguments,
  const ExpressionNode& expressionNode)
{
  if (const auto it = builtinFunctions.find(name); it != builtinFunctions.end())
  {
    const auto& function = it->second;
    if (arguments.size() != function.arity)
    {
      throw EvaluationError{
        expressionNode,
        fmt::format(
          "{}() expects {} arguments, but got {}",
          name,
          function.arity,
          arguments.size())};
    }

    try
    {
      return function.evaluate(context, arguments, expressionNode);
    }
    catch (const ConversionError& e)
    {
      throw EvaluationError{expressionNode, e.what()};
    }
    catch (const DereferenceError& e)
    {
      throw EvaluationError{expressionNode, e.what()};
    }
  }

  throw EvaluationError{expressionNode, fmt::format("Unknown function: '{}'", name)};
}

template <typename Evaluator>
Value evaluate(
  EvaluationContext& context,
  const Evaluator& evaluator,
  const CallExpression& expression,
  const ExpressionNode& expressionNode)
{
  const auto arguments = expression.arguments
                         | std::views::transform([&](const auto& argument) {
                             return argument.accept(evaluator);
                           })
                         | kdl::ranges::to<std::vector>();

  return evaluateCall(context, expression.name, arguments, expressionNode);
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
      | kdl::ranges::to<std::vector>()}};
  }

  return ArrayExpression{
    std::move(optimizedExpressions) | kdl::ranges::to<std::vector>()};
}

Expression optimize(
  EvaluationContext& context, const MapExpression& expression, const ExpressionNode&)
{
  auto optimizedExpressions =
    expression.elements
    | std::views::transform(
      kdl::unpack([&](const auto& key, const auto& valueExpression) {
        return std::pair{key, valueExpression.optimize(context)};
      }));

  const auto isLiteral = std::ranges::all_of(
    optimizedExpressions, [](const auto& entry) { return entry.second.isLiteral(); });

  if (isLiteral)
  {
    return LiteralExpression{Value{
      optimizedExpressions
      | std::views::transform(
        kdl::unpack([&](const auto& key, const auto& valueExpression) {
          return std::pair{key, valueExpression.evaluate(context)};
        }))
      | kdl::ranges::to<MapType>()}};
  }

  return MapExpression{
    std::move(optimizedExpressions)
    | kdl::ranges::to<std::map<std::string, ExpressionNode>>()};
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
      expression.operation, optimizedOperand.evaluate(context), expressionNode)};
  }

  return UnaryExpression{expression.operation, std::move(optimizedOperand)};
}

Expression optimize(
  EvaluationContext& context,
  const BinaryExpression& expression,
  const ExpressionNode& expressionNode)
{
  auto optimizedLeftOperand = expression.leftOperand.optimize(context);
  auto optimizedRightOperand = expression.rightOperand.optimize(context);

  const auto isLiteral =
    optimizedLeftOperand.isLiteral() && optimizedRightOperand.isLiteral();

  if (isLiteral)
  {
    return LiteralExpression{evaluateBinaryExpression(
      context,
      expression.operation,
      [&] { return optimizedLeftOperand.evaluate(context); },
      [&] { return optimizedRightOperand.evaluate(context); },
      expressionNode)};
  }

  return BinaryExpression{
    expression.operation,
    std::move(optimizedLeftOperand),
    std::move(optimizedRightOperand)};
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
    return LiteralExpression{evaluateSubscript(leftValue, rightValue, expressionNode)};
  }

  return SubscriptExpression{
    std::move(optimizedLeftOperand), std::move(optimizedRightOperand)};
}

Expression optimize(
  EvaluationContext& context,
  const DotExpression& expression,
  const ExpressionNode& expressionNode)
{
  auto optimizedOperand = expression.operand.optimize(context);

  if (optimizedOperand.isLiteral())
  {
    const auto operandValue = optimizedOperand.evaluate(context);
    return LiteralExpression{
      evaluateSubscript(operandValue, Value{expression.fieldName}, expressionNode)};
  }

  return DotExpression{std::move(optimizedOperand), expression.fieldName};
}

Expression optimize(
  EvaluationContext& context,
  const CallExpression& expression,
  const ExpressionNode& expressionNode)
{
  auto optimizedArguments = expression.arguments
                            | std::views::transform([&](const auto& argument) {
                                return argument.optimize(context);
                              })
                            | kdl::ranges::to<std::vector>();

  const auto allLiteral =
    std::ranges::all_of(optimizedArguments, &ExpressionNode::isLiteral);

  if (allLiteral)
  {
    const auto values = optimizedArguments
                        | std::views::transform([&](const auto& argument) {
                            return argument.evaluate(context);
                          })
                        | kdl::ranges::to<std::vector>();

    return LiteralExpression{
      evaluateCall(context, expression.name, values, expressionNode)};
  }

  return CallExpression{expression.name, std::move(optimizedArguments)};
}

Expression optimize(
  EvaluationContext& context, const SwitchExpression& expression, const ExpressionNode&)
{
  auto optimizedExpressions = std::vector<ExpressionNode>{};
  for (const auto& case_ : expression.cases)
  {
    auto optimizedExpression = case_.optimize(context);
    if (optimizedExpression.isLiteral())
    {
      auto value = optimizedExpression.evaluate(context);
      if (value == Value::Undefined)
      {
        continue;
      }

      if (optimizedExpressions.empty())
      {
        return LiteralExpression{std::move(value)};
      }
    }

    optimizedExpressions.push_back(std::move(optimizedExpression));
  }

  if (optimizedExpressions.empty())
  {
    return LiteralExpression{Value::Undefined};
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


size_t precedence(const BinaryOperation& operation)
{
  return std::visit([](const auto& op) { return op.precedence; }, operation);
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

struct ExpressionNode::Data
{
  Expression expression;
  std::optional<FileLocation> location;
};

ExpressionNode::ExpressionNode(std::shared_ptr<Data> data)
  : m_data{std::move(data)}
{
}

ExpressionNode::ExpressionNode(
  Expression&& expression, std::optional<FileLocation> location)
  : m_data{std::make_shared<Data>(Data{std::move(expression), std::move(location)})}
{
  rebalanceByPrecedence();
}

bool ExpressionNode::isLiteral() const
{
  return std::holds_alternative<LiteralExpression>(m_data->expression);
}

Value ExpressionNode::evaluate(EvaluationContext& context) const
{
  return accept(
    [&](const auto& evaluator, const auto& expression, const auto& containingNode) {
      return el::evaluate(context, evaluator, expression, containingNode)
        .producedBy(containingNode);
    });
}

Value ExpressionNode::tryEvaluate(EvaluationContext& context) const
{
  return accept(
    [&](const auto& evaluator, const auto& expression, const auto& containingNode) {
      try
      {
        return el::evaluate(context, evaluator, expression, containingNode)
          .producedBy(containingNode);
      }
      catch (const EvaluationError&)
      {
        return Value::Undefined;
      }
    });
}

ExpressionNode ExpressionNode::optimize(EvaluationContext& context) const
{
  return ExpressionNode{std::make_shared<Data>(
    Data{optimizeExpression(context, m_data->expression, *this), m_data->location})};
}

const std::optional<FileLocation>& ExpressionNode::location() const
{
  return m_data->location;
}

const Expression& ExpressionNode::expression() const
{
  return m_data->expression;
}

std::string ExpressionNode::asString() const
{
  auto str = std::stringstream{};
  str << *this;
  return str.str();
}

bool operator==(const ExpressionNode& lhs, const ExpressionNode& rhs)
{
  return lhs.m_data->expression == rhs.m_data->expression;
}

std::ostream& operator<<(std::ostream& lhs, const ExpressionNode& rhs)
{
  lhs << rhs.m_data->expression;
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
        const auto myPrecedence = precedence(m_data->expression);
        const auto leftPrecedence =
          precedence(binaryExpression.leftOperand.m_data->expression);
        const auto rightPrecedence =
          precedence(binaryExpression.rightOperand.m_data->expression);

        if (myPrecedence > std::min(leftPrecedence, rightPrecedence))
        {
          if (leftPrecedence < rightPrecedence)
          {
            // push this operator into the right subtree, rotating the right node up, and
            // rebalancing the right subtree again
            auto leftExpressionNode = std::move(binaryExpression.leftOperand);
            auto& leftBinaryExpression =
              std::get<BinaryExpression>(leftExpressionNode.m_data->expression);

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
              std::get<BinaryExpression>(rightExpressionNode.m_data->expression);

            binaryExpression.rightOperand = std::move(rightBinaryExpression.leftOperand);
            rightBinaryExpression.leftOperand = std::move(*this);
            *this = std::move(rightExpressionNode);

            binaryExpression.leftOperand.rebalanceByPrecedence();
          }
        }
      },
      [](auto&) {}),
    m_data->expression);
}


bool operator==(const LiteralExpression& lhs, const LiteralExpression& rhs)
{
  return lhs.value == rhs.value;
}

std::ostream& operator<<(std::ostream& lhs, const LiteralExpression& rhs)
{
  return lhs << rhs.value;
}


bool operator==(const VariableExpression& lhs, const VariableExpression& rhs)
{
  return lhs.variableName == rhs.variableName;
}

std::ostream& operator<<(std::ostream& lhs, const VariableExpression& rhs)
{
  return lhs << rhs.variableName;
}


bool operator==(const ArrayExpression& lhs, const ArrayExpression& rhs)
{
  return lhs.elements == rhs.elements;
}

std::ostream& operator<<(std::ostream& lhs, const ArrayExpression& rhs)
{
  lhs << "[";
  for (size_t i = 0; i < rhs.elements.size(); ++i)
  {
    lhs << rhs.elements[i];
    if (i < rhs.elements.size() - 1)
    {
      lhs << ", ";
    }
  }
  lhs << "]";

  return lhs;
}


bool operator==(const MapExpression& lhs, const MapExpression& rhs)
{
  return lhs.elements == rhs.elements;
}

std::ostream& operator<<(std::ostream& lhs, const MapExpression& rhs)
{
  if (rhs.elements.empty())
  {
    return lhs << "{}";
  }

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

namespace binop
{
kdl_reflect_impl(Multiplication);
kdl_reflect_impl(Division);
kdl_reflect_impl(Modulus);
kdl_reflect_impl(Addition);
kdl_reflect_impl(Subtraction);
kdl_reflect_impl(BitwiseShiftLeft);
kdl_reflect_impl(BitwiseShiftRight);
kdl_reflect_impl(Less);
kdl_reflect_impl(LessOrEqual);
kdl_reflect_impl(Greater);
kdl_reflect_impl(GreaterOrEqual);
kdl_reflect_impl(Equal);
kdl_reflect_impl(NotEqual);
kdl_reflect_impl(BitwiseAnd);
kdl_reflect_impl(BitwiseXOr);
kdl_reflect_impl(BitwiseOr);
kdl_reflect_impl(LogicalAnd);
kdl_reflect_impl(LogicalOr);
kdl_reflect_impl(BoundedRange);
kdl_reflect_impl(Case);
kdl_reflect_impl(InfixCall);
} // namespace binop


bool operator==(const BinaryExpression& lhs, const BinaryExpression& rhs)
{
  return lhs.operation == rhs.operation && lhs.leftOperand == rhs.leftOperand
         && lhs.rightOperand == rhs.rightOperand;
}

std::ostream& operator<<(std::ostream& lhs, const BinaryExpression& rhs)
{
  std::visit(
    kdl::overload(
      [&](const binop::Addition&) {
        lhs << rhs.leftOperand << " + " << rhs.rightOperand;
      },
      [&](const binop::Subtraction&) {
        lhs << rhs.leftOperand << " - " << rhs.rightOperand;
      },
      [&](const binop::Multiplication&) {
        lhs << rhs.leftOperand << " * " << rhs.rightOperand;
      },
      [&](const binop::Division&) {
        lhs << rhs.leftOperand << " / " << rhs.rightOperand;
      },
      [&](const binop::Modulus&) { lhs << rhs.leftOperand << " % " << rhs.rightOperand; },
      [&](const binop::LogicalAnd&) {
        lhs << rhs.leftOperand << " && " << rhs.rightOperand;
      },
      [&](const binop::LogicalOr&) {
        lhs << rhs.leftOperand << " || " << rhs.rightOperand;
      },
      [&](const binop::BitwiseAnd&) {
        lhs << rhs.leftOperand << " & " << rhs.rightOperand;
      },
      [&](const binop::BitwiseXOr&) {
        lhs << rhs.leftOperand << " ^ " << rhs.rightOperand;
      },
      [&](const binop::BitwiseOr&) {
        lhs << rhs.leftOperand << " | " << rhs.rightOperand;
      },
      [&](const binop::BitwiseShiftLeft&) {
        lhs << rhs.leftOperand << " << " << rhs.rightOperand;
      },
      [&](const binop::BitwiseShiftRight&) {
        lhs << rhs.leftOperand << " >> " << rhs.rightOperand;
      },
      [&](const binop::Less&) { lhs << rhs.leftOperand << " < " << rhs.rightOperand; },
      [&](const binop::LessOrEqual&) {
        lhs << rhs.leftOperand << " <= " << rhs.rightOperand;
      },
      [&](const binop::Greater&) { lhs << rhs.leftOperand << " > " << rhs.rightOperand; },
      [&](const binop::GreaterOrEqual&) {
        lhs << rhs.leftOperand << " >= " << rhs.rightOperand;
      },
      [&](const binop::Equal&) { lhs << rhs.leftOperand << " == " << rhs.rightOperand; },
      [&](const binop::NotEqual&) {
        lhs << rhs.leftOperand << " != " << rhs.rightOperand;
      },
      [&](const binop::BoundedRange&) {
        lhs << rhs.leftOperand << ".." << rhs.rightOperand;
      },
      [&](const binop::Case&) { lhs << rhs.leftOperand << " -> " << rhs.rightOperand; },
      [&](const binop::InfixCall& infixCall) {
        lhs << rhs.leftOperand << " " << infixCall.functionName << " "
            << rhs.rightOperand;
      }),
    rhs.operation);

  return lhs;
}


bool operator==(const SubscriptExpression& lhs, const SubscriptExpression& rhs)
{
  return lhs.leftOperand == rhs.leftOperand && lhs.rightOperand == rhs.rightOperand;
}

std::ostream& operator<<(std::ostream& lhs, const SubscriptExpression& rhs)
{
  return lhs << rhs.leftOperand << "[" << rhs.rightOperand << "]";
}


bool operator==(const DotExpression& lhs, const DotExpression& rhs)
{
  return lhs.operand == rhs.operand && lhs.fieldName == rhs.fieldName;
}

std::ostream& operator<<(std::ostream& lhs, const DotExpression& rhs)
{
  return lhs << rhs.operand << "." << rhs.fieldName;
}


bool operator==(const CallExpression& lhs, const CallExpression& rhs)
{
  return lhs.name == rhs.name && lhs.arguments == rhs.arguments;
}

std::ostream& operator<<(std::ostream& lhs, const CallExpression& rhs)
{
  lhs << rhs.name << "(";
  for (size_t i = 0; i < rhs.arguments.size(); ++i)
  {
    lhs << rhs.arguments[i];
    if (i < rhs.arguments.size() - 1)
    {
      lhs << ", ";
    }
  }
  return lhs << ")";
}

std::optional<size_t> builtinFunctionArity(const std::string& name)
{
  const auto it = builtinFunctions.find(name);
  return it != builtinFunctions.end() ? std::optional{it->second.arity} : std::nullopt;
}


bool operator==(const SwitchExpression& lhs, const SwitchExpression& rhs)
{
  return lhs.cases == rhs.cases;
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
