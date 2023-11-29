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

#include "TextureBrowserView.h"

#include <QMenu>
#include <QTextStream>

#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureManager.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/FontManager.h"
#include "Renderer/GL.h"
#include "Renderer/PrimType.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "Renderer/TextureFont.h"
#include "Renderer/Transformation.h"
#include "Renderer/VertexArray.h"
#include "View/MapDocument.h"

#include "kdl/string_utils.h"
#include <kdl/memory_utils.h>
#include <kdl/skip_iterator.h>
#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/vec.h>

#include <string>
#include <vector>

namespace TrenchBroom::View
{

TextureBrowserView::TextureBrowserView(
  QScrollBar* scrollBar,
  GLContextManager& contextManager,
  std::weak_ptr<MapDocument> document_)
  : CellView{contextManager, scrollBar}
  , m_document{std::move(document_)}
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection += document->textureUsageCountsDidChangeNotifier.connect(
    this, &TextureBrowserView::usageCountDidChange);
}

TextureBrowserView::~TextureBrowserView()
{
  clear();
}

void TextureBrowserView::setSortOrder(const TextureSortOrder sortOrder)
{
  if (sortOrder != m_sortOrder)
  {
    m_sortOrder = sortOrder;
    invalidate();
    update();
  }
}

void TextureBrowserView::setGroup(const bool group)
{
  if (group != m_group)
  {
    m_group = group;
    invalidate();
    update();
  }
}

void TextureBrowserView::setHideUnused(const bool hideUnused)
{
  if (hideUnused != m_hideUnused)
  {
    m_hideUnused = hideUnused;
    invalidate();
    update();
  }
}

void TextureBrowserView::setFilterText(const std::string& filterText)
{
  if (filterText != m_filterText)
  {
    m_filterText = filterText;
    invalidate();
    update();
  }
}

const Assets::Texture* TextureBrowserView::selectedTexture() const
{
  return m_selectedTexture;
}

void TextureBrowserView::setSelectedTexture(const Assets::Texture* selectedTexture)
{
  if (m_selectedTexture != selectedTexture)
  {
    m_selectedTexture = selectedTexture;
    update();
  }
}

void TextureBrowserView::revealTexture(const Assets::Texture* texture)
{
  scrollToCell([=](const Cell& cell) {
    const auto* cellTexture = cellData(cell).texture;
    return cellTexture == texture;
  });
}

void TextureBrowserView::usageCountDidChange()
{
  invalidate();
  update();
}

void TextureBrowserView::doInitLayout(Layout& layout)
{
  const auto scaleFactor = pref(Preferences::TextureBrowserIconSize);

  layout.setOuterMargin(5.0f);
  layout.setGroupMargin(5.0f);
  layout.setRowMargin(15.0f);
  layout.setCellMargin(10.0f);
  layout.setTitleMargin(2.0f);
  layout.setCellWidth(scaleFactor * 64.0f, scaleFactor * 64.0f);
  layout.setCellHeight(scaleFactor * 64.0f, scaleFactor * 128.0f);
}

void TextureBrowserView::doReloadLayout(Layout& layout)
{
  const auto& fontPath = pref(Preferences::RendererFontPath());
  const auto fontSize = pref(Preferences::BrowserFontSize);
  assert(fontSize > 0);

  const auto font = Renderer::FontDescriptor{fontPath, static_cast<size_t>(fontSize)};

  if (m_group)
  {
    for (const auto& collection : getCollections())
    {
      layout.addGroup(collection.name(), static_cast<float>(fontSize) + 2.0f);
      addTexturesToLayout(layout, getTextures(collection), collection.name(), font);
    }
  }
  else
  {
    addTexturesToLayout(layout, getTextures(), "", font);
  }
}

void TextureBrowserView::addTexturesToLayout(
  Layout& layout,
  const std::vector<const Assets::Texture*>& textures,
  const std::string& groupName,
  const Renderer::FontDescriptor& font)
{
  for (const auto* texture : textures)
  {
    addTextureToLayout(layout, texture, groupName, font);
  }
}

