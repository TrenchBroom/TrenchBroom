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

#include "render/SpikeGuideRenderer.h"

#include "gl/ActiveShader.h"
#include "gl/PrimType.h"
#include "gl/Shaders.h"
#include "gl/VboManager.h"
#include "render/RenderContext.h"

#include "vm/ray.h"
#include "vm/vec.h"

namespace tb::render
{

void SpikeGuideRenderer::setColor(const Color& color)
{
  m_color = color;
  m_valid = false;
}

void SpikeGuideRenderer::add(const vm::ray3d& ray)
{
  addSpike(ray);
  m_valid = false;
}

void SpikeGuideRenderer::clear()
{
  m_spikeVertices.clear();
  m_spikeArray = gl::VertexArray{};
  m_valid = true;
}

void SpikeGuideRenderer::prepare(gl::Gl& gl, gl::VboManager& vboManager)
{
  if (!m_valid)
  {
    validate();
  }
  m_spikeArray.prepare(gl, vboManager);
}

void SpikeGuideRenderer::render(RenderContext& renderContext)
{
  auto& gl = renderContext.gl();

  auto shader =
    gl::ActiveShader{gl, renderContext.shaderManager(), gl::Shaders::VaryingPCShader};

  if (m_spikeArray.setup(gl, shader.program()))
  {
    m_spikeArray.render(gl, gl::PrimType::Lines);
    m_spikeArray.cleanup(gl, shader.program());
  }
}

void SpikeGuideRenderer::addSpike(const vm::ray3d& ray)
{
  constexpr auto mix = 0.5;
  constexpr auto maxLength = 1024.0;

  m_spikeVertices.emplace_back(vm::vec3f(ray.origin), m_color.to<RgbaF>().toVec());
  m_spikeVertices.emplace_back(
    vm::vec3f{vm::point_at_distance(ray, maxLength * 0.75)}, m_color.to<RgbaF>().toVec());
  m_spikeVertices.emplace_back(
    vm::vec3f{vm::point_at_distance(ray, maxLength * 0.75)}, m_color.to<RgbaF>().toVec());
  m_spikeVertices.emplace_back(
    vm::vec3f{vm::point_at_distance(ray, maxLength)},
    blendColor(m_color.to<RgbaF>(), mix).toVec());
}

void SpikeGuideRenderer::validate()
{
  m_spikeArray = gl::VertexArray::move(std::move(m_spikeVertices));
  m_valid = true;
}

} // namespace tb::render
