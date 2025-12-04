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

#pragma once

#include "Color.h"
#include "render/GLVertexType.h"
#include "render/Renderable.h"
#include "render/VertexArray.h"

#include "vm/ray.h"

#include <vector>

namespace tb::render
{

class SpikeGuideRenderer : public DirectRenderable
{
private:
  Color m_color;

  using SpikeVertex = GLVertexTypes::P3C4::Vertex;

  std::vector<SpikeVertex> m_spikeVertices;
  VertexArray m_spikeArray;

  bool m_valid = false;

public:
  void setColor(const Color& color);
  void add(const vm::ray3d& ray);
  void clear();

private:
  void doPrepareVertices(VboManager& vboManager) override;
  void doRender(RenderContext& renderContext) override;

private:
  void addSpike(const vm::ray3d& ray);

  void validate();
};
} // namespace tb::render
