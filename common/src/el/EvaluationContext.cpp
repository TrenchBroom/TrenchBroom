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
  : m_store{std::make_unique<VariableTable>()}
{
}

EvaluationContext::EvaluationContext(const VariableStore& store)
  : m_store{store.clone()}
{
}

EvaluationContext::~EvaluationContext() = default;

Value EvaluationContext::variableValue(const std::string& name) const
{
  return m_store->value(name);
}

void EvaluationContext::declareVariable(const std::string& name, const Value& value)
{
  m_store->declare(name, value);
}

} // namespace tb::el
