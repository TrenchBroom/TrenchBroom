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

namespace TrenchBroom {
    namespace View {
        class MapDocument;

        class MapInspector : public TabBookPage {
            Q_OBJECT
        public:
            MapInspector(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
        private:
            void createGui(std::weak_ptr<MapDocument> document);
            QWidget* createLayerEditor(QWidget* parent, std::weak_ptr<MapDocument> document);
            QWidget* createModEditor(QWidget* parent, std::weak_ptr<MapDocument> document);
        };
    }
}

#endif /* defined(TrenchBroom_MapInspector) */
