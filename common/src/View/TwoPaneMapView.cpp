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

#include "TwoPaneMapView.h"

#include "Model/PointFile.h"
#include "Renderer/Camera.h"
#include "View/CyclingMapView.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapView3D.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QSettings>

namespace TrenchBroom {
    namespace View {
        const char* TwoPaneMapView::SaveStateKey = "2PaneMapViewHSplitter";

        TwoPaneMapView::TwoPaneMapView(MapDocumentWPtr document, MapViewToolBox& toolBox,
                                       Renderer::MapRenderer& mapRenderer,
                                       GLContextManager& contextManager, Logger* logger, QWidget* parent) :
        MultiMapView(parent),
        m_logger(logger),
        m_document(document),
        m_splitter(nullptr),
        m_mapView3D(nullptr),
        m_mapView2D(nullptr) {
            createGui(toolBox, mapRenderer, contextManager);
        }

        TwoPaneMapView::~TwoPaneMapView() {
            saveLayoutToPrefs();
        }

        void TwoPaneMapView::createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager) {

            // See comment in CyclingMapView::createGui
            m_splitter = new QSplitter();
            QHBoxLayout* layout = new QHBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            setLayout(layout);
            layout->addWidget(m_splitter);

            m_mapView3D = new MapView3D(m_document, toolBox, mapRenderer, contextManager, m_logger, nullptr);
            m_mapView2D = new CyclingMapView(m_document, toolBox, mapRenderer, contextManager,
                CyclingMapView::View_2D, m_logger, nullptr);

            m_mapView3D->linkCamera(m_linkHelper);
            m_mapView2D->linkCamera(m_linkHelper);

            addMapView(m_mapView3D);
            addMapView(m_mapView2D);

            m_splitter->addWidget(m_mapView3D->widgetContainer());
            m_splitter->addWidget(m_mapView2D);

            // Configure minimum child sizes and initial splitter position at 50%
            m_mapView2D->setMinimumSize(100, 100);
            m_mapView3D->widgetContainer()->setMinimumSize(100, 100);
            m_splitter->setSizes(QList<int>{1, 1});

            // Load from preferences
            QSettings settings;
            m_splitter->restoreState(settings.value(SaveStateKey).toByteArray());
        }

        void TwoPaneMapView::saveLayoutToPrefs() {
            QSettings settings;
            settings.setValue(SaveStateKey, m_splitter->saveState());
        }

        void TwoPaneMapView::doMaximizeView(MapView* view) {
            assert(view == m_mapView2D || view == m_mapView3D);
            if (view == m_mapView2D) {
                m_mapView2D->hide();
            }
            if (view == m_mapView3D) {
                m_mapView3D->widgetContainer()->hide();
            }
        }

        void TwoPaneMapView::doRestoreViews() {
            m_mapView3D->widgetContainer()->show();
            m_mapView2D->show();
        }
    }
}
