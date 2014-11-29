/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__LayerEditor__
#define __TrenchBroom__LayerEditor__

#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

namespace TrenchBroom {
    namespace View {
        class LayerCommand;
        class LayerListView;
        
        class LayerEditor : public wxPanel {
        private:
            static const int MoveSelectionToLayerCommandId = 1;
            static const int SelectAllInLayerCommandId = 2;
            static const int ToggleLayerVisibleCommandId = 3;
            static const int ToggleLayerLockedCommandId = 4;
            static const int RemoveLayerCommandId = 5;

            MapDocumentWPtr m_document;
            LayerListView* m_layerList;
        public:
            LayerEditor(wxWindow* parent, MapDocumentWPtr document);
        private:
            void OnCurrentLayerSelected(LayerCommand& event);
            void OnLayerRightClick(LayerCommand& event);

            class CollectMoveableNodes;
            void OnMoveSelectionToLayer(wxCommandEvent& event);
            void OnUpdateMoveSelectionToLayerUI(wxUpdateUIEvent& event);
            
            void OnToggleLayerVisibleFromMenu(wxCommandEvent& event);
            void OnToggleLayerVisibleFromList(LayerCommand& event);
            void toggleLayerVisible(Model::Layer* layer);
            
            void OnToggleLayerLockedFromMenu(wxCommandEvent& event);
            void OnToggleLayerLockedFromList(LayerCommand& event);
            void toggleLayerLocked(Model::Layer* layer);

            void OnSelectAllInLayer(wxCommandEvent& event);
            void OnAddLayer(wxCommandEvent& event);
            void OnRemoveLayer(wxCommandEvent& event);
            void OnUpdateRemoveLayerUI(wxUpdateUIEvent& event);
        private:
            void moveSelectedNodesToLayer(MapDocumentSPtr document, Model::Layer* layer);
            void createGui();
        };
    }
}

#endif /* defined(__TrenchBroom__LayerEditor__) */
