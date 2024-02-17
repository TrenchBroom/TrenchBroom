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

#include "Transformation.h"

#include "Renderer/GL.h"

#include "vm/forward.h"
#include "vm/mat.h"

#include <cassert>

namespace TrenchBroom
{
namespace Renderer
{
Transformation::Transformation(
  const vm::mat4x4f& projection, const vm::mat4x4f& view, const vm::mat4x4f& model)
{
  pushTransformation(projection, view, model);
}

Transformation::~Transformation()
{
  if (m_projectionStack.size() > 1)
    loadProjectionMatrix(m_projectionStack.front());
  if (m_viewStack.size() > 1 || m_modelStack.size() > 1)
    loadModelViewMatrix(m_viewStack.front() * m_modelStack.front());
}

const vm::mat4x4f& Transformation::projectionMatrix() const
{
  assert(!m_projectionStack.empty());
  return m_projectionStack.back();
}

const vm::mat4x4f& Transformation::viewMatrix() const
{
  assert(!m_viewStack.empty());
  return m_viewStack.back();
}

const vm::mat4x4f& Transformation::modelMatrix() const
{
  assert(!m_modelStack.empty());
  return m_modelStack.back();
}

Transformation Transformation::slice() const
{
  return Transformation(
    m_projectionStack.back(), m_viewStack.back(), m_modelStack.back());
}

void Transformation::pushTransformation(
  const vm::mat4x4f& projection, const vm::mat4x4f& view, const vm::mat4x4f& model)
{
  m_projectionStack.push_back(projection);
  m_viewStack.push_back(view);
  m_modelStack.push_back(model);

  loadProjectionMatrix(m_projectionStack.back());
  loadModelViewMatrix(m_viewStack.back() * m_modelStack.back());
}

void Transformation::popTransformation()
{
  assert(m_projectionStack.size() > 1);
  assert(m_viewStack.size() > 1);
  assert(m_modelStack.size() > 1);

  m_projectionStack.pop_back();
  m_viewStack.pop_back();
  m_modelStack.pop_back();

  loadProjectionMatrix(m_projectionStack.back());
  loadModelViewMatrix(m_viewStack.back() * m_modelStack.back());
}

void Transformation::pushModelMatrix(const vm::mat4x4f& matrix)
{
  m_modelStack.push_back(m_modelStack.back() * matrix);
  loadModelViewMatrix(m_viewStack.back() * m_modelStack.back());
}

void Transformation::replaceAndPushModelMatrix(const vm::mat4x4f& matrix)
{
  m_modelStack.push_back(matrix);
  loadModelViewMatrix(m_viewStack.back() * m_modelStack.back());
}

void Transformation::popModelMatrix()
{
  assert(m_modelStack.size() > 1);
  m_modelStack.pop_back();
  loadModelViewMatrix(m_viewStack.back() * m_modelStack.back());
}

void Transformation::loadProjectionMatrix(const vm::mat4x4f& matrix)
{
  glAssert(glMatrixMode(GL_PROJECTION));
  glAssert(glLoadMatrixf(reinterpret_cast<const float*>(matrix.v)));
}

void Transformation::loadModelViewMatrix(const vm::mat4x4f& matrix)
{
  glAssert(glMatrixMode(GL_MODELVIEW));
  glAssert(glLoadMatrixf(reinterpret_cast<const float*>(matrix.v)));
}

ReplaceTransformation::ReplaceTransformation(
  Transformation& transformation,
  const vm::mat4x4f& projectionMatrix,
  const vm::mat4x4f& viewMatrix,
  const vm::mat4x4f& modelMatrix)
  : m_transformation(transformation)
{
  m_transformation.pushTransformation(projectionMatrix, viewMatrix, modelMatrix);
}

ReplaceTransformation::~ReplaceTransformation()
{
  m_transformation.popTransformation();
}

MultiplyModelMatrix::MultiplyModelMatrix(
  Transformation& transformation, const vm::mat4x4f& modelMatrix)
  : m_transformation(transformation)
{
  m_transformation.pushModelMatrix(modelMatrix);
}

MultiplyModelMatrix::~MultiplyModelMatrix()
{
  m_transformation.popModelMatrix();
}

ReplaceModelMatrix::ReplaceModelMatrix(
  Transformation& transformation, const vm::mat4x4f& modelMatrix)
  : m_transformation(transformation)
{
  m_transformation.replaceAndPushModelMatrix(modelMatrix);
}

ReplaceModelMatrix::~ReplaceModelMatrix()
{
  m_transformation.popModelMatrix();
}
} // namespace Renderer
} // namespace TrenchBroom
