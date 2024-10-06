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
#include "EL/Expression.h"
#include "EL/Types.h"
#include "EL/Value.h"
#include "EL/VariableStore.h"

#include "kdl/reflection_impl.h"

namespace tb::assets
{

namespace
{
std::string materialName(const EL::Value& value)
{
  using namespace std::string_literals;
  return value.type() == EL::ValueType::String ? value.stringValue() : ""s;
}

DecalSpecification convertToDecal(const EL::Value& value)
{
  switch (value.type())
  {
  case EL::ValueType::Map:
    return {materialName(value[DecalSpecificationKeys::Material])};
  case EL::ValueType::String:
    return {materialName(value)};
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
  : m_expression{EL::LiteralExpression{EL::Value::Undefined}}
{
}

DecalDefinition::DecalDefinition(const FileLocation& location)
  : m_expression{EL::LiteralExpression{EL::Value::Undefined}, location}
{
}

DecalDefinition::DecalDefinition(EL::ExpressionNode expression)
  : m_expression{std::move(expression)}
{
}

void DecalDefinition::append(const DecalDefinition& other)
{
  const auto location = m_expression.location();

  auto cases =
    std::vector<EL::ExpressionNode>{std::move(m_expression), other.m_expression};
  m_expression = EL::ExpressionNode{EL::SwitchExpression{std::move(cases)}, location};
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

} // namespace tb::assets
