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
#include "render/Circle.h"
#include "render/Renderable.h"

#include <map>

namespace tb
{
namespace gl
{
class VboManager;
}

namespace render
{
class ActiveShader;
class RenderContext;

class PointHandleRenderer : public DirectRenderable
{
private:
  using HandleMap = std::map<Color, std::vector<vm::vec3f>>;

  HandleMap m_pointHandles;
  HandleMap m_highlights;

  Circle m_handle;
  Circle m_highlight;

public:
  PointHandleRenderer();

  void addPoint(const Color& color, const vm::vec3f& position);
  void addHighlight(const Color& color, const vm::vec3f& position);

  void prepare(gl::VboManager& vboManager) override;
  void render(RenderContext& renderContext) override;

private:
  void renderHandles(
    RenderContext& renderContext, const HandleMap& map, Circle& circle, float opacity);

  void clear();
};

} // namespace render
} // namespace tb
