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

#include "render/Sphere.h"

#include "gl/PrimType.h"
#include "gl/VertexType.h"
#include "mdl/BasicShapes.h"
#include "render/RenderContext.h"

namespace tb::render
{

Sphere::Sphere(const float radius, const size_t iterations)
{
  using Vertex = gl::VertexTypes::P3::Vertex;

  const auto positions = mdl::sphere(radius, iterations);
  m_array =
    gl::VertexArray::move(Vertex::toList(positions.size(), std::begin(positions)));
}

bool Sphere::prepared() const
{
  return m_array.prepared();
}

void Sphere::prepare(gl::Gl& gl, gl::VboManager& vboManager)
{
  m_array.prepare(gl, vboManager);
}

void Sphere::render(RenderContext& renderContext)
{
  m_array.render(renderContext.gl(), gl::PrimType::Triangles);
}

} // namespace tb::render
