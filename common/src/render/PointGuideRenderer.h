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
#include "render/Renderable.h"
#include "render/SpikeGuideRenderer.h"

#include "vm/vec.h"

namespace tb::mdl
{
class Map;
}

namespace tb::render
{
class RenderContext;
class VboManager;

class PointGuideRenderer : public DirectRenderable
{
private:
  static const double SpikeLength;

  mdl::Map& m_map;

  Color m_color;
  vm::vec3d m_position;
  SpikeGuideRenderer m_spikeRenderer;

public:
  explicit PointGuideRenderer(mdl::Map& map);

  void setColor(const Color& color);
  void setPosition(const vm::vec3d& position);

private:
  void doPrepareVertices(VboManager& vboManager) override;
  void doRender(RenderContext& renderContext) override;
};

} // namespace tb::render
