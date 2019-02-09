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

#include "StringUtils.h"
#include "View/BorderLine.h"
//#include "View/CollapsibleTitledPanel.h"
#include "View/EntityBrowser.h"
//#include "View/EntityDefinitionFileChooser.h"
#include "View/EntityAttributeEditor.h"
//#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"

#include <QVBoxLayout>
#include <QSplitter>

namespace TrenchBroom {
    namespace View {
        EntityInspector::EntityInspector(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        TabBookPage(parent) {
            createGui(document, contextManager);
        }

        void EntityInspector::createGui(MapDocumentWPtr document, GLContextManager& contextManager) {
            QSplitter* splitter = new QSplitter(Qt::Vertical);
            //splitter->setSashGravity(0.0);
            //splitter->SetName("EntityInspectorSplitter");
            
            splitter->addWidget(createAttributeEditor(splitter, document));
            splitter->addWidget(createEntityBrowser(splitter, document, contextManager));

            m_attributeEditor->setMinimumSize(100, 150);
            //m_entityBrowser->setMinimumSize(100, 150);
            
            auto* outerSizer = new QVBoxLayout();
            outerSizer->addWidget(splitter, 1);
            outerSizer->addWidget(new BorderLine(nullptr, BorderLine::Direction_Horizontal), 0);
            outerSizer->addWidget(createEntityDefinitionFileChooser(this, document), 0);
            setLayout(outerSizer);

            // FIXME: persist
            //wxPersistenceManager::Get().RegisterAndRestore(splitter);
        }
        
        QWidget* EntityInspector::createAttributeEditor(QWidget* parent, MapDocumentWPtr document) {
            m_attributeEditor = new EntityAttributeEditor(parent, document);
            return m_attributeEditor;
        }
        
        QWidget* EntityInspector::createEntityBrowser(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager) {
#if 0
            TitledPanel* panel = new TitledPanel(parent, "Entity Browser");
            m_entityBrowser = new EntityBrowser(panel->getPanel(), document, contextManager);
            
            auto* sizer = new QVBoxLayout();
            sizer->addWidget(m_entityBrowser, 1, wxEXPAND);
            panel->getPanel()->SetSizer(sizer);
            
            return panel;
#endif
            m_entityBrowser = new EntityBrowser(nullptr, document, contextManager);
            return m_entityBrowser;
        }
        
        QWidget* EntityInspector::createEntityDefinitionFileChooser(QWidget* parent, MapDocumentWPtr document) {
#if 0
            CollapsibleTitledPanel* panel = new CollapsibleTitledPanel(parent, "Entity Definitions", false);
            m_entityDefinitionFileChooser = new EntityDefinitionFileChooser(panel->getPanel(), document);

            auto* sizer = new QVBoxLayout();
            sizer->addWidget(m_entityDefinitionFileChooser, 1, wxEXPAND);
            panel->getPanel()->SetSizer(sizer);

            return panel;
#endif
            return new QWidget();
        }
    }
}
