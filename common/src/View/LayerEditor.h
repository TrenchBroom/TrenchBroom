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

namespace TrenchBroom {
    namespace View {
        class LayerCommand;
        class LayerListBox;
        
        class LayerEditor : public QWidget {
            Q_OBJECT
        private:
            static const int MoveSelectionToLayerCommandId = 1;
            static const int SelectAllInLayerCommandId = 2;
            static const int ToggleLayerVisibleCommandId = 3;
            static const int ToggleLayerLockedCommandId = 4;
            static const int RemoveLayerCommandId = 5;

            MapDocumentWPtr m_document;
            LayerListBox* m_layerList;
        public:
            LayerEditor(QWidget* parent, MapDocumentWPtr document);
        private:
            void OnSetCurrentLayer(LayerCommand& event);
            void OnLayerRightClick(LayerCommand& event);

            class CollectMoveableNodes;
            void OnMoveSelectionToLayer();
            bool canMoveSelectionToLayer() const;
            
            void OnToggleLayerVisibleFromMenu();
            void OnToggleLayerVisibleFromList(LayerCommand& event);
            bool canToggleLayerVisible() const;
            void toggleLayerVisible(Model::Layer* layer);
            
            void OnToggleLayerLockedFromMenu();
            void OnToggleLayerLockedFromList(LayerCommand& event);
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
        };
    }
}

#endif /* defined(TrenchBroom_LayerEditor) */
