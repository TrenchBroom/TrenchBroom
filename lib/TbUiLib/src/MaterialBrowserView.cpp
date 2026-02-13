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

#include "ui/MaterialBrowserView.h"

#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QTextStream>

#include "PreferenceManager.h"
#include "Preferences.h"
#include "gl/ActiveShader.h"
#include "gl/FontManager.h"
#include "gl/Material.h"
#include "gl/MaterialCollection.h"
#include "gl/MaterialManager.h"
#include "gl/PrimType.h"
#include "gl/Shaders.h"
#include "gl/Texture.h"
#include "gl/TextureFont.h"
#include "gl/VertexArray.h"
#include "gl/VertexType.h"
#include "mdl/Map.h"
#include "mdl/Map_Assets.h"
#include "mdl/Map_Selection.h"
#include "render/Transformation.h"
#include "ui/MapDocument.h"

#include "kd/contracts.h"
#include "kd/ranges/to.h"
#include "kd/string_compare.h"
#include "kd/string_utils.h"
#include "kd/vector_utils.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

#include <ranges>
#include <string>
#include <vector>

namespace tb::ui
{

MaterialBrowserView::MaterialBrowserView(
  QScrollBar* scrollBar, gl::ContextManager& contextManager, MapDocument& document)
  : CellView{contextManager, scrollBar}
  , m_document{document}
{
  m_notifierConnection += m_document.materialUsageCountsDidChangeNotifier.connect(
    this, &MaterialBrowserView::reloadMaterials);
  m_notifierConnection += m_document.resourcesWereProcessedNotifier.connect(
    this, &MaterialBrowserView::resourcesWereProcessed);
}

MaterialBrowserView::~MaterialBrowserView()
{
  clear();
}

void MaterialBrowserView::setSortOrder(const MaterialSortOrder sortOrder)
{
  if (sortOrder != m_sortOrder)
  {
    m_sortOrder = sortOrder;
    reloadMaterials();
  }
}

void MaterialBrowserView::setGroup(const bool group)
{
  if (group != m_group)
  {
    m_group = group;
    reloadMaterials();
  }
}

void MaterialBrowserView::setHideUnused(const bool hideUnused)
{
  if (hideUnused != m_hideUnused)
  {
    m_hideUnused = hideUnused;
    reloadMaterials();
  }
}

void MaterialBrowserView::setFilterText(const std::string& filterText)
{
  if (filterText != m_filterText)
  {
    m_filterText = filterText;
    reloadMaterials();
  }
}

const gl::Material* MaterialBrowserView::selectedMaterial() const
{
  return m_selectedMaterial;
}

void MaterialBrowserView::setSelectedMaterial(const gl::Material* selectedMaterial)
{
  if (m_selectedMaterial != selectedMaterial)
  {
    m_selectedMaterial = selectedMaterial;
    update();
  }
}

void MaterialBrowserView::revealMaterial(const gl::Material* material)
{
  scrollToCell([&](const Cell& cell) {
    const auto& cellMaterial = cellData(cell);
    return &cellMaterial == material;
  });
}

void MaterialBrowserView::resourcesWereProcessed(const std::vector<gl::ResourceId>&)
{
  reloadMaterials();
}

void MaterialBrowserView::reloadMaterials()
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
  const auto& fontPath = pref(Preferences::RendererFontPath);
  const auto fontSize = pref(Preferences::BrowserFontSize);
  contract_assert(fontSize > 0);

  const auto font = gl::FontDescriptor{fontPath, size_t(fontSize)};

  if (m_group)
  {
    for (const auto* collection : getCollections())
    {
      layout.addGroup(collection->path().string(), float(fontSize) + 2.0f);
      addMaterialsToLayout(layout, getMaterials(*collection), font);
    }
  }
  else
  {
    addMaterialsToLayout(layout, getMaterials(), font);
  }
}

void MaterialBrowserView::addMaterialsToLayout(
  Layout& layout,
  const std::vector<const gl::Material*>& materials,
  const gl::FontDescriptor& font)
{
  for (const auto* material : materials)
  {
    addMaterialToLayout(layout, *material, font);
  }
}

