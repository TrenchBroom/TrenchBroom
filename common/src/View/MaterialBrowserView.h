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
#include "View/CellView.h"
#include "render/FontDescriptor.h"

#include <memory>
#include <string>
#include <vector>

class QScrollBar;

namespace tb::asset
{
class Material;
class MaterialCollection;
class ResourceId;
} // namespace tb::asset

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

  const asset::Material* m_selectedMaterial = nullptr;

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

  const asset::Material* selectedMaterial() const;
  void setSelectedMaterial(const asset::Material* selectedMaterial);

  void revealMaterial(const asset::Material* material);

private:
  void resourcesWereProcessed(const std::vector<asset::ResourceId>& resources);

  void reloadMaterials();

  void doInitLayout(Layout& layout) override;
  void doReloadLayout(Layout& layout) override;

  void addMaterialsToLayout(
    Layout& layout,
    const std::vector<const asset::Material*>& materials,
    const render::FontDescriptor& font);
  void addMaterialToLayout(
    Layout& layout, const asset::Material& material, const render::FontDescriptor& font);

  std::vector<const asset::MaterialCollection*> getCollections() const;
  std::vector<const asset::Material*> getMaterials(
    const asset::MaterialCollection& collection) const;
  std::vector<const asset::Material*> getMaterials() const;

  std::vector<const asset::Material*> filterMaterials(
    std::vector<const asset::Material*> materials) const;
  std::vector<const asset::Material*> sortMaterials(
    std::vector<const asset::Material*> materials) const;

  void doClear() override;
  void doRender(Layout& layout, float y, float height) override;
  bool shouldRenderFocusIndicator() const override;
  const Color& getBackgroundColor() override;

  void renderBounds(Layout& layout, float y, float height);
  const Color& materialColor(const asset::Material& material) const;
  void renderMaterials(Layout& layout, float y, float height);

  void doLeftClick(Layout& layout, float x, float y) override;
  QString tooltip(const Cell& cell) override;
  void doContextMenu(Layout& layout, float x, float y, QContextMenuEvent* event) override;

  const asset::Material& cellData(const Cell& cell) const;
signals:
  void materialSelected(const asset::Material* material);
};

} // namespace tb::View
