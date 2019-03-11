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

#include "Inspector.h"

#include "View/EntityInspector.h"
#include "View/FaceInspector.h"
#include "View/MapInspector.h"
#include "View/TabBook.h"
#include "View/TabBar.h"

#include <QVBoxLayout>

namespace TrenchBroom {
    namespace View {
        Inspector::Inspector(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        QWidget(parent),
        m_tabBook(nullptr),
        m_mapInspector(nullptr),
        m_entityInspector(nullptr),
        m_faceInspector(nullptr) {
            
            m_tabBook = new TabBook(this);

            m_mapInspector = new MapInspector(m_tabBook, document, contextManager);
            m_entityInspector = new EntityInspector(m_tabBook, document, contextManager);
            m_faceInspector = (FaceInspector*) new TabBookPage(nullptr); // FIXME: new FaceInspector(m_tabBook, document, contextManager);
            
            m_tabBook->addPage(m_mapInspector, "Map");
            m_tabBook->addPage(m_entityInspector, "Entity");
            m_tabBook->addPage(m_faceInspector, "Face");
            
            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(m_tabBook);
            setLayout(sizer);
        }

        void Inspector::connectTopWidgets(QWidget* master) {
            // FIXME: Not sure how to do this in Qt
            //master->Bind(wxEVT_SIZE, &Inspector::OnTopWidgetSize, this);
        }

        void Inspector::switchToPage(const InspectorPage page) {
            m_tabBook->switchToPage(static_cast<size_t>(page));
        }

        bool Inspector::cancelMouseDrag() {
            return false; // FIXME: m_faceInspector->cancelMouseDrag();
        }

        void Inspector::OnTopWidgetSize() {
//            if (IsBeingDeleted()) return;
//            m_tabBook->setTabBarHeight(event.GetSize().y);
//            event.Skip();
        }
    }
}
