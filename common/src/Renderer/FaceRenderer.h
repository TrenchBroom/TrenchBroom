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
#include "Renderer/Renderable.h"

#include "vm/forward.h"
#include "vm/vec.h"

#include <memory>
#include <unordered_map>

namespace TrenchBroom
{
namespace Assets
{
class Texture;
}

namespace Renderer
{
class BrushIndexArray;
class BrushVertexArray;
class RenderBatch;

class FaceRenderer : public IndexedRenderable
{
private:
  struct RenderFunc;

  using TextureToBrushIndicesMap =
    const std::unordered_map<const Assets::Texture*, std::shared_ptr<BrushIndexArray>>;

  std::shared_ptr<BrushVertexArray> m_vertexArray;
  std::shared_ptr<TextureToBrushIndicesMap> m_indexArrayMap;
  Color m_faceColor;
  bool m_grayscale;
  bool m_tint;
  Color m_tintColor;
  float m_alpha;

public:
  FaceRenderer();
  FaceRenderer(
    std::shared_ptr<BrushVertexArray> vertexArray,
    std::shared_ptr<TextureToBrushIndicesMap> indexArrayMap,
    const Color& faceColor);

  FaceRenderer(const FaceRenderer& other);
  FaceRenderer& operator=(FaceRenderer other);
  friend void swap(FaceRenderer& left, FaceRenderer& right);

  void setGrayscale(bool grayscale);
  void setTint(bool tint);
  void setTintColor(const Color& color);
  void setAlpha(float alpha);

  void render(RenderBatch& renderBatch);

private:
  void prepareVerticesAndIndices(VboManager& vboManager) override;
  void doRender(RenderContext& context) override;
};

void swap(FaceRenderer& left, FaceRenderer& right);
} // namespace Renderer
} // namespace TrenchBroom
