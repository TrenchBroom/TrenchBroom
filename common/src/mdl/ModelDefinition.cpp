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

#include "ModelDefinition.h"

#include "el/ELExceptions.h"
#include "el/EvaluationContext.h"
#include "el/Expression.h"
#include "el/Types.h"
#include "el/Value.h"
#include "el/VariableStore.h"

#include "kdl/reflection_impl.h"
#include "kdl/string_compare.h"
#include "kdl/string_format.h"

#include "vm/scalar.h"
#include "vm/vec_io.h"

#include <fmt/format.h>

namespace tb::mdl
{
namespace
{

std::filesystem::path path(const el::Value& value)
{
  if (value.type() != el::ValueType::String)
  {
    return {};
  }
  const auto& path = value.stringValue();
  return kdl::cs::str_is_prefix(path, ":") ? path.substr(1) : path;
}

size_t index(const el::Value& value)
{
  if (!value.convertibleTo(el::ValueType::Number))
  {
    return 0;
  }
  const auto intValue = value.convertTo(el::ValueType::Number).integerValue();
  return static_cast<size_t>(vm::max(0l, intValue));
}

ModelSpecification convertToModel(const el::Value& value)
{
  switch (value.type())
  {
  case el::ValueType::Map:
    return ModelSpecification{
      path(value[ModelSpecificationKeys::Path]),
      index(value[ModelSpecificationKeys::Skin]),
      index(value[ModelSpecificationKeys::Frame])};
  case el::ValueType::String:
    return ModelSpecification{path(value), 0, 0};
  case el::ValueType::Boolean:
  case el::ValueType::Number:
  case el::ValueType::Array:
  case el::ValueType::Range:
  case el::ValueType::Null:
  case el::ValueType::Undefined:
    break;
  }

  return ModelSpecification{};
}

Result<vm::vec3d> scaleValue(const el::Value& value)
{
  if (value.type() == el::ValueType::Number)
  {
    const auto scale = value.numberValue();
    return vm::vec3d{scale, scale, scale};
  }

  if (value.type() != el::ValueType::String)
  {
    return Error{fmt::format("Invalid scale value: {}", value.describe())};
  }

  const auto stringValue = value.stringValue();
  if (kdl::str_is_blank(stringValue))
  {
    return Error{fmt::format("Invalid scale value: {}", value.describe())};
  }

  if (const auto scale = vm::parse<double, 3>(stringValue))
  {
    return *scale;
  }

  if (!value.convertibleTo(el::ValueType::Number))
  {
    return Error{fmt::format("Invalid scale value: {}", value.describe())};
  }

  const auto scale = value.convertTo(el::ValueType::Number).numberValue();
  return vm::vec3d{scale, scale, scale};
}

Result<vm::vec3d> convertToScale(const el::Value& value)
{
  if (value.type() == el::ValueType::Array)
  {
    for (const auto& x : value.arrayValue())
    {
      if (const auto scale = scaleValue(x); scale.is_success())
      {
        return scale;
      }
    }

    return Error{fmt::format("Invalid scale value: {}", value.describe())};
  }

  return scaleValue(value);
}

} // namespace

ModelDefinition::ModelDefinition()
  : m_expression{el::LiteralExpression{el::Value::Undefined}}
{
}

ModelDefinition::ModelDefinition(const FileLocation& location)
  : m_expression{el::LiteralExpression{el::Value::Undefined}, location}
{
}

ModelDefinition::ModelDefinition(el::ExpressionNode expression)
  : m_expression{std::move(expression)}
{
}

void ModelDefinition::append(ModelDefinition other)
{
  const auto location = m_expression.location();

  auto cases = std::vector{std::move(m_expression), std::move(other.m_expression)};
  m_expression = el::ExpressionNode{el::SwitchExpression{std::move(cases)}, location};
}

Result<ModelSpecification> ModelDefinition::modelSpecification(
  const el::VariableStore& variableStore) const
{
  const auto context = el::EvaluationContext{variableStore};
  return m_expression.evaluate(el::EvaluationContext{variableStore})
         | kdl::transform([](const auto& value) { return convertToModel(value); });
}

ModelSpecification ModelDefinition::defaultModelSpecification() const
{
  return convertToModel(m_expression.tryEvaluate(el::EvaluationContext{}));
}

Result<vm::vec3d> ModelDefinition::scale(
  const el::VariableStore& variableStore,
  const std::optional<el::ExpressionNode>& defaultScaleExpression) const
{
  const auto context = el::EvaluationContext{variableStore};
  return m_expression.evaluate(context)
         | kdl::and_then([&](const auto& value) -> Result<vm::vec3d> {
             switch (value.type())
             {
             case el::ValueType::Map:
               if (const auto scale =
                     convertToScale(value[ModelSpecificationKeys::Scale]);
                   scale.is_success())
               {
                 return scale;
               }
             case el::ValueType::String:
             case el::ValueType::Boolean:
             case el::ValueType::Number:
             case el::ValueType::Array:
             case el::ValueType::Range:
             case el::ValueType::Null:
             case el::ValueType::Undefined:
               break;
             }

             if (defaultScaleExpression)
             {
               if (const auto scale = defaultScaleExpression->evaluate(context)
                                      | kdl::and_then(convertToScale);
                   scale.is_success())
               {
                 return scale;
               }
             }

             return vm::vec3d{1, 1, 1};
           });
}

kdl_reflect_impl(ModelDefinition);

vm::vec3d safeGetModelScale(
  const ModelDefinition& definition,
  const el::VariableStore& variableStore,
  const std::optional<el::ExpressionNode>& defaultScaleExpression)
{
  return definition.scale(variableStore, defaultScaleExpression)
    .value_or(vm::vec3d{1, 1, 1});
}

} // namespace tb::mdl
