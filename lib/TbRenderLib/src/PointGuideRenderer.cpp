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

#include "render/PointGuideRenderer.h"

#include "vm/ray.h"
#include "vm/vec.h"

namespace tb::render
{

void PointGuideRenderer::setColor(const Color& color)
{
  if (color != m_color)
  {
    m_spikeRenderer.setColor(color);
    m_color = color;
  }
}

void PointGuideRenderer::setPosition(const vm::vec3d& position)
{
  if (position != m_position)
  {
    m_spikeRenderer.clear();

    m_spikeRenderer.add(vm::ray3d{position, {1, 0, 0}});
    m_spikeRenderer.add(vm::ray3d{position, {-1, 0, 0}});
    m_spikeRenderer.add(vm::ray3d{position, {0, 1, 0}});
    m_spikeRenderer.add(vm::ray3d{position, {0, -1, 0}});
    m_spikeRenderer.add(vm::ray3d{position, {0, 0, 1}});
    m_spikeRenderer.add(vm::ray3d{position, {0, 0, -1}});

    m_position = position;
  }
}

void PointGuideRenderer::prepare(gl::Gl& gl, gl::VboManager& vboManager)
{
  m_spikeRenderer.prepare(gl, vboManager);
}

void PointGuideRenderer::render(RenderContext& renderContext)
{
  m_spikeRenderer.render(renderContext);
}

} // namespace tb::render
