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

#include "gl/IndexArray.h"

#include "kd/contracts.h"

#include <utility>

namespace tb::gl
{

IndexArray::IndexArray() = default;

bool IndexArray::empty() const
{
  return indexCount() == 0;
}

size_t IndexArray::sizeInBytes() const
{
  return m_holder.get() == nullptr ? 0 : m_holder->sizeInBytes();
}

size_t IndexArray::indexCount() const
{
  return m_holder.get() == nullptr ? 0 : m_holder->indexCount();
}

bool IndexArray::prepared() const
{
  return m_prepared;
}

void IndexArray::prepare(Gl& gl, VboManager& vboManager)
{
  if (!prepared() && !empty())
  {
    m_holder->prepare(gl, vboManager);
  }
  m_prepared = true;
}

bool IndexArray::setup(Gl& gl)
{
  if (empty())
  {
    return false;
  }

  contract_assert(prepared());
  contract_assert(!m_setup);

  m_holder->setup(gl);
  m_setup = true;
  return true;
}

void IndexArray::render(
  Gl& gl, const PrimType primType, const size_t offset, size_t count)
{
  contract_pre(prepared());

  if (!empty())
  {
    if (!m_setup)
    {
      if (setup(gl))
      {
        m_holder->render(gl, primType, offset, count);
        cleanup(gl);
      }
    }
    else
    {
      m_holder->render(gl, primType, offset, count);
    }
  }
}

void IndexArray::cleanup(Gl& gl)
{
  contract_pre(m_setup);
  contract_pre(!empty());

  m_holder->cleanup(gl);
  m_setup = false;
}

IndexArray::IndexArray(BaseHolder::Ptr holder)
  : m_holder{std::move(holder)}
{
}

} // namespace tb::gl
