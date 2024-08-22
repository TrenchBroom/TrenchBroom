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

#include "EntityInspector.h"

#include <QVBoxLayout>

#include "View/BorderLine.h"
#include "View/CollapsibleTitledPanel.h"
#include "View/EntityBrowser.h"
#include "View/EntityDefinitionFileChooser.h"
#include "View/EntityPropertyEditor.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"
#include "View/Splitter.h"
#include "View/SwitchableTitledPanel.h"

namespace TrenchBroom::View
{

EntityInspector::EntityInspector(
  std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent)
  : TabBookPage{parent}
{
  createGui(std::move(document), contextManager);
}

EntityInspector::~EntityInspector()
{
  saveWindowState(m_splitter);
}

void EntityInspector::createGui(
  std::weak_ptr<MapDocument> document, GLContextManager& contextManager)
{
  m_splitter = new Splitter{Qt::Vertical};
  m_splitter->setObjectName("EntityInspector_Splitter");

  m_splitter->addWidget(createAttributeEditor(m_splitter, document));
  m_splitter->addWidget(createEntityBrowser(m_splitter, document, contextManager));

  // when the window resizes, keep the attribute editor size constant
  m_splitter->setStretchFactor(0, 0);
  m_splitter->setStretchFactor(1, 1);

  m_attributeEditor->setMinimumSize(100, 150);
  m_entityBrowser->setMinimumSize(100, 150);

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(m_splitter, 1);
  setLayout(layout);

  restoreWindowState(m_splitter);
}

QWidget* EntityInspector::createAttributeEditor(
  QWidget* parent, std::weak_ptr<MapDocument> document)
{
  m_attributeEditor = new EntityPropertyEditor{std::move(document), parent};
  return m_attributeEditor;
}

QWidget* EntityInspector::createEntityBrowser(
  QWidget* parent, std::weak_ptr<MapDocument> document, GLContextManager& contextManager)
{
  auto* panel = new SwitchableTitledPanel{
    tr("Entity Browser"), {{tr("Browser"), tr("Settings")}}, parent};

  m_entityBrowser = new EntityBrowser{document, contextManager};

  auto* entityBrowserLayout = new QVBoxLayout{};
  entityBrowserLayout->setContentsMargins(0, 0, 0, 0);
  entityBrowserLayout->addWidget(m_entityBrowser, 1);
  panel->getPanel(0)->setLayout(entityBrowserLayout);

  auto* entityDefinitionFileEditor = new EntityDefinitionFileChooser{document};

  auto* entityDefinitionFileEditorLayout = new QVBoxLayout{};
  entityDefinitionFileEditorLayout->setContentsMargins(0, 0, 0, 0);
  entityDefinitionFileEditorLayout->addWidget(entityDefinitionFileEditor, 1);
  panel->getPanel(1)->setLayout(entityDefinitionFileEditorLayout);

  return panel;
}

} // namespace TrenchBroom::View
