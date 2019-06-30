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

#include "ThreePaneMapView.h"

#include "Model/PointFile.h"
#include "Renderer/Camera.h"
#include "View/CyclingMapView.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapView2D.h"
#include "View/MapView3D.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QSettings>

namespace TrenchBroom {
    namespace View {
        const char* ThreePaneMapView::HSaveStateKey = "3PaneMapViewHSplitter";
        const char* ThreePaneMapView::VSaveStateKey = "3PaneMapViewVSplitter";

        ThreePaneMapView::ThreePaneMapView(QWidget* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager) :
        MultiMapView(parent),
        m_logger(logger),
        m_document(document),
        m_hSplitter(nullptr),
        m_vSplitter(nullptr),
        m_mapView3D(nullptr),
        m_mapViewXY(nullptr),
        m_mapViewZZ(nullptr) {
            createGui(toolBox, mapRenderer, contextManager);
        }

        ThreePaneMapView::~ThreePaneMapView() {
            saveLayoutToPrefs();
        }

        void ThreePaneMapView::createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager) {
            m_hSplitter = new QSplitter();
            m_vSplitter = new QSplitter(Qt::Vertical);

            m_mapView3D = new MapView3D(nullptr, m_logger, m_document, toolBox, mapRenderer, contextManager);
            m_mapViewXY = new MapView2D(nullptr, m_logger, m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_XY);
            m_mapViewZZ = new CyclingMapView(nullptr, m_logger, m_document, toolBox, mapRenderer, contextManager, CyclingMapView::View_ZZ);

            m_mapView3D->linkCamera(m_linkHelper);
            m_mapViewXY->linkCamera(m_linkHelper);
            m_mapViewZZ->linkCamera(m_linkHelper);

            addMapView(m_mapView3D);
            addMapView(m_mapViewXY);
            addMapView(m_mapViewZZ);

            // See comment in CyclingMapView::createGui
            auto* layout = new QHBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            setLayout(layout);
            layout->addWidget(m_hSplitter);

            // Add splitter children
            m_vSplitter->addWidget(m_mapViewXY->widgetContainer());
            m_vSplitter->addWidget(m_mapViewZZ);

            m_hSplitter->addWidget(m_mapView3D->widgetContainer());
            m_hSplitter->addWidget(m_vSplitter);

            // Configure minimum child sizes and initial splitter position at 50%
            m_mapViewXY->widgetContainer()->setMinimumSize(100, 100);
            m_mapViewZZ->setMinimumSize(100, 100);
            m_mapView3D->widgetContainer()->setMinimumSize(100, 100);

            m_hSplitter->setSizes(QList<int>{1, 1});
            m_vSplitter->setSizes(QList<int>{1, 1});

            // Load from preferences
            QSettings settings;
            m_hSplitter->restoreState(settings.value(HSaveStateKey).toByteArray());
            m_vSplitter->restoreState(settings.value(VSaveStateKey).toByteArray());
        }

        void ThreePaneMapView::saveLayoutToPrefs() {
            QSettings settings;
            settings.setValue(HSaveStateKey, m_hSplitter->saveState());
            settings.setValue(VSaveStateKey, m_vSplitter->saveState());
        }

        void ThreePaneMapView::doMaximizeView(MapView* view) {
            assert(view == m_mapView3D || view == m_mapViewXY || view == m_mapViewZZ);
            if (view == m_mapView3D) {
                m_vSplitter->hide();
            } else if (view == m_mapViewXY) {
                m_mapViewZZ->hide();
                m_mapView3D->widgetContainer()->hide();
            } else if (view == m_mapViewZZ) {
                m_mapViewXY->widgetContainer()->hide();
                m_mapView3D->widgetContainer()->hide();
            }
        }

        void ThreePaneMapView::doRestoreViews() {
            for (int i=0; i<2; ++i) {
                m_hSplitter->widget(i)->show();
                m_vSplitter->widget(i)->show();
            }
        }
    }
}
