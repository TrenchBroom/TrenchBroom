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

namespace tb::mdl
{
namespace
{

std::filesystem::path path(el::EvaluationContext& context, const el::Value& value)
{
  if (value.type() != el::ValueType::String)
  {
    return {};
  }
  const auto& path = value.stringValue(context);
  return kdl::cs::str_is_prefix(path, ":") ? path.substr(1) : path;
}

size_t index(el::EvaluationContext& context, const el::Value& value)
{
  if (!value.convertibleTo(el::ValueType::Number))
  {
    return 0;
  }

  const auto intValue =
    value.convertTo(context, el::ValueType::Number).integerValue(context);
  return static_cast<size_t>(vm::max(0l, intValue));
}

ModelSpecification convertToModel(el::EvaluationContext& context, const el::Value& value)
{
  switch (value.type())
  {
  case el::ValueType::Map:
    return ModelSpecification{
      path(context, value.atOrDefault(context, ModelSpecificationKeys::Path)),
      index(context, value.atOrDefault(context, ModelSpecificationKeys::Skin)),
      index(context, value.atOrDefault(context, ModelSpecificationKeys::Frame)),
    };
  case el::ValueType::String:
    return ModelSpecification{path(context, value), 0, 0};
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

std::optional<vm::vec3d> scaleValue(
  el::EvaluationContext& context, const el::Value& value)
{
  if (value.type() == el::ValueType::Number)
  {
    const auto scale = value.numberValue(context);
    return vm::vec3d{scale, scale, scale};
  }

  if (value.type() != el::ValueType::String)
  {
    return std::nullopt;
  }

  const auto stringValue = value.stringValue(context);
  if (kdl::str_is_blank(stringValue))
  {
    return std::nullopt;
  }

  if (const auto scale = vm::parse<double, 3>(stringValue))
  {
    return *scale;
  }

  if (!value.convertibleTo(el::ValueType::Number))
  {
    return std::nullopt;
  }

  const auto scale = value.convertTo(context, el::ValueType::Number).numberValue(context);
  return vm::vec3d{scale, scale, scale};
}

std::optional<vm::vec3d> convertToScale(
  el::EvaluationContext& context, const el::Value& value)
{
  if (value.type() == el::ValueType::Array)
  {
    for (const auto& x : value.arrayValue(context))
    {
      if (const auto scale = scaleValue(context, x))
      {
        return scale;
      }
    }

    return std::nullopt;
  }

  return scaleValue(context, value);
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
  return el::withEvaluationContext(
    [&](auto& context) {
      return convertToModel(context, m_expression.evaluate(context));
    },
    variableStore);
}

Result<ModelSpecification> ModelDefinition::defaultModelSpecification() const
{
  return el::withEvaluationContext([&](auto& context) {
    return convertToModel(context, m_expression.tryEvaluate(context));
  });
}

Result<vm::vec3d> ModelDefinition::scale(
  const el::VariableStore& variableStore,
  const std::optional<el::ExpressionNode>& defaultScaleExpression) const
{
  return el::withEvaluationContext(
    [&](auto& context) {
      const auto value = m_expression.evaluate(context);

      switch (value.type())
      {
      case el::ValueType::Map:
        if (
          const auto scale = convertToScale(
            context, value.atOrDefault(context, ModelSpecificationKeys::Scale)))
        {
          return *scale;
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
        if (
          const auto scale =
            convertToScale(context, defaultScaleExpression->evaluate(context)))
        {
          return *scale;
        }
      }

      return vm::vec3d{1, 1, 1};
    },
    variableStore);
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
