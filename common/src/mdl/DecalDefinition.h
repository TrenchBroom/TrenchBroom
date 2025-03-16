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

#include "el/Expression.h"

#include "kdl/reflection_decl.h"

namespace tb
{
struct FileLocation;
}

namespace tb::mdl
{

namespace DecalSpecificationKeys
{
constexpr auto Material = "texture";
} // namespace DecalSpecificationKeys

struct DecalSpecification
{
  std::string materialName;

  kdl_reflect_decl(DecalSpecification, materialName);
};

class DecalDefinition
{
private:
  el::ExpressionNode m_expression;

public:
  DecalDefinition();
  explicit DecalDefinition(const FileLocation& location);
  explicit DecalDefinition(el::ExpressionNode expression);

  void append(const DecalDefinition& other);

  /**
   * Evaluates the decal expresion, using the given variable store to interpolate
   * variables.
   *
   * @param variableStore the variable store to use when interpolating variables
   * @return the decal specification
   *
   * @throws el::Exception if the expression could not be evaluated
   */
  DecalSpecification decalSpecification(const el::VariableStore& variableStore) const;

  /**
   * Evaluates the decal expresion.
   *
   * @return the decal specification
   *
   * @throws el::Exception if the expression could not be evaluated
   */
  DecalSpecification defaultDecalSpecification() const;

  kdl_reflect_decl(DecalDefinition, m_expression);
};

} // namespace tb::mdl
