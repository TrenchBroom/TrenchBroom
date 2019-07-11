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

#ifndef TrenchBroom_EntityInspector
#define TrenchBroom_EntityInspector

#include "View/TabBook.h"
#include "View/ViewTypes.h"

class QSplitter;

namespace TrenchBroom {
    namespace View {
        class EntityBrowser;
        class EntityDefinitionFileChooser;
        class EntityAttributeEditor;
        class GLContextManager;

        class EntityInspector : public TabBookPage {
            Q_OBJECT
        private:
            QSplitter* m_splitter;
            EntityAttributeEditor* m_attributeEditor;
            EntityBrowser* m_entityBrowser;
            EntityDefinitionFileChooser* m_entityDefinitionFileChooser;
        public:
            EntityInspector(MapDocumentWPtr document, GLContextManager& contextManager, QWidget* parent = nullptr);
            ~EntityInspector() override;
        private:
            void createGui(MapDocumentWPtr document, GLContextManager& contextManager);
            QWidget* createAttributeEditor(QWidget* parent, MapDocumentWPtr document);
            QWidget* createEntityBrowser(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager);
            QWidget* createEntityDefinitionFileChooser(QWidget* parent, MapDocumentWPtr document);
        };
    }
}

#endif /* defined(TrenchBroom_EntityInspector) */
