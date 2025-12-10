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

#include "MaterialBrowser.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QtGlobal>

#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/GameInfo.h"
#include "mdl/Map.h"
#include "mdl/Material.h"
#include "mdl/MaterialManager.h"
#include "ui/MapDocument.h"
#include "ui/MaterialBrowserView.h"
#include "ui/QtUtils.h"
#include "ui/ViewConstants.h"

// for use in QVariant
Q_DECLARE_METATYPE(tb::ui::MaterialSortOrder)

namespace tb::ui
{

MaterialBrowser::MaterialBrowser(
  MapDocument& document, GLContextManager& contextManager, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
{
  createGui(contextManager);
  bindEvents();
  connectObservers();
  reload();
}

const mdl::Material* MaterialBrowser::selectedMaterial() const
{
  return m_view->selectedMaterial();
}

void MaterialBrowser::setSelectedMaterial(const mdl::Material* selectedMaterial)
{
  m_view->setSelectedMaterial(selectedMaterial);
}

void MaterialBrowser::revealMaterial(const mdl::Material* material)
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

  m_view = new MaterialBrowserView{m_scrollBar, contextManager, m_document};

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
  m_notifierConnection += m_document.documentWasLoadedNotifier.connect(
    this, &MaterialBrowser::documentDidChange);
  m_notifierConnection += m_document.documentDidChangeNotifier.connect(
    this, &MaterialBrowser::documentDidChange);
  m_notifierConnection += m_document.currentMaterialNameDidChangeNotifier.connect(
    this, &MaterialBrowser::currentMaterialNameDidChange);

  auto& prefs = PreferenceManager::instance();
  m_notifierConnection += prefs.preferenceDidChangeNotifier.connect(
    this, &MaterialBrowser::preferenceDidChange);
}

void MaterialBrowser::documentDidChange()
{
  reload();
}

void MaterialBrowser::currentMaterialNameDidChange()
{
  updateSelectedMaterial();
}

void MaterialBrowser::preferenceDidChange(const std::filesystem::path& path)
{
  if (
    path == pref(m_document.map().gameInfo().gamePathPreference)
    || path == Preferences::MaterialBrowserIconSize.path())
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
  auto& map = m_document.map();

  const auto& materialName = map.currentMaterialName();
  const auto* material = map.materialManager().material(materialName);
  m_view->setSelectedMaterial(material);
}

} // namespace tb::ui
