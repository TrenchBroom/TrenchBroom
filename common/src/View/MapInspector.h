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

#ifndef TrenchBroom_MapInspector
#define TrenchBroom_MapInspector

#include "View/TabBook.h"

#include <memory>

class QWidget;
class QCheckBox;
class QLineEdit;

namespace TrenchBroom {
    namespace View {
        class MapDocument;

        class MapInspector : public TabBookPage {
            Q_OBJECT
        public:
            explicit MapInspector(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
        private:
            void createGui(std::weak_ptr<MapDocument> document);
            QWidget* createLayerEditor(std::weak_ptr<MapDocument> document);
            QWidget* createMapProperties(std::weak_ptr<MapDocument> document);
            QWidget* createModEditor(std::weak_ptr<MapDocument> document);
        };

        class MapPropertiesEditor : public QWidget {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;

            QCheckBox* m_checkBox;
            QLineEdit* m_sizeBox;
        public:
            explicit MapPropertiesEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
            ~MapPropertiesEditor() override;
        private:
            void createGui();
        private:
            void bindObservers();
            void unbindObservers();

            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);
            void updateGui();
        };
    }
}

#endif /* defined(TrenchBroom_MapInspector) */
