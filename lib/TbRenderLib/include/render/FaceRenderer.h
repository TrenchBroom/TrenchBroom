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

#include <memory>
#include <unordered_map>

namespace tb
{
namespace gl
{
class Material;
}

namespace render
{
class BrushIndexArray;
class BrushVertexArray;
class RenderBatch;

class FaceRenderer : public IndexedRenderable
{
private:
  using MaterialToBrushIndicesMap =
    const std::unordered_map<const gl::Material*, std::shared_ptr<BrushIndexArray>>;

  std::shared_ptr<BrushVertexArray> m_vertexArray;
  std::shared_ptr<MaterialToBrushIndicesMap> m_indexArrayMap;
  Color m_faceColor;
  bool m_grayscale = false;
  bool m_tint = false;
  Color m_tintColor;
  float m_alpha = 1.0;

public:
  FaceRenderer();
  FaceRenderer(
    std::shared_ptr<BrushVertexArray> vertexArray,
    std::shared_ptr<MaterialToBrushIndicesMap> indexArrayMap,
    Color faceColor);

  void setGrayscale(bool grayscale);
  void setTint(bool tint);
  void setTintColor(const Color& color);
  void setAlpha(float alpha);

  void render(RenderBatch& renderBatch);

private:
  void prepareVerticesAndIndices(gl::VboManager& vboManager) override;
  void render(RenderContext& context) override;
};

} // namespace render
} // namespace tb
