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

#include "Renderer/Renderer_Forward.h"
#include "View/View_Forward.h"

#include <memory>

#include <QWidget>

namespace TrenchBroom {
    namespace View {
        enum class InspectorPage {
            Map = 0,
            Entity = 1,
            Face = 2
        };

        class Inspector : public QWidget {
            Q_OBJECT
        private:
            TabBook* m_tabBook;
            MapInspector* m_mapInspector;
            EntityInspector* m_entityInspector;
            FaceInspector* m_faceInspector;

            QWidget* m_topWidgetMaster;
        public:
            Inspector(std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent = nullptr);
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
