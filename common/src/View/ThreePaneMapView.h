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

#ifndef TrenchBroom_ThreePaneMapView
#define TrenchBroom_ThreePaneMapView

#include "View/CameraLinkHelper.h"
#include "View/MultiMapView.h"

#include <memory>

class QSplitter;

namespace TrenchBroom {
    class Logger;

    namespace Renderer {
        class MapRenderer;
        class VboManager;
    }

    namespace View {
        class CyclingMapView;
        class GLContextManager;
        class MapDocument;
        class MapViewBase;
        class MapView2D;
        class MapView3D;
        class MapViewToolBox;

        class ThreePaneMapView : public MultiMapView {
            Q_OBJECT
        private:
            Logger* m_logger;
            std::weak_ptr<MapDocument> m_document;

            CameraLinkHelper m_linkHelper;
            QSplitter* m_hSplitter;
            QSplitter* m_vSplitter;
            MapView3D* m_mapView3D;
            MapView2D* m_mapViewXY;
            CyclingMapView* m_mapViewZZ;
        public:
            ThreePaneMapView(std::weak_ptr<MapDocument> document, MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer,
                             GLContextManager& contextManager, Logger* logger, QWidget* parent = nullptr);
            ~ThreePaneMapView() override;
        private:
            void createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager);
        private: // implement MultiMapView subclassing interface
            void doMaximizeView(MapView* view) override;
            void doRestoreViews() override;
        };
    }
}

#endif /* defined(TrenchBroom_ThreePaneMapView) */
