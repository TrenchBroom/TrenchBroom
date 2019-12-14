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

#include <QResizeEvent>
#include <QVBoxLayout>

namespace TrenchBroom {
    namespace View {
        Inspector::Inspector(std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent) :
        QWidget(parent),
        m_tabBook(nullptr),
        m_mapInspector(nullptr),
        m_entityInspector(nullptr),
        m_faceInspector(nullptr),
        m_topWidgetMaster(nullptr) {
            m_tabBook = new TabBook();

            m_mapInspector = new MapInspector(document);
            m_entityInspector = new EntityInspector(document, contextManager);
            m_faceInspector = new FaceInspector(document, contextManager);

            m_tabBook->addPage(m_mapInspector, "Map");
            m_tabBook->addPage(m_entityInspector, "Entity");
            m_tabBook->addPage(m_faceInspector, "Face");

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(m_tabBook);
            setLayout(layout);
        }

        void Inspector::connectTopWidgets(QWidget* master) {
            if (m_topWidgetMaster != nullptr) {
                m_topWidgetMaster->removeEventFilter(this);
            }
            m_topWidgetMaster = master;
            if (m_topWidgetMaster != nullptr) {
                m_topWidgetMaster->installEventFilter(this);
            }
        }

        void Inspector::switchToPage(const InspectorPage page) {
            m_tabBook->switchToPage(static_cast<int>(page));
        }

        bool Inspector::cancelMouseDrag() {
            return m_faceInspector->cancelMouseDrag();
        }

        bool Inspector::eventFilter(QObject* target, QEvent* event) {
            if (target == m_topWidgetMaster && event->type() == QEvent::Resize) {
                auto* sizeEvent = static_cast<QResizeEvent*>(event);
                m_tabBook->setTabBarHeight(sizeEvent->size().height());
                return false;
            } else {
                return QObject::eventFilter(target, event);
            }
        }
    }
}
