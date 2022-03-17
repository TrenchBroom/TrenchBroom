/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "EL/ELExceptions.h"
#include "EL/EvaluationContext.h"
#include "EL/Expressions.h"
#include "EL/Types.h"
#include "EL/Value.h"
#include "EL/VariableStore.h"

#include <kdl/reflection_impl.h>
#include <kdl/string_compare.h>

#include <vecmath/scalar.h>
#include <vecmath/vec_io.h>

#include <ostream>

namespace TrenchBroom {
namespace Assets {
ModelSpecification::ModelSpecification()
  : path{""}
  , skinIndex{0}
  , frameIndex{0} {}

ModelSpecification::ModelSpecification(
  const IO::Path& i_path, const size_t i_skinIndex, const size_t i_frameIndex)
  : path{i_path}
  , skinIndex{i_skinIndex}
  , frameIndex{i_frameIndex} {}

kdl_reflect_impl(ModelSpecification);

ModelDefinition::ModelDefinition()
  : m_expression{EL::LiteralExpression{EL::Value::Undefined}, 0, 0} {}

ModelDefinition::ModelDefinition(const size_t line, const size_t column)
  : m_expression{EL::LiteralExpression{EL::Value::Undefined}, line, column} {}

ModelDefinition::ModelDefinition(const EL::Expression& expression)
  : m_expression{expression} {}

void ModelDefinition::append(const ModelDefinition& other) {
  const size_t line = m_expression.line();
  const size_t column = m_expression.column();

  auto cases = std::vector<EL::Expression>{std::move(m_expression), other.m_expression};

  m_expression = EL::Expression{EL::SwitchExpression{std::move(cases)}, line, column};
}

static IO::Path path(const EL::Value& value) {
  if (value.type() != EL::ValueType::String) {
    return IO::Path();
  }
  const std::string& path = value.stringValue();
  return IO::Path{kdl::cs::str_is_prefix(path, ":") ? path.substr(1) : path};
}

static size_t index(const EL::Value& value) {
  if (!value.convertibleTo(EL::ValueType::Number)) {
    return 0;
  }
  const EL::IntegerType intValue = value.convertTo(EL::ValueType::Number).integerValue();
  return static_cast<size_t>(vm::max(0l, intValue));
}

static ModelSpecification convertToModel(const EL::Value& value) {
  switch (value.type()) {
    case EL::ValueType::Map:
      return ModelSpecification{path(value["path"]), index(value["skin"]), index(value["frame"])};
    case EL::ValueType::String:
      return ModelSpecification{path(value), 0, 0};
    case EL::ValueType::Boolean:
    case EL::ValueType::Number:
    case EL::ValueType::Array:
    case EL::ValueType::Range:
    case EL::ValueType::Null:
    case EL::ValueType::Undefined:
      break;
  }

  return ModelSpecification{};
}

ModelSpecification ModelDefinition::modelSpecification(
  const EL::VariableStore& variableStore) const {
  const auto context = EL::EvaluationContext{variableStore};
  return convertToModel(m_expression.evaluate(context));
}

ModelSpecification ModelDefinition::defaultModelSpecification() const {
  return modelSpecification(EL::NullVariableStore{});
}

static std::optional<vm::vec3> scaleValue(const EL::Value& value) {
  if (value.type() == EL::ValueType::Number) {
    const auto scale = value.numberValue();
    return vm::vec3{scale, scale, scale};
  }

  if (value.type() != EL::ValueType::String) {
    return std::nullopt;
  }

  if (const auto scale = vm::parse<FloatType, 3>(value.stringValue())) {
    return *scale;
  }

  if (!value.convertibleTo(EL::ValueType::Number)) {
    return std::nullopt;
  }

  const auto scale = value.convertTo(EL::ValueType::Number).numberValue();
  return vm::vec3{scale, scale, scale};
}

static std::optional<vm::vec3> convertToScale(const EL::Value& value) {
  if (value.type() == EL::ValueType::Array) {
    for (const auto& x : value.arrayValue()) {
      if (const auto scale = scaleValue(x)) {
        return scale;
      }
    }

    return std::nullopt;
  }

  return scaleValue(value);
}

vm::vec3 ModelDefinition::scale(
  const EL::VariableStore& variableStore,
  const std::optional<EL::Expression>& defaultScaleExpression) const {
  const auto context = EL::EvaluationContext{variableStore};
  const auto value = m_expression.evaluate(context);

  switch (value.type()) {
    case EL::ValueType::Map:
      if (const auto scale = convertToScale(value["scale"])) {
        return *scale;
      }
    case EL::ValueType::String:
    case EL::ValueType::Boolean:
    case EL::ValueType::Number:
    case EL::ValueType::Array:
    case EL::ValueType::Range:
    case EL::ValueType::Null:
    case EL::ValueType::Undefined:
      break;
  }

  if (defaultScaleExpression) {
    if (const auto scale = convertToScale(defaultScaleExpression->evaluate(context))) {
      return *scale;
    }
  }

  return vm::vec3{1, 1, 1};
}

kdl_reflect_impl(ModelDefinition);

vm::vec3 safeGetModelScale(
  const ModelDefinition& definition, const EL::VariableStore& variableStore,
  const std::optional<EL::Expression>& defaultScaleExpression) {
  try {
    return definition.scale(variableStore, defaultScaleExpression);
  } catch (const EL::Exception&) { return vm::vec3{1, 1, 1}; }
}
} // namespace Assets
} // namespace TrenchBroom
