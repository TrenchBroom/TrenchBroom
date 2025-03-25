/*
 Copyright (C) 2010 Kristian Duske

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

#include "FileLocation.h"
#include "Macros.h"
#include "el/Expression.h"
#include "el/Value.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace tb::el
{

class EvaluationContext
{
private:
  std::unique_ptr<VariableStore> m_variables;
  std::unordered_map<Value, ExpressionNode> m_trace;

public:
  EvaluationContext();
  explicit EvaluationContext(const VariableStore& variables);
  ~EvaluationContext();

  Value variableValue(const std::string& name) const;

  std::optional<ExpressionNode> expression(const Value& value) const;
  std::optional<FileLocation> location(const Value& value) const;

  void trace(const Value& value, const ExpressionNode& expression);

  deleteCopyAndMove(EvaluationContext);
};

} // namespace tb::el
