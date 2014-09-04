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

#include "LayerObserver.h"

#include "Model/Map.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    LayerObserver::LayerObserver(View::MapDocumentWPtr document) :
    m_document(document) {
        addObservers();
        bindObservers();
    }

    LayerObserver::~LayerObserver() {
        unbindObservers();
        removeObservers();
    }

    void LayerObserver::bindObservers() {
        View::MapDocumentSPtr doc = lock(m_document);
        doc->documentWasNewedNotifier.addObserver(this, &LayerObserver::documentWasNewedOrLoader);
        doc->documentWasLoadedNotifier.addObserver(this, &LayerObserver::documentWasNewedOrLoader);
        doc->documentWillBeClearedNotifier.addObserver(this, &LayerObserver::documentWillBeCleared);
        doc->layersWereAddedNotifier.addObserver(this, &LayerObserver::layersWereAdded);
        doc->layersWillBeRemovedNotifier.addObserver(layersWillBeRemovedNotifier);
        doc->layersWereRemovedNotifier.addObserver(this, &LayerObserver::layersWereRemoved);
    }
    
    void LayerObserver::unbindObservers() {
        if (!expired(m_document)) {
            View::MapDocumentSPtr doc = lock(m_document);
            doc->documentWasNewedNotifier.removeObserver(this, &LayerObserver::documentWasNewedOrLoader);
            doc->documentWasLoadedNotifier.removeObserver(this, &LayerObserver::documentWasNewedOrLoader);
            doc->documentWillBeClearedNotifier.removeObserver(this, &LayerObserver::documentWillBeCleared);
            doc->layersWereAddedNotifier.removeObserver(this, &LayerObserver::layersWereAdded);
            doc->layersWillBeRemovedNotifier.removeObserver(layersWillBeRemovedNotifier);
            doc->layersWereRemovedNotifier.removeObserver(this, &LayerObserver::layersWereRemoved);
        }
    }
    
    void LayerObserver::documentWasNewedOrLoader() {
        addObservers();
    }
    
    void LayerObserver::documentWillBeCleared() {
        removeObservers();
    }
    
    void LayerObserver::layersWereAdded(const Model::LayerList& layers) {
        addObservers(layers);
        layersWereAddedNotifier(layers);
    }
    
    void LayerObserver::layersWereRemoved(const Model::LayerList& layers) {
        removeObservers(layers);
        layersWereRemovedNotifier(layers);
    }
    
    void LayerObserver::addObservers() {
        View::MapDocumentSPtr doc = lock(m_document);
        const Model::Map* map = doc->map();
        if (map != NULL) {
            addObservers(map->layers());
            layersWereAddedNotifier(map->layers());
        }
    }
    
    void LayerObserver::removeObservers() {
        if (!expired(m_document)) {
            View::MapDocumentSPtr doc = lock(m_document);
            const Model::Map* map = doc->map();
            if (map != NULL) {
                layersWillBeRemovedNotifier(map->layers());
                layersWereRemovedNotifier(map->layers());
                removeObservers(map->layers());
            }
        }
    }

    void LayerObserver::addObservers(const Model::LayerList& layers) {
        Model::LayerList::const_iterator it, end;
        for (it = layers.begin(), end = layers.end(); it != end; ++it) {
            Model::Layer* layer = *it;
            layer->layerDidChangeNotifier.addObserver(layerDidChangeNotifier);
        }
    }
    
    void LayerObserver::removeObservers(const Model::LayerList& layers) {
        Model::LayerList::const_iterator it, end;
        for (it = layers.begin(), end = layers.end(); it != end; ++it) {
            Model::Layer* layer = *it;
            layer->layerDidChangeNotifier.removeObserver(layerDidChangeNotifier);
        }
    }
}
