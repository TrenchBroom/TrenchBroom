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

#include "el/EvaluationContext.h"
#include "el/Expression.h"
#include "el/Types.h"
#include "el/Value.h"
#include "el/VariableStore.h"

#include "kdl/reflection_impl.h"

namespace tb::mdl
{

namespace
{
std::string materialName(const el::Value& value)
{
  using namespace std::string_literals;
  return value.type() == el::ValueType::String ? value.stringValue() : ""s;
}

DecalSpecification convertToDecal(const el::Value& value)
{
  switch (value.type())
  {
  case el::ValueType::Map:
    return {materialName(value.at(DecalSpecificationKeys::Material))};
  case el::ValueType::String:
    return {materialName(value)};
  case el::ValueType::Boolean:
  case el::ValueType::Number:
  case el::ValueType::Array:
  case el::ValueType::Range:
  case el::ValueType::Null:
  case el::ValueType::Undefined:
    break;
  }

  return {};
}
} // namespace

kdl_reflect_impl(DecalSpecification);

DecalDefinition::DecalDefinition()
  : m_expression{el::LiteralExpression{el::Value::Undefined}}
{
}

DecalDefinition::DecalDefinition(const FileLocation& location)
  : m_expression{el::LiteralExpression{el::Value::Undefined}, location}
{
}

DecalDefinition::DecalDefinition(el::ExpressionNode expression)
  : m_expression{std::move(expression)}
{
}

void DecalDefinition::append(const DecalDefinition& other)
{
  const auto location = m_expression.location();

  auto cases =
    std::vector<el::ExpressionNode>{std::move(m_expression), other.m_expression};
  m_expression = el::ExpressionNode{el::SwitchExpression{std::move(cases)}, location};
}

DecalSpecification DecalDefinition::decalSpecification(
  const el::VariableStore& variableStore) const
{
  return convertToDecal(m_expression.evaluate(el::EvaluationContext{variableStore}));
}

DecalSpecification DecalDefinition::defaultDecalSpecification() const
{
  return decalSpecification(el::NullVariableStore{});
}

kdl_reflect_impl(DecalDefinition);

} // namespace tb::mdl
