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

#include "PointGuideRenderer.h"

#include "FloatType.h"

#include "kdl/memory_utils.h"

#include <vecmath/ray.h>
#include <vecmath/vec.h>

namespace TrenchBroom
{
namespace Renderer
{
const FloatType PointGuideRenderer::SpikeLength = 512.0;

PointGuideRenderer::PointGuideRenderer(std::weak_ptr<View::MapDocument> document)
  : m_document(document)
{
}

void PointGuideRenderer::setColor(const Color& color)
{
  if (color == m_color)
    return;

  m_spikeRenderer.setColor(color);
  m_color = color;
}

void PointGuideRenderer::setPosition(const vm::vec3& position)
{
  if (position == m_position)
    return;

  m_spikeRenderer.clear();

  auto document = kdl::mem_lock(m_document);
  m_spikeRenderer.add(vm::ray3(position, vm::vec3::pos_x()), SpikeLength, document);
  m_spikeRenderer.add(vm::ray3(position, vm::vec3::neg_x()), SpikeLength, document);
  m_spikeRenderer.add(vm::ray3(position, vm::vec3::pos_y()), SpikeLength, document);
  m_spikeRenderer.add(vm::ray3(position, vm::vec3::neg_y()), SpikeLength, document);
  m_spikeRenderer.add(vm::ray3(position, vm::vec3::pos_z()), SpikeLength, document);
  m_spikeRenderer.add(vm::ray3(position, vm::vec3::neg_z()), SpikeLength, document);

  m_position = position;
}

void PointGuideRenderer::doPrepareVertices(VboManager& vboManager)
{
  m_spikeRenderer.prepareVertices(vboManager);
}

void PointGuideRenderer::doRender(RenderContext& renderContext)
{
  m_spikeRenderer.render(renderContext);
}
} // namespace Renderer
} // namespace TrenchBroom
