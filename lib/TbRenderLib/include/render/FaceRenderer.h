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

#include "base/Color.h"
#include "render/Renderable.h"

#include "vm/vec.h"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace tb
{
namespace gl
{
class Gl;
class Material;
class MaterialRenderFunc;
} // namespace gl

namespace render
{
class BrushIndexArray;
class BrushVertexArray;
class RenderBatch;

/**
 * One drawable sub-range within a transparent-pass material's index buffer, carrying
 * enough information to sort it against every other material's groups by distance to
 * the camera and draw it on its own. See FaceRenderer's sorted-draw mode.
 */
struct TransparentDrawItem
{
  const gl::Material* material;
  size_t indexPos;
  size_t indexCount;
  vm::vec3f sortPosition;
};

class FaceRenderer : public IndexedRenderable
{
private:
  using MaterialToBrushIndicesMap =
    const std::unordered_map<const gl::Material*, std::shared_ptr<BrushIndexArray>>;

  std::shared_ptr<BrushVertexArray> m_vertexArray;
  std::shared_ptr<MaterialToBrushIndicesMap> m_indexArrayMap;
  std::shared_ptr<const std::vector<TransparentDrawItem>> m_sortedDrawItems;
  std::vector<const TransparentDrawItem*> m_transparentDrawOrder;
  Color m_faceColor;
  bool m_grayscale = false;
  bool m_tint = false;
  Color m_tintColor;
  float m_alpha = 1.0;
  bool m_disableDepthWrite = false;

public:
  FaceRenderer();
  /**
   * If `sortedDrawItems` is non-null, faces are drawn back-to-front by distance to the
   * camera each frame, one draw call per item, instead of one draw call per material.
   * See TransparentDrawItem.
   */
  FaceRenderer(
    std::shared_ptr<BrushVertexArray> vertexArray,
    std::shared_ptr<MaterialToBrushIndicesMap> indexArrayMap,
    Color faceColor,
    std::shared_ptr<const std::vector<TransparentDrawItem>> sortedDrawItems = nullptr);

  void setGrayscale(bool grayscale);
  void setTint(bool tint);
  void setTintColor(const Color& color);
  void setAlpha(float alpha);
  void setDisableDepthWrite(bool disableDepthWrite);

  void render(RenderBatch& renderBatch);

private:
  void prepare(gl::Gl& gl, gl::VboManager& vboManager) override;
  void render(RenderContext& context) override;

  /**
   * Draws m_sortedDrawItems back-to-front by distance to the camera, one draw call per
   * item, switching materials as needed between items.
   */
  void renderTransparentItems(
    RenderContext& context,
    gl::MaterialRenderFunc& func,
    const std::function<void(const gl::Material*)>& setMaterialUniforms);

  void renderOpaqueItems(
    RenderContext& context,
    gl::MaterialRenderFunc& func,
    const std::function<void(const gl::Material*)>& setMaterialUniforms) const;
};

} // namespace render
} // namespace tb
