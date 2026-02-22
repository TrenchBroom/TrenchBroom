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
#include "gl/IndexRangeRenderer.h"
#include "render/Renderable.h"

#include "vm/mat.h"

namespace tb
{
namespace gl
{
class Camera;
}

namespace render
{
class RenderBatch;

class Compass : public DirectRenderable
{
private:
  static const size_t m_segments;
  static const float m_shaftLength;
  static const float m_shaftRadius;
  static const float m_headLength;
  static const float m_headRadius;

  gl::IndexRangeRenderer m_arrowRenderer;
  gl::IndexRangeRenderer m_backgroundRenderer;
  gl::IndexRangeRenderer m_backgroundOutlineRenderer;
  bool m_prepared = false;

public:
  Compass();
  ~Compass() override;

  void render(RenderBatch& renderBatch);

private:
  void prepare(gl::VboManager& vboManager) override;
  void render(RenderContext& renderContext) override;

  void makeArrows();
  void makeBackground();

  vm::mat4x4f cameraRotationMatrix(const gl::Camera& camera) const;

protected:
  void renderBackground(RenderContext& renderContext);
  void renderSolidAxis(
    RenderContext& renderContext, const vm::mat4x4f& transformation, const Color& color);
  void renderAxisOutline(
    RenderContext& renderContext, const vm::mat4x4f& transformation, const Color& color);
  void renderAxis(RenderContext& renderContext, const vm::mat4x4f& transformation);

private:
  virtual void doRenderCompass(
    RenderContext& renderContext, const vm::mat4x4f& cameraTransformation) = 0;
};

} // namespace render
} // namespace tb
