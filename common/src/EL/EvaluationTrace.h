/*
 Copyright (C) 2024 Kristian Duske

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

#include "EL/Expression.h"
#include "EL/Value.h"
#include "FileLocation.h"

#include <optional>
#include <unordered_map>

namespace TrenchBroom::EL
{

class EvaluationTrace
{
private:
  std::unordered_map<Value, ExpressionNode> m_data;

public:
  std::optional<ExpressionNode> getExpression(const Value& value) const;
  std::optional<FileLocation> getLocation(const Value& value) const;

  void addTrace(const Value& value, const ExpressionNode& expression);
};

} // namespace TrenchBroom::EL
