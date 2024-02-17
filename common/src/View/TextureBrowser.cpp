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

#include "TextureBrowser.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QtGlobal>

#include "Assets/Texture.h"
#include "Assets/TextureManager.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"
#include "View/TextureBrowserView.h"
#include "View/ViewConstants.h"

#include "kdl/memory_utils.h"

// for use in QVariant
Q_DECLARE_METATYPE(TrenchBroom::View::TextureSortOrder)

namespace TrenchBroom::View
{

TextureBrowser::TextureBrowser(
  std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent)
  : QWidget{parent}
  , m_document{std::move(document)}
{
  createGui(contextManager);
  bindEvents();
  connectObservers();
  reload();
}

const Assets::Texture* TextureBrowser::selectedTexture() const
{
  return m_view->selectedTexture();
}

void TextureBrowser::setSelectedTexture(const Assets::Texture* selectedTexture)
{
  m_view->setSelectedTexture(selectedTexture);
}

void TextureBrowser::revealTexture(const Assets::Texture* texture)
{
  setFilterText("");
  m_view->revealTexture(texture);
}

void TextureBrowser::setSortOrder(const TextureSortOrder sortOrder)
{
  m_view->setSortOrder(sortOrder);
  switch (sortOrder)
  {
  case TextureSortOrder::Name:
    m_sortOrderChoice->setCurrentIndex(0);
    break;
  case TextureSortOrder::Usage:
    m_sortOrderChoice->setCurrentIndex(1);
    break;
    switchDefault();
  }
}

void TextureBrowser::setGroup(const bool group)
{
  m_view->setGroup(group);
  m_groupButton->setChecked(group);
}

void TextureBrowser::setHideUnused(const bool hideUnused)
{
  m_view->setHideUnused(hideUnused);
  m_usedButton->setChecked(hideUnused);
}

void TextureBrowser::setFilterText(const std::string& filterText)
{
  m_view->setFilterText(filterText);
  m_filterBox->setText(QString::fromStdString(filterText));
}

/**
 * See EntityBrowser::createGui
 */
void TextureBrowser::createGui(GLContextManager& contextManager)
{
  auto* browserPanel = new QWidget{};
  m_scrollBar = new QScrollBar{Qt::Vertical};

  auto document = kdl::mem_lock(m_document);
  m_view = new TextureBrowserView{m_scrollBar, contextManager, document};

  auto* browserPanelSizer = new QHBoxLayout{};
  browserPanelSizer->setContentsMargins(0, 0, 0, 0);
  browserPanelSizer->setSpacing(0);
  browserPanelSizer->addWidget(m_view, 1);
  browserPanelSizer->addWidget(m_scrollBar, 0);
  browserPanel->setLayout(browserPanelSizer);

  m_sortOrderChoice = new QComboBox{};
  m_sortOrderChoice->addItem(tr("Name"), QVariant::fromValue(TextureSortOrder::Name));
  m_sortOrderChoice->addItem(tr("Usage"), QVariant::fromValue(TextureSortOrder::Usage));
  m_sortOrderChoice->setCurrentIndex(0);
  m_sortOrderChoice->setToolTip(tr("Select ordering criterion"));
  connect(
    m_sortOrderChoice, QOverload<int>::of(&QComboBox::activated), this, [=](int index) {
      auto sortOrder =
        static_cast<TextureSortOrder>(m_sortOrderChoice->itemData(index).toInt());
      m_view->setSortOrder(sortOrder);
    });

  m_groupButton = new QPushButton{tr("Group")};
  m_groupButton->setToolTip(tr("Group textures by texture collection"));
  m_groupButton->setCheckable(true);
  connect(m_groupButton, &QAbstractButton::clicked, this, [=]() {
    m_view->setGroup(m_groupButton->isChecked());
  });

  m_usedButton = new QPushButton{tr("Used")};
  m_usedButton->setToolTip(tr("Only show textures currently in use"));
  m_usedButton->setCheckable(true);
  connect(m_usedButton, &QAbstractButton::clicked, this, [=]() {
    m_view->setHideUnused(m_usedButton->isChecked());
  });

  m_filterBox = createSearchBox();
  connect(m_filterBox, &QLineEdit::textEdited, this, [=]() {
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

void TextureBrowser::bindEvents()
{
  connect(
    m_view, &TextureBrowserView::textureSelected, this, &TextureBrowser::textureSelected);
}

void TextureBrowser::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection +=
    document->documentWasNewedNotifier.connect(this, &TextureBrowser::documentWasNewed);
  m_notifierConnection +=
    document->documentWasLoadedNotifier.connect(this, &TextureBrowser::documentWasLoaded);
  m_notifierConnection +=
    document->nodesWereAddedNotifier.connect(this, &TextureBrowser::nodesWereAdded);
  m_notifierConnection +=
    document->nodesWereRemovedNotifier.connect(this, &TextureBrowser::nodesWereRemoved);
  m_notifierConnection +=
    document->nodesDidChangeNotifier.connect(this, &TextureBrowser::nodesDidChange);
  m_notifierConnection += document->brushFacesDidChangeNotifier.connect(
    this, &TextureBrowser::brushFacesDidChange);
  m_notifierConnection += document->textureCollectionsDidChangeNotifier.connect(
    this, &TextureBrowser::textureCollectionsDidChange);
  m_notifierConnection += document->currentTextureNameDidChangeNotifier.connect(
    this, &TextureBrowser::currentTextureNameDidChange);

  auto& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &TextureBrowser::preferenceDidChange);
}

void TextureBrowser::documentWasNewed(MapDocument*)
{
  reload();
}

void TextureBrowser::documentWasLoaded(MapDocument*)
{
  reload();
}

void TextureBrowser::nodesWereAdded(const std::vector<Model::Node*>&)
{
  reload();
}

void TextureBrowser::nodesWereRemoved(const std::vector<Model::Node*>&)
{
  reload();
}

void TextureBrowser::nodesDidChange(const std::vector<Model::Node*>&)
{
  reload();
}

void TextureBrowser::brushFacesDidChange(const std::vector<Model::BrushFaceHandle>&)
{
  reload();
}

void TextureBrowser::textureCollectionsDidChange()
{
  reload();
}

void TextureBrowser::currentTextureNameDidChange(const std::string& /* textureName */)
{
  updateSelectedTexture();
}

void TextureBrowser::preferenceDidChange(const std::filesystem::path& path)
{
  auto document = kdl::mem_lock(m_document);
  if (
    path == Preferences::TextureBrowserIconSize.path()
    || document->isGamePathPreference(path))
  {
    reload();
  }
  else
  {
    m_view->update();
  }
}

void TextureBrowser::reload()
{
  if (m_view)
  {
    updateSelectedTexture();
    m_view->invalidate();
    m_view->update();
  }
}

void TextureBrowser::updateSelectedTexture()
{
  auto document = kdl::mem_lock(m_document);
  const auto& textureName = document->currentTextureName();
  const auto* texture = document->textureManager().texture(textureName);
  m_view->setSelectedTexture(texture);
}

} // namespace TrenchBroom::View
