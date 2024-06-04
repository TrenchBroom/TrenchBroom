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

#include "MaterialBrowser.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QtGlobal>

#include "Assets/Material.h"
#include "Assets/MaterialManager.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/MapDocument.h"
#include "View/MaterialBrowserView.h"
#include "View/QtUtils.h"
#include "View/ViewConstants.h"

#include "kdl/memory_utils.h"

// for use in QVariant
Q_DECLARE_METATYPE(TrenchBroom::View::MaterialSortOrder)

namespace TrenchBroom::View
{

MaterialBrowser::MaterialBrowser(
  std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent)
  : QWidget{parent}
  , m_document{std::move(document)}
{
  createGui(contextManager);
  bindEvents();
  connectObservers();
  reload();
}

const Assets::Material* MaterialBrowser::selectedMaterial() const
{
  return m_view->selectedMaterial();
}

void MaterialBrowser::setSelectedMaterial(const Assets::Material* selectedMaterial)
{
  m_view->setSelectedMaterial(selectedMaterial);
}

void MaterialBrowser::revealMaterial(const Assets::Material* material)
{
  setFilterText("");
  m_view->revealMaterial(material);
}

void MaterialBrowser::setSortOrder(const MaterialSortOrder sortOrder)
{
  m_view->setSortOrder(sortOrder);
  switch (sortOrder)
  {
  case MaterialSortOrder::Name:
    m_sortOrderChoice->setCurrentIndex(0);
    break;
  case MaterialSortOrder::Usage:
    m_sortOrderChoice->setCurrentIndex(1);
    break;
    switchDefault();
  }
}

void MaterialBrowser::setGroup(const bool group)
{
  m_view->setGroup(group);
  m_groupButton->setChecked(group);
}

void MaterialBrowser::setHideUnused(const bool hideUnused)
{
  m_view->setHideUnused(hideUnused);
  m_usedButton->setChecked(hideUnused);
}

void MaterialBrowser::setFilterText(const std::string& filterText)
{
  m_view->setFilterText(filterText);
  m_filterBox->setText(QString::fromStdString(filterText));
}

/**
 * See EntityBrowser::createGui
 */
void MaterialBrowser::createGui(GLContextManager& contextManager)
{
  auto* browserPanel = new QWidget{};
  m_scrollBar = new QScrollBar{Qt::Vertical};

  auto document = kdl::mem_lock(m_document);
  m_view = new MaterialBrowserView{m_scrollBar, contextManager, document};

  auto* browserPanelSizer = new QHBoxLayout{};
  browserPanelSizer->setContentsMargins(0, 0, 0, 0);
  browserPanelSizer->setSpacing(0);
  browserPanelSizer->addWidget(m_view, 1);
  browserPanelSizer->addWidget(m_scrollBar, 0);
  browserPanel->setLayout(browserPanelSizer);

  m_sortOrderChoice = new QComboBox{};
  m_sortOrderChoice->addItem(tr("Name"), QVariant::fromValue(MaterialSortOrder::Name));
  m_sortOrderChoice->addItem(tr("Usage"), QVariant::fromValue(MaterialSortOrder::Usage));
  m_sortOrderChoice->setCurrentIndex(0);
  m_sortOrderChoice->setToolTip(tr("Select ordering criterion"));
  connect(
    m_sortOrderChoice, QOverload<int>::of(&QComboBox::activated), this, [&](int index) {
      auto sortOrder =
        static_cast<MaterialSortOrder>(m_sortOrderChoice->itemData(index).toInt());
      m_view->setSortOrder(sortOrder);
    });

  m_groupButton = new QPushButton{tr("Group")};
  m_groupButton->setToolTip(tr("Group materials by material collection"));
  m_groupButton->setCheckable(true);
  connect(m_groupButton, &QAbstractButton::clicked, this, [&]() {
    m_view->setGroup(m_groupButton->isChecked());
  });

  m_usedButton = new QPushButton{tr("Used")};
  m_usedButton->setToolTip(tr("Only show materials currently in use"));
  m_usedButton->setCheckable(true);
  connect(m_usedButton, &QAbstractButton::clicked, this, [&]() {
    m_view->setHideUnused(m_usedButton->isChecked());
  });

  m_filterBox = createSearchBox();
  connect(m_filterBox, &QLineEdit::textEdited, this, [&]() {
    m_view->setFilterText(m_filterBox->text().toStdString());
  });

  auto* controlLayout = new QHBoxLayout{};
  controlLayout->setContentsMargins(
    LayoutConstants::NarrowHMargin,
    LayoutConstants::NarrowVMargin,
    LayoutConstants::NarrowHMargin,
    LayoutConstants::NarrowVMargin);
  controlLayout->setSpacing(LayoutConstants::NarrowHMargin);
  controlLayout->addWidget(m_sortOrderChoice);
  controlLayout->addWidget(m_groupButton);
  controlLayout->addWidget(m_usedButton);
  controlLayout->addWidget(m_filterBox, 1);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->setSpacing(0);
  outerLayout->addWidget(browserPanel, 1);
  outerLayout->addLayout(controlLayout, 0);

  setLayout(outerLayout);
}

void MaterialBrowser::bindEvents()
{
  connect(
    m_view,
    &MaterialBrowserView::materialSelected,
    this,
    &MaterialBrowser::materialSelected);
}

void MaterialBrowser::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection +=
    document->documentWasNewedNotifier.connect(this, &MaterialBrowser::documentWasNewed);
  m_notifierConnection += document->documentWasLoadedNotifier.connect(
    this, &MaterialBrowser::documentWasLoaded);
  m_notifierConnection +=
    document->nodesWereAddedNotifier.connect(this, &MaterialBrowser::nodesWereAdded);
  m_notifierConnection +=
    document->nodesWereRemovedNotifier.connect(this, &MaterialBrowser::nodesWereRemoved);
  m_notifierConnection +=
    document->nodesDidChangeNotifier.connect(this, &MaterialBrowser::nodesDidChange);
  m_notifierConnection += document->brushFacesDidChangeNotifier.connect(
    this, &MaterialBrowser::brushFacesDidChange);
  m_notifierConnection += document->materialCollectionsDidChangeNotifier.connect(
    this, &MaterialBrowser::materialCollectionsDidChange);
  m_notifierConnection += document->currentMaterialNameDidChangeNotifier.connect(
    this, &MaterialBrowser::currentMaterialNameDidChange);

