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

#include <QWidget>

#include "NotifierConnection.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class QPushButton;
class QComboBox;
class QLineEdit;
class QScrollBar;

namespace tb::asset
{
class Material;
}

namespace tb::Model
{
class BrushFaceHandle;
class Node;
} // namespace tb::Model

namespace tb::View
{
class GLContextManager;
class MapDocument;
class MaterialBrowserView;
enum class MaterialSortOrder;

class MaterialBrowser : public QWidget
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;
  QComboBox* m_sortOrderChoice = nullptr;
  QPushButton* m_groupButton = nullptr;
  QPushButton* m_usedButton = nullptr;
  QLineEdit* m_filterBox = nullptr;
  QScrollBar* m_scrollBar = nullptr;
  MaterialBrowserView* m_view = nullptr;

  NotifierConnection m_notifierConnection;

public:
  MaterialBrowser(
    std::weak_ptr<MapDocument> document,
    GLContextManager& contextManager,
    QWidget* parent = nullptr);

  const asset::Material* selectedMaterial() const;
  void setSelectedMaterial(const asset::Material* selectedMaterial);
  void revealMaterial(const asset::Material* material);

  void setSortOrder(MaterialSortOrder sortOrder);
  void setGroup(bool group);
  void setHideUnused(bool hideUnused);
  void setFilterText(const std::string& filterText);
signals:
  void materialSelected(const asset::Material* material);

private:
  void createGui(GLContextManager& contextManager);
  void bindEvents();

  void connectObservers();

  void documentWasNewed(MapDocument* document);
  void documentWasLoaded(MapDocument* document);
  void nodesWereAdded(const std::vector<Model::Node*>& nodes);
  void nodesWereRemoved(const std::vector<Model::Node*>& nodes);
  void nodesDidChange(const std::vector<Model::Node*>& nodes);
  void brushFacesDidChange(const std::vector<Model::BrushFaceHandle>& faces);
  void materialCollectionsDidChange();
  void currentMaterialNameDidChange(const std::string& materialName);
  void preferenceDidChange(const std::filesystem::path& path);

  void reload();
  void updateSelectedMaterial();
};

} // namespace tb::View
