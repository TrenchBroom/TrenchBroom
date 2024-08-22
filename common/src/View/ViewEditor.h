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

#include <QWidget>

#include "Model/TagType.h"
#include "NotifierConnection.h"

#include <filesystem>
#include <memory>
#include <vector>

class QCheckBox;
class QWidget;
class QButtonGroup;

namespace TrenchBroom::Assets
{
class EntityDefinition;
class EntityDefinitionManager;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::Model
{
class EditorContext;
class SmartTag;
} // namespace TrenchBroom::Model

namespace TrenchBroom::View
{
class MapDocument;
class PopupButton;

class EntityDefinitionCheckBoxList : public QWidget
{
  Q_OBJECT
private:
  Assets::EntityDefinitionManager& m_entityDefinitionManager;
  Model::EditorContext& m_editorContext;

  std::vector<QCheckBox*> m_groupCheckBoxes;
  std::vector<QCheckBox*> m_defCheckBoxes;

public:
  EntityDefinitionCheckBoxList(
    Assets::EntityDefinitionManager& entityDefinitionManager,
    Model::EditorContext& editorContext,
    QWidget* parent = nullptr);

  void refresh();
private slots:
  void groupCheckBoxChanged(size_t groupIndex, bool checked);
  void defCheckBoxChanged(const Assets::EntityDefinition* definition, bool checked);
  void showAllClicked();
  void hideAllClicked();

private:
  void hideAll(bool hidden);
  void createGui();
};

class ViewEditor : public QWidget
{
  Q_OBJECT
private:
  using CheckBoxList = std::vector<QCheckBox*>;

  std::weak_ptr<MapDocument> m_document;

  QCheckBox* m_showEntityClassnamesCheckBox = nullptr;

  QCheckBox* m_showGroupBoundsCheckBox = nullptr;
  QCheckBox* m_showBrushEntityBoundsCheckBox = nullptr;
  QCheckBox* m_showPointEntityBoundsCheckBox = nullptr;

  QCheckBox* m_showPointEntitiesCheckBox = nullptr;
  QCheckBox* m_showPointEntityModelsCheckBox = nullptr;

  EntityDefinitionCheckBoxList* m_entityDefinitionCheckBoxList = nullptr;

  QCheckBox* m_showBrushesCheckBox = nullptr;
  std::vector<std::pair<Model::TagType::Type, QCheckBox*>> m_tagCheckBoxes;

  QButtonGroup* m_renderModeRadioGroup = nullptr;
  QCheckBox* m_shadeFacesCheckBox = nullptr;
  QCheckBox* m_showFogCheckBox = nullptr;
  QCheckBox* m_showEdgesCheckBox = nullptr;

  QButtonGroup* m_entityLinkRadioGroup = nullptr;

  QCheckBox* m_showSoftBoundsCheckBox = nullptr;

  NotifierConnection m_notifierConnection;

public:
  explicit ViewEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

private:
  void connectObservers();

  void documentWasNewedOrLoaded(MapDocument* document);
  void editorContextDidChange();
  void entityDefinitionsDidChange();
  void preferenceDidChange(const std::filesystem::path& path);

  void createGui();

  QWidget* createEntityDefinitionsPanel(QWidget* parent);
  QWidget* createEntitiesPanel(QWidget* parent);
  QWidget* createBrushesPanel(QWidget* parent);
  void createTagFilter(QWidget* parent);
  void createEmptyTagFilter(QWidget* parent);
  void createTagFilter(QWidget* parent, const std::vector<Model::SmartTag>& tags);

  QWidget* createRendererPanel(QWidget* parent);

  void refreshGui();
  void refreshEntityDefinitionsPanel();
  void refreshEntitiesPanel();
  void refreshBrushesPanel();
  void refreshRendererPanel();

  void showEntityClassnamesChanged(bool checked);
  void showGroupBoundsChanged(bool checked);
  void showBrushEntityBoundsChanged(bool checked);
  void showPointEntityBoundsChanged(bool checked);
  void showPointEntitiesChanged(bool checked);
  void showPointEntityModelsChanged(bool checked);
  void showBrushesChanged(bool checked);
  void showTagChanged(bool checked, Model::TagType::Type tagType);
  void faceRenderModeChanged(int id);
  void shadeFacesChanged(bool checked);
  void showFogChanged(bool checked);
  void showEdgesChanged(bool checked);
  void entityLinkModeChanged(int id);
  void showSoftMapBoundsChanged(bool checked);
  void restoreDefaultsClicked();
};

class ViewPopupEditor : public QWidget
{
  Q_OBJECT
private:
  PopupButton* m_button = nullptr;
  ViewEditor* m_editor = nullptr;

public:
  explicit ViewPopupEditor(
    std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
};

} // namespace TrenchBroom::View
