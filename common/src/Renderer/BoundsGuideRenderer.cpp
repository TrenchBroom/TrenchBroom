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

#include "BoundsGuideRenderer.h"

#include "kdl/memory_utils.h"

#include "vm/bbox.h"
#include "vm/ray.h"

namespace TrenchBroom
{
namespace Renderer
{
const FloatType BoundsGuideRenderer::SpikeLength = 512.0;

BoundsGuideRenderer::BoundsGuideRenderer(std::weak_ptr<View::MapDocument> document)
  : m_document(document)
{
}

void BoundsGuideRenderer::setColor(const Color& color)
{
  if (m_color == color)
    return;

  m_spikeRenderer.setColor(color);
  m_color = color;
}

void BoundsGuideRenderer::setBounds(const vm::bbox3& bounds)
{
  if (m_bounds == bounds)
    return;

  m_bounds = bounds;
  m_spikeRenderer.clear();

  auto document = kdl::mem_lock(m_document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::min, vm::bbox3::Corner::min, vm::bbox3::Corner::min),
      vm::vec3::neg_x()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::min, vm::bbox3::Corner::min, vm::bbox3::Corner::min),
      vm::vec3::neg_y()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::min, vm::bbox3::Corner::min, vm::bbox3::Corner::min),
      vm::vec3::neg_z()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::min, vm::bbox3::Corner::min, vm::bbox3::Corner::max),
      vm::vec3::neg_x()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::min, vm::bbox3::Corner::min, vm::bbox3::Corner::max),
      vm::vec3::neg_y()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::min, vm::bbox3::Corner::min, vm::bbox3::Corner::max),
      vm::vec3::pos_z()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::min, vm::bbox3::Corner::max, vm::bbox3::Corner::min),
      vm::vec3::neg_x()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::min, vm::bbox3::Corner::max, vm::bbox3::Corner::min),
      vm::vec3::pos_y()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::min, vm::bbox3::Corner::max, vm::bbox3::Corner::min),
      vm::vec3::neg_z()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::min, vm::bbox3::Corner::max, vm::bbox3::Corner::max),
      vm::vec3::neg_x()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::min, vm::bbox3::Corner::max, vm::bbox3::Corner::max),
      vm::vec3::pos_y()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::min, vm::bbox3::Corner::max, vm::bbox3::Corner::max),
      vm::vec3::pos_z()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::max, vm::bbox3::Corner::min, vm::bbox3::Corner::min),
      vm::vec3::pos_x()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::max, vm::bbox3::Corner::min, vm::bbox3::Corner::min),
      vm::vec3::neg_y()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::max, vm::bbox3::Corner::min, vm::bbox3::Corner::min),
      vm::vec3::neg_z()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::max, vm::bbox3::Corner::min, vm::bbox3::Corner::max),
      vm::vec3::pos_x()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::max, vm::bbox3::Corner::min, vm::bbox3::Corner::max),
      vm::vec3::neg_y()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::max, vm::bbox3::Corner::min, vm::bbox3::Corner::max),
      vm::vec3::pos_z()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::max, vm::bbox3::Corner::max, vm::bbox3::Corner::min),
      vm::vec3::pos_x()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::max, vm::bbox3::Corner::max, vm::bbox3::Corner::min),
      vm::vec3::pos_y()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::max, vm::bbox3::Corner::max, vm::bbox3::Corner::min),
      vm::vec3::neg_z()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::max, vm::bbox3::Corner::max, vm::bbox3::Corner::max),
      vm::vec3::pos_x()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::max, vm::bbox3::Corner::max, vm::bbox3::Corner::max),
      vm::vec3::pos_y()),
    SpikeLength,
    document);
  m_spikeRenderer.add(
    vm::ray3(
      m_bounds.corner(
        vm::bbox3::Corner::max, vm::bbox3::Corner::max, vm::bbox3::Corner::max),
      vm::vec3::pos_z()),
    SpikeLength,
    document);
}

void BoundsGuideRenderer::doPrepareVertices(VboManager& vboManager)
{
  m_spikeRenderer.prepareVertices(vboManager);
}

void BoundsGuideRenderer::doRender(RenderContext& renderContext)
{
  m_spikeRenderer.render(renderContext);
}
} // namespace Renderer
} // namespace TrenchBroom
