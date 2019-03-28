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

#ifndef TrenchBroom_LayerListBox
#define TrenchBroom_LayerListBox

#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <QWidget>

#include <vector>

class QListWidget;

namespace TrenchBroom {
    namespace View {
        class LayerListBox : public QWidget {
            Q_OBJECT
        private:
            MapDocumentWPtr m_document;
            QListWidget* m_listWidget;
        public:
            LayerListBox(QWidget* parent, MapDocumentWPtr document);
            ~LayerListBox() override;

            Model::Layer* selectedLayer() const;
            void setSelectedLayer(Model::Layer* layer);

            void OnSelectionChanged();
            void OnDoubleClick();
            void OnRightClick();
        signals:
            void LAYER_SELECTED_EVENT(Model::Layer* layer);
            void LAYER_SET_CURRENT_EVENT(Model::Layer* layer);
            void LAYER_RIGHT_CLICK_EVENT(Model::Layer* layer);
            void LAYER_TOGGLE_VISIBLE_EVENT(Model::Layer* layer);
            void LAYER_TOGGLE_LOCKED_EVENT(Model::Layer* layer);
        private:
            void bindObservers();
            void unbindObservers();

            void documentDidChange(MapDocument* document);
            void nodesDidChange(const Model::NodeList& nodes);
            void currentLayerDidChange(const Model::Layer* layer);

            void rebuildList();
            void bindEvents();
//        private:
//            Item* createItem(QWidget* parent, const wxSize& margins, size_t index) override;
        };
    }
}

#endif /* defined(TrenchBroom_LayerListBox) */
