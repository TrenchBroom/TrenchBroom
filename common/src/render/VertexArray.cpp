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

#include "VertexArray.h"

#include "render/PrimType.h"

#include "kd/contracts.h"

namespace tb::render
{

VertexArray::BaseHolder::~BaseHolder() = default;

VertexArray::VertexArray() = default;

bool VertexArray::empty() const
{
  return vertexCount() == 0;
}

size_t VertexArray::sizeInBytes() const
{
  return m_holder ? m_holder->sizeInBytes() : 0;
}

size_t VertexArray::vertexCount() const
{
  return m_holder ? m_holder->vertexCount() : 0;
}

bool VertexArray::prepared() const
{
  return m_prepared;
}

void VertexArray::prepare(gl::VboManager& vboManager)
{
  if (!prepared() && !empty())
  {
    m_holder->prepare(vboManager);
  }
  m_prepared = true;
}

bool VertexArray::setup()
{
  if (empty())
  {
    return false;
  }

  contract_assert(prepared());
  contract_assert(!m_setup);

  m_holder->setup();
  m_setup = true;
  return true;
}

void VertexArray::cleanup()
{
  contract_pre(m_setup);
  contract_pre(!empty());

  m_holder->cleanup();
  m_setup = false;
}

void VertexArray::render(const PrimType primType)
{
  render(primType, 0, static_cast<GLsizei>(vertexCount()));
}

void VertexArray::render(const PrimType primType, const GLint index, const GLsizei count)
{
  contract_pre(prepared());

  if (!m_setup)
  {
    if (setup())
    {
      glAssert(glDrawArrays(toGL(primType), index, count));
      cleanup();
    }
  }
  else
  {
    glAssert(glDrawArrays(toGL(primType), index, count));
  }
}

void VertexArray::render(
  const PrimType primType,
  const gl::Indices& indices,
  const gl::Counts& counts,
  const GLint primCount)
{
  contract_pre(prepared());

  if (!m_setup)
  {
    if (setup())
    {
      const auto* indexArray = indices.data();
      const auto* countArray = counts.data();
      glAssert(glMultiDrawArrays(toGL(primType), indexArray, countArray, primCount));
      cleanup();
    }
  }
  else
  {
    const auto* indexArray = indices.data();
    const auto* countArray = counts.data();
    glAssert(glMultiDrawArrays(toGL(primType), indexArray, countArray, primCount));
  }
}

void VertexArray::render(
  const PrimType primType, const gl::Indices& indices, const GLsizei count)
{
  contract_pre(prepared());

  if (!m_setup)
  {
    if (setup())
    {
      const auto* indexArray = indices.data();
      glAssert(glDrawElements(toGL(primType), count, GL_UNSIGNED_INT, indexArray));
      cleanup();
    }
  }
  else
  {
    const auto* indexArray = indices.data();
    glAssert(glDrawElements(toGL(primType), count, GL_UNSIGNED_INT, indexArray));
  }
}

VertexArray::VertexArray(std::shared_ptr<BaseHolder> holder)
  : m_holder{std::move(holder)}
{
}

} // namespace tb::render
