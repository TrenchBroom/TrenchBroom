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

#include "View/ControlListBox.h"

#include <memory>
#include <vector>

class QColor;
class QLabel;
class QAbstractButton;
class QListWidget;

namespace TrenchBroom {
    namespace Model {
        class LayerNode;
        class Node;
    }

    namespace View {
        class MapDocument;

        class LayerColorIndicator : public QWidget {
            Q_OBJECT
        public:
            explicit LayerColorIndicator(QWidget* parent = nullptr);
            void setColor(const QColor& color);
        };

        class LayerListBoxWidget : public ControlListBoxItemRenderer {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;
            Model::LayerNode* m_layer;
            QAbstractButton* m_activeButton;
            QLabel* m_nameText;
            QLabel* m_infoText;
            QAbstractButton* m_hiddenButton;
            QAbstractButton* m_lockButton;
            QAbstractButton* m_moveLayerUpButton;
            QAbstractButton* m_moveLayerDownButton;
        public:
            LayerListBoxWidget(std::weak_ptr<MapDocument> document, Model::LayerNode* layer, QWidget* parent = nullptr);

            void updateItem() override;
        private:
            void updateLayerItem();
        public:
            Model::LayerNode* layer() const;
        private:
            bool eventFilter(QObject* target, QEvent* event) override;
        signals:
            void layerActiveClicked(Model::LayerNode* layer);
            void layerVisibilityToggled(Model::LayerNode* layer);
            void layerLockToggled(Model::LayerNode* layer);
            void layerMovedUp(Model::LayerNode* layer);
            void layerMovedDown(Model::LayerNode* layer);
            void layerDoubleClicked(Model::LayerNode* layer);
            void layerRightClicked(Model::LayerNode* layer);
        };

        class LayerListBox : public ControlListBox {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;
        public:
            explicit LayerListBox(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
            ~LayerListBox() override;

            Model::LayerNode* selectedLayer() const;
            void setSelectedLayer(Model::LayerNode* layer);
        private:
            size_t itemCount() const override;

            ControlListBoxItemRenderer* createItemRenderer(QWidget* parent, size_t index) override;

            void selectedRowChanged(int index) override;
        private:
            void bindObservers();
            void unbindObservers();

            void documentDidChange(MapDocument* document);
            void nodesDidChange(const std::vector<Model::Node*>& nodes);
            void currentLayerDidChange(const Model::LayerNode* layer);

            const LayerListBoxWidget* widgetAtRow(int row) const;
            Model::LayerNode* layerForRow(int row) const;
        signals:
            void layerSelected(Model::LayerNode* layer);
            void layerSetCurrent(Model::LayerNode* layer);
            void layerRightClicked(Model::LayerNode* layer);
            void layerVisibilityToggled(Model::LayerNode* layer);
            void layerLockToggled(Model::LayerNode* layer);
            void layerMovedUp(Model::LayerNode* layer);
            void layerMovedDown(Model::LayerNode* layer);
        };
    }
}

#endif /* defined(TrenchBroom_LayerListBox) */
