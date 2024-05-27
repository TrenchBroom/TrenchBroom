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

#include "MaterialBrowserView.h"

#include <QMenu>
#include <QTextStream>

#include "Assets/Material.h"
#include "Assets/MaterialCollection.h"
#include "Assets/MaterialManager.h"
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

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

#include <string>
#include <vector>

namespace TrenchBroom::View
{

MaterialBrowserView::MaterialBrowserView(
  QScrollBar* scrollBar,
  GLContextManager& contextManager,
  std::weak_ptr<MapDocument> document_)
  : CellView{contextManager, scrollBar}
  , m_document{std::move(document_)}
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection += document->textureUsageCountsDidChangeNotifier.connect(
    this, &MaterialBrowserView::reloadTextures);
}

MaterialBrowserView::~MaterialBrowserView()
{
  clear();
}

void MaterialBrowserView::setSortOrder(const TextureSortOrder sortOrder)
{
  if (sortOrder != m_sortOrder)
  {
    m_sortOrder = sortOrder;
    reloadTextures();
  }
}

void MaterialBrowserView::setGroup(const bool group)
{
  if (group != m_group)
  {
    m_group = group;
    reloadTextures();
  }
}

void MaterialBrowserView::setHideUnused(const bool hideUnused)
{
  if (hideUnused != m_hideUnused)
  {
    m_hideUnused = hideUnused;
    reloadTextures();
  }
}

void MaterialBrowserView::setFilterText(const std::string& filterText)
{
  if (filterText != m_filterText)
  {
    m_filterText = filterText;
    reloadTextures();
  }
}

const Assets::Material* MaterialBrowserView::selectedTexture() const
{
  return m_selectedTexture;
}

void MaterialBrowserView::setSelectedTexture(const Assets::Material* selectedTexture)
{
  if (m_selectedTexture != selectedTexture)
  {
    m_selectedTexture = selectedTexture;
    update();
  }
}

void MaterialBrowserView::revealTexture(const Assets::Material* texture)
{
  scrollToCell([&](const Cell& cell) {
    const auto& cellTexture = cellData(cell);
    return &cellTexture == texture;
  });
}

void MaterialBrowserView::reloadTextures()
{
  invalidate();
  update();
}

void MaterialBrowserView::doInitLayout(Layout& layout)
{
  const auto scaleFactor = pref(Preferences::MaterialBrowserIconSize);

  layout.setOuterMargin(5.0f);
  layout.setGroupMargin(5.0f);
  layout.setRowMargin(15.0f);
  layout.setCellMargin(10.0f);
  layout.setTitleMargin(2.0f);
  layout.setCellWidth(scaleFactor * 64.0f, scaleFactor * 64.0f);
  layout.setCellHeight(scaleFactor * 64.0f, scaleFactor * 128.0f);
}

void MaterialBrowserView::doReloadLayout(Layout& layout)
{
  const auto& fontPath = pref(Preferences::RendererFontPath());
  const auto fontSize = pref(Preferences::BrowserFontSize);
  assert(fontSize > 0);

  const auto font = Renderer::FontDescriptor{fontPath, size_t(fontSize)};

  if (m_group)
  {
    for (const auto* collection : getCollections())
    {
      layout.addGroup(collection->path().string(), float(fontSize) + 2.0f);
      addTexturesToLayout(layout, getTextures(*collection), font);
    }
  }
  else
  {
    addTexturesToLayout(layout, getTextures(), font);
  }
}

void MaterialBrowserView::addTexturesToLayout(
  Layout& layout,
  const std::vector<const Assets::Material*>& textures,
  const Renderer::FontDescriptor& font)
{
  for (const auto* texture : textures)
  {
    addTextureToLayout(layout, texture, font);
  }
}

