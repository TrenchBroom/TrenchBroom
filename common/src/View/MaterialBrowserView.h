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

#include "NotifierConnection.h"
#include "Renderer/FontDescriptor.h"
#include "View/CellView.h"

#include <memory>
#include <string>
#include <vector>

class QScrollBar;

namespace tb::assets
{
class Material;
class MaterialCollection;
class ResourceId;
} // namespace tb::assets

namespace tb::View
{

class GLContextManager;
class MapDocument;
using MaterialGroupData = std::string;

enum class MaterialSortOrder
{
  Name,
  Usage
};

class MaterialBrowserView : public CellView
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;
  bool m_group = false;
  bool m_hideUnused = false;
  MaterialSortOrder m_sortOrder = MaterialSortOrder::Name;
  std::string m_filterText;

  const assets::Material* m_selectedMaterial = nullptr;

  NotifierConnection m_notifierConnection;

public:
  MaterialBrowserView(
    QScrollBar* scrollBar,
    GLContextManager& contextManager,
    std::weak_ptr<MapDocument> document);
  ~MaterialBrowserView() override;

  void setSortOrder(MaterialSortOrder sortOrder);
  void setGroup(bool group);
  void setHideUnused(bool hideUnused);
  void setFilterText(const std::string& filterText);

  const assets::Material* selectedMaterial() const;
  void setSelectedMaterial(const assets::Material* selectedMaterial);

  void revealMaterial(const assets::Material* material);

private:
  void resourcesWereProcessed(const std::vector<assets::ResourceId>& resources);

  void reloadMaterials();

  void doInitLayout(Layout& layout) override;
  void doReloadLayout(Layout& layout) override;

  void addMaterialsToLayout(
    Layout& layout,
    const std::vector<const assets::Material*>& materials,
    const Renderer::FontDescriptor& font);
  void addMaterialToLayout(
    Layout& layout,
    const assets::Material& material,
    const Renderer::FontDescriptor& font);

  std::vector<const assets::MaterialCollection*> getCollections() const;
  std::vector<const assets::Material*> getMaterials(
    const assets::MaterialCollection& collection) const;
  std::vector<const assets::Material*> getMaterials() const;

  std::vector<const assets::Material*> filterMaterials(
    std::vector<const assets::Material*> materials) const;
  std::vector<const assets::Material*> sortMaterials(
    std::vector<const assets::Material*> materials) const;

  void doClear() override;
  void doRender(Layout& layout, float y, float height) override;
  bool shouldRenderFocusIndicator() const override;
  const Color& getBackgroundColor() override;

  void renderBounds(Layout& layout, float y, float height);
  const Color& materialColor(const assets::Material& material) const;
  void renderMaterials(Layout& layout, float y, float height);

  void doLeftClick(Layout& layout, float x, float y) override;
  QString tooltip(const Cell& cell) override;
  void doContextMenu(Layout& layout, float x, float y, QContextMenuEvent* event) override;

  const assets::Material& cellData(const Cell& cell) const;
signals:
  void materialSelected(const assets::Material* material);
};

} // namespace tb::View
