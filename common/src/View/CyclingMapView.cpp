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

#include "CyclingMapView.h"

#include "TrenchBroom.h"
#include "Model/Brush.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapView2D.h"
#include "View/MapView3D.h"

#include <vecmath/scalar.h>

#include <QStackedLayout>
#include <QShortcut>

namespace TrenchBroom {
    namespace View {
        CyclingMapView::CyclingMapView(MapDocumentWPtr document, MapViewToolBox& toolBox,
                                       Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager,
                                       const View views, Logger* logger, QWidget* parent) :
        MapViewContainer(parent),
        m_logger(logger),
        m_document(std::move(document)),
        m_currentMapView(nullptr),
        m_layout(nullptr) {
            setObjectName("CyclingMapView");
            createGui(toolBox, mapRenderer, contextManager, views);
        }

        void CyclingMapView::createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager, const View views) {
            if (views & View_3D) {
                addMapView(new MapView3D(m_document, toolBox, mapRenderer, contextManager, m_logger));
            }
            if (views & View_XY) {
                addMapView(new MapView2D(m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_XY, m_logger));
            }
            if (views & View_XZ) {
                addMapView(new MapView2D(m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_XZ, m_logger));
            }
            if (views & View_YZ) {
                addMapView(new MapView2D(m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_YZ, m_logger));
            }

            m_layout = new QStackedLayout();
            // NOTE: It's important to setLayout() before adding widgets, rather than after. Otherwise, they
            // get setVisible immediately (and the first render calls happen during the for loop),
            // which breaks multisampling
            setLayout(m_layout);

            for (size_t i = 0; i < m_mapViews.size(); ++i) {
                m_layout->addWidget(m_mapViews[i]->widgetContainer());
            }

            assert(!m_mapViews.empty());
            switchToMapView(m_mapViews[0]);
        }

        void CyclingMapView::addMapView(MapViewBase* mapView) {
            m_mapViews.push_back(mapView);
            mapView->setContainer(this);
        }

        void CyclingMapView::switchToMapView(MapViewBase* mapView) {
            m_currentMapView = mapView;

            m_layout->setCurrentWidget(m_currentMapView->widgetContainer());
            m_currentMapView->requestActivate();
        }

        void CyclingMapView::doFlashSelection() {
            m_currentMapView->flashSelection();
        }

        bool CyclingMapView::doGetIsCurrent() const {
            return m_currentMapView->isCurrent();
        }

        MapViewBase* CyclingMapView::doGetFirstMapViewBase() {
            return m_currentMapView;
        }

        bool CyclingMapView::doCanSelectTall() {
            return m_currentMapView->canSelectTall();
        }

        void CyclingMapView::doSelectTall() {
            m_currentMapView->selectTall();
        }

        void CyclingMapView::doFocusCameraOnSelection(const bool animate) {
            m_currentMapView->focusCameraOnSelection(animate);
        }

        void CyclingMapView::doMoveCameraToPosition(const vm::vec3& position, const bool animate) {
            m_currentMapView->moveCameraToPosition(position, animate);
        }

        void CyclingMapView::doMoveCameraToCurrentTracePoint() {
            for (size_t i = 0; i < m_mapViews.size(); ++i) {
                m_mapViews[i]->moveCameraToCurrentTracePoint();
            }
        }

        bool CyclingMapView::doCanMaximizeCurrentView() const {
            return false;
        }

        bool CyclingMapView::doCurrentViewMaximized() const {
            return true;
        }

        void CyclingMapView::doToggleMaximizeCurrentView() {}

        MapView* CyclingMapView::doGetCurrentMapView() const {
            return m_currentMapView;
        }

        void CyclingMapView::doRefreshViews() {
            for (MapView* mapView : m_mapViews) {
                mapView->refreshViews();
            }
        }

        void CyclingMapView::doLinkCamera(CameraLinkHelper& helper) {
            for (size_t i = 0; i < m_mapViews.size(); ++i) {
                m_mapViews[i]->linkCamera(helper);
            }
        }

        void CyclingMapView::doCycleMapView() {
            for (size_t i = 0; i < m_mapViews.size(); ++i) {
                if (m_currentMapView == m_mapViews[i]) {
                    switchToMapView(m_mapViews[vm::succ(i, m_mapViews.size())]);
                    focusCameraOnSelection(false);
                    break;
                }
            }
        }

        void CyclingMapView::doUpdateLastActivation(const bool active) {
            for (size_t i = 0; i < m_mapViews.size(); ++i) {
                m_mapViews[i]->MapView::updateLastActivation(active);
            }
        }

        bool CyclingMapView::doCancelMouseDrag() {
            auto result = false;
            for (size_t i = 0; i < m_mapViews.size(); ++i) {
                result |= m_mapViews[i]->cancelMouseDrag();
            }
            return result;
        }
    }
}
