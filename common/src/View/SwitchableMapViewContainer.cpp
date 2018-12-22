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

#include "SwitchableMapViewContainer.h"

#include "TrenchBroom.h"

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

#include <vecmath/scalar.h>

#include <wx/sizer.h>
#include <QGridLayout>

namespace TrenchBroom {
    namespace View {
        SwitchableMapViewContainer::SwitchableMapViewContainer(QWidget* parent, Logger* logger, MapDocumentWPtr document, GLContextManager& contextManager) :
        QWidget(parent),
        m_logger(logger),
        m_document(document),
        m_contextManager(contextManager),
        // FIXME: Port MapViewBar
//        m_mapViewBar(new MapViewBar(this, m_document)),
//        m_toolBox(new MapViewToolBox(m_document, m_mapViewBar->toolBook())),
        m_mapViewBar(nullptr),
        m_toolBox(new MapViewToolBox(m_document, nullptr)),
        m_mapRenderer(new Renderer::MapRenderer(m_document)),
        m_mapView(nullptr) {
            switchToMapView(static_cast<MapViewLayout>(pref(Preferences::MapViewLayout)));
            bindObservers();
        }
        
        SwitchableMapViewContainer::~SwitchableMapViewContainer() {
            unbindObservers();
            
            // we must destroy our children before we destroy our resources because they might still use them in their destructors
            delete m_mapView;
            
            delete m_toolBox;
            m_toolBox = nullptr;
            
            delete m_mapRenderer;
            m_mapRenderer = nullptr;
        }

        void SwitchableMapViewContainer::connectTopWidgets(Inspector* inspector) {
            inspector->connectTopWidgets(m_mapViewBar);
        }

        bool SwitchableMapViewContainer::viewportHasFocus() const {
            return m_mapView != nullptr && m_mapView->isCurrent();
        }

        void SwitchableMapViewContainer::switchToMapView(const MapViewLayout viewId) {
            if (m_mapView != nullptr) {
                delete m_mapView;
                m_mapView = nullptr;
            }

            switch (viewId) {
                case MapViewLayout_1Pane:
                    // FIXME: Can we pass nullptr as the parent here, since it's added to `this` down below?
                    m_mapView = new CyclingMapView(this, m_logger, m_document, *m_toolBox, *m_mapRenderer, m_contextManager, CyclingMapView::View_ALL);
                    break;
                default:
                    // FIXME: Implement others
                    assert(0);
#if 0
                case MapViewLayout_2Pane:
                    m_mapView = new TwoPaneMapView(this, m_logger, m_document, *m_toolBox, *m_mapRenderer, m_contextManager);
                    break;
                case MapViewLayout_3Pane:
                    m_mapView = new ThreePaneMapView(this, m_logger, m_document, *m_toolBox, *m_mapRenderer, m_contextManager);
                    break;
                case MapViewLayout_4Pane:
                    m_mapView = new FourPaneMapView(this, m_logger, m_document, *m_toolBox, *m_mapRenderer, m_contextManager);
                    break;
#endif
            }

            // delete the old sizer first
            if (layout() != nullptr) {
                delete layout();
            }

            // FIXME: add m_mapViewBar too. Use a vertical box layout.
            auto* layout = new QGridLayout();
            layout->addWidget(m_mapView, 0, 0, 1, 1);
            setLayout(layout);
            
            m_mapView->setFocus();
        }

        bool SwitchableMapViewContainer::anyToolActive() const {
            return createComplexBrushToolActive() || clipToolActive() || rotateObjectsToolActive() || scaleObjectsToolActive() || shearObjectsToolActive() || anyVertexToolActive();
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
            assert(canToggleCreateComplexBrushTool());
            m_toolBox->toggleCreateComplexBrushTool();
        }
        
        bool SwitchableMapViewContainer::clipToolActive() const {
            return m_toolBox->clipToolActive();
        }

        bool SwitchableMapViewContainer::canToggleClipTool() const {
            return clipToolActive() || lock(m_document)->selectedNodes().hasOnlyBrushes();
        }

        void SwitchableMapViewContainer::toggleClipTool() {
            assert(canToggleClipTool());
            m_toolBox->toggleClipTool();
        }
        
        ClipTool* SwitchableMapViewContainer::clipTool() {
            return m_toolBox->clipTool();
        }

