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

#include "CyclingMapView.h"

#include "Model/Brush.h"
#include "Renderer/Camera.h"
#include "View/CommandIds.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapView2D.h"
#include "View/MapView3D.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        CyclingMapView::CyclingMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager, const View views) :
        MapViewContainer(parent),
        m_logger(logger),
        m_document(document),
        m_currentMapView(NULL) {
            createGui(toolBox, mapRenderer, contextManager, views);
            bindEvents();
        }

        void CyclingMapView::createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager, const View views) {
            if (views & View_3D)
                m_mapViews.push_back(new MapView3D(this, m_logger, m_document, toolBox, mapRenderer, contextManager));
            if (views & View_XY)
                m_mapViews.push_back(new MapView2D(this, m_logger, m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_XY));
            if (views & View_XZ)
                m_mapViews.push_back(new MapView2D(this, m_logger, m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_XZ));
            if (views & View_YZ)
                m_mapViews.push_back(new MapView2D(this, m_logger, m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_YZ));
            
            for (size_t i = 0; i < m_mapViews.size(); ++i)
                m_mapViews[i]->Hide();
            
            assert(!m_mapViews.empty());
            switchToMapView(m_mapViews[0]);
        }

        void CyclingMapView::bindEvents() {
            Bind(wxEVT_MENU, &CyclingMapView::OnCycleMapView, this, CommandIds::Actions::CycleMapViews);
        }

        void CyclingMapView::OnCycleMapView(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            for (size_t i = 0; i < m_mapViews.size(); ++i) {
                if (m_currentMapView == m_mapViews[i]) {
                    switchToMapView(m_mapViews[Math::succ(i, m_mapViews.size())]);
                    break;
                }
            }
        }

        void CyclingMapView::switchToMapView(MapViewBase* mapView) {
            mapView->Show();
            mapView->SetFocus();

            if (m_currentMapView != NULL)
                m_currentMapView->Hide();
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(mapView, 1, wxEXPAND);
            SetSizer(sizer);
            Layout();

            m_currentMapView = mapView;
        }

        void CyclingMapView::doFlashSelection() {
            m_currentMapView->flashSelection();
        }

        bool CyclingMapView::doGetIsCurrent() const {
            return m_currentMapView->isCurrent();
        }

        void CyclingMapView::doSetToolBoxDropTarget() {
            for (size_t i = 0; i < m_mapViews.size(); ++i)
                m_mapViews[i]->setToolBoxDropTarget();
        }
        
        void CyclingMapView::doClearDropTarget() {
            for (size_t i = 0; i < m_mapViews.size(); ++i)
                m_mapViews[i]->clearDropTarget();
        }

        bool CyclingMapView::doCanSelectTall() {
            return m_currentMapView->canSelectTall();
        }
        
        void CyclingMapView::doSelectTall() {
            m_currentMapView->selectTall();
        }

        void CyclingMapView::doFocusCameraOnSelection() {
            m_currentMapView->focusCameraOnSelection();
        }
        
        void CyclingMapView::doMoveCameraToPosition(const Vec3& position) {
            m_currentMapView->moveCameraToPosition(position);
        }
        
        void CyclingMapView::doMoveCameraToCurrentTracePoint() {
            for (size_t i = 0; i < m_mapViews.size(); ++i)
                m_mapViews[i]->moveCameraToCurrentTracePoint();
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

        void CyclingMapView::doLinkCamera(CameraLinkHelper& helper) {
            for (size_t i = 0; i < m_mapViews.size(); ++i)
                m_mapViews[i]->linkCamera(helper);
        }
    }
}
