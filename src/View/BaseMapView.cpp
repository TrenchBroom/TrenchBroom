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

#include "BaseMapView.h"
#include "PreferenceManager.h"
#include "Renderer/Camera.h"
#include "View/ControllerFacade.h"
#include "View/MapDocument.h"
#include "View/Tool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        BaseMapView::BaseMapView(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller, Renderer::Camera& camera, const GLAttribs& attribs, const wxGLContext* sharedContext) :
        ToolView(parent, camera, attribs, sharedContext),
        m_document(document),
        m_controller(controller) {
            bindObservers();
        }

        BaseMapView::~BaseMapView() {
            unbindObservers();
        }

        void BaseMapView::OnKey(wxKeyEvent& event) {
            m_movementRestriction.setVerticalRestriction(m_inputState.modifierKeysDown(ModifierKeys::MKAlt));
            event.Skip();
        }
        
        void BaseMapView::toggleMovementRestriction() {
            m_movementRestriction.toggleHorizontalRestriction(m_camera);
            Refresh();
        }

        void BaseMapView::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &BaseMapView::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &BaseMapView::documentWasNewedOrLoaded);
            document->objectWasAddedNotifier.addObserver(this, &BaseMapView::objectWasAddedOrDidChange);
            document->objectDidChangeNotifier.addObserver(this, &BaseMapView::objectWasAddedOrDidChange);
            document->faceDidChangeNotifier.addObserver(this, &BaseMapView::faceDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &BaseMapView::selectionDidChange);
            document->modsDidChangeNotifier.addObserver(this, &BaseMapView::modsDidChange);

            ControllerSPtr controller = lock(m_controller);
            controller->commandDoneNotifier.addObserver(this, &BaseMapView::commandDoneOrUndone);
            controller->commandUndoneNotifier.addObserver(this, &BaseMapView::commandDoneOrUndone);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &BaseMapView::preferenceDidChange);

            m_camera.cameraDidChangeNotifier.addObserver(this, &BaseMapView::cameraDidChange);
        }
        
        void BaseMapView::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &BaseMapView::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &BaseMapView::documentWasNewedOrLoaded);
                document->objectWasAddedNotifier.removeObserver(this, &BaseMapView::objectWasAddedOrDidChange);
                document->objectDidChangeNotifier.removeObserver(this, &BaseMapView::objectWasAddedOrDidChange);
                document->faceDidChangeNotifier.removeObserver(this, &BaseMapView::faceDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &BaseMapView::selectionDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &BaseMapView::modsDidChange);
            }
            
            if (!expired(m_controller)) {
                ControllerSPtr controller = lock(m_controller);
                controller->commandDoneNotifier.removeObserver(this, &BaseMapView::commandDoneOrUndone);
                controller->commandUndoneNotifier.removeObserver(this, &BaseMapView::commandDoneOrUndone);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &BaseMapView::preferenceDidChange);

            m_camera.cameraDidChangeNotifier.removeObserver(this, &BaseMapView::cameraDidChange);
        }
        
        void BaseMapView::documentWasNewedOrLoaded() {
            resetCamera();
        }

        void BaseMapView::objectWasAddedOrDidChange(Model::Object* object) {
            Refresh();
        }

        void BaseMapView::faceDidChange(Model::BrushFace* face) {
            Refresh();
        }

        void BaseMapView::selectionDidChange(const Model::SelectionResult& result) {
            Refresh();
        }

        void BaseMapView::modsDidChange() {
            Refresh();
        }

        void BaseMapView::commandDoneOrUndone(Controller::Command::Ptr command) {
            updateHits();
            Refresh();
        }

        void BaseMapView::preferenceDidChange(const IO::Path& path) {
            Refresh();
        }

        void BaseMapView::cameraDidChange(const Renderer::Camera* camera) {
            Refresh();
        }
        
        void BaseMapView::bindEvents() {
            Bind(wxEVT_KEY_DOWN, &BaseMapView::OnKey, this);
            Bind(wxEVT_KEY_UP, &BaseMapView::OnKey, this);
        }
        
        void BaseMapView::doUpdateViewport(int x, int y, int width, int height) {
            const Renderer::Camera::Viewport viewport(x, y, width, height);
            m_camera.setViewport(viewport);
        }

        Ray3d BaseMapView::doGetPickRay(const int x, const int y) const {
            return m_camera.pickRay(x, y);
        }

        Hits BaseMapView::doGetHits(const Ray3d& pickRay) const {
            MapDocumentSPtr document = lock(m_document);
            return document->pick(pickRay);
        }
    }
}