void MaterialBrowserView::addMaterialToLayout(
  Layout& layout, const gl::Material& material, const gl::FontDescriptor& font)
{
  const auto maxCellWidth = layout.maxCellWidth();

  const auto materialName = std::filesystem::path{material.name()}.filename().string();
  const auto titleHeight = fontManager().font(font).measure(materialName).y();

  const auto scaleFactor = pref(Preferences::MaterialBrowserIconSize);
  const auto* texture = material.texture();
  const auto textureSize = texture ? texture->sizef() : vm::vec2f{64, 64};
  const auto scaledTextureSize = vm::round(scaleFactor * textureSize);

  layout.addItem(
    &material,
    materialName,
    scaledTextureSize.x(),
    scaledTextureSize.y(),
    maxCellWidth,
    titleHeight + 4.0f);
}

std::vector<const gl::MaterialCollection*> MaterialBrowserView::getCollections() const
{
  const auto& map = m_document.map();
  const auto enabledMaterialCollections = mdl::enabledMaterialCollections(map);

  auto result = std::vector<const gl::MaterialCollection*>{};
  for (const auto& collection : map.materialManager().collections())
  {
    if (kdl::vec_contains(enabledMaterialCollections, collection.path()))
    {
      result.push_back(&collection);
    }
  }
  return result;
}

std::vector<const gl::Material*> MaterialBrowserView::getMaterials(
  const gl::MaterialCollection& collection) const
{
  return sortMaterials(filterMaterials(
    collection.materials() | std::views::transform([](const auto& t) { return &t; })
    | kdl::ranges::to<std::vector>()));
}

std::vector<const gl::Material*> MaterialBrowserView::getMaterials() const
{
  auto materials = std::vector<const gl::Material*>{};
  for (const auto& collection : getCollections())
  {
    for (const auto& material : collection->materials())
    {
      materials.push_back(&material);
    }
  }
  return sortMaterials(filterMaterials(materials));
}

std::vector<const gl::Material*> MaterialBrowserView::filterMaterials(
  std::vector<const gl::Material*> materials) const
{
  if (m_hideUnused)
  {
    std::erase_if(
      materials, [](const auto* material) { return material->usageCount() == 0; });
  }
  if (!m_filterText.empty())
  {
    std::erase_if(materials, [&](const auto* material) {
      return std::ranges::none_of(
        kdl::str_split(m_filterText, " "), [&](const auto& pattern) {
          return kdl::ci::str_contains(material->name(), pattern);
        });
    });
  }
  return materials;
}

std::vector<const gl::Material*> MaterialBrowserView::sortMaterials(
  std::vector<const gl::Material*> materials) const
{
  const auto compareNames = [](const auto& lhs, const auto& rhs) {
    return kdl::ci::string_less{}(lhs->name(), rhs->name());
  };

  switch (m_sortOrder)
  {
  case MaterialSortOrder::Name:
    return kdl::vec_sort(std::move(materials), compareNames);
  case MaterialSortOrder::Usage:
    return kdl::vec_sort(std::move(materials), [&](const auto* lhs, const auto* rhs) {
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
  const auto viewLeft = float(0);
  const auto viewTop = float(size().height());
  const auto viewRight = float(size().width());
  const auto viewBottom = float(0);

  const auto transformation = render::Transformation{
    vm::ortho_matrix(-1.0f, 1.0f, viewLeft, viewTop, viewRight, viewBottom),
    vm::view_matrix(vm::vec3f{0, 0, -1}, vm::vec3f{0, 1, 0})
      * vm::translation_matrix(vm::vec3f{0.0f, 0.0f, 0.1f})};

  renderBounds(layout, y, height);
  renderMaterials(layout, y, height);
}

bool MaterialBrowserView::shouldRenderFocusIndicator() const
{
  return false;
}

const Color& MaterialBrowserView::getBackgroundColor()
{
  return pref(Preferences::BrowserBackgroundColor);
}

void MaterialBrowserView::renderBounds(Layout& layout, const float y, const float height)
{
  using BoundsVertex = gl::VertexTypes::P2C4::Vertex;
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
            const auto& material = cellData(cell);
            const auto& color = materialColor(material);
            vertices.emplace_back(
              vm::vec2f{bounds.left() - 2.0f, height - (bounds.top() - 2.0f - y)},
              color.to<RgbaF>().toVec());
            vertices.emplace_back(
              vm::vec2f{bounds.left() - 2.0f, height - (bounds.bottom() + 2.0f - y)},
              color.to<RgbaF>().toVec());
            vertices.emplace_back(
              vm::vec2f{bounds.right() + 2.0f, height - (bounds.bottom() + 2.0f - y)},
              color.to<RgbaF>().toVec());
            vertices.emplace_back(
              vm::vec2f{bounds.right() + 2.0f, height - (bounds.top() - 2.0f - y)},
              color.to<RgbaF>().toVec());
          }
        }
      }
    }
  }

  auto vertexArray = gl::VertexArray::move(std::move(vertices));
  auto shader =
    gl::ActiveShader{shaderManager(), gl::Shaders::MaterialBrowserBorderShader};

  vertexArray.prepare(vboManager());
  vertexArray.render(gl::PrimType::Quads);
}

