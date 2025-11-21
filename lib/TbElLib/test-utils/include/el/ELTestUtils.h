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

#pragma once

#include "el/Expression.h"
#include "el/Value.h"

#include <string>
#include <vector>

namespace tb::el
{

ExpressionNode lit(Value value);

template <typename T>
ExpressionNode lit(T&& value)
{
  return lit(Value{std::forward<T>(value)});
}

ExpressionNode var(std::string name);

ExpressionNode arr(std::vector<ExpressionNode> array = {});
ExpressionNode map(std::map<std::string, ExpressionNode> map = {});

ExpressionNode plus(ExpressionNode operand);
ExpressionNode minus(ExpressionNode operand);
ExpressionNode logNeg(ExpressionNode operand);
ExpressionNode bitNeg(ExpressionNode operand);
ExpressionNode grp(ExpressionNode operand);
ExpressionNode lbRng(ExpressionNode operand);
ExpressionNode rbRng(ExpressionNode operand);

ExpressionNode add(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode sub(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode mul(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode div(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode mod(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode logAnd(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode logOr(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode bitAnd(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode bitOr(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode bitXOr(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode bitShL(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode bitShR(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode ls(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode lsEq(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode gr(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode grEq(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode eq(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode neq(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode bRng(ExpressionNode leftOperand, ExpressionNode rightOperand);
ExpressionNode cs(ExpressionNode leftOperand, ExpressionNode rightOperand);

ExpressionNode scr(ExpressionNode leftOperand, ExpressionNode rightOperand);

ExpressionNode swt(std::vector<ExpressionNode> cases);

} // namespace tb::el