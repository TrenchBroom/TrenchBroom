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

#ifndef TrenchBroom_LayerEditor
#define TrenchBroom_LayerEditor

#include <memory>
#include <string>

#include <QWidget>

class QAbstractButton;

namespace TrenchBroom {
    namespace Model {
        class LayerNode;
    }

    namespace View {
        class LayerListBox;
        class MapDocument;

        class LayerEditor : public QWidget {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;
            LayerListBox* m_layerList;

            QAbstractButton* m_addLayerButton;
            QAbstractButton* m_removeLayerButton;
            QAbstractButton* m_showAllLayersButton;
        public:
            explicit LayerEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
        private:
            void onSetCurrentLayer(Model::LayerNode* layer);
            void onLayerRightClick(Model::LayerNode* layer);

            class CollectMoveableNodes;
            void onMoveSelectionToLayer();
            bool canMoveSelectionToLayer() const;

            void onToggleLayerVisibleFromMenu();
            void onToggleLayerVisibleFromList(Model::LayerNode* layer);
            bool canToggleLayerVisible() const;
            void toggleLayerVisible(Model::LayerNode* layer);

            void onToggleLayerLockedFromMenu();
            void onToggleLayerLockedFromList(Model::LayerNode* layer);
            bool canToggleLayerLocked() const;
            void toggleLayerLocked(Model::LayerNode* layer);

            void onSelectAllInLayer();

            void onAddLayer();
            std::string queryLayerName();

            void onRemoveLayer();
            bool canRemoveLayer() const;

            void onShowAllLayers();
        private:
            Model::LayerNode* findVisibleAndUnlockedLayer(const Model::LayerNode* except) const;
            void moveSelectedNodesToLayer(std::shared_ptr<MapDocument> document, Model::LayerNode* layer);
            void createGui();
        private slots:
            void updateButtons();
        };
    }
}

#endif /* defined(TrenchBroom_LayerEditor) */
