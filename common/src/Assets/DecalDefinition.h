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

#pragma once

#include "EL/Expression.h"

#include <kdl/reflection_decl.h>

#include <vecmath/vec.h>

#include <iosfwd>

namespace TrenchBroom::Assets
{

namespace DecalSpecificationKeys
{
constexpr auto Texture = "texture";
} // namespace DecalSpecificationKeys

struct DecalSpecification
{
  std::string textureName;

  kdl_reflect_decl(DecalSpecification, textureName);
};

class DecalDefinition
{
private:
  EL::Expression m_expression;

public:
  DecalDefinition();
  DecalDefinition(size_t line, size_t column);
  explicit DecalDefinition(EL::Expression expression);

  void append(const DecalDefinition& other);

  /**
   * Evaluates the decal expresion, using the given variable store to interpolate
   * variables.
   *
   * @param variableStore the variable store to use when interpolating variables
   * @return the decal specification
   *
   * @throws EL::Exception if the expression could not be evaluated
   */
  DecalSpecification decalSpecification(const EL::VariableStore& variableStore) const;

  /**
   * Evaluates the decal expresion.
   *
   * @return the decal specification
   *
   * @throws EL::Exception if the expression could not be evaluated
   */
  DecalSpecification defaultDecalSpecification() const;

  kdl_reflect_decl(DecalDefinition, m_expression);
};

} // namespace TrenchBroom::Assets