        bool SwitchableMapViewContainer::rotateObjectsToolActive() const {
            return m_toolBox->rotateObjectsToolActive();
        }

        bool SwitchableMapViewContainer::canToggleRotateObjectsTool() const {
            return rotateObjectsToolActive() || lock(m_document)->hasSelectedNodes();
        }

        void SwitchableMapViewContainer::toggleRotateObjectsTool() {
            assert(canToggleRotateObjectsTool());
            m_toolBox->toggleRotateObjectsTool();
        }
        
        bool SwitchableMapViewContainer::scaleObjectsToolActive() const {
            return m_toolBox->scaleObjectsToolActive();
        }

        bool SwitchableMapViewContainer::shearObjectsToolActive() const {
            return m_toolBox->shearObjectsToolActive();
        }
        
        bool SwitchableMapViewContainer::canToggleScaleObjectsTool() const {
            return scaleObjectsToolActive() || lock(m_document)->hasSelectedNodes();
        }
        
        void SwitchableMapViewContainer::toggleScaleObjectsTool() {
            assert(canToggleScaleObjectsTool());
            m_toolBox->toggleScaleObjectsTool();
        }

        bool SwitchableMapViewContainer::canToggleShearObjectsTool() const {
            return shearObjectsToolActive() || lock(m_document)->hasSelectedNodes();
        }

        void SwitchableMapViewContainer::toggleShearObjectsTool() {
            assert(canToggleShearObjectsTool());
            m_toolBox->toggleShearObjectsTool();
        }
        
        bool SwitchableMapViewContainer::canToggleVertexTools() const {
            return vertexToolActive() || edgeToolActive() || faceToolActive() || lock(m_document)->selectedNodes().hasOnlyBrushes();
        }
        
        bool SwitchableMapViewContainer::anyVertexToolActive() const {
            return vertexToolActive() || edgeToolActive() || faceToolActive();
        }

        bool SwitchableMapViewContainer::vertexToolActive() const {
            return m_toolBox->vertexToolActive();
        }
        
        bool SwitchableMapViewContainer::edgeToolActive() const {
            return m_toolBox->edgeToolActive();
        }
        
        bool SwitchableMapViewContainer::faceToolActive() const {
            return m_toolBox->faceToolActive();
        }
        
        void SwitchableMapViewContainer::toggleVertexTool() {
            assert(canToggleVertexTools());
            m_toolBox->toggleVertexTool();
        }
        
        void SwitchableMapViewContainer::toggleEdgeTool() {
            assert(canToggleVertexTools());
            m_toolBox->toggleEdgeTool();
        }
        
        void SwitchableMapViewContainer::toggleFaceTool() {
            assert(canToggleVertexTools());
            m_toolBox->toggleFaceTool();
        }

        VertexTool* SwitchableMapViewContainer::vertexTool() {
            return m_toolBox->vertexTool();
        }
        
        EdgeTool* SwitchableMapViewContainer::edgeTool() {
            return m_toolBox->edgeTool();
        }
        
        FaceTool* SwitchableMapViewContainer::faceTool() {
            return m_toolBox->faceTool();
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
            m_mapView->update();
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
        
        void SwitchableMapViewContainer::doFlipObjects(const vm::direction direction) {
            m_mapView->flipObjects(direction);
        }
        
        vm::vec3 SwitchableMapViewContainer::doGetPasteObjectsDelta(const vm::bbox3& bounds, const vm::bbox3& referenceBounds) const {
            return m_mapView->pasteObjectsDelta(bounds, referenceBounds);
        }
        
        void SwitchableMapViewContainer::doFocusCameraOnSelection(const bool animate) {
            m_mapView->focusCameraOnSelection(animate);
        }
        
        void SwitchableMapViewContainer::doMoveCameraToPosition(const vm::vec3& position, const bool animate) {
            m_mapView->moveCameraToPosition(position, animate);
        }
        
        void SwitchableMapViewContainer::doMoveCameraToCurrentTracePoint() {
            m_mapView->moveCameraToCurrentTracePoint();
        }

        void SwitchableMapViewContainer::doFlashSelection() {
            m_mapView->flashSelection();
        }

        bool SwitchableMapViewContainer::doCancelMouseDrag() {
            return m_mapView->cancelMouseDrag();
        }
    }
}