void TextureBrowserView::addTextureToLayout(
  Layout& layout,
  const Assets::Texture* texture,
  const std::string& groupName,
  const Renderer::FontDescriptor& font)
{
  const auto maxCellWidth = layout.maxCellWidth();

  const auto textureName = std::filesystem::path{texture->name()}.filename().string();

  const auto textureFont =
    fontManager().selectFontSize(font, textureName, maxCellWidth, 6);
  const auto groupFont = fontManager().selectFontSize(font, groupName, maxCellWidth, 6);

  const auto defaultTextHeight =
    fontManager().font(font).measure(groupName + textureName).y();
  const auto textureNameSize = fontManager().font(textureFont).measure(textureName);
  const auto groupNameSize = fontManager().font(groupFont).measure(groupName);

  const auto totalSize = vm::vec2f(
    vm::max(groupNameSize.x(), textureNameSize.x()), 2.0f * defaultTextHeight + 4.0f);

  const auto scaleFactor = pref(Preferences::TextureBrowserIconSize);
  const auto scaledTextureWidth =
    vm::round(scaleFactor * static_cast<float>(texture->width()));
  const auto scaledTextureHeight =
    vm::round(scaleFactor * static_cast<float>(texture->height()));

  auto cellData = TextureCellData{
    texture,
    textureName,
    groupName,
    vm::vec2f{(maxCellWidth - textureNameSize.x()) / 2.0f, defaultTextHeight + 3.0f},
    vm::vec2f{(maxCellWidth - groupNameSize.x()) / 2.0f, 1.0f},
    textureFont,
    groupFont};

  layout.addItem(
    std::move(cellData),
    scaledTextureWidth,
    scaledTextureHeight,
    maxCellWidth,
    totalSize.y());
}

const std::vector<Assets::TextureCollection>& TextureBrowserView::getCollections() const
{
  auto document = kdl::mem_lock(m_document);
  return document->textureManager().collections();
}

std::vector<const Assets::Texture*> TextureBrowserView::getTextures(
  const Assets::TextureCollection& collection) const
{
  return sortTextures(filterTextures(
    kdl::vec_transform(collection.textures(), [](const auto& t) { return &t; })));
}

std::vector<const Assets::Texture*> TextureBrowserView::getTextures() const
{
  auto doc = kdl::mem_lock(m_document);
  return sortTextures(filterTextures(doc->textureManager().textures()));
}

std::vector<const Assets::Texture*> TextureBrowserView::filterTextures(
  std::vector<const Assets::Texture*> textures) const
{
  if (m_hideUnused)
  {
    textures = kdl::vec_erase_if(std::move(textures), [](const auto* texture) {
      return texture->usageCount() == 0;
    });
  }
  if (!m_filterText.empty())
  {
    textures = kdl::vec_erase_if(std::move(textures), [&](const auto* texture) {
      return !kdl::all_of(kdl::str_split(m_filterText, " "), [&](const auto& pattern) {
        return kdl::ci::str_contains(texture->name(), pattern);
      });
    });
  }
  return textures;
}

std::vector<const Assets::Texture*> TextureBrowserView::sortTextures(
  std::vector<const Assets::Texture*> textures) const
{
  const auto compareNames = [](const auto& lhs, const auto& rhs) {
    return kdl::ci::string_less{}(lhs->name(), rhs->name());
  };

  switch (m_sortOrder)
  {
  case TextureSortOrder::Name:
    return kdl::vec_sort(std::move(textures), compareNames);
  case TextureSortOrder::Usage:
    return kdl::vec_sort(std::move(textures), [&](const auto* lhs, const auto* rhs) {
      return lhs->usageCount() < rhs->usageCount()   ? false
             : lhs->usageCount() > rhs->usageCount() ? true
                                                     : compareNames(lhs, rhs);
    });
    switchDefault();
  }
}

void TextureBrowserView::doClear() {}

