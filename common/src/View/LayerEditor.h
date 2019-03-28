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

#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <QWidget>

class QAbstractButton;

namespace TrenchBroom {
    namespace View {
        class LayerListBox;

        class LayerEditor : public QWidget {
            Q_OBJECT
        private:
            MapDocumentWPtr m_document;
            LayerListBox* m_layerList;

            QAbstractButton* m_addLayerButton;
            QAbstractButton* m_removeLayerButton;
            QAbstractButton* m_showAllLayersButton;
        public:
            LayerEditor(QWidget* parent, MapDocumentWPtr document);
        private:
            void OnSetCurrentLayer(Model::Layer* layer);
            void OnLayerRightClick(Model::Layer* layer);

            class CollectMoveableNodes;
            void OnMoveSelectionToLayer();
            bool canMoveSelectionToLayer() const;

            void OnToggleLayerVisibleFromMenu();
            void OnToggleLayerVisibleFromList(Model::Layer* layer);
            bool canToggleLayerVisible() const;
            void toggleLayerVisible(Model::Layer* layer);

            void OnToggleLayerLockedFromMenu();
            void OnToggleLayerLockedFromList(Model::Layer* layer);
            bool canToggleLayerLocked() const;
            void toggleLayerLocked(Model::Layer* layer);

            void OnSelectAllInLayer();

            void OnAddLayer();
            String queryLayerName();

            void OnRemoveLayer();
            bool canRemoveLayer() const;

            void OnShowAllLayers();
        private:
            Model::Layer* findVisibleAndUnlockedLayer(const Model::Layer* except) const;
            void moveSelectedNodesToLayer(MapDocumentSPtr document, Model::Layer* layer);
            void createGui();
            void updateButtons();
        };
    }
}

#endif /* defined(TrenchBroom_LayerEditor) */
