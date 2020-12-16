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

#include "View/BorderLine.h"
#include "View/CollapsibleTitledPanel.h"
#include "View/EntityBrowser.h"
#include "View/EntityDefinitionFileChooser.h"
#include "View/EntityPropertyEditor.h"
#include "View/MapDocument.h"
#include "View/Splitter.h"
#include "View/TitledPanel.h"
#include "View/QtUtils.h"

#include <QVBoxLayout>

namespace TrenchBroom {
    namespace View {
        EntityInspector::EntityInspector(std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent) :
        TabBookPage(parent),
        m_splitter(nullptr),
        m_attributeEditor(nullptr),
        m_entityBrowser(nullptr),
        m_entityDefinitionFileChooser(nullptr) {
            createGui(std::move(document), contextManager);
        }

        EntityInspector::~EntityInspector() {
            saveWindowState(m_splitter);
        }

        void EntityInspector::createGui(std::weak_ptr<MapDocument> document, GLContextManager& contextManager) {
            m_splitter = new Splitter(Qt::Vertical);
            m_splitter->setObjectName("EntityInspector_Splitter");

            m_splitter->addWidget(createAttributeEditor(m_splitter, document));
            m_splitter->addWidget(createEntityBrowser(m_splitter, document, contextManager));

            // when the window resizes, keep the attribute editor size constant
            m_splitter->setStretchFactor(0, 0);
            m_splitter->setStretchFactor(1, 1);

            m_attributeEditor->setMinimumSize(100, 150);
            m_entityBrowser->setMinimumSize(100, 150);

            auto* outerSizer = new QVBoxLayout();
            outerSizer->setContentsMargins(0, 0, 0, 0);
            outerSizer->setSpacing(0);
            outerSizer->addWidget(m_splitter, 1);
            outerSizer->addWidget(new BorderLine(BorderLine::Direction::Horizontal), 0);
            outerSizer->addWidget(createEntityDefinitionFileChooser(this, document), 0);
            setLayout(outerSizer);

            restoreWindowState(m_splitter);
        }

        QWidget* EntityInspector::createAttributeEditor(QWidget* parent, std::weak_ptr<MapDocument> document) {
            m_attributeEditor = new EntityPropertyEditor(std::move(document), parent);
            return m_attributeEditor;
        }

        QWidget* EntityInspector::createEntityBrowser(QWidget* parent, std::weak_ptr<MapDocument> document, GLContextManager& contextManager) {
            auto* panel = new TitledPanel(tr("Entity Browser"), parent);
            m_entityBrowser = new EntityBrowser(std::move(document), contextManager);

            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(m_entityBrowser, 1);
            panel->getPanel()->setLayout(sizer);

            return panel;
        }

        QWidget* EntityInspector::createEntityDefinitionFileChooser(QWidget* parent, std::weak_ptr<MapDocument> document) {
            CollapsibleTitledPanel* panel = new CollapsibleTitledPanel(tr("Entity Definitions"), false, parent);
            m_entityDefinitionFileChooser = new EntityDefinitionFileChooser(document);

            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(m_entityDefinitionFileChooser, 1);
            panel->getPanel()->setLayout(sizer);

            return panel;
        }
    }
}
