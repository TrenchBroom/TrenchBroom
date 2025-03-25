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

#include "EvaluationContext.h"

#include "el/Value.h"
#include "el/VariableStore.h"

#include <string>

namespace tb::el
{

EvaluationContext::EvaluationContext()
  : m_variables{std::make_unique<VariableTable>()}
{
}

EvaluationContext::EvaluationContext(const VariableStore& store)
  : m_variables{store.clone()}
{
}

EvaluationContext::~EvaluationContext() = default;

Value EvaluationContext::variableValue(const std::string& name) const
{
  return m_variables->value(name);
}

std::optional<ExpressionNode> EvaluationContext::expression(const Value& value) const
{
  const auto it = m_trace.find(value);
  return it != m_trace.end() ? std::optional{it->second} : std::nullopt;
}

std::optional<FileLocation> EvaluationContext::location(const Value& value) const
{
  if (const auto expression = this->expression(value))
  {
    return expression->location();
  }
  return std::nullopt;
}

void EvaluationContext::trace(const Value& value, const ExpressionNode& expression)
{
  m_trace.emplace(value, expression);
}


} // namespace tb::el
