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

#include "LayerListView.h"

#include "Model/Layer.h"
#include "Model/Map.h"
#include "View/MapDocument.h"
#include "View/wxUtils.h"

namespace TrenchBroom {
    namespace View {
        LayerListView::LayerListView(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_VIRTUAL | wxLC_HRULES | wxLC_VRULES | wxBORDER_NONE),
        m_document(document),
        m_controller(controller) {
            AppendColumn("Name");
            AppendColumn("#");
            
            reload();
            bindObservers();
            bindEvents();
        }
        
        LayerListView::~LayerListView() {
            unbindObservers();
        }

        size_t LayerListView::getSelection() const {
            assert(GetSelectedItemCount() > 0);
            return getListCtrlSelection(this).front();
        }
        
        void LayerListView::OnSize(wxSizeEvent& event) {
            const int newWidth = std::max(1, GetClientSize().x - GetColumnWidth(1));
            SetColumnWidth(0, newWidth);
            event.Skip();
        }

        wxListItemAttr* LayerListView::OnGetItemAttr(const long item) const {
            const Model::Map* map = lock(m_document)->map();
            const Model::LayerList& layers = map->layers();
            _UNUSED(layers);
            assert(item >= 0 && static_cast<size_t>(item) < layers.size());
            
            static wxListItemAttr attr;
            if (item == 0) {
                attr.SetFont(GetFont().Italic());
                return &attr;
            }
            
            return NULL;
        }
        
        wxString LayerListView::OnGetItemText(const long item, const long column) const {
            const Model::Map* map = lock(m_document)->map();
            const Model::LayerList& layers = map->layers();
            assert(item >= 0 && static_cast<size_t>(item) < layers.size());
            const Model::Layer* layer = layers[static_cast<size_t>(item)];
            
            assert(column >= 0 && column < 2);
            wxString result;
            if (column == 0)
                result << layer->name();
            else
                result << layer->objects().size();
            return result;
        }
        
        void LayerListView::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &LayerListView::documentWasChanged);
            document->documentWasLoadedNotifier.addObserver(this, &LayerListView::documentWasChanged);
            document->layersWereAddedNotifier.addObserver(this, &LayerListView::layersWereAdded);
            document->layersWereRemovedNotifier.addObserver(this, &LayerListView::layersWereRemoved);
            document->layersDidChangeNotifier.addObserver(this, &LayerListView::layersDidChange);
        }
        
        void LayerListView::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &LayerListView::documentWasChanged);
                document->documentWasLoadedNotifier.removeObserver(this, &LayerListView::documentWasChanged);
                document->layersWereAddedNotifier.removeObserver(this, &LayerListView::layersWereAdded);
                document->layersWereRemovedNotifier.removeObserver(this, &LayerListView::layersWereRemoved);
                document->layersDidChangeNotifier.removeObserver(this, &LayerListView::layersDidChange);
            }
        }
        
        void LayerListView::documentWasChanged() {
            reload();
        }
        
        void LayerListView::layersWereAdded(const Model::LayerList& layers) {
            reload();
        }
        
        void LayerListView::layersWereRemoved(const Model::LayerList& layers) {
            reload();
        }
        
        void LayerListView::layersDidChange(const Model::LayerList& layers) {
            reload();
        }

        void LayerListView::bindEvents() {
            Bind(wxEVT_SIZE, &LayerListView::OnSize, this);
        }

        void LayerListView::reload() {
            const Model::Map* map = lock(m_document)->map();
            if (map == NULL) {
                SetItemCount(0);
            } else {
                const Model::LayerList& layers = map->layers();
                SetItemCount(static_cast<long>(layers.size()));
            }
            Refresh();
        }
    }
}
