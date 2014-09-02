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

#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxListEvent;

namespace TrenchBroom {
    namespace View {
        class LayerListView;
        
        class LayerEditor : public wxPanel {
        private:
            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;
            
            LayerListView* m_layerList;
        public:
            LayerEditor(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller);
            
            void OnCurrentLayerSelected(wxListEvent& event);
            void OnCurrentLayerDeselected(wxListEvent& event);
            
            void OnAddLayerClicked(wxCommandEvent& event);
            void OnRemoveLayerClicked(wxCommandEvent& event);
            void OnUpdateRemoveLayerUI(wxUpdateUIEvent& event);
        private:
            void createGui();
        };
    }
}

#endif /* defined(__TrenchBroom__LayerEditor__) */
