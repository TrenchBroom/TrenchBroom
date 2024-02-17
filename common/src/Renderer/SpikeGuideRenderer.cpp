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

#include "SpikeGuideRenderer.h"

#include "Model/BrushNode.h"
#include "Model/Hit.h"
#include "Model/HitFilter.h"
#include "Model/PickResult.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "Renderer/VboManager.h"
#include "View/MapDocument.h"

#include "vecmath/forward.h"
#include "vecmath/ray.h"
#include "vecmath/vec.h"

#include <memory>

namespace TrenchBroom
{
namespace Renderer
{
SpikeGuideRenderer::SpikeGuideRenderer()
  : m_valid(false)
{
}

void SpikeGuideRenderer::setColor(const Color& color)
{
  m_color = color;
  m_valid = false;
}

void SpikeGuideRenderer::add(
  const vm::ray3& ray,
  const FloatType length,
  std::shared_ptr<View::MapDocument> document)
{
  Model::PickResult pickResult = Model::PickResult::byDistance();
  document->pick(ray, pickResult);

  using namespace Model::HitFilters;
  const auto& hit =
    pickResult.first(type(Model::BrushNode::BrushHitType) && minDistance(1.0));
  if (hit.isMatch())
  {
    if (hit.distance() <= length)
      addPoint(vm::point_at_distance(ray, hit.distance() - 0.01));
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
  m_spikeArray = VertexArray();
  m_pointArray = VertexArray();
  m_valid = true;
}

void SpikeGuideRenderer::doPrepareVertices(VboManager& vboManager)
{
  if (!m_valid)
    validate();
  m_pointArray.prepare(vboManager);
  m_spikeArray.prepare(vboManager);
}

void SpikeGuideRenderer::doRender(RenderContext& renderContext)
{
  ActiveShader shader(renderContext.shaderManager(), Shaders::VaryingPCShader);
  m_spikeArray.render(PrimType::Lines);

  glAssert(glPointSize(3.0f));
  m_pointArray.render(PrimType::Points);
  glAssert(glPointSize(1.0f));
}

void SpikeGuideRenderer::addPoint(const vm::vec3& position)
{
  m_pointVertices.emplace_back(vm::vec3f(position), m_color);
}

void SpikeGuideRenderer::addSpike(
  const vm::ray3& ray, const FloatType length, const FloatType maxLength)
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
} // namespace Renderer
} // namespace TrenchBroom
