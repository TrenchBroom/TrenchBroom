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

#ifndef __TrenchBroom__LayerListView__
#define __TrenchBroom__LayerListView__

#include "LayerObserver.h"
#include "Model/Layer.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

#include <vector>

class wxScrolledWindow;

namespace TrenchBroom {
    namespace View {
        class LayerCommand;
    }
}

typedef void (wxEvtHandler::*LayerCommandFunction)(TrenchBroom::View::LayerCommand &);

wxDECLARE_EVENT(LAYER_SELECTED_EVENT, TrenchBroom::View::LayerCommand);
#define LayeredSelectedHandler(func) wxEVENT_HANDLER_CAST(LayerCommandFunction, func)

wxDECLARE_EVENT(LAYER_RIGHT_CLICK_EVENT, TrenchBroom::View::LayerCommand);
#define LayeredRightClickHandler(func) wxEVENT_HANDLER_CAST(LayerCommandFunction, func)

namespace TrenchBroom {
    namespace View {
        class LayerEntry;
        
        class LayerCommand : public wxCommandEvent {
        protected:
            Model::Layer* m_layer;
        public:
            LayerCommand(wxEventType commandType, int id = 0);
            
            Model::Layer* layer() const;
            void setLayer(Model::Layer* layer);
            
            virtual wxEvent* Clone() const;
        };

        class LayerListView : public wxPanel {
        private:
            typedef std::vector<LayerEntry*> LayerEntryList;
            
            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;
            
            LayerObserver m_layerObserver;
            
            wxScrolledWindow* m_scrollWindow;
            LayerEntryList m_entries;
            
            int m_selection;
        public:
            LayerListView(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller);
            ~LayerListView();
            
            Model::Layer* selectedLayer() const;
            void setSelectedLayer(Model::Layer* layer);
            
            void OnMouseEntryDown(wxMouseEvent& event);
            void OnMouseEntryRightUp(wxMouseEvent& event);
            void OnMouseVoidDown(wxMouseEvent& event);
        private:
            void bindObservers();
            void unbindObservers();
            void layersWereAdded(const Model::LayerList& layers);
            void layersWereRemoved(const Model::LayerList& layers);
            void layerDidChange(Model::Layer* layer, Model::Layer::Attr_Type attr);

            void createGui();
            
            void reload();
            void refresh();
        };
    }
}

#endif /* defined(__TrenchBroom__LayerListView__) */
