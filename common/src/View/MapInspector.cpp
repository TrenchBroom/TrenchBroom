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

#include "MapInspector.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>

#include "Model/EntityProperties.h"
#include "Model/WorldNode.h"
#include "View/BorderLine.h"
#include "View/ClickableLabel.h"
#include "View/CollapsibleTitledPanel.h"
#include "View/LayerEditor.h"
#include "View/MapDocument.h"
#include "View/ModEditor.h"
#include "View/QtUtils.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"

#include "kdl/memory_utils.h"

#include "vm/vec_io.h"

#include <optional>
#include <utility>

namespace TrenchBroom
{
namespace View
{
// MapInspector

MapInspector::MapInspector(std::weak_ptr<MapDocument> document, QWidget* parent)
  : TabBookPage(parent)
  , m_mapPropertiesEditor(nullptr)
  , m_modEditor(nullptr)
{
  createGui(document);
}

MapInspector::~MapInspector()
{
  saveWindowState(m_mapPropertiesEditor);
  saveWindowState(m_modEditor);
}

void MapInspector::createGui(std::weak_ptr<MapDocument> document)
{
  m_mapPropertiesEditor = createMapPropertiesEditor(document);
  m_modEditor = createModEditor(document);

  auto* sizer = new QVBoxLayout();
  sizer->setContentsMargins(0, 0, 0, 0);
  sizer->setSpacing(0);

  sizer->addWidget(createLayerEditor(document), 1);
  sizer->addWidget(new BorderLine(BorderLine::Direction::Horizontal), 0);
  sizer->addWidget(m_mapPropertiesEditor, 0);
  sizer->addWidget(new BorderLine(BorderLine::Direction::Horizontal), 0);
  sizer->addWidget(m_modEditor, 0);
  setLayout(sizer);
}

QWidget* MapInspector::createLayerEditor(std::weak_ptr<MapDocument> document)
{
  TitledPanel* titledPanel = new TitledPanel(tr("Layers"));
  LayerEditor* layerEditor = new LayerEditor(document);

  auto* sizer = new QVBoxLayout();
  sizer->setContentsMargins(0, 0, 0, 0);
  sizer->addWidget(layerEditor, 1);
  titledPanel->getPanel()->setLayout(sizer);

  return titledPanel;
}

CollapsibleTitledPanel* MapInspector::createMapPropertiesEditor(
  std::weak_ptr<MapDocument> document)
{
  CollapsibleTitledPanel* titledPanel = new CollapsibleTitledPanel(tr("Map Properties"));
  titledPanel->setObjectName("MapInspector_MapPropertiesPanel");

  auto* editor = new MapPropertiesEditor(document);

  auto* sizer = new QVBoxLayout();
  sizer->setContentsMargins(0, 0, 0, 0);
  sizer->addWidget(editor, 1);
  titledPanel->getPanel()->setLayout(sizer);

  restoreWindowState(titledPanel);

  return titledPanel;
}

CollapsibleTitledPanel* MapInspector::createModEditor(std::weak_ptr<MapDocument> document)
{
  CollapsibleTitledPanel* titledPanel = new CollapsibleTitledPanel(tr("Mods"));
  titledPanel->setObjectName("MapInspector_ModsPanel");

  ModEditor* modEditor = new ModEditor(document);

  auto* sizer = new QVBoxLayout();
  sizer->setContentsMargins(0, 0, 0, 0);
  sizer->addWidget(modEditor, 1);
  titledPanel->getPanel()->setLayout(sizer);

  restoreWindowState(titledPanel);

  return titledPanel;
}

// MapPropertiesEditor

MapPropertiesEditor::MapPropertiesEditor(
  std::weak_ptr<MapDocument> document, QWidget* parent)
  : QWidget(parent)
  , m_document(document)
  , m_updatingGui(false)
  , m_softBoundsDisabled(nullptr)
  , m_softBoundsFromGame(nullptr)
  , m_softBoundsFromGameMinLabel(nullptr)
  , m_softBoundsFromGameMaxLabel(nullptr)
  , m_softBoundsFromMap(nullptr)
  , m_softBoundsFromMapMinEdit(nullptr)
  , m_softBoundsFromMapMaxEdit(nullptr)
{
  createGui();
  connectObservers();
}

static std::optional<vm::vec3> parseVec(const QString& qString)
{
  const std::string string = qString.toStdString();
  if (const auto vec = vm::parse<double, 3u>(string))
  {
    return *vec;
  }
  else if (const auto val = vm::parse<double, 1u>(string))
  {
    return vm::vec3::fill(val->x());
  }
  else
  {
    return std::nullopt;
  }
}

std::optional<vm::bbox3> MapPropertiesEditor::parseLineEdits()
{
  const auto min = parseVec(m_softBoundsFromMapMinEdit->text());
  const auto max = parseVec(m_softBoundsFromMapMaxEdit->text());

  if (!min.has_value() || !max.has_value())
  {
    return std::nullopt;
  }

  for (size_t i = 0; i < 3; ++i)
  {
    if ((*min)[i] >= (*max)[i])
    {
      return std::nullopt;
    }
  }

  return vm::bbox3(*min, *max);
}

void MapPropertiesEditor::createGui()
{
  m_softBoundsDisabled = new QRadioButton();
  auto* softBoundsDisabledLabel = new ClickableLabel(tr("Soft bounds disabled"));

  m_softBoundsFromGame = new QRadioButton();
  m_softBoundsFromGameMinLabel = new QLabel();
  m_softBoundsFromGameMaxLabel = new QLabel();
  auto* softBoundsFromGameLabel = new ClickableLabel(tr("Use game default"));

  auto* minCaptionLabel = new QLabel(tr("Min:"));
  auto* maxCaptionLabel = new QLabel(tr("Max:"));

  makeInfo(minCaptionLabel);
  makeInfo(maxCaptionLabel);
  makeInfo(m_softBoundsFromGameMinLabel);
  makeInfo(m_softBoundsFromGameMaxLabel);

  auto* softBoundsFromGameValueLayout = new QHBoxLayout();
  softBoundsFromGameValueLayout->setContentsMargins(0, 0, 0, 0);
  softBoundsFromGameValueLayout->setSpacing(LayoutConstants::MediumHMargin);
  softBoundsFromGameValueLayout->addWidget(minCaptionLabel);
  softBoundsFromGameValueLayout->addWidget(m_softBoundsFromGameMinLabel);
  softBoundsFromGameValueLayout->addWidget(maxCaptionLabel);
  softBoundsFromGameValueLayout->addWidget(m_softBoundsFromGameMaxLabel);
  softBoundsFromGameValueLayout->addStretch(1);

  auto* softBoundsFromGameLayout = new QVBoxLayout();
  softBoundsFromGameLayout->setContentsMargins(0, 0, 0, 0);
  softBoundsFromGameLayout->setSpacing(LayoutConstants::NarrowVMargin);
  softBoundsFromGameLayout->addWidget(softBoundsFromGameLabel);
  softBoundsFromGameLayout->addLayout(softBoundsFromGameValueLayout);

  m_softBoundsFromMap = new QRadioButton();
  auto* softBoundsFromMapLabel = new ClickableLabel(tr("Use custom bounds"));
  m_softBoundsFromMapMinEdit = new QLineEdit();
  m_softBoundsFromMapMinEdit->setPlaceholderText("min");
  m_softBoundsFromMapMaxEdit = new QLineEdit();
  m_softBoundsFromMapMaxEdit->setPlaceholderText("max");

  auto* softBoundsFromMapValueLayout = new QHBoxLayout();
  softBoundsFromMapValueLayout->setSpacing(LayoutConstants::MediumHMargin);
  softBoundsFromMapValueLayout->addWidget(m_softBoundsFromMapMinEdit);
  softBoundsFromMapValueLayout->addWidget(m_softBoundsFromMapMaxEdit);
  softBoundsFromMapValueLayout->addStretch(1);

  auto* softBoundsFromMapLayout = new QVBoxLayout();
  softBoundsFromMapLayout->setContentsMargins(0, 0, 0, 0);
  softBoundsFromMapLayout->setSpacing(LayoutConstants::NarrowVMargin);
  softBoundsFromMapLayout->addWidget(softBoundsFromMapLabel);
  softBoundsFromMapLayout->addLayout(softBoundsFromMapValueLayout);

  auto* gridLayout = new QGridLayout();
  gridLayout->setContentsMargins(
    LayoutConstants::MediumHMargin,
    LayoutConstants::MediumVMargin,
    LayoutConstants::MediumHMargin,
    LayoutConstants::MediumVMargin);
  gridLayout->setHorizontalSpacing(LayoutConstants::NarrowHMargin);
  gridLayout->setVerticalSpacing(LayoutConstants::MediumVMargin);

  gridLayout->addWidget(m_softBoundsDisabled, 0, 0, Qt::AlignTop);
  gridLayout->addWidget(softBoundsDisabledLabel, 0, 1, Qt::AlignTop);
  gridLayout->addWidget(m_softBoundsFromGame, 1, 0, Qt::AlignTop);
  gridLayout->addLayout(softBoundsFromGameLayout, 1, 1, Qt::AlignTop);
  gridLayout->addWidget(m_softBoundsFromMap, 2, 0, Qt::AlignTop);
  gridLayout->addLayout(softBoundsFromMapLayout, 2, 1, Qt::AlignTop);

  setLayout(gridLayout);

  connect(
    softBoundsDisabledLabel,
    &ClickableLabel::clicked,
    m_softBoundsDisabled,
    &QAbstractButton::click);
  connect(
    softBoundsFromGameLabel,
    &ClickableLabel::clicked,
    m_softBoundsFromGame,
    &QAbstractButton::click);
  connect(
    softBoundsFromMapLabel,
    &ClickableLabel::clicked,
    m_softBoundsFromMap,
    &QAbstractButton::click);

  connect(
    m_softBoundsDisabled, &QAbstractButton::clicked, this, [this](const bool checked) {
      auto document = kdl::mem_lock(m_document);
      if (checked)
      {
        document->setSoftMapBounds({Model::Game::SoftMapBoundsType::Map, std::nullopt});
      }
    });
  connect(
    m_softBoundsFromGame, &QAbstractButton::clicked, this, [this](const bool checked) {
      auto document = kdl::mem_lock(m_document);
      if (checked)
      {
        document->setSoftMapBounds({Model::Game::SoftMapBoundsType::Game, std::nullopt});
      }
    });
  connect(
    m_softBoundsFromMap, &QAbstractButton::clicked, this, [this](const bool checked) {
      auto document = kdl::mem_lock(m_document);

      m_softBoundsFromMapMinEdit->setEnabled(true);
      m_softBoundsFromMapMaxEdit->setEnabled(true);

      if (checked)
      {
        const std::optional<vm::bbox3> parsed = parseLineEdits();
        // Only commit the change to the document right now if both text fields can be
        // parsed. Otherwise, it will be committed below in textEditingFinished once both
        // text fields have a valid value entered.
        if (parsed.has_value())
        {
          document->setSoftMapBounds(
            {Model::Game::SoftMapBoundsType::Map, parsed.value()});
        }
      }
    });

  const auto textEditingFinished = [this]() {
    // QLineEdit::editingFinished is emitted not just in response to user actions,
    // but also e.g. if another radio button is clicked and the min/max line edits get
    // disabled. So unfortunately we need to check if we are inside updateGui() and avoid
    // committing a change in that case.
    if (m_updatingGui)
    {
      return;
    }
    auto document = kdl::mem_lock(m_document);

    const std::optional<vm::bbox3> parsed = parseLineEdits();
    if (parsed.has_value())
    {
      document->setSoftMapBounds({Model::Game::SoftMapBoundsType::Map, parsed.value()});
    }
  };
  connect(
    m_softBoundsFromMapMinEdit, &QLineEdit::editingFinished, this, textEditingFinished);
  connect(
    m_softBoundsFromMapMaxEdit, &QLineEdit::editingFinished, this, textEditingFinished);

  updateGui();
}

void MapPropertiesEditor::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection += document->documentWasNewedNotifier.connect(
    this, &MapPropertiesEditor::documentWasNewed);
  m_notifierConnection += document->documentWasLoadedNotifier.connect(
    this, &MapPropertiesEditor::documentWasLoaded);
  m_notifierConnection +=
    document->nodesDidChangeNotifier.connect(this, &MapPropertiesEditor::nodesDidChange);
}

void MapPropertiesEditor::documentWasNewed(MapDocument*)
{
  updateGui();
}

void MapPropertiesEditor::documentWasLoaded(MapDocument*)
{
  updateGui();
}

void MapPropertiesEditor::nodesDidChange(const std::vector<Model::Node*>& nodes)
{
  auto document = kdl::mem_lock(m_document);
  if (!document)
  {
    return;
  }

  for (Model::Node* node : nodes)
  {
    if (node == document->world())
    {
      updateGui();
      return;
    }
  }
}

static QString formatVec(const std::optional<vm::bbox3>& bbox, const bool max)
{
  if (!bbox.has_value())
  {
    return QObject::tr("None");
  }
  const vm::vec3& vec = max ? bbox->max : bbox->min;
  std::stringstream str;
  if (vec.x() == vec.y() && vec.y() == vec.z())
  {
    // Just print the first component to save space
    str << vec.x();
  }
  else
  {
    str << vec;
  }
  return QString::fromStdString(str.str());
}

/**
 * Refresh the UI from the model
 */
void MapPropertiesEditor::updateGui()
{
  const kdl::set_temp flagChange(m_updatingGui, true);

  auto document = kdl::mem_lock(m_document);
  if (!document)
  {
    return;
  }

  auto game = document->game();
  if (game == nullptr)
  {
    return;
  }

  const std::optional<vm::bbox3> gameBounds = game->config().softMapBounds;
  m_softBoundsFromGameMinLabel->setText(formatVec(gameBounds, false));
  m_softBoundsFromGameMaxLabel->setText(formatVec(gameBounds, true));

  const auto bounds = document->softMapBounds();

  if (bounds.source == Model::Game::SoftMapBoundsType::Map && !bounds.bounds.has_value())
  {
    m_softBoundsDisabled->setChecked(true);

    m_softBoundsFromMapMinEdit->setEnabled(false);
    m_softBoundsFromMapMaxEdit->setEnabled(false);
  }
  else if (bounds.source == Model::Game::SoftMapBoundsType::Map)
  {
    m_softBoundsFromMap->setChecked(true);

    m_softBoundsFromMapMinEdit->setEnabled(true);
    m_softBoundsFromMapMaxEdit->setEnabled(true);

    m_softBoundsFromMapMinEdit->setText(formatVec(bounds.bounds, false));
    m_softBoundsFromMapMaxEdit->setText(formatVec(bounds.bounds, true));
  }
  else
  {
    m_softBoundsFromGame->setChecked(true);

    m_softBoundsFromMapMinEdit->setEnabled(false);
    m_softBoundsFromMapMaxEdit->setEnabled(false);
  }
}
} // namespace View
} // namespace TrenchBroom
