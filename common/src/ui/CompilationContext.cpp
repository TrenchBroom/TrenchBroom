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

#include "CompilationContext.h"

#include "el/EvaluationContext.h"
#include "el/Interpolate.h"
#include "el/Types.h"

#include "kdl/memory_utils.h"

namespace tb::ui
{

CompilationContext::CompilationContext(
  std::weak_ptr<MapDocument> document,
  const el::VariableStore& variables,
  TextOutputAdapter output,
  bool test)
  : m_document{std::move(document)}
  , m_variables{variables.clone()}
  , m_output{std::move(output)}
  , m_test{test}
{
}

std::shared_ptr<MapDocument> CompilationContext::document() const
{
  return kdl::mem_lock(m_document);
}

bool CompilationContext::test() const
{
  return m_test;
}

Result<std::string> CompilationContext::interpolate(const std::string& input) const
{
  auto context = el::EvaluationContext{*m_variables};
  return el::interpolate(input, context);
}

std::string CompilationContext::variableValue(const std::string& variableName) const
{
  return m_variables->value(variableName).convertTo(el::ValueType::String).stringValue();
}

} // namespace tb::ui