void TextureBrowserView::doRender(Layout& layout, const float y, const float height)
{
  auto document = kdl::mem_lock(m_document);
  document->textureManager().commitChanges();

  const auto viewLeft = static_cast<float>(0);
  const auto viewTop = static_cast<float>(size().height());
  const auto viewRight = static_cast<float>(size().width());
  const auto viewBottom = static_cast<float>(0);

  const auto transformation = Renderer::Transformation{
    vm::ortho_matrix(-1.0f, 1.0f, viewLeft, viewTop, viewRight, viewBottom),
    vm::view_matrix(vm::vec3f::neg_z(), vm::vec3f::pos_y())
      * vm::translation_matrix(vm::vec3f{0.0f, 0.0f, 0.1f})};

  glAssert(glDisable(GL_DEPTH_TEST));
  glAssert(glFrontFace(GL_CCW));

  renderBounds(layout, y, height);
  renderTextures(layout, y, height);
  renderNames(layout, y, height);
}

bool TextureBrowserView::doShouldRenderFocusIndicator() const
{
  return false;
}

const Color& TextureBrowserView::getBackgroundColor()
{
  return pref(Preferences::BrowserBackgroundColor);
}

void TextureBrowserView::renderBounds(Layout& layout, const float y, const float height)
{
  using BoundsVertex = Renderer::GLVertexTypes::P2C4::Vertex;
  auto vertices = std::vector<BoundsVertex>{};

  for (const auto& group : layout.groups())
  {
    if (group.intersectsY(y, height))
    {
      for (const auto& row : group.rows())
      {
        if (row.intersectsY(y, height))
        {
          for (const auto& cell : row.cells())
          {
            const auto& bounds = cell.itemBounds();
            const auto* texture = cellData(cell).texture;
            const auto& color = textureColor(*texture);
            vertices.emplace_back(
              vm::vec2f{bounds.left() - 2.0f, height - (bounds.top() - 2.0f - y)}, color);
            vertices.emplace_back(
              vm::vec2f{bounds.left() - 2.0f, height - (bounds.bottom() + 2.0f - y)},
              color);
            vertices.emplace_back(
              vm::vec2f{bounds.right() + 2.0f, height - (bounds.bottom() + 2.0f - y)},
              color);
            vertices.emplace_back(
              vm::vec2f{bounds.right() + 2.0f, height - (bounds.top() - 2.0f - y)},
              color);
          }
        }
      }
    }
  }

  auto vertexArray = Renderer::VertexArray::move(std::move(vertices));
  auto shader = Renderer::ActiveShader{
    shaderManager(), Renderer::Shaders::TextureBrowserBorderShader};

  vertexArray.prepare(vboManager());
  vertexArray.render(Renderer::PrimType::Quads);
}

const Color& TextureBrowserView::textureColor(const Assets::Texture& texture) const
{
  if (&texture == m_selectedTexture)
  {
    return pref(Preferences::TextureBrowserSelectedColor);
  }
  if (texture.usageCount() > 0)
  {
    return pref(Preferences::TextureBrowserUsedColor);
  }
  return pref(Preferences::TextureBrowserDefaultColor);
}

void TextureBrowserView::renderTextures(Layout& layout, const float y, const float height)
{
  using TextureVertex = Renderer::GLVertexTypes::P2T2::Vertex;

  auto shader =
    Renderer::ActiveShader{shaderManager(), Renderer::Shaders::TextureBrowserShader};
  shader.set("ApplyTinting", false);
  shader.set("Texture", 0);
  shader.set("Brightness", pref(Preferences::Brightness));

  for (const auto& group : layout.groups())
  {
    if (group.intersectsY(y, height))
    {
      for (const auto& row : group.rows())
      {
        if (row.intersectsY(y, height))
        {
          for (const auto& cell : row.cells())
          {
            const auto& bounds = cell.itemBounds();
            const auto* texture = cellData(cell).texture;

            auto vertexArray = Renderer::VertexArray::move(std::vector<TextureVertex>{
              TextureVertex{{bounds.left(), height - (bounds.top() - y)}, {0, 0}},
              TextureVertex{{bounds.left(), height - (bounds.bottom() - y)}, {0, 1}},
              TextureVertex{{bounds.right(), height - (bounds.bottom() - y)}, {1, 1}},
              TextureVertex{{bounds.right(), height - (bounds.top() - y)}, {1, 0}},
            });

            shader.set("GrayScale", texture->overridden());
            texture->activate();

            vertexArray.prepare(vboManager());
            vertexArray.render(Renderer::PrimType::Quads);

            texture->deactivate();
          }
        }
      }
    }
  }
}

