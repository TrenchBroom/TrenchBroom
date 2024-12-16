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

#include "SpikeGuideRenderer.h"

#include "mdl/BrushNode.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/PickResult.h"
#include "render/ActiveShader.h"
#include "render/PrimType.h"
#include "render/RenderContext.h"
#include "render/Shaders.h"
#include "render/VboManager.h"
#include "ui/MapDocument.h"

#include "vm/ray.h"
#include "vm/vec.h"

#include <memory>

namespace tb::render
{

void SpikeGuideRenderer::setColor(const Color& color)
{
  m_color = color;
  m_valid = false;
}

void SpikeGuideRenderer::add(
  const vm::ray3d& ray, const double length, std::shared_ptr<ui::MapDocument> document)
{
  using namespace mdl::HitFilters;

  auto pickResult = mdl::PickResult::byDistance();
  document->pick(ray, pickResult);

  if (const auto& hit =
        pickResult.first(type(mdl::BrushNode::BrushHitType) && minDistance(1.0));
      hit.isMatch())
  {
    if (hit.distance() <= length)
    {
      addPoint(vm::point_at_distance(ray, hit.distance() - 0.01));
    }
    addSpike(ray, vm::min(length, hit.distance()), length);
  }
  else
  {
    addSpike(ray, length, length);
  }
  m_valid = false;
}

void SpikeGuideRenderer::clear()
{
  m_spikeVertices.clear();
  m_pointVertices.clear();
  m_spikeArray = VertexArray{};
  m_pointArray = VertexArray{};
  m_valid = true;
}

void SpikeGuideRenderer::doPrepareVertices(VboManager& vboManager)
{
  if (!m_valid)
  {
    validate();
  }
  m_pointArray.prepare(vboManager);
  m_spikeArray.prepare(vboManager);
}

void SpikeGuideRenderer::doRender(RenderContext& renderContext)
{
  auto shader = ActiveShader{renderContext.shaderManager(), Shaders::VaryingPCShader};
  m_spikeArray.render(PrimType::Lines);

  glAssert(glPointSize(3.0f));
  m_pointArray.render(PrimType::Points);
  glAssert(glPointSize(1.0f));
}

void SpikeGuideRenderer::addPoint(const vm::vec3d& position)
{
  m_pointVertices.emplace_back(vm::vec3f(position), m_color);
}

void SpikeGuideRenderer::addSpike(
  const vm::ray3d& ray, const double length, const double maxLength)
{
  const auto mix = static_cast<float>(maxLength / length / 2.0);

  m_spikeVertices.emplace_back(vm::vec3f(ray.origin), m_color);
  m_spikeVertices.emplace_back(
    vm::vec3f(vm::point_at_distance(ray, length)), Color(m_color, m_color.a() * mix));
}

void SpikeGuideRenderer::validate()
{
  m_pointArray = VertexArray::move(std::move(m_pointVertices));
  m_spikeArray = VertexArray::move(std::move(m_spikeVertices));
  m_valid = true;
}

} // namespace tb::render
