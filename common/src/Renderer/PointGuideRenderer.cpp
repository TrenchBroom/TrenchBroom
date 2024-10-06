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

#include "PointGuideRenderer.h"

#include "kdl/memory_utils.h"

#include "vm/ray.h"
#include "vm/vec.h"

#include <utility>

namespace tb::Renderer
{
const double PointGuideRenderer::SpikeLength = 512.0;

PointGuideRenderer::PointGuideRenderer(std::weak_ptr<View::MapDocument> document)
  : m_document{std::move(document)}
{
}

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

    auto document = kdl::mem_lock(m_document);
    m_spikeRenderer.add(vm::ray3d(position, vm::vec3d{1, 0, 0}), SpikeLength, document);
    m_spikeRenderer.add(vm::ray3d(position, vm::vec3d{-1, 0, 0}), SpikeLength, document);
    m_spikeRenderer.add(vm::ray3d(position, vm::vec3d{0, 1, 0}), SpikeLength, document);
    m_spikeRenderer.add(vm::ray3d(position, vm::vec3d{0, -1, 0}), SpikeLength, document);
    m_spikeRenderer.add(vm::ray3d(position, vm::vec3d{0, 0, 1}), SpikeLength, document);
    m_spikeRenderer.add(vm::ray3d(position, vm::vec3d{0, 0, -1}), SpikeLength, document);

    m_position = position;
  }
}

void PointGuideRenderer::doPrepareVertices(VboManager& vboManager)
{
  m_spikeRenderer.prepareVertices(vboManager);
}

void PointGuideRenderer::doRender(RenderContext& renderContext)
{
  m_spikeRenderer.render(renderContext);
}

} // namespace tb::Renderer
