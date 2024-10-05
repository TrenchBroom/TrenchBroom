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
#include "View/TabBook.h"

#include "vm/bbox.h"

#include <memory>
#include <optional>
#include <vector>

class QWidget;
class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;

namespace TrenchBroom::Model
{
class Node;
}

namespace TrenchBroom::View
{
class CollapsibleTitledPanel;
class MapDocument;

class MapInspector : public TabBookPage
{
  Q_OBJECT
private:
  CollapsibleTitledPanel* m_mapPropertiesEditor = nullptr;
  CollapsibleTitledPanel* m_modEditor = nullptr;

public:
  explicit MapInspector(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
  ~MapInspector() override;

private:
  void createGui(std::weak_ptr<MapDocument> document);
  QWidget* createLayerEditor(std::weak_ptr<MapDocument> document);
  CollapsibleTitledPanel* createMapPropertiesEditor(std::weak_ptr<MapDocument> document);
  CollapsibleTitledPanel* createModEditor(std::weak_ptr<MapDocument> document);
};

/**
 * Currently just the soft bounds editor
 */
class MapPropertiesEditor : public QWidget
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;
  bool m_updatingGui = false;

  QRadioButton* m_softBoundsDisabled = nullptr;
  QRadioButton* m_softBoundsFromGame = nullptr;
  QLabel* m_softBoundsFromGameMinLabel = nullptr;
  QLabel* m_softBoundsFromGameMaxLabel = nullptr;
  QRadioButton* m_softBoundsFromMap = nullptr;
  QLineEdit* m_softBoundsFromMapMinEdit = nullptr;
  QLineEdit* m_softBoundsFromMapMaxEdit = nullptr;

  NotifierConnection m_notifierConnection;

public:
  explicit MapPropertiesEditor(
    std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

private:
  std::optional<vm::bbox3d> parseLineEdits();
  void createGui();

private:
  void connectObservers();

  void documentWasNewed(MapDocument* document);
  void documentWasLoaded(MapDocument* document);
  void nodesDidChange(const std::vector<Model::Node*>& nodes);
  void updateGui();
};

} // namespace TrenchBroom::View
