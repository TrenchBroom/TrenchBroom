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

#include "BoundsGuideRenderer.h"

#include "mdl/Map.h"

#include "vm/ray.h"

namespace tb::render
{
const double BoundsGuideRenderer::SpikeLength = 512.0;

BoundsGuideRenderer::BoundsGuideRenderer(mdl::Map& map)
  : m_map{map}
{
}

void BoundsGuideRenderer::setColor(const Color& color)
{
  if (m_color != color)
  {
    m_spikeRenderer.setColor(color);
    m_color = color;
  }
}

void BoundsGuideRenderer::setBounds(const vm::bbox3d& bounds)
{
  if (m_bounds != bounds)
  {
    m_bounds = bounds;
    m_spikeRenderer.clear();

    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::min, vm::bbox3d::corner::min, vm::bbox3d::corner::min),
        vm::vec3d{-1, 0, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::min, vm::bbox3d::corner::min, vm::bbox3d::corner::min),
        vm::vec3d{0, -1, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::min, vm::bbox3d::corner::min, vm::bbox3d::corner::min),
        vm::vec3d{0, 0, -1}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::min, vm::bbox3d::corner::min, vm::bbox3d::corner::max),
        vm::vec3d{-1, 0, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::min, vm::bbox3d::corner::min, vm::bbox3d::corner::max),
        vm::vec3d{0, -1, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::min, vm::bbox3d::corner::min, vm::bbox3d::corner::max),
        vm::vec3d{0, 0, 1}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::min, vm::bbox3d::corner::max, vm::bbox3d::corner::min),
        vm::vec3d{-1, 0, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::min, vm::bbox3d::corner::max, vm::bbox3d::corner::min),
        vm::vec3d{0, 1, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::min, vm::bbox3d::corner::max, vm::bbox3d::corner::min),
        vm::vec3d{0, 0, -1}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::min, vm::bbox3d::corner::max, vm::bbox3d::corner::max),
        vm::vec3d{-1, 0, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::min, vm::bbox3d::corner::max, vm::bbox3d::corner::max),
        vm::vec3d{0, 1, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::min, vm::bbox3d::corner::max, vm::bbox3d::corner::max),
        vm::vec3d{0, 0, 1}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::max, vm::bbox3d::corner::min, vm::bbox3d::corner::min),
        vm::vec3d{1, 0, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::max, vm::bbox3d::corner::min, vm::bbox3d::corner::min),
        vm::vec3d{0, -1, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::max, vm::bbox3d::corner::min, vm::bbox3d::corner::min),
        vm::vec3d{0, 0, -1}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::max, vm::bbox3d::corner::min, vm::bbox3d::corner::max),
        vm::vec3d{1, 0, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::max, vm::bbox3d::corner::min, vm::bbox3d::corner::max),
        vm::vec3d{0, -1, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::max, vm::bbox3d::corner::min, vm::bbox3d::corner::max),
        vm::vec3d{0, 0, 1}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::max, vm::bbox3d::corner::max, vm::bbox3d::corner::min),
        vm::vec3d{1, 0, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::max, vm::bbox3d::corner::max, vm::bbox3d::corner::min),
        vm::vec3d{0, 1, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::max, vm::bbox3d::corner::max, vm::bbox3d::corner::min),
        vm::vec3d{0, 0, -1}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::max, vm::bbox3d::corner::max, vm::bbox3d::corner::max),
        vm::vec3d{1, 0, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::max, vm::bbox3d::corner::max, vm::bbox3d::corner::max),
        vm::vec3d{0, 1, 0}},
      SpikeLength,
      m_map);
    m_spikeRenderer.add(
      vm::ray3d{
        m_bounds.corner_position(
          vm::bbox3d::corner::max, vm::bbox3d::corner::max, vm::bbox3d::corner::max),
        vm::vec3d{0, 0, 1}},
      SpikeLength,
      m_map);
  }
}

void BoundsGuideRenderer::doPrepareVertices(VboManager& vboManager)
{
  m_spikeRenderer.prepareVertices(vboManager);
}

void BoundsGuideRenderer::doRender(RenderContext& renderContext)
{
  m_spikeRenderer.render(renderContext);
}

} // namespace tb::render
