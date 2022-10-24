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

#include "Sphere.h"

#include "Renderer/GLVertex.h"
#include "Renderer/GLVertexType.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderUtils.h"

#include <vecmath/vec.h>

namespace TrenchBroom
{
namespace Renderer
{
Sphere::Sphere(const float radius, const size_t iterations)
{
  using Vertex = GLVertexTypes::P3::Vertex;

  const auto positions = sphere3D(radius, iterations);
  m_array = VertexArray::move(Vertex::toList(positions.size(), std::begin(positions)));
}

bool Sphere::prepared() const
{
  return m_array.prepared();
}

void Sphere::prepare(VboManager& vboManager)
{
  m_array.prepare(vboManager);
}

void Sphere::render()
{
  m_array.render(PrimType::Triangles);
}
} // namespace Renderer
} // namespace TrenchBroom
