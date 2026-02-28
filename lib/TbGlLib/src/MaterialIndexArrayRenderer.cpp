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

#include "gl/MaterialIndexArrayRenderer.h"

namespace tb::gl
{

MaterialIndexArrayRenderer::MaterialIndexArrayRenderer() = default;

MaterialIndexArrayRenderer::MaterialIndexArrayRenderer(
  VertexArray vertexArray, IndexArray indexArray, MaterialIndexArrayMap indexArrayMap)
  : m_vertexArray{std::move(vertexArray)}
  , m_indexArray{std::move(indexArray)}
  , m_indexRanges{std::move(indexArrayMap)}
{
}

bool MaterialIndexArrayRenderer::empty() const
{
  return m_indexArray.empty();
}

void MaterialIndexArrayRenderer::prepare(Gl& gl, VboManager& vboManager)
{
  m_vertexArray.prepare(gl, vboManager);
  m_indexArray.prepare(gl, vboManager);
}

void MaterialIndexArrayRenderer::render(
  Gl& gl, ShaderProgram& currentProgram, MaterialRenderFunc& func)
{
  if (m_vertexArray.setup(gl, currentProgram))
  {
    if (m_indexArray.setup(gl))
    {
      m_indexRanges.render(gl, m_indexArray, func);
      m_indexArray.cleanup(gl);
    }
    m_vertexArray.cleanup(gl, currentProgram);
  }
}

} // namespace tb::gl
