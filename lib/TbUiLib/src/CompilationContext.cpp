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

#include "ui/CompilationContext.h"

#include "el/EvaluationContext.h"
#include "el/Interpolate.h"
#include "el/Types.h"

namespace tb::ui
{

CompilationContext::CompilationContext(
  const mdl::Map& map,
  const el::VariableStore& variables,
  TextOutputAdapter output,
  bool test)
  : m_map{map}
  , m_variables{variables.clone()}
  , m_output{std::move(output)}
  , m_test{test}
{
}

const mdl::Map& CompilationContext::map() const
{
  return m_map;
}

bool CompilationContext::test() const
{
  return m_test;
}

Result<std::string> CompilationContext::interpolate(const std::string& input) const
{
  return el::interpolate(*m_variables, input);
}

Result<std::string> CompilationContext::variableValue(
  const std::string& variableName) const
{
  return el::withEvaluationContext(
    [&](auto& context) {
      return context.variableValue(variableName)
        .convertTo(context, el::ValueType::String)
        .stringValue(context);
    },
    *m_variables);
}

} // namespace tb::ui
