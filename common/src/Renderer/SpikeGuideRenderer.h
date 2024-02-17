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

#pragma once

#include "Color.h"
#include "FloatType.h"
#include "Renderer/GLVertexType.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"

#include "vm/forward.h"

#include <memory>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
class Picker;
}

namespace View
{
// FIXME: Renderer should not depend on View
class MapDocument;
} // namespace View

namespace Renderer
{
class SpikeGuideRenderer : public DirectRenderable
{
private:
  Color m_color;

  using SpikeVertex = GLVertexTypes::P3C4::Vertex;
  using PointVertex = GLVertexTypes::P3C4::Vertex;

  std::vector<SpikeVertex> m_spikeVertices;
  std::vector<PointVertex> m_pointVertices;

  VertexArray m_spikeArray;
  VertexArray m_pointArray;

  bool m_valid;

public:
  SpikeGuideRenderer();

  void setColor(const Color& color);
  void add(
    const vm::ray3& ray, FloatType length, std::shared_ptr<View::MapDocument> document);
  void clear();

private:
  void doPrepareVertices(VboManager& vboManager) override;
  void doRender(RenderContext& renderContext) override;

private:
  void addPoint(const vm::vec3& position);
  void addSpike(const vm::ray3& ray, FloatType length, FloatType maxLength);

  void validate();
};
} // namespace Renderer
} // namespace TrenchBroom
