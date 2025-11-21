/*
 Copyright (C) 2025 Kristian Duske

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

#include "el/ELTestUtils.h"

namespace tb::el
{

ExpressionNode lit(Value value)
{
  return ExpressionNode{LiteralExpression{std::move(value)}};
}

ExpressionNode var(std::string name)
{
  return ExpressionNode{VariableExpression{std::move(name)}};
}

ExpressionNode arr(std::vector<ExpressionNode> array)
{
  return ExpressionNode{ArrayExpression{std::move(array)}};
}

ExpressionNode map(std::map<std::string, ExpressionNode> map)
{
  return ExpressionNode{MapExpression{std::move(map)}};
}

ExpressionNode plus(ExpressionNode operand)
{
  return ExpressionNode{UnaryExpression{UnaryOperation::Plus, std::move(operand)}};
}

ExpressionNode minus(ExpressionNode operand)
{
  return ExpressionNode{UnaryExpression{UnaryOperation::Minus, std::move(operand)}};
}

ExpressionNode logNeg(ExpressionNode operand)
{
  return ExpressionNode{
    UnaryExpression{UnaryOperation::LogicalNegation, std::move(operand)}};
}

ExpressionNode bitNeg(ExpressionNode operand)
{
  return ExpressionNode{
    UnaryExpression{UnaryOperation::BitwiseNegation, std::move(operand)}};
}

ExpressionNode grp(ExpressionNode operand)
{
  return ExpressionNode{UnaryExpression{UnaryOperation::Group, std::move(operand)}};
}

ExpressionNode lbRng(ExpressionNode operand)
{
  return ExpressionNode{
    UnaryExpression{UnaryOperation::LeftBoundedRange, std::move(operand)}};
}

ExpressionNode rbRng(ExpressionNode operand)
{
  return ExpressionNode{
    UnaryExpression{UnaryOperation::RightBoundedRange, std::move(operand)}};
}

ExpressionNode add(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::Addition, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode sub(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::Subtraction, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode mul(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::Multiplication, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode div(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::Division, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode mod(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::Modulus, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode logAnd(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::LogicalAnd, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode logOr(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::LogicalOr, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode bitAnd(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::BitwiseAnd, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode bitOr(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::BitwiseOr, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode bitXOr(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::BitwiseXOr, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode bitShL(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::BitwiseShiftLeft, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode bitShR(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::BitwiseShiftRight, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode ls(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::Less, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode lsEq(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::LessOrEqual, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode gr(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::Greater, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode grEq(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::GreaterOrEqual, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode eq(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::Equal, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode neq(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::NotEqual, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode bRng(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::BoundedRange, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode cs(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{BinaryExpression{
    BinaryOperation::Case, std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode scr(ExpressionNode leftOperand, ExpressionNode rightOperand)
{
  return ExpressionNode{
    SubscriptExpression{std::move(leftOperand), std::move(rightOperand)}};
}

ExpressionNode swt(std::vector<ExpressionNode> cases)
{
  return ExpressionNode{SwitchExpression{std::move(cases)}};
}

} // namespace tb::el