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

#include "SwitchableMapViewContainer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/PointFile.h"
#include "Renderer/MapRenderer.h"
#include "View/CyclingMapView.h"
#include "View/TwoPaneMapView.h"
#include "View/ThreePaneMapView.h"
#include "View/FourPaneMapView.h"
#include "View/GLContextManager.h"
#include "View/Inspector.h"
#include "View/MapDocument.h"
#include "View/MapViewContainer.h"
#include "View/MapViewBar.h"
#include "View/MapViewToolBox.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        SwitchableMapViewContainer::SwitchableMapViewContainer(wxWindow* parent, Logger* logger, MapDocumentWPtr document, GLContextManager& contextManager) :
        wxPanel(parent),
        m_logger(logger),
        m_document(document),
        m_contextManager(contextManager),
        m_mapViewBar(new MapViewBar(this, m_document)),
        m_toolBox(new MapViewToolBox(m_document, m_mapViewBar->toolBook())),
        m_mapRenderer(new Renderer::MapRenderer(m_document)),
        m_mapView(NULL) {
            switchToMapView(static_cast<MapViewLayout>(pref(Preferences::MapViewLayout)));
            bindObservers();
        }
        
        SwitchableMapViewContainer::~SwitchableMapViewContainer() {
            unbindObservers();
            
            // we must destroy our children before we destroy our resources because they might still use them in their destructors
            DestroyChildren();
            
            delete m_toolBox;
            m_toolBox = NULL;
            
            delete m_mapRenderer;
            m_mapRenderer = NULL;
        }

        void SwitchableMapViewContainer::connectTopWidgets(Inspector* inspector) {
            inspector->connectTopWidgets(m_mapViewBar);
        }

        bool SwitchableMapViewContainer::viewportHasFocus() const {
            return m_mapView != NULL && m_mapView->isCurrent();
        }

        void SwitchableMapViewContainer::switchToMapView(const MapViewLayout viewId) {
            if (m_mapView != NULL) {
                m_mapView->Destroy();
                m_mapView = NULL;
            }

            switch (viewId) {
                case MapViewLayout_1Pane:
                    m_mapView = new CyclingMapView(this, m_logger, m_document, *m_toolBox, *m_mapRenderer, m_contextManager, CyclingMapView::View_ALL);
                    break;
                case MapViewLayout_2Pane:
                    m_mapView = new TwoPaneMapView(this, m_logger, m_document, *m_toolBox, *m_mapRenderer, m_contextManager);
                    break;
                case MapViewLayout_3Pane:
                    m_mapView = new ThreePaneMapView(this, m_logger, m_document, *m_toolBox, *m_mapRenderer, m_contextManager);
                    break;
                case MapViewLayout_4Pane:
                    m_mapView = new FourPaneMapView(this, m_logger, m_document, *m_toolBox, *m_mapRenderer, m_contextManager);
                    break;
            }
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_mapViewBar, 0, wxEXPAND);
            sizer->Add(m_mapView, 1, wxEXPAND);
            SetSizer(sizer);
            Layout();
            
            m_mapView->SetFocus();
        }

        bool SwitchableMapViewContainer::anyToolActive() const {
            return createComplexBrushToolActive() || clipToolActive() || rotateObjectsToolActive() || vertexToolActive();
        }

        void SwitchableMapViewContainer::deactivateTool() {
            m_toolBox->deactivateAllTools();
        }

        bool SwitchableMapViewContainer::createComplexBrushToolActive() const {
            return m_toolBox->createComplexBrushToolActive();
        }

        bool SwitchableMapViewContainer::canToggleCreateComplexBrushTool() const {
            return true;
        }

        void SwitchableMapViewContainer::toggleCreateComplexBrushTool() {
            m_toolBox->toggleCreateComplexBrushTool();
        }
        
        bool SwitchableMapViewContainer::clipToolActive() const {
            return m_toolBox->clipToolActive();
        }

        bool SwitchableMapViewContainer::canToggleClipTool() const {
            return clipToolActive() || lock(m_document)->selectedNodes().hasOnlyBrushes();
        }

        void SwitchableMapViewContainer::toggleClipTool() {
            m_toolBox->toggleClipTool();
        }
        
        bool SwitchableMapViewContainer::rotateObjectsToolActive() const {
            return m_toolBox->rotateObjectsToolActive();
        }

        bool SwitchableMapViewContainer::canToggleRotateObjectsTool() const {
            return rotateObjectsToolActive() || lock(m_document)->hasSelectedNodes();
        }

        void SwitchableMapViewContainer::toggleRotateObjectsTool() {
            m_toolBox->toggleRotateObjectsTool();
        }
        
        bool SwitchableMapViewContainer::vertexToolActive() const {
            return m_toolBox->vertexToolActive();
        }

        bool SwitchableMapViewContainer::canToggleVertexTool() const {
            return vertexToolActive() || lock(m_document)->selectedNodes().hasOnlyBrushes();
        }

        void SwitchableMapViewContainer::toggleVertexTool() {
            m_toolBox->toggleVertexTool();
        }

        bool SwitchableMapViewContainer::canMoveCameraToNextTracePoint() const {
            MapDocumentSPtr document = lock(m_document);
            if (!document->isPointFileLoaded())
                return false;
            
            Model::PointFile* pointFile = document->pointFile();
            return pointFile->hasNextPoint();
        }
        
        bool SwitchableMapViewContainer::canMoveCameraToPreviousTracePoint() const {
            MapDocumentSPtr document = lock(m_document);
            if (!document->isPointFileLoaded())
                return false;
            
            Model::PointFile* pointFile = document->pointFile();
            return pointFile->hasPreviousPoint();
        }
        
        void SwitchableMapViewContainer::moveCameraToNextTracePoint() {
            MapDocumentSPtr document = lock(m_document);
            assert(document->isPointFileLoaded());
            
            m_mapView->moveCameraToCurrentTracePoint();
            
            Model::PointFile* pointFile = document->pointFile();
            pointFile->advance();
        }
        
        void SwitchableMapViewContainer::moveCameraToPreviousTracePoint() {
            MapDocumentSPtr document = lock(m_document);
            assert(document->isPointFileLoaded());
            
            Model::PointFile* pointFile = document->pointFile();
            pointFile->retreat();

            m_mapView->moveCameraToCurrentTracePoint();
        }

        bool SwitchableMapViewContainer::canMaximizeCurrentView() const {
            return m_mapView->canMaximizeCurrentView();
        }
        
        bool SwitchableMapViewContainer::currentViewMaximized() const {
            return m_mapView->currentViewMaximized();
        }
        
        void SwitchableMapViewContainer::toggleMaximizeCurrentView() {
            m_mapView->toggleMaximizeCurrentView();
        }

        void SwitchableMapViewContainer::bindObservers() {
            m_toolBox->refreshViewsNotifier.addObserver(this, &SwitchableMapViewContainer::refreshViews);
        }
        
        void SwitchableMapViewContainer::unbindObservers() {
            m_toolBox->refreshViewsNotifier.removeObserver(this, &SwitchableMapViewContainer::refreshViews);
        }

        void SwitchableMapViewContainer::refreshViews(Tool* tool) {
            m_mapView->Refresh();
        }

        bool SwitchableMapViewContainer::doGetIsCurrent() const {
            return m_mapView->isCurrent();
        }
        
        void SwitchableMapViewContainer::doSetToolBoxDropTarget() {
            m_mapView->setToolBoxDropTarget();
        }
        
        void SwitchableMapViewContainer::doClearDropTarget() {
            m_mapView->clearDropTarget();
        }
        
        bool SwitchableMapViewContainer::doCanSelectTall() {
            return m_mapView->canSelectTall();
        }
        
        void SwitchableMapViewContainer::doSelectTall() {
            m_mapView->selectTall();
        }

        bool SwitchableMapViewContainer::doCanFlipObjects() const {
            return m_mapView->canFlipObjects();
        }
        
        void SwitchableMapViewContainer::doFlipObjects(const Math::Direction direction) {
            m_mapView->flipObjects(direction);
        }
        
        Vec3 SwitchableMapViewContainer::doGetPasteObjectsDelta(const BBox3& bounds, const BBox3& referenceBounds) const {
            return m_mapView->pasteObjectsDelta(bounds, referenceBounds);
        }
        
        void SwitchableMapViewContainer::doFocusCameraOnSelection() {
            m_mapView->focusCameraOnSelection();
        }
        
        void SwitchableMapViewContainer::doMoveCameraToPosition(const Vec3& position) {
            m_mapView->moveCameraToPosition(position);
        }
        
        void SwitchableMapViewContainer::doMoveCameraToCurrentTracePoint() {
            m_mapView->moveCameraToCurrentTracePoint();
        }

        void SwitchableMapViewContainer::doFlashSelection() {
            m_mapView->flashSelection();
        }
    }
}
