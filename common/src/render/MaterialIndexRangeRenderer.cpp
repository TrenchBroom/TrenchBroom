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

#include "MaterialIndexRangeRenderer.h"

#include <utility>

namespace tb::render
{

MaterialRenderer::~MaterialRenderer() = default;

MaterialIndexRangeRenderer::MaterialIndexRangeRenderer() = default;

MaterialIndexRangeRenderer::MaterialIndexRangeRenderer(
  VertexArray vertexArray, MaterialIndexRangeMap indexRange)
  : m_vertexArray{std::move(vertexArray)}
  , m_indexRange{std::move(indexRange)}
{
}

MaterialIndexRangeRenderer::MaterialIndexRangeRenderer(
  VertexArray vertexArray, const mdl::Material* material, IndexRangeMap indexRange)
  : m_vertexArray{std::move(vertexArray)}
  , m_indexRange{material, std::move(indexRange)}
{
}

MaterialIndexRangeRenderer::~MaterialIndexRangeRenderer() = default;

bool MaterialIndexRangeRenderer::empty() const
{
  return m_vertexArray.empty();
}

void MaterialIndexRangeRenderer::prepare(VboManager& vboManager)
{
  m_vertexArray.prepare(vboManager);
}

void MaterialIndexRangeRenderer::render(MaterialRenderFunc& func)
{
  if (m_vertexArray.setup())
  {
    m_indexRange.render(m_vertexArray, func);
    m_vertexArray.cleanup();
  }
}

MultiMaterialIndexRangeRenderer::MultiMaterialIndexRangeRenderer(
  std::vector<std::unique_ptr<MaterialIndexRangeRenderer>> renderers)
  : m_renderers{std::move(renderers)}
{
}

MultiMaterialIndexRangeRenderer::~MultiMaterialIndexRangeRenderer() = default;

bool MultiMaterialIndexRangeRenderer::empty() const
{
  for (const auto& renderer : m_renderers)
  {
    if (!renderer->empty())
    {
      return false;
    }
  }
  return true;
}

void MultiMaterialIndexRangeRenderer::prepare(VboManager& vboManager)
{
  for (auto& renderer : m_renderers)
  {
    renderer->prepare(vboManager);
  }
}

void MultiMaterialIndexRangeRenderer::render(MaterialRenderFunc& func)
{
  for (auto& renderer : m_renderers)
  {
    renderer->render(func);
  }
}

} // namespace tb::render