  auto& prefs = PreferenceManager::instance();
  m_notifierConnection += prefs.preferenceDidChangeNotifier.connect(
    this, &MaterialBrowser::preferenceDidChange);
}

void MaterialBrowser::documentWasNewed(MapDocument*)
{
  reload();
}

void MaterialBrowser::documentWasLoaded(MapDocument*)
{
  reload();
}

void MaterialBrowser::nodesWereAdded(const std::vector<Model::Node*>&)
{
  reload();
}

void MaterialBrowser::nodesWereRemoved(const std::vector<Model::Node*>&)
{
  reload();
}

void MaterialBrowser::nodesDidChange(const std::vector<Model::Node*>&)
{
  reload();
}

void MaterialBrowser::brushFacesDidChange(const std::vector<Model::BrushFaceHandle>&)
{
  reload();
}

void MaterialBrowser::materialCollectionsDidChange()
{
  reload();
}

void MaterialBrowser::currentMaterialNameDidChange(const std::string& /* materialName */)
{
  updateSelectedMaterial();
}

void MaterialBrowser::preferenceDidChange(const std::filesystem::path& path)
{
  auto document = kdl::mem_lock(m_document);
  if (
    path == Preferences::MaterialBrowserIconSize.path()
    || document->isGamePathPreference(path))
  {
    reload();
  }
  else
  {
    m_view->update();
  }
}

void MaterialBrowser::reload()
{
  if (m_view)
  {
    updateSelectedMaterial();
    m_view->invalidate();
    m_view->update();
  }
}

void MaterialBrowser::updateSelectedMaterial()
{
  auto document = kdl::mem_lock(m_document);
  const auto& materialName = document->currentMaterialName();
  const auto* material = document->materialManager().material(materialName);
  m_view->setSelectedMaterial(material);
}

} // namespace TrenchBroom::View
