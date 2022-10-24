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

#include "Renderer/BrushRendererArrays.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>

namespace TrenchBroom
{
// BrushIndexArray

namespace Renderer
{

// DirtyRangeTracker

DirtyRangeTracker::DirtyRangeTracker(const size_t initial_capacity)
  : m_dirtyPos(0)
  , m_dirtySize(0)
  , m_capacity(initial_capacity)
{
}

DirtyRangeTracker::DirtyRangeTracker()
  : m_dirtyPos(0)
  , m_dirtySize(0)
  , m_capacity(0)
{
}

void DirtyRangeTracker::expand(const size_t newcap)
{
  if (newcap <= m_capacity)
  {
    throw std::invalid_argument("new capacity must be greater");
  }

  const size_t oldcap = m_capacity;
  m_capacity = newcap;
  markDirty(oldcap, newcap - oldcap);
}

size_t DirtyRangeTracker::capacity() const
{
  return m_capacity;
}

void DirtyRangeTracker::markDirty(const size_t pos, const size_t size)
{
  // bounds check
  if (pos + size > m_capacity)
  {
    throw std::invalid_argument("markDirty provided range out of bounds");
  }

  const size_t newPos = std::min(pos, m_dirtyPos);
  const size_t newEnd = std::max(pos + size, m_dirtyPos + m_dirtySize);

  m_dirtyPos = newPos;
  m_dirtySize = newEnd - newPos;
}

bool DirtyRangeTracker::clean() const
{
  return m_dirtySize == 0;
}

// IndexHolder

IndexHolder::IndexHolder()
  : VboHolder<Index>(VboType::ElementArrayBuffer)
{
}

IndexHolder::IndexHolder(std::vector<Index>& elements)
  : VboHolder<Index>(VboType::ElementArrayBuffer, elements)
{
}

void IndexHolder::zeroRange(const size_t offsetWithinBlock, const size_t count)
{
  Index* dest = getPointerToWriteElementsTo(offsetWithinBlock, count);
  std::memset(dest, 0, count * sizeof(Index));
}

void IndexHolder::render(const PrimType primType, const size_t offset, size_t count) const
{
  const GLsizei renderCount = static_cast<GLsizei>(count);
  const GLvoid* renderOffset =
    reinterpret_cast<GLvoid*>(m_vbo->offset() + sizeof(Index) * offset);

  glAssert(glDrawElements(toGL(primType), renderCount, glType<Index>(), renderOffset));
}

std::shared_ptr<IndexHolder> IndexHolder::swap(std::vector<IndexHolder::Index>& elements)
{
  return std::make_shared<IndexHolder>(elements);
}

VertexArrayInterface::~VertexArrayInterface() {}

// BrushIndexArray

BrushIndexArray::BrushIndexArray()
  : m_indexHolder()
  , m_allocationTracker(0)
{
}

bool BrushIndexArray::hasValidIndices() const
{
  return m_allocationTracker.hasAllocations();
}

std::pair<AllocationTracker::Block*, GLuint*> BrushIndexArray::
  getPointerToInsertElementsAt(const size_t elementCount)
{
  auto block = m_allocationTracker.allocate(elementCount);
  if (block != nullptr)
  {
    GLuint* dest = m_indexHolder.getPointerToWriteElementsTo(block->pos, elementCount);
    return {block, dest};
  }

  // retry
  const size_t newSize = std::max(
    2 * m_allocationTracker.capacity(), m_allocationTracker.capacity() + elementCount);
  m_allocationTracker.expand(newSize);
  m_indexHolder.resize(newSize);

  // insert again
  block = m_allocationTracker.allocate(elementCount);
  assert(block != nullptr);

  GLuint* dest = m_indexHolder.getPointerToWriteElementsTo(block->pos, elementCount);
  return {block, dest};
}

void BrushIndexArray::zeroElementsWithKey(AllocationTracker::Block* key)
{
  const auto pos = key->pos;
  const auto size = key->size;
  m_allocationTracker.free(key);

  m_indexHolder.zeroRange(pos, size);
}

void BrushIndexArray::render(const PrimType primType) const
{
  assert(m_indexHolder.prepared());
  m_indexHolder.render(primType, 0, m_indexHolder.size());
}

bool BrushIndexArray::prepared() const
{
  return m_indexHolder.prepared();
}

void BrushIndexArray::prepare(VboManager& vboManager)
{
  m_indexHolder.prepare(vboManager);
  assert(m_indexHolder.prepared());
}

void BrushIndexArray::setupIndices()
{
  m_indexHolder.bindBlock();
}

void BrushIndexArray::cleanupIndices()
{
  m_indexHolder.unbindBlock();
}

// BrushVertexArray

BrushVertexArray::BrushVertexArray()
  : m_vertexHolder()
  , m_allocationTracker(0)
{
}

std::pair<AllocationTracker::Block*, BrushVertexArray::Vertex*> BrushVertexArray::
  getPointerToInsertVerticesAt(const size_t vertexCount)
{
  auto block = m_allocationTracker.allocate(vertexCount);
  if (block != nullptr)
  {
    Vertex* dest = m_vertexHolder.getPointerToWriteElementsTo(block->pos, vertexCount);
    return {block, dest};
  }

  // retry
  const size_t newSize = std::max(
    2 * m_allocationTracker.capacity(), m_allocationTracker.capacity() + vertexCount);
  m_allocationTracker.expand(newSize);
  m_vertexHolder.resize(newSize);

  // insert again
  block = m_allocationTracker.allocate(vertexCount);
  assert(block != nullptr);

  Vertex* dest = m_vertexHolder.getPointerToWriteElementsTo(block->pos, vertexCount);
  return {block, dest};
}

void BrushVertexArray::deleteVerticesWithKey(AllocationTracker::Block* key)
{
  m_allocationTracker.free(key);

  // there's no need to actually delete the vertices from the VBO.
  // because we only ever do indexed drawing from it.
  // Marking the space free in m_allocationTracker will allow
  // us to re-use the space later
}

bool BrushVertexArray::setupVertices()
{
  return m_vertexHolder.setupVertices();
}

void BrushVertexArray::cleanupVertices()
{
  m_vertexHolder.cleanupVertices();
}

bool BrushVertexArray::prepared() const
{
  return m_vertexHolder.prepared();
}

void BrushVertexArray::prepare(VboManager& vboManager)
{
  m_vertexHolder.prepare(vboManager);
  assert(m_vertexHolder.prepared());
}
} // namespace Renderer
} // namespace TrenchBroom
