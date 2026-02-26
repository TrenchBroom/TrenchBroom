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

#include "render/TextRenderer.h"

#include "gl/ActiveShader.h"
#include "gl/AttrString.h"
#include "gl/Camera.h"
#include "gl/FontManager.h"
#include "gl/GlInterface.h"
#include "gl/PrimType.h"
#include "gl/Shaders.h"
#include "gl/TextureFont.h"
#include "mdl/BasicShapes.h"
#include "render/RenderContext.h"
#include "render/TextAnchor.h"

#include "vm/mat_ext.h"
#include "vm/vec.h"

#include <utility>

namespace tb::render
{

const float TextRenderer::DefaultMaxViewDistance = 768.0f;
const float TextRenderer::DefaultMinZoomFactor = 0.5f;
const vm::vec2f TextRenderer::DefaultInset = vm::vec2f(4.0f, 4.0f);
const size_t TextRenderer::RectCornerSegments = 3;
const float TextRenderer::RectCornerRadius = 3.0f;

TextRenderer::TextRenderer(
  gl::FontDescriptor fontDescriptor,
  const float maxViewDistance,
  const float minZoomFactor,
  const vm::vec2f& inset)
  : m_fontDescriptor{std::move(fontDescriptor)}
  , m_maxViewDistance{maxViewDistance}
  , m_minZoomFactor{minZoomFactor}
  , m_inset{inset}
{
}

void TextRenderer::renderString(
  RenderContext& renderContext,
  const Color& textColor,
  const Color& backgroundColor,
  const gl::AttrString& string,
  const TextAnchor& position)
{
  renderString(renderContext, textColor, backgroundColor, string, position, false);
}

void TextRenderer::renderStringOnTop(
  RenderContext& renderContext,
  const Color& textColor,
  const Color& backgroundColor,
  const gl::AttrString& string,
  const TextAnchor& position)
{
  renderString(renderContext, textColor, backgroundColor, string, position, true);
}

void TextRenderer::renderString(
  RenderContext& renderContext,
  const Color& textColor,
  const Color& backgroundColor,
  const gl::AttrString& string,
  const TextAnchor& position,
  const bool onTop)
{

  const auto& camera = renderContext.camera();
  const auto distance = camera.perpendicularDistanceTo(position.position(camera));
  if (distance <= 0.0f || !isVisible(renderContext, string, position, distance, onTop))
  {
    return;
  }

  auto& fontManager = renderContext.fontManager();
  auto& font = fontManager.font(m_fontDescriptor);

  auto vertices = font.quads(string, true);
  const auto alphaFactor = computeAlphaFactor(renderContext, distance, onTop);
  const auto size = font.measure(string);
  const auto offset = position.offset(camera, size);

  if (onTop)
  {
    addEntry(
      m_entriesOnTop,
      Entry{
        std::move(vertices),
        size,
        offset,
        blendColor(textColor.to<RgbaF>(), alphaFactor),
        blendColor(backgroundColor.to<RgbaF>(), alphaFactor)});
  }
  else
  {
    addEntry(
      m_entries,
      Entry{
        std::move(vertices),
        size,
        offset,
        blendColor(textColor.to<RgbaF>(), alphaFactor),
        blendColor(backgroundColor.to<RgbaF>(), alphaFactor)});
  }
}

bool TextRenderer::isVisible(
  RenderContext& renderContext,
  const gl::AttrString& string,
  const TextAnchor& position,
  const float distance,
  const bool onTop) const
{
  if (!onTop)
  {
    if (renderContext.render3D() && distance > m_maxViewDistance)
    {
      return false;
    }
    if (renderContext.render2D() && renderContext.camera().zoom() < m_minZoomFactor)
    {
      return false;
    }
  }

  const auto& camera = renderContext.camera();
  const auto& viewport = camera.viewport();

  const auto size = stringSize(renderContext, string);
  const auto offset = vm::vec2f{position.offset(camera, size)} - m_inset;
  const auto actualSize = size + 2.0f * m_inset;

  return viewport.contains(offset.x(), offset.y(), actualSize.x(), actualSize.y());
}

float TextRenderer::computeAlphaFactor(
  const RenderContext& renderContext, const float distance, const bool onTop) const
{
  if (onTop)
  {
    return 1.0f;
  }

  if (renderContext.render3D())
  {
    const auto a = m_maxViewDistance - distance;
    return std::min(a / 128.0f, 1.0f);
  }

  const auto z = renderContext.camera().zoom();
  const auto d = z - m_minZoomFactor;
  return std::min(d / 0.3f, 1.0f);
}

void TextRenderer::addEntry(EntryCollection& collection, const Entry& entry)
{
  collection.entries.push_back(entry);
  collection.textVertexCount += entry.vertices.size();
  collection.rectVertexCount += mdl::roundedRect2DVertexCount(RectCornerSegments);
}

vm::vec2f TextRenderer::stringSize(
  RenderContext& renderContext, const gl::AttrString& string) const
{
  auto& fontManager = renderContext.fontManager();
  auto& font = fontManager.font(m_fontDescriptor);
  return vm::round(font.measure(string));
}

void TextRenderer::prepare(gl::Gl& gl, gl::VboManager& vboManager)
{
  prepare(gl, m_entries, false, vboManager);
  prepare(gl, m_entriesOnTop, true, vboManager);
}

void TextRenderer::render(RenderContext& renderContext)
{
  auto& gl = renderContext.gl();

  const auto& viewport = renderContext.camera().viewport();
  const auto projection = vm::ortho_matrix(
    0.0f,
    1.0f,
    static_cast<float>(viewport.x),
    static_cast<float>(viewport.height),
    static_cast<float>(viewport.width),
    static_cast<float>(viewport.y));
  const auto view = vm::view_matrix(vm::vec3f{0, 0, -1}, vm::vec3f{0, 1, 0});
  auto ortho = ReplaceTransformation{renderContext.transformation(), projection, view};

  render(m_entries, renderContext);

  gl.disable(GL_DEPTH_TEST);
  render(m_entriesOnTop, renderContext);
  gl.enable(GL_DEPTH_TEST);
}

void TextRenderer::prepare(
  gl::Gl& gl, EntryCollection& collection, const bool onTop, gl::VboManager& vboManager)
{
  auto textVertices = std::vector<TextVertex>{};
  textVertices.reserve(collection.textVertexCount);

  auto rectVertices = std::vector<RectVertex>{};
  rectVertices.reserve(collection.rectVertexCount);

  for (const auto& entry : collection.entries)
  {
    addEntry(entry, onTop, textVertices, rectVertices);
  }

  collection.textArray = gl::VertexArray::move(std::move(textVertices));
  collection.rectArray = gl::VertexArray::move(std::move(rectVertices));

  collection.textArray.prepare(gl, vboManager);
  collection.rectArray.prepare(gl, vboManager);
}

void TextRenderer::addEntry(
  const Entry& entry,
  const bool /* onTop */,
  std::vector<TextVertex>& textVertices,
  std::vector<RectVertex>& rectVertices)
{
  const auto& stringVertices = entry.vertices;
  const auto& stringSize = entry.size;

  const auto& offset = entry.offset;

  const auto& textColor = entry.textColor;
  const auto& rectColor = entry.backgroundColor;

  for (size_t i = 0; i < stringVertices.size() / 2; ++i)
  {
    const auto& position2 = stringVertices[2 * i];
    const auto& uvCoords = stringVertices[2 * i + 1];
    textVertices.emplace_back(
      vm::vec3f{position2 + offset.xy(), -offset.z()},
      uvCoords,
      textColor.to<RgbaF>().toVec());
  }

  const auto rect =
    mdl::roundedRect2D(stringSize + 2.0f * m_inset, RectCornerRadius, RectCornerSegments);
  for (size_t i = 0; i < rect.size(); ++i)
  {
    const auto& vertex = rect[i];
    rectVertices.emplace_back(
      vm::vec3f{vertex + offset.xy() + stringSize / 2.0f, -offset.z()},
      rectColor.to<RgbaF>().toVec());
  }
}

void TextRenderer::render(EntryCollection& collection, RenderContext& renderContext)
{
  auto& gl = renderContext.gl();

  auto& fontManager = renderContext.fontManager();
  auto& font = fontManager.font(m_fontDescriptor);

  gl.disable(GL_TEXTURE_2D);

  auto backgroundShader = gl::ActiveShader{
    gl, renderContext.shaderManager(), gl::Shaders::TextBackgroundShader};

  if (collection.rectArray.setup(gl, backgroundShader.program()))
  {
    collection.rectArray.render(gl, gl::PrimType::Triangles);
    collection.rectArray.cleanup(gl, backgroundShader.program());
  }

  gl.enable(GL_TEXTURE_2D);

  auto textShader =
    gl::ActiveShader{gl, renderContext.shaderManager(), gl::Shaders::ColoredTextShader};
  textShader.set("Texture", 0);

  if (collection.textArray.setup(gl, textShader.program()))
  {
    font.activate(gl);
    collection.textArray.render(gl, gl::PrimType::Quads);
    collection.textArray.cleanup(gl, textShader.program());
    font.deactivate(gl);
  }
}

} // namespace tb::render
