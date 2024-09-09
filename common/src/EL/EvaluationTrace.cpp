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

#include "EvaluationTrace.h"

namespace TrenchBroom::EL
{

std::optional<Expression> EvaluationTrace::getExpression(const Value& value) const
{
  auto it = m_data.find(value);
  return it != m_data.end() ? std::optional{it->second} : std::nullopt;
}

std::optional<FileLocation> EvaluationTrace::getLocation(const Value& value) const
{
  if (const auto expression = getExpression(value))
  {
    return expression->location();
  }
  return std::nullopt;
}

void EvaluationTrace::addTrace(const Value& value, const Expression& expression)
{
  m_data.emplace(value, expression);
}

} // namespace TrenchBroom::EL