void TextureBrowserView::renderNames(Layout& layout, const float y, const float height)
{
  renderGroupTitleBackgrounds(layout, y, height);
  renderStrings(layout, y, height);
}

void TextureBrowserView::renderGroupTitleBackgrounds(
  Layout& layout, const float y, const float height)
{
  using Vertex = Renderer::GLVertexTypes::P2::Vertex;
  auto vertices = std::vector<Vertex>{};

  for (const auto& group : layout.groups())
  {
    if (group.intersectsY(y, height))
    {
      const auto titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
      vertices.emplace_back(
        vm::vec2f{titleBounds.left(), height - (titleBounds.top() - y)});
      vertices.emplace_back(
        vm::vec2f{titleBounds.left(), height - (titleBounds.bottom() - y)});
      vertices.emplace_back(
        vm::vec2f{titleBounds.right(), height - (titleBounds.bottom() - y)});
      vertices.emplace_back(
        vm::vec2f{titleBounds.right(), height - (titleBounds.top() - y)});
    }
  }

  auto shader =
    Renderer::ActiveShader{shaderManager(), Renderer::Shaders::VaryingPUniformCShader};
  shader.set("Color", pref(Preferences::BrowserGroupBackgroundColor));

  auto vertexArray = Renderer::VertexArray::move(std::move(vertices));
  vertexArray.prepare(vboManager());
  vertexArray.render(Renderer::PrimType::Quads);
}

void TextureBrowserView::renderStrings(Layout& layout, const float y, const float height)
{
  using StringRendererMap = std::map<Renderer::FontDescriptor, Renderer::VertexArray>;
  auto stringRenderers = StringRendererMap{};

  for (const auto& [descriptor, vertices] : collectStringVertices(layout, y, height))
  {
    stringRenderers[descriptor] = Renderer::VertexArray::ref(vertices);
    stringRenderers[descriptor].prepare(vboManager());
  }

  auto shader =
    Renderer::ActiveShader{shaderManager(), Renderer::Shaders::ColoredTextShader};
  shader.set("Texture", 0);

  for (auto& [descriptor, vertexArray] : stringRenderers)
  {
    auto& font = fontManager().font(descriptor);
    font.activate();
    vertexArray.render(Renderer::PrimType::Quads);
    font.deactivate();
  }
}

