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

#pragma once

#include "EL/Expression.h"
#include "FloatType.h"

#include <kdl/reflection_decl.h>
#include <vecmath/vec.h>

#include <filesystem>
#include <iosfwd>
#include <optional>

namespace TrenchBroom
{
namespace Assets
{

namespace ModelSpecificationKeys
{
constexpr auto Path = "path";
constexpr auto Skin = "skin";
constexpr auto Frame = "frame";
constexpr auto Scale = "scale";
} // namespace ModelSpecificationKeys

struct ModelSpecification
{
  std::filesystem::path path;
  size_t skinIndex;
  size_t frameIndex;

  ModelSpecification();
  ModelSpecification(
    const std::filesystem::path& path, size_t skinIndex, size_t frameIndex);

  kdl_reflect_decl(ModelSpecification, path, skinIndex, frameIndex);
};

class ModelDefinition
{
private:
  EL::Expression m_expression;

public:
  ModelDefinition();
  ModelDefinition(size_t line, size_t column);
  explicit ModelDefinition(const EL::Expression& expression);

  void append(const ModelDefinition& other);

  /**
   * Evaluates the model expresion, using the given variable store to interpolate
   * variables.
   *
   * @param variableStore the variable store to use when interpolating variables
   * @return the model specification
   *
   * @throws EL::Exception if the expression could not be evaluated
   */
  ModelSpecification modelSpecification(const EL::VariableStore& variableStore) const;

  /**
   * Evaluates the model expresion.
   *
   * @return the model specification
   *
   * @throws EL::Exception if the expression could not be evaluated
   */
  ModelSpecification defaultModelSpecification() const;

  /**
   * Evaluates the model expression using the given variable store to interpolate
   * variables, and returns the scale value configured for the model, if any. If the model
   * expression doesn't have its own scale expression, then the given scale expression is
   * used instead.
   *
   * @throws EL::Exception if the expression could not be evaluated
   */
  vm::vec3 scale(
    const EL::VariableStore& variableStore,
    const std::optional<EL::Expression>& defaultScaleExpression) const;

  kdl_reflect_decl(ModelDefinition, m_expression);
};

/**
 * Returns the model scale value for the given parameters or a default scale of 1, 1, 1 if
 * an error occurs.
 */
vm::vec3 safeGetModelScale(
  const ModelDefinition& definition,
  const EL::VariableStore& variableStore,
  const std::optional<EL::Expression>& defaultScaleExpression);
} // namespace Assets
} // namespace TrenchBroom
