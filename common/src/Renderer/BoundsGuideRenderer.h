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
#include "Renderer/Renderable.h"
#include "Renderer/SpikeGuideRenderer.h"

#include "vm/bbox.h"

#include <memory>

namespace TrenchBroom
{
namespace View
{
class MapDocument; // FIXME: Renderer should not depend on View
}

namespace Renderer
{
class BoundsGuideRenderer : public DirectRenderable
{
private:
  static const FloatType SpikeLength;

  std::weak_ptr<View::MapDocument> m_document;

  Color m_color;
  vm::bbox3 m_bounds;
  SpikeGuideRenderer m_spikeRenderer;

public:
  explicit BoundsGuideRenderer(std::weak_ptr<View::MapDocument> document);

  void setColor(const Color& color);
  void setBounds(const vm::bbox3& bounds);

private:
  void doPrepareVertices(VboManager& vboManager) override;
  void doRender(RenderContext& renderContext) override;
};
} // namespace Renderer
} // namespace TrenchBroom
