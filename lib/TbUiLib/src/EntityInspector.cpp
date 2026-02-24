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

#include "ui/EntityInspector.h"

#include <QVBoxLayout>

#include "ui/EntityBrowser.h"
#include "ui/EntityDefinitionFileChooser.h"
#include "ui/EntityPropertyEditor.h"
#include "ui/Splitter.h"
#include "ui/SwitchableTitledPanel.h"
#include "ui/WidgetState.h"

namespace tb::ui
{

EntityInspector::EntityInspector(
  AppController& appController, MapDocument& document, QWidget* parent)
  : TabBookPage{parent}
{
  createGui(appController, document);
}

EntityInspector::~EntityInspector()
{
  saveWidgetState(m_splitter);
}

void EntityInspector::createGui(AppController& appController, MapDocument& document)
{
  m_splitter = new Splitter{Qt::Vertical};
  m_splitter->setObjectName("EntityInspector_Splitter");

  m_splitter->addWidget(createEntityPropertyEditor(document, m_splitter));
  m_splitter->addWidget(createEntityBrowser(appController, document, m_splitter));

  // when the window resizes, keep the attribute editor size constant
  m_splitter->setStretchFactor(0, 0);
  m_splitter->setStretchFactor(1, 1);

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(m_splitter, 1);
  setLayout(layout);

  restoreWidgetState(m_splitter);
}

QWidget* EntityInspector::createEntityPropertyEditor(
  MapDocument& document, QWidget* parent)
{
  m_attributeEditor = new EntityPropertyEditor{document, parent};
  return m_attributeEditor;
}

QWidget* EntityInspector::createEntityBrowser(
  AppController& appController, MapDocument& document, QWidget* parent)
{
  auto* panel = new SwitchableTitledPanel{
    tr("Entity Browser"), {{tr("Browser"), tr("Settings")}}, parent};

  m_entityBrowser = new EntityBrowser{appController, document};

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

} // namespace tb::ui
