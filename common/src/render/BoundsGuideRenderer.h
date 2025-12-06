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

#include "vm/bbox.h"

namespace tb::render
{

class BoundsGuideRenderer : public DirectRenderable
{
private:
  static const double SpikeLength;

  Color m_color;
  vm::bbox3d m_bounds;
  SpikeGuideRenderer m_spikeRenderer;

public:
  void setColor(const Color& color);
  void setBounds(const vm::bbox3d& bounds);

private:
  void doPrepareVertices(VboManager& vboManager) override;
  void doRender(RenderContext& renderContext) override;
};

} // namespace tb::render
