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
        class Layer;
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
            QAbstractButton* m_moveLayerUpButton;
            QAbstractButton* m_moveLayerDownButton;
        public:
            explicit LayerEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
        private:
            void onSetCurrentLayer(Model::Layer* layer);
            void onLayerRightClick(Model::Layer* layer);

            class CollectMoveableNodes;
            void onMoveSelectionToLayer();
            bool canMoveSelectionToLayer() const;

            void onToggleLayerVisibleFromMenu();
            void onToggleLayerVisibleFromList(Model::Layer* layer);
            bool canToggleLayerVisible() const;
            void toggleLayerVisible(Model::Layer* layer);

            void onToggleLayerLockedFromMenu();
            void onToggleLayerLockedFromList(Model::Layer* layer);
            bool canToggleLayerLocked() const;
            void toggleLayerLocked(Model::Layer* layer);

            void onSelectAllInLayer();

            void onAddLayer();
            std::string queryLayerName();

            void onRemoveLayer();
            bool canRemoveLayer() const;

            void onShowAllLayers();

            void onMoveLayerUpFromMenu();
            void onMoveLayerUpFromList(Model::Layer* layer);
            bool canMoveLayerUp() const;

            void onMoveLayerDownFromMenu();
            void onMoveLayerDownFromList(Model::Layer* layer);
            bool canMoveLayerDown() const;

            void moveLayer(Model::Layer* layer, int delta);
        private:
            Model::Layer* findVisibleAndUnlockedLayer(const Model::Layer* except) const;
            void moveSelectedNodesToLayer(std::shared_ptr<MapDocument> document, Model::Layer* layer);
            void createGui();
        private slots:
            void updateButtons();
        };
    }
}

#endif /* defined(TrenchBroom_LayerEditor) */
