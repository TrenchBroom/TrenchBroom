/*
 Copyright (C) 2023 Daniel Walder

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

#include "DecalDefinition.h"

#include "EL/EvaluationContext.h"
#include "EL/Expressions.h"
#include "EL/Types.h"
#include "EL/Value.h"
#include "EL/VariableStore.h"

#include "kdl/reflection_impl.h"
#include "kdl/string_compare.h"

#include "vecmath/scalar.h"
#include "vecmath/vec_io.h"

namespace TrenchBroom
{
namespace Assets
{

namespace
{
std::string textureName(const EL::Value& value)
{
  using namespace std::string_literals;
  return value.type() == EL::ValueType::String ? value.stringValue() : ""s;
}

DecalSpecification convertToDecal(const EL::Value& value)
{
  switch (value.type())
  {
  case EL::ValueType::Map:
    return {textureName(value[DecalSpecificationKeys::Texture])};
  case EL::ValueType::String:
    return {textureName(value)};
  case EL::ValueType::Boolean:
  case EL::ValueType::Number:
  case EL::ValueType::Array:
  case EL::ValueType::Range:
  case EL::ValueType::Null:
  case EL::ValueType::Undefined:
    break;
  }

  return {};
}
} // namespace

kdl_reflect_impl(DecalSpecification);

DecalDefinition::DecalDefinition()
  : m_expression{EL::LiteralExpression{EL::Value::Undefined}, 0, 0}
{
}

DecalDefinition::DecalDefinition(const size_t line, const size_t column)
  : m_expression{EL::LiteralExpression{EL::Value::Undefined}, line, column}
{
}

DecalDefinition::DecalDefinition(EL::Expression expression)
  : m_expression{std::move(expression)}
{
}

void DecalDefinition::append(const DecalDefinition& other)
{
  const auto line = m_expression.line();
  const auto column = m_expression.column();

  auto cases = std::vector<EL::Expression>{std::move(m_expression), other.m_expression};
  m_expression = EL::Expression{EL::SwitchExpression{std::move(cases)}, line, column};
}

DecalSpecification DecalDefinition::decalSpecification(
  const EL::VariableStore& variableStore) const
{
  return convertToDecal(m_expression.evaluate(EL::EvaluationContext{variableStore}));
}

DecalSpecification DecalDefinition::defaultDecalSpecification() const
{
  return decalSpecification(EL::NullVariableStore{});
}

kdl_reflect_impl(DecalDefinition);

} // namespace Assets
} // namespace TrenchBroom
