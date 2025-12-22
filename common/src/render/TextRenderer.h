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
#include "gl/FontDescriptor.h"
#include "gl/VertexType.h"
#include "render/Renderable.h"
#include "render/VertexArray.h"

#include "vm/vec.h"

#include <vector>

namespace tb
{
namespace gl
{
class AttrString;
class VboManager;
} // namespace gl

namespace render
{
class RenderContext;
class TextAnchor;

class TextRenderer : public DirectRenderable
{
private:
  static const float DefaultMaxViewDistance;
  static const float DefaultMinZoomFactor;
  static const vm::vec2f DefaultInset;
  static const size_t RectCornerSegments;
  static const float RectCornerRadius;

  struct Entry
  {
    std::vector<vm::vec2f> vertices;
    vm::vec2f size;
    vm::vec3f offset;
    Color textColor;
    Color backgroundColor;
  };

  struct EntryCollection
  {
    std::vector<Entry> entries;
    size_t textVertexCount = 0;
    size_t rectVertexCount = 0;

    VertexArray textArray;
    VertexArray rectArray;
  };

  using TextVertex = gl::VertexTypes::P3UV2C4::Vertex;
  using RectVertex = gl::VertexTypes::P3C4::Vertex;

  gl::FontDescriptor m_fontDescriptor;
  float m_maxViewDistance;
  float m_minZoomFactor;
  vm::vec2f m_inset;

  EntryCollection m_entries;
  EntryCollection m_entriesOnTop;

public:
  explicit TextRenderer(
    gl::FontDescriptor fontDescriptor,
    float maxViewDistance = DefaultMaxViewDistance,
    float minZoomFactor = DefaultMinZoomFactor,
    const vm::vec2f& inset = DefaultInset);

  void renderString(
    RenderContext& renderContext,
    const Color& textColor,
    const Color& backgroundColor,
    const gl::AttrString& string,
    const TextAnchor& position);
  void renderStringOnTop(
    RenderContext& renderContext,
    const Color& textColor,
    const Color& backgroundColor,
    const gl::AttrString& string,
    const TextAnchor& position);

private:
  void renderString(
    RenderContext& renderContext,
    const Color& textColor,
    const Color& backgroundColor,
    const gl::AttrString& string,
    const TextAnchor& position,
    bool onTop);

  bool isVisible(
    RenderContext& renderContext,
    const gl::AttrString& string,
    const TextAnchor& position,
    float distance,
    bool onTop) const;
  float computeAlphaFactor(
    const RenderContext& renderContext, float distance, bool onTop) const;
  void addEntry(EntryCollection& collection, const Entry& entry);

  vm::vec2f stringSize(RenderContext& renderContext, const gl::AttrString& string) const;

private:
  void doPrepareVertices(gl::VboManager& vboManager) override;
  void prepare(EntryCollection& collection, bool onTop, gl::VboManager& vboManager);

  void addEntry(
    const Entry& entry,
    bool onTop,
    std::vector<TextVertex>& textVertices,
    std::vector<RectVertex>& rectVertices);

  void doRender(RenderContext& renderContext) override;
  void render(EntryCollection& collection, RenderContext& renderContext);

  void clear();
};

} // namespace render
} // namespace tb
