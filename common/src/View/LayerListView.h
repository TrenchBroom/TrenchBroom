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

#ifndef TrenchBroom_LayerListView
#define TrenchBroom_LayerListView

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
#define LayerSelectedHandler(func) wxEVENT_HANDLER_CAST(LayerCommandFunction, func)

wxDECLARE_EVENT(LAYER_SET_CURRENT_EVENT, TrenchBroom::View::LayerCommand);
#define LayerSetCurrentHandler(func) wxEVENT_HANDLER_CAST(LayerCommandFunction, func)

wxDECLARE_EVENT(LAYER_RIGHT_CLICK_EVENT, TrenchBroom::View::LayerCommand);
#define LayerRightClickHandler(func) wxEVENT_HANDLER_CAST(LayerCommandFunction, func)

wxDECLARE_EVENT(LAYER_TOGGLE_VISIBLE_EVENT, TrenchBroom::View::LayerCommand);
#define LayerToggleVisibleHandler(func) wxEVENT_HANDLER_CAST(LayerCommandFunction, func)

wxDECLARE_EVENT(LAYER_TOGGLE_LOCKED_EVENT, TrenchBroom::View::LayerCommand);
#define LayerToggleLockedHandler(func) wxEVENT_HANDLER_CAST(LayerCommandFunction, func)

namespace TrenchBroom {
    namespace View {
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
            class LayerEntry;

            typedef std::vector<LayerEntry*> LayerEntryList;

            MapDocumentWPtr m_document;

            wxScrolledWindow* m_scrollWindow;
            LayerEntryList m_entries;

            int m_selection;
        public:
            LayerListView(wxWindow* parent, MapDocumentWPtr document);
            ~LayerListView();

            Model::Layer* selectedLayer() const;
            void setSelectedLayer(Model::Layer* layer);

            void OnMouseEntryDown(wxMouseEvent& event);
            void OnMouseEntryDClick(wxMouseEvent& event);
            void OnMouseEntryRightUp(wxMouseEvent& event);
            void OnMouseVoidDown(wxMouseEvent& event);
        private:
            void bindObservers();
            void unbindObservers();

            void documentDidChange(MapDocument* document);
            void nodesDidChange(const Model::NodeList& nodes);
            void currentLayerDidChange();

            void createGui();

            void reload();
            void refresh();
        };
    }
}

#endif /* defined(TrenchBroom_LayerListView) */
