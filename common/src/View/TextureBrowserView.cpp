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

#include "kdl/memory_utils.h"
#include "kdl/skip_iterator.h"
#include "kdl/string_compare.h"
#include "kdl/string_utils.h"
#include "kdl/vector_utils.h"

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
    const auto& cellTexture = cellData(cell);
    return &cellTexture == texture;
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

  const auto font = Renderer::FontDescriptor{fontPath, size_t(fontSize)};

  if (m_group)
  {
    for (const auto* collection : getCollections())
    {
      layout.addGroup(collection->path().u8string(), float(fontSize) + 2.0f);
      addTexturesToLayout(layout, getTextures(*collection), font);
    }
  }
  else
  {
    addTexturesToLayout(layout, getTextures(), font);
  }
}

void TextureBrowserView::addTexturesToLayout(
  Layout& layout,
  const std::vector<const Assets::Texture*>& textures,
  const Renderer::FontDescriptor& font)
{
  for (const auto* texture : textures)
  {
    addTextureToLayout(layout, texture, font);
  }
}

void TextureBrowserView::addTextureToLayout(
  Layout& layout, const Assets::Texture* texture, const Renderer::FontDescriptor& font)
{
  const auto maxCellWidth = layout.maxCellWidth();

  const auto textureName = std::filesystem::path{texture->name()}.filename().string();
  const auto titleHeight = fontManager().font(font).measure(textureName).y();

  const auto scaleFactor = pref(Preferences::TextureBrowserIconSize);
  const auto scaledTextureWidth = vm::round(scaleFactor * float(texture->width()));
  const auto scaledTextureHeight = vm::round(scaleFactor * float(texture->height()));

  layout.addItem(
    texture,
    textureName,
    scaledTextureWidth,
    scaledTextureHeight,
    maxCellWidth,
    titleHeight + 4.0f);
}

std::vector<const Assets::TextureCollection*> TextureBrowserView::getCollections() const
{
  auto document = kdl::mem_lock(m_document);
  const auto enabledTextureCollections = document->enabledTextureCollections();

  auto result = std::vector<const Assets::TextureCollection*>{};
  for (const auto& collection : document->textureManager().collections())
  {
    if (kdl::vec_contains(enabledTextureCollections, collection.path()))
    {
      result.push_back(&collection);
    }
  }
  return result;
}

std::vector<const Assets::Texture*> TextureBrowserView::getTextures(
  const Assets::TextureCollection& collection) const
{
  return sortTextures(filterTextures(
    kdl::vec_transform(collection.textures(), [](const auto& t) { return &t; })));
}

std::vector<const Assets::Texture*> TextureBrowserView::getTextures() const
{
  auto document = kdl::mem_lock(m_document);
  auto textures = std::vector<const Assets::Texture*>{};
  for (const auto& collection : getCollections())
  {
    for (const auto& texture : collection->textures())
    {
      if (!texture.overridden())
      {
        textures.push_back(&texture);
      }
    }
  }
  return sortTextures(filterTextures(textures));
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

  const auto viewLeft = float(0);
  const auto viewTop = float(size().height());
  const auto viewRight = float(size().width());
  const auto viewBottom = float(0);

  const auto transformation = Renderer::Transformation{
    vm::ortho_matrix(-1.0f, 1.0f, viewLeft, viewTop, viewRight, viewBottom),
    vm::view_matrix(vm::vec3f::neg_z(), vm::vec3f::pos_y())
      * vm::translation_matrix(vm::vec3f{0.0f, 0.0f, 0.1f})};

  renderBounds(layout, y, height);
  renderTextures(layout, y, height);
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
            const auto& texture = cellData(cell);
            const auto& color = textureColor(texture);
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
            const auto& texture = cellData(cell);

            auto vertexArray = Renderer::VertexArray::move(std::vector<TextureVertex>{
              TextureVertex{{bounds.left(), height - (bounds.top() - y)}, {0, 0}},
              TextureVertex{{bounds.left(), height - (bounds.bottom() - y)}, {0, 1}},
              TextureVertex{{bounds.right(), height - (bounds.bottom() - y)}, {1, 1}},
              TextureVertex{{bounds.right(), height - (bounds.top() - y)}, {1, 0}},
            });

            shader.set("GrayScale", texture.overridden());
            texture.activate();

            vertexArray.prepare(vboManager());
            vertexArray.render(Renderer::PrimType::Quads);

            texture.deactivate();
          }
        }
      }
    }
  }
}

void TextureBrowserView::doLeftClick(Layout& layout, const float x, const float y)
{
  if (const auto* cell = layout.cellAt(x, y))
  {
    const auto& texture = cellData(*cell);
    if (!texture.overridden())
    {
      // NOTE: wx had the ability for the textureSelected event to veto the selection, but
      // it wasn't used.
      setSelectedTexture(&texture);

      emit textureSelected(&texture);

      update();
    }
  }
}

QString TextureBrowserView::tooltip(const Cell& cell)
{
  auto tooltip = QString{};
  auto ss = QTextStream{&tooltip};
  ss << QString::fromStdString(cellData(cell).name()) << "\n";
  ss << cellData(cell).width() << "x" << cellData(cell).height();
  return tooltip;
}

void TextureBrowserView::doContextMenu(
  Layout& layout, float x, float y, QContextMenuEvent* event)
{
  if (const auto* cell = layout.cellAt(x, y))
  {
    const auto& texture = cellData(*cell);
    if (!cellData(*cell).overridden())
    {

      auto menu = QMenu{this};
      menu.addAction(tr("Select Faces"), this, [&, texture = &texture]() {
        auto doc = kdl::mem_lock(m_document);
        doc->selectFacesWithTexture(texture);
      });
      menu.exec(event->globalPos());
    }
  }
}

const Assets::Texture& TextureBrowserView::cellData(const Cell& cell) const
{
  return *cell.itemAs<const Assets::Texture*>();
}

} // namespace TrenchBroom::View