TextureBrowserView::StringMap TextureBrowserView::collectStringVertices(
  Layout& layout, const float y, const float height)
{
  auto defaultDescriptor = Renderer::FontDescriptor{
    pref(Preferences::RendererFontPath()),
    static_cast<size_t>(pref(Preferences::BrowserFontSize))};

  const auto textColor = std::vector<Color>{pref(Preferences::BrowserTextColor)};
  const auto subTextColor = std::vector<Color>{pref(Preferences::BrowserSubTextColor)};

  auto stringVertices = StringMap{};
  for (const auto& group : layout.groups())
  {
    if (group.intersectsY(y, height))
    {
      const auto& title = group.item();
      if (!title.empty())
      {
        const auto titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
        const auto offset = vm::vec2f(
          titleBounds.left() + 2.0f,
          height - (titleBounds.top() - y) - titleBounds.height);

        auto& font = fontManager().font(defaultDescriptor);
        const auto quads = font.quads(title, false, offset);
        const auto titleVertices = TextVertex::toList(
          quads.size() / 2,
          kdl::skip_iterator{std::begin(quads), std::end(quads), 0, 2},
          kdl::skip_iterator{std::begin(quads), std::end(quads), 1, 2},
          kdl::skip_iterator{std::begin(textColor), std::end(textColor), 0, 0});
        auto& vertices = stringVertices[defaultDescriptor];
        vertices.insert(
          std::end(vertices), std::begin(titleVertices), std::end(titleVertices));
      }

      for (const auto& row : group.rows())
      {
        if (row.intersectsY(y, height))
        {
          for (const auto& cell : row.cells())
          {
            const auto titleBounds = cell.titleBounds();
            const auto& textureFont = fontManager().font(cellData(cell).mainTitleFont);
            const auto& groupFont = fontManager().font(cellData(cell).subTitleFont);

            // y is relative to top, but OpenGL coords are relative to bottom, so invert
            const auto titleOffset =
              vm::vec2f(titleBounds.left(), y + height - titleBounds.bottom());

            const auto textureNameOffset = titleOffset + cellData(cell).mainTitleOffset;
            const auto groupNameOffset = titleOffset + cellData(cell).subTitleOffset;

            const auto& textureName = cellData(cell).mainTitle;
            const auto& groupName = cellData(cell).subTitle;

            const auto textureNameQuads =
              textureFont.quads(textureName, false, textureNameOffset);
            const auto groupNameQuads =
              groupFont.quads(groupName, false, groupNameOffset);

            const auto textureNameVertices = TextVertex::toList(
              textureNameQuads.size() / 2,
              kdl::skip_iterator{
                std::begin(textureNameQuads), std::end(textureNameQuads), 0, 2},
              kdl::skip_iterator{
                std::begin(textureNameQuads), std::end(textureNameQuads), 1, 2},
              kdl::skip_iterator{std::begin(textColor), std::end(textColor), 0, 0});

            const auto groupNameVertices = TextVertex::toList(
              groupNameQuads.size() / 2,
              kdl::skip_iterator{
                std::begin(groupNameQuads), std::end(groupNameQuads), 0, 2},
              kdl::skip_iterator{
                std::begin(groupNameQuads), std::end(groupNameQuads), 1, 2},
              kdl::skip_iterator{std::begin(subTextColor), std::end(subTextColor), 0, 0});

            auto& mainTitleVertices = stringVertices[cellData(cell).mainTitleFont];
            mainTitleVertices =
              kdl::vec_concat(std::move(mainTitleVertices), textureNameVertices);

            auto& subTitleVertices = stringVertices[cellData(cell).subTitleFont];
            subTitleVertices =
              kdl::vec_concat(std::move(subTitleVertices), groupNameVertices);
          }
        }
      }
    }
  }

  return stringVertices;
}

void TextureBrowserView::doLeftClick(Layout& layout, const float x, const float y)
{
  if (const auto* cell = layout.cellAt(x, y))
  {
    if (!cellData(*cell).texture->overridden())
    {
      auto* texture = cellData(*cell).texture;

      // NOTE: wx had the ability for the textureSelected event to veto the selection, but
      // it wasn't used.
      setSelectedTexture(texture);

      emit textureSelected(texture);

      update();
    }
  }
}

QString TextureBrowserView::tooltip(const Cell& cell)
{
  auto tooltip = QString{};
  auto ss = QTextStream{&tooltip};
  ss << QString::fromStdString(cellData(cell).texture->name()) << "\n";
  ss << cellData(cell).texture->width() << "x" << cellData(cell).texture->height();
  return tooltip;
}

void TextureBrowserView::doContextMenu(
  Layout& layout, float x, float y, QContextMenuEvent* event)
{
  if (const auto* cell = layout.cellAt(x, y))
  {
    if (!cellData(*cell).texture->overridden())
    {
      auto* texture = cellData(*cell).texture;

      auto menu = QMenu{this};
      menu.addAction(tr("Select Faces"), this, [=]() {
        auto doc = kdl::mem_lock(m_document);
        doc->selectFacesWithTexture(texture);
      });
      menu.exec(event->globalPos());
    }
  }
}

const TextureCellData& TextureBrowserView::cellData(const Cell& cell) const
{
  return cell.itemAs<TextureCellData>();
}

} // namespace TrenchBroom::View
