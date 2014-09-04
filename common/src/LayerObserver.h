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

#ifndef __TrenchBroom__LayerObserver__
#define __TrenchBroom__LayerObserver__

#include "Notifier.h"
#include "Model/Layer.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    class LayerObserver {
    private:
        View::MapDocumentWPtr m_document;
    public:
        LayerObserver(View::MapDocumentWPtr document);
        ~LayerObserver();
        
        Notifier1<const Model::LayerList&> layersWereAddedNotifier;
        Notifier1<const Model::LayerList&> layersWillBeRemovedNotifier;
        Notifier1<const Model::LayerList&> layersWereRemovedNotifier;
        Notifier2<Model::Layer*, Model::Layer::Attr_Type> layerDidChangeNotifier;
    private:
        void bindObservers();
        void unbindObservers();
        
        void documentWasNewedOrLoader();
        void documentWillBeCleared();
        void layersWereAdded(const Model::LayerList& layers);
        void layersWereRemoved(const Model::LayerList& layers);

        void addObservers();
        void removeObservers();
        
        void addObservers(const Model::LayerList& layers);
        void removeObservers(const Model::LayerList& layers);
    };
}

#endif /* defined(__TrenchBroom__LayerObserver__) */
