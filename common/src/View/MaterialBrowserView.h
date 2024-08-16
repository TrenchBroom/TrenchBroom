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

#pragma once

#include "NotifierConnection.h"
#include "Renderer/FontDescriptor.h"
#include "View/CellView.h"

#include <memory>
#include <string>
#include <vector>

class QScrollBar;

namespace TrenchBroom::Assets
{
class Material;
class MaterialCollection;
class ResourceId;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::View
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

  const Assets::Material* m_selectedMaterial = nullptr;

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

  const Assets::Material* selectedMaterial() const;
  void setSelectedMaterial(const Assets::Material* selectedMaterial);

  void revealMaterial(const Assets::Material* material);

private:
  void resourcesWereProcessed(const std::vector<Assets::ResourceId>& resources);

  void reloadMaterials();

  void doInitLayout(Layout& layout) override;
  void doReloadLayout(Layout& layout) override;

  void addMaterialsToLayout(
    Layout& layout,
    const std::vector<const Assets::Material*>& materials,
    const Renderer::FontDescriptor& font);
  void addMaterialToLayout(
    Layout& layout,
    const Assets::Material& material,
    const Renderer::FontDescriptor& font);

  std::vector<const Assets::MaterialCollection*> getCollections() const;
  std::vector<const Assets::Material*> getMaterials(
    const Assets::MaterialCollection& collection) const;
  std::vector<const Assets::Material*> getMaterials() const;

  std::vector<const Assets::Material*> filterMaterials(
    std::vector<const Assets::Material*> materials) const;
  std::vector<const Assets::Material*> sortMaterials(
    std::vector<const Assets::Material*> materials) const;

  void doClear() override;
  void doRender(Layout& layout, float y, float height) override;
  bool doShouldRenderFocusIndicator() const override;
  const Color& getBackgroundColor() override;

  void renderBounds(Layout& layout, float y, float height);
  const Color& materialColor(const Assets::Material& material) const;
  void renderMaterials(Layout& layout, float y, float height);

  void doLeftClick(Layout& layout, float x, float y) override;
  QString tooltip(const Cell& cell) override;
  void doContextMenu(Layout& layout, float x, float y, QContextMenuEvent* event) override;

  const Assets::Material& cellData(const Cell& cell) const;
signals:
  void materialSelected(const Assets::Material* material);
};

} // namespace TrenchBroom::View