const Color& MaterialBrowserView::materialColor(const gl::Material& material) const
{
  if (&material == m_selectedMaterial)
  {
    return pref(Preferences::MaterialBrowserSelectedColor);
  }
  if (material.usageCount() > 0)
  {
    return pref(Preferences::MaterialBrowserUsedColor);
  }
  return pref(Preferences::MaterialBrowserDefaultColor);
}

void MaterialBrowserView::renderMaterials(
  Layout& layout, const float y, const float height)
{
  using Vertex = gl::VertexTypes::P2UV2::Vertex;

  auto shader = gl::ActiveShader{shaderManager(), gl::Shaders::MaterialBrowserShader};
  shader.set("ApplyTinting", false);
  shader.set("Material", 0);
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
            const auto& material = cellData(cell);

            auto vertexArray = gl::VertexArray::move(std::vector<Vertex>{
              Vertex{{bounds.left(), height - (bounds.top() - y)}, {0, 0}},
              Vertex{{bounds.left(), height - (bounds.bottom() - y)}, {0, 1}},
              Vertex{{bounds.right(), height - (bounds.bottom() - y)}, {1, 1}},
              Vertex{{bounds.right(), height - (bounds.top() - y)}, {1, 0}},
            });

            material.activate(
              pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));

            vertexArray.prepare(vboManager());
            vertexArray.render(gl::PrimType::Quads);

            material.deactivate();
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
    const auto& material = cellData(*cell);
    setSelectedMaterial(&material);
    emit materialSelected(&material);

    update();
  }
}

QString MaterialBrowserView::tooltip(const Cell& cell)
{
  const auto& material = cellData(cell);

  auto tooltip = QString{};
  auto ss = QTextStream{&tooltip};
  ss << QString::fromStdString(material.name()) << "\n";

  if (const auto* texture = material.texture())
  {
    ss << texture->width() << "x" << texture->height();
  }
  else
  {
    ss << "Loading...";
  }
  return tooltip;
}

void MaterialBrowserView::doContextMenu(
  Layout& layout, float x, float y, QContextMenuEvent* event)
{
  if (const auto* cell = layout.cellAt(x, y))
  {
    auto menu = QMenu{this};
    menu.addAction(tr("Select Faces"), this, [&, material = &cellData(*cell)]() {
      selectBrushFacesWithMaterial(m_document.map(), material->name());
    });

    menu.addAction(tr("Select Brushes"), this, [&, material = &cellData(*cell)]() {
      selectBrushesWithMaterial(m_document.map(), material->name());
    });

    menu.addSeparator();

    menu.addAction(tr("Copy Name"), this, [&, material = &cellData(*cell)]() {
      auto* clipboard = QApplication::clipboard();
      clipboard->setText(QString::fromStdString(material->name()));
    });

    menu.exec(event->globalPos());
  }
}

const gl::Material& MaterialBrowserView::cellData(const Cell& cell) const
{
  return *cell.itemAs<const gl::Material*>();
}

} // namespace tb::ui
