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

#ifndef TrenchBroom_Inspector
#define TrenchBroom_Inspector

#include "View/ViewTypes.h"

#include <QWidget>

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }

    namespace View {
        class EntityInspector;
        class FaceInspector;
        class GLContextManager;
        class MapInspector;
        class TabBook;

        class Inspector : public QWidget {
            Q_OBJECT
        public:
            typedef enum {
                InspectorPage_Map = 0,
                InspectorPage_Entity = 1,
                InspectorPage_Face = 2
            } InspectorPage;

        private:
            TabBook* m_tabBook;
            MapInspector* m_mapInspector;
            EntityInspector* m_entityInspector;
            FaceInspector* m_faceInspector;

            QWidget* m_topWidgetMaster;
        public:
            Inspector(MapDocumentWPtr document, GLContextManager& contextManager, QWidget* parent = nullptr);
            void connectTopWidgets(QWidget* master);
            void switchToPage(InspectorPage page);
            bool cancelMouseDrag();
        private:
            /**
             * Event filter used to capture resize events of the top widget, used to synchronize the inspectors tab bar
             * height to the height of the map view's top bar.
             *
             * @param target the target that receives an event
             * @param event the event
             * @return true if the event should not be propagated to the target and false otherwise
             */
            bool eventFilter(QObject* target, QEvent* event) override;
        };
    }
}

#endif /* defined(TrenchBroom_Inspector) */
