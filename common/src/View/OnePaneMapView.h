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

#pragma once

#include "View/CameraLinkHelper.h"
#include "View/MultiMapView.h"

#include <memory>

namespace TrenchBroom {
    class Logger;

    namespace Renderer {
        class MapRenderer;
    }

    namespace View {
        class CyclingMapView;
        class GLContextManager;
        class MapDocument;
        class MapViewToolBox;

        class OnePaneMapView : public MultiMapView {
        private:
            Logger* m_logger;
            std::weak_ptr<MapDocument> m_document;

            CameraLinkHelper m_linkHelper;
            CyclingMapView* m_mapView;
        public:
            explicit OnePaneMapView(Logger* logger, std::weak_ptr<MapDocument> document, MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager, QWidget* parent = nullptr);
        private:
            void createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager);
        };
    }
}

#endif /* defined(TrenchBroom_OnePaneMapView) */
