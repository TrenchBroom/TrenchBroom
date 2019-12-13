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

#include "MapInspector.h"

#include "View/BorderLine.h"
#include "View/CollapsibleTitledPanel.h"
#include "View/LayerEditor.h"
#include "View/ModEditor.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"

#include <QVBoxLayout>

namespace TrenchBroom {
    namespace View {
        MapInspector::MapInspector(std::weak_ptr<MapDocument> document, QWidget* parent) :
        TabBookPage(parent) {
            createGui(document);
        }

        void MapInspector::createGui(std::weak_ptr<MapDocument> document) {
            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->setSpacing(0);

            sizer->addWidget(createLayerEditor(this, document), 1);
            sizer->addWidget(new BorderLine(BorderLine::Direction::Horizontal), 0);
            sizer->addWidget(createModEditor(this, document), 0);
            setLayout(sizer);
        }

        QWidget* MapInspector::createLayerEditor(QWidget* parent, std::weak_ptr<MapDocument> document) {
            TitledPanel* titledPanel = new TitledPanel(tr("Layers"), parent);
            LayerEditor* layerEditor = new LayerEditor(document);

            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(layerEditor, 1);
            titledPanel->getPanel()->setLayout(sizer);

            return titledPanel;
        }

        QWidget* MapInspector::createModEditor(QWidget* parent, std::weak_ptr<MapDocument> document) {
            CollapsibleTitledPanel* titledPanel = new CollapsibleTitledPanel(tr("Mods"), false, parent);
            ModEditor* modEditor = new ModEditor(document);

            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(modEditor, 1);
            titledPanel->getPanel()->setLayout(sizer);

            return titledPanel;
        }
    }
}
