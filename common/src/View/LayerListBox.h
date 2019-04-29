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

#ifndef TrenchBroom_LayerListBox
#define TrenchBroom_LayerListBox

#include "Model/ModelTypes.h"
#include "View/ControlListBox.h"
#include "View/ViewTypes.h"

#include <vector>

class QLabel;
class QAbstractButton;
class QListWidget;

namespace TrenchBroom {
    namespace View {
        class LayerListBoxWidget : public QWidget {
            Q_OBJECT
        private:
            MapDocumentWPtr m_document;
            Model::Layer* m_layer;
            QLabel* m_nameText;
            QLabel* m_infoText;
            QAbstractButton* m_hiddenButton;
            QAbstractButton* m_lockButton;

        public:
            LayerListBoxWidget(QWidget* parent, MapDocumentWPtr document, Model::Layer* layer);
            Model::Layer* layer() const;
            void refresh();

        private:
            void mouseReleaseEvent(QMouseEvent* event) override;

        signals:
            void layerVisibilityToggled(Model::Layer* layer);
            void layerLockToggled(Model::Layer* layer);
            void layerRightClicked(Model::Layer* layer);
        };

        class LayerListBox : public QWidget {
            Q_OBJECT
        private:
            MapDocumentWPtr m_document;
            QListWidget* m_list;
            Model::LayerList m_layersInUi;
        public:
            LayerListBox(QWidget* parent, MapDocumentWPtr document);
            ~LayerListBox() override;

            Model::Layer* selectedLayer() const;
            void setSelectedLayer(Model::Layer* layer);

        signals:
            void layerSelected(Model::Layer* layer);
            void layerSetCurrent(Model::Layer* layer);
            void layerRightClicked(Model::Layer* layer);
            void layerVisibilityToggled(Model::Layer* layer);
            void layerLockToggled(Model::Layer* layer);
        private:
            void createGui();
            void bindObservers();
            void unbindObservers();

            void documentDidChange(MapDocument* document);
            void nodesDidChange(const Model::NodeList& nodes);
            void currentLayerDidChange(const Model::Layer* layer);

            void bindEvents();
            void refreshList();

            LayerListBoxWidget* widgetAtRow(int row);
            Model::Layer* layerForItem(QListWidgetItem* item);
            Model::Layer* layerForRow(int row);
        };
    }
}

#endif /* defined(TrenchBroom_LayerListBox) */
