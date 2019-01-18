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

#include "FourPaneMapView.h"

#include "Renderer/Camera.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapView2D.h"
#include "View/MapView3D.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QSettings>

namespace TrenchBroom {
    namespace View {
        const char* FourPaneMapView::HSaveStateKey = "4PaneMapViewHSplitter";
        const char* FourPaneMapView::VSaveStateKey = "4PaneMapViewVSplitter";

        FourPaneMapView::FourPaneMapView(QWidget* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager) :
        MultiMapView(parent),
        m_logger(logger),
        m_document(document),
        m_mapView3D(nullptr),
        m_mapViewXY(nullptr),
        m_mapViewXZ(nullptr),
        m_mapViewYZ(nullptr) {
            createGui(toolBox, mapRenderer, contextManager);
        }

        FourPaneMapView::~FourPaneMapView() {
            saveLayoutToPrefs();
        }
        
        void FourPaneMapView::createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager) {
            m_hSplitter = new QSplitter();
            m_leftVSplitter = new QSplitter(Qt::Vertical);
            m_rightVSplitter = new QSplitter(Qt::Vertical);
            
            m_mapView3D = new MapView3D(nullptr, m_logger, m_document, toolBox, mapRenderer, contextManager);
            m_mapViewXY = new MapView2D(nullptr, m_logger, m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_XY);
            m_mapViewXZ = new MapView2D(nullptr, m_logger, m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_XZ);
            m_mapViewYZ = new MapView2D(nullptr, m_logger, m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_YZ);
            
            m_mapView3D->linkCamera(m_linkHelper);
            m_mapViewXY->linkCamera(m_linkHelper);
            m_mapViewXZ->linkCamera(m_linkHelper);
            m_mapViewYZ->linkCamera(m_linkHelper);
            
            addMapView(m_mapView3D);
            addMapView(m_mapViewXY);
            addMapView(m_mapViewXZ);
            addMapView(m_mapViewYZ);

            // See comment in CyclingMapView::createGui
            auto* layout = new QHBoxLayout();
            setLayout(layout);
            layout->addWidget(m_hSplitter);

            // left and right columns
            m_hSplitter->addWidget(m_leftVSplitter);
            m_hSplitter->addWidget(m_rightVSplitter);

            // add children
            m_leftVSplitter->addWidget(m_mapView3D->widgetContainer());
            m_leftVSplitter->addWidget(m_mapViewYZ->widgetContainer());

            m_rightVSplitter->addWidget(m_mapViewXY->widgetContainer());
            m_rightVSplitter->addWidget(m_mapViewXZ->widgetContainer());

            // Configure minimum child sizes and initial splitter position at 50%
            m_mapView3D->widgetContainer()->setMinimumSize(100, 100);
            m_mapViewYZ->widgetContainer()->setMinimumSize(100, 100);
            m_mapViewXY->widgetContainer()->setMinimumSize(100, 100);
            m_mapViewXZ->widgetContainer()->setMinimumSize(100, 100);

            m_hSplitter->setSizes(QList<int>{1, 1});
            m_leftVSplitter->setSizes(QList<int>{1, 1});
            m_rightVSplitter->setSizes(QList<int>{1, 1});

            // Load from preferences
            QSettings settings;
            m_hSplitter->restoreState(settings.value(HSaveStateKey).toByteArray());
            m_leftVSplitter->restoreState(settings.value(VSaveStateKey).toByteArray());
            m_rightVSplitter->restoreState(settings.value(VSaveStateKey).toByteArray());

            connect(m_leftVSplitter, &QSplitter::splitterMoved, this, &FourPaneMapView::onSplitterMoved);
            connect(m_rightVSplitter, &QSplitter::splitterMoved, this, &FourPaneMapView::onSplitterMoved);
        }

        void FourPaneMapView::saveLayoutToPrefs() {
            QSettings settings;
            settings.setValue(HSaveStateKey, m_hSplitter->saveState());
            settings.setValue(VSaveStateKey, m_leftVSplitter->saveState());
        }

        void FourPaneMapView::onSplitterMoved(int pos, int index) {
            QSplitter* moved = qobject_cast<QSplitter*>(QObject::sender());
            QSplitter* other = (moved == m_leftVSplitter) ? m_rightVSplitter : m_leftVSplitter;

            assert(index == 1);
            assert(moved == m_leftVSplitter || moved == m_rightVSplitter);

            other->setSizes(moved->sizes());
        }

        void FourPaneMapView::doMaximizeView(MapView* view) {
            assert(view == m_mapView3D || view == m_mapViewXY || view == m_mapViewXZ || view == m_mapViewYZ);

            QWidget* viewAsWidget = dynamic_cast<MapViewBase*>(view)->widgetContainer();
            assert(viewAsWidget != nullptr);

            const bool inLeft = m_leftVSplitter->isAncestorOf(viewAsWidget);
            if (inLeft) {
                m_rightVSplitter->hide();

                if (m_leftVSplitter->widget(0) == viewAsWidget) {
                    m_leftVSplitter->widget(1)->hide();
                } else {
                    m_leftVSplitter->widget(0)->hide();
                }
            } else {
                m_leftVSplitter->hide();

                if (m_rightVSplitter->widget(0) == viewAsWidget) {
                    m_rightVSplitter->widget(1)->hide();
                } else {
                    m_rightVSplitter->widget(0)->hide();
                }
            }
        }
        
        void FourPaneMapView::doRestoreViews() {
            for (int i=0; i<2; ++i) {
                m_hSplitter->widget(i)->show();
                m_leftVSplitter->widget(i)->show();
                m_rightVSplitter->widget(i)->show();
            }
        }
    }
}