void MaterialBrowserView::addTextureToLayout(
  Layout& layout, const Assets::Material* texture, const Renderer::FontDescriptor& font)
{
  const auto maxCellWidth = layout.maxCellWidth();

  const auto textureName = std::filesystem::path{texture->name()}.filename().string();
  const auto titleHeight = fontManager().font(font).measure(textureName).y();

  const auto scaleFactor = pref(Preferences::MaterialBrowserIconSize);
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

std::vector<const Assets::MaterialCollection*> MaterialBrowserView::getCollections() const
{
  auto document = kdl::mem_lock(m_document);
  const auto enabledTextureCollections = document->enabledTextureCollections();

  auto result = std::vector<const Assets::MaterialCollection*>{};
  for (const auto& collection : document->textureManager().collections())
  {
    if (kdl::vec_contains(enabledTextureCollections, collection.path()))
    {
      result.push_back(&collection);
    }
  }
  return result;
}

std::vector<const Assets::Material*> MaterialBrowserView::getTextures(
  const Assets::MaterialCollection& collection) const
{
  return sortTextures(filterTextures(
    kdl::vec_transform(collection.textures(), [](const auto& t) { return &t; })));
}

std::vector<const Assets::Material*> MaterialBrowserView::getTextures() const
{
  auto document = kdl::mem_lock(m_document);
  auto textures = std::vector<const Assets::Material*>{};
  for (const auto& collection : getCollections())
  {
    for (const auto& texture : collection->textures())
    {
      textures.push_back(&texture);
    }
  }
  return sortTextures(filterTextures(textures));
}

std::vector<const Assets::Material*> MaterialBrowserView::filterTextures(
  std::vector<const Assets::Material*> textures) const
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

std::vector<const Assets::Material*> MaterialBrowserView::sortTextures(
  std::vector<const Assets::Material*> textures) const
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

void MaterialBrowserView::doClear() {}

void MaterialBrowserView::doRender(Layout& layout, const float y, const float height)
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

bool MaterialBrowserView::doShouldRenderFocusIndicator() const
{
  return false;
}

const Color& MaterialBrowserView::getBackgroundColor()
{
  return pref(Preferences::BrowserBackgroundColor);
}

void MaterialBrowserView::renderBounds(Layout& layout, const float y, const float height)
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
    shaderManager(), Renderer::Shaders::MaterialBrowserBorderShader};

  vertexArray.prepare(vboManager());
  vertexArray.render(Renderer::PrimType::Quads);
}

const Color& MaterialBrowserView::textureColor(const Assets::Material& texture) const
{
  if (&texture == m_selectedTexture)
  {
    return pref(Preferences::MaterialBrowserSelectedColor);
  }
  if (texture.usageCount() > 0)
  {
    return pref(Preferences::MaterialBrowserUsedColor);
  }
  return pref(Preferences::MaterialBrowserDefaultColor);
}

void MaterialBrowserView::renderTextures(
  Layout& layout, const float y, const float height)
{
  using TextureVertex = Renderer::GLVertexTypes::P2T2::Vertex;

  auto shader =
    Renderer::ActiveShader{shaderManager(), Renderer::Shaders::MaterialBrowserShader};
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

void MaterialBrowserView::doLeftClick(Layout& layout, const float x, const float y)
{
  if (const auto* cell = layout.cellAt(x, y))
  {
    const auto& texture = cellData(*cell);
    // NOTE: wx had the ability for the textureSelected event to veto the selection, but
    // it wasn't used.
    setSelectedTexture(&texture);

    emit textureSelected(&texture);

    update();
  }
}

QString MaterialBrowserView::tooltip(const Cell& cell)
{
  auto tooltip = QString{};
  auto ss = QTextStream{&tooltip};
  ss << QString::fromStdString(cellData(cell).name()) << "\n";
  ss << cellData(cell).width() << "x" << cellData(cell).height();
  return tooltip;
}

void MaterialBrowserView::doContextMenu(
  Layout& layout, float x, float y, QContextMenuEvent* event)
{
  if (const auto* cell = layout.cellAt(x, y))
  {
    const auto& texture = cellData(*cell);
    auto menu = QMenu{this};
    menu.addAction(tr("Select Faces"), this, [&, texture = &texture]() {
      auto doc = kdl::mem_lock(m_document);
      doc->selectFacesWithTexture(texture);
    });
    menu.exec(event->globalPos());
  }
}

const Assets::Material& MaterialBrowserView::cellData(const Cell& cell) const
{
  return *cell.itemAs<const Assets::Material*>();
}

} // namespace TrenchBroom::View
