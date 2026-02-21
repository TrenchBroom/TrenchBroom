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

#include "gl/VertexArray.h"
#include "gl/VertexType.h"
#include "render/Renderable.h"

#include "vm/bbox.h"

#include <vector>

namespace tb
{
namespace gl
{
class OrthographicCamera;
class VboManager;
} // namespace gl

namespace render
{
class RenderContext;

class GridRenderer : public DirectRenderable
{
private:
  using Vertex = gl::VertexTypes::P3::Vertex;
  gl::VertexArray m_vertexArray;

public:
  GridRenderer(const gl::OrthographicCamera& camera, const vm::bbox3d& worldBounds);

  void prepareVertices(gl::VboManager& vboManager) override;
  void render(RenderContext& renderContext) override;

private:
  static std::vector<Vertex> vertices(
    const gl::OrthographicCamera& camera, const vm::bbox3d& worldBounds);
};

} // namespace render
} // namespace tb
