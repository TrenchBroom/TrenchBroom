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

#include "EntityAttributeEditor.h"

#include "View/EntityAttributeGrid.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/SmartAttributeEditorManager.h"

#include <QSplitter>
#include <QVBoxLayout>

namespace TrenchBroom {
    namespace View {
        EntityAttributeEditor::EntityAttributeEditor(QWidget* parent, MapDocumentWPtr document) :
        QWidget(parent),
        m_document(document) {
            createGui(this, document);
        }

        // FIXME: Change this to a signal/slot connection to m_attributeGrid
        void EntityAttributeEditor::OnIdle() {
#if 0
            const String& attributeName = m_attributeGrid->selectedRowName();
            if (!attributeName.empty() && attributeName != m_lastSelectedAttributeName) {
                MapDocumentSPtr document = lock(m_document);
                m_smartEditorManager->switchEditor(attributeName, document->allSelectedAttributableNodes());
                m_lastSelectedAttributeName = attributeName;
            }
#endif
        }
        
        void EntityAttributeEditor::createGui(QWidget* parent, MapDocumentWPtr document) {
            auto* splitter = new QSplitter(Qt::Vertical);

            // Configure the sash gravity so the first widget gets most of the space
            splitter->setSizes(QList<int>{1'000'000, 1});
            //splitter->SetName("EntityAttributeEditorSplitter");
            
            m_attributeGrid = new EntityAttributeGrid(nullptr, document);
            m_smartEditorManager = new SmartAttributeEditorManager(nullptr, document);

            splitter->addWidget(m_attributeGrid);
            splitter->addWidget(m_smartEditorManager);

            m_attributeGrid->setMinimumSize(100, 50);
            m_smartEditorManager->setMinimumSize(500, 100);

            auto* sizer = new QVBoxLayout();
            sizer->addWidget(splitter, 1);
            setLayout(sizer);

            // FIXME:
            //wxPersistenceManager::Get().RegisterAndRestore(splitter);
        }
    }
}
