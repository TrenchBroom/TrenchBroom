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

#include "TextRenderer.h"

#include "AttrString.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/Camera.h"
#include "Renderer/FontManager.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "Renderer/TextAnchor.h"
#include "Renderer/TextureFont.h"

#include "vm/forward.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

namespace TrenchBroom
{
namespace Renderer
{
const float TextRenderer::DefaultMaxViewDistance = 768.0f;
const float TextRenderer::DefaultMinZoomFactor = 0.5f;
const vm::vec2f TextRenderer::DefaultInset = vm::vec2f(4.0f, 4.0f);
const size_t TextRenderer::RectCornerSegments = 3;
const float TextRenderer::RectCornerRadius = 3.0f;

TextRenderer::Entry::Entry(
  std::vector<vm::vec2f>& i_vertices,
  const vm::vec2f& i_size,
  const vm::vec3f& i_offset,
  const Color& i_textColor,
  const Color& i_backgroundColor)
  : size(i_size)
  , offset(i_offset)
  , textColor(i_textColor)
  , backgroundColor(i_backgroundColor)
{
  using std::swap;
  swap(vertices, i_vertices);
}

TextRenderer::EntryCollection::EntryCollection()
  : textVertexCount(0)
  , rectVertexCount(0)
{
}

TextRenderer::TextRenderer(
  const FontDescriptor& fontDescriptor,
  const float maxViewDistance,
  const float minZoomFactor,
  const vm::vec2f& inset)
  : m_fontDescriptor(fontDescriptor)
  , m_maxViewDistance(maxViewDistance)
  , m_minZoomFactor(minZoomFactor)
  , m_inset(inset)
{
}

void TextRenderer::renderString(
  RenderContext& renderContext,
  const Color& textColor,
  const Color& backgroundColor,
  const AttrString& string,
  const TextAnchor& position)
{
  renderString(renderContext, textColor, backgroundColor, string, position, false);
}

void TextRenderer::renderStringOnTop(
  RenderContext& renderContext,
  const Color& textColor,
  const Color& backgroundColor,
  const AttrString& string,
  const TextAnchor& position)
{
  renderString(renderContext, textColor, backgroundColor, string, position, true);
}

void TextRenderer::renderString(
  RenderContext& renderContext,
  const Color& textColor,
  const Color& backgroundColor,
  const AttrString& string,
  const TextAnchor& position,
  const bool onTop)
{

  const Camera& camera = renderContext.camera();
  const float distance = camera.perpendicularDistanceTo(position.position(camera));
  if (distance <= 0.0f)
    return;

  if (!isVisible(renderContext, string, position, distance, onTop))
    return;

  FontManager& fontManager = renderContext.fontManager();
  TextureFont& font = fontManager.font(m_fontDescriptor);

  std::vector<vm::vec2f> vertices = font.quads(string, true);
  const float alphaFactor = computeAlphaFactor(renderContext, distance, onTop);
  const vm::vec2f size = font.measure(string);
  const vm::vec3f offset = position.offset(camera, size);

  if (onTop)
    addEntry(
      m_entriesOnTop,
      Entry(
        vertices,
        size,
        offset,
        Color(textColor, alphaFactor * textColor.a()),
        Color(backgroundColor, alphaFactor * backgroundColor.a())));
  else
    addEntry(
      m_entries,
      Entry(
        vertices,
        size,
        offset,
        Color(textColor, alphaFactor * textColor.a()),
        Color(backgroundColor, alphaFactor * backgroundColor.a())));
}

bool TextRenderer::isVisible(
  RenderContext& renderContext,
  const AttrString& string,
  const TextAnchor& position,
  const float distance,
  const bool onTop) const
{
  if (!onTop)
  {
    if (renderContext.render3D() && distance > m_maxViewDistance)
      return false;
    if (renderContext.render2D() && renderContext.camera().zoom() < m_minZoomFactor)
      return false;
  }

  const Camera& camera = renderContext.camera();
  const Camera::Viewport& viewport = camera.viewport();

  const vm::vec2f size = stringSize(renderContext, string);
  const vm::vec2f offset = vm::vec2f(position.offset(camera, size)) - m_inset;
  const vm::vec2f actualSize = size + 2.0f * m_inset;

  return viewport.contains(offset.x(), offset.y(), actualSize.x(), actualSize.y());
}

float TextRenderer::computeAlphaFactor(
  const RenderContext& renderContext, const float distance, const bool onTop) const
{
  if (onTop)
    return 1.0f;

  if (renderContext.render3D())
  {
    const float a = m_maxViewDistance - distance;
    if (a > 128.0f)
      return 1.0f;
    return a / 128.0f;
  }
  else
  {
    const float z = renderContext.camera().zoom();
    const float d = z - m_minZoomFactor;
    if (d > 0.3f)
      return 1.0f;
    return d / 0.3f;
  }
}

void TextRenderer::addEntry(EntryCollection& collection, const Entry& entry)
{
  collection.entries.push_back(entry);
  collection.textVertexCount += entry.vertices.size();
  collection.rectVertexCount += roundedRect2DVertexCount(RectCornerSegments);
}

vm::vec2f TextRenderer::stringSize(
  RenderContext& renderContext, const AttrString& string) const
{
  FontManager& fontManager = renderContext.fontManager();
  TextureFont& font = fontManager.font(m_fontDescriptor);
  return round(font.measure(string));
}

void TextRenderer::doPrepareVertices(VboManager& vboManager)
{
  prepare(m_entries, false, vboManager);
  prepare(m_entriesOnTop, true, vboManager);
}

void TextRenderer::prepare(
  EntryCollection& collection, const bool onTop, VboManager& vboManager)
{
  std::vector<TextVertex> textVertices;
  textVertices.reserve(collection.textVertexCount);

  std::vector<RectVertex> rectVertices;
  rectVertices.reserve(collection.rectVertexCount);

  for (const Entry& entry : collection.entries)
  {
    addEntry(entry, onTop, textVertices, rectVertices);
  }

  collection.textArray = VertexArray::move(std::move(textVertices));
  collection.rectArray = VertexArray::move(std::move(rectVertices));

  collection.textArray.prepare(vboManager);
  collection.rectArray.prepare(vboManager);
}

void TextRenderer::addEntry(
  const Entry& entry,
  const bool /* onTop */,
  std::vector<TextVertex>& textVertices,
  std::vector<RectVertex>& rectVertices)
{
  const std::vector<vm::vec2f>& stringVertices = entry.vertices;
  const vm::vec2f& stringSize = entry.size;

  const vm::vec3f& offset = entry.offset;

  const Color& textColor = entry.textColor;
  const Color& rectColor = entry.backgroundColor;

  for (size_t i = 0; i < stringVertices.size() / 2; ++i)
  {
    const vm::vec2f& position2 = stringVertices[2 * i];
    const vm::vec2f& texCoords = stringVertices[2 * i + 1];
    textVertices.emplace_back(
      vm::vec3f(position2 + offset.xy(), -offset.z()), texCoords, textColor);
  }

  const std::vector<vm::vec2f> rect =
    roundedRect2D(stringSize + 2.0f * m_inset, RectCornerRadius, RectCornerSegments);
  for (size_t i = 0; i < rect.size(); ++i)
  {
    const vm::vec2f& vertex = rect[i];
    rectVertices.emplace_back(
      vm::vec3f(vertex + offset.xy() + stringSize / 2.0f, -offset.z()), rectColor);
  }
}

void TextRenderer::doRender(RenderContext& renderContext)
{
  const Camera::Viewport& viewport = renderContext.camera().viewport();
  const vm::mat4x4f projection = vm::ortho_matrix(
    0.0f,
    1.0f,
    static_cast<float>(viewport.x),
    static_cast<float>(viewport.height),
    static_cast<float>(viewport.width),
    static_cast<float>(viewport.y));
  const vm::mat4x4f view = vm::view_matrix(vm::vec3f::neg_z(), vm::vec3f::pos_y());
  ReplaceTransformation ortho(renderContext.transformation(), projection, view);

  render(m_entries, renderContext);

  glAssert(glDisable(GL_DEPTH_TEST));
  render(m_entriesOnTop, renderContext);
  glAssert(glEnable(GL_DEPTH_TEST));
}

void TextRenderer::render(EntryCollection& collection, RenderContext& renderContext)
{
  FontManager& fontManager = renderContext.fontManager();
  TextureFont& font = fontManager.font(m_fontDescriptor);

  glAssert(glDisable(GL_TEXTURE_2D));

  ActiveShader backgroundShader(
    renderContext.shaderManager(), Shaders::TextBackgroundShader);
  collection.rectArray.render(PrimType::Triangles);

  glAssert(glEnable(GL_TEXTURE_2D));

  ActiveShader textShader(renderContext.shaderManager(), Shaders::ColoredTextShader);
  textShader.set("Texture", 0);
  font.activate();
  collection.textArray.render(PrimType::Quads);
  font.deactivate();
}
} // namespace Renderer
} // namespace TrenchBroom
