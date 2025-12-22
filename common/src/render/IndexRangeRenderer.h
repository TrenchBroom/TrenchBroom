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

#pragma once

#include "render/IndexRangeMap.h"
#include "render/IndexRangeMapBuilder.h"
#include "render/VertexArray.h"

namespace tb
{
namespace gl
{
class VboManager;
}

namespace render
{

class IndexRangeRenderer
{
private:
  VertexArray m_vertexArray;
  IndexRangeMap m_indexArray;

public:
  IndexRangeRenderer();
  template <typename VertexSpec>
  explicit IndexRangeRenderer(IndexRangeMapBuilder<VertexSpec>& builder)
    : m_vertexArray{VertexArray::move(std::move(builder.vertices()))}
    , m_indexArray{std::move(builder.indices())}
  {
  }

  IndexRangeRenderer(VertexArray vertexArray, IndexRangeMap indexArray);

  void prepare(gl::VboManager& vboManager);
  void render();
};

} // namespace render
} // namespace tb
