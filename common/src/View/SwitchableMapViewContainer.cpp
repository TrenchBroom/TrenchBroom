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

#include "Renderer/MapRenderer.h"
#include "Renderer/Vbo.h"
#include "View/MapViewBar.h"
#include "View/MapViewToolBox.h"

namespace TrenchBroom {
    namespace View {
        SwitchableMapViewContainer::SwitchableMapViewContainer(wxWindow* parent, Logger* logger, MapDocumentWPtr document) :
        MapViewContainer(parent),
        m_logger(logger),
        m_document(document),
        m_mapViewBar(new MapViewBar(this, m_document)),
        m_toolBox(new MapViewToolBox(m_document, m_mapViewBar->toolBook())),
        m_mapRenderer(new Renderer::MapRenderer(m_document)),
        m_vbo(new Renderer::Vbo(0xFFFFFF)),
        m_mapView(NULL) {
        }
        
        SwitchableMapViewContainer::~SwitchableMapViewContainer() {
            // we must destroy our children before we destroy our resources because they might still use them in their destructors
            DestroyChildren();
            
            delete m_toolBox;
            m_toolBox = NULL;
            
            delete m_mapRenderer;
            m_mapRenderer = NULL;
            
            delete m_vbo;
            m_vbo = NULL;
        }

        Vec3 SwitchableMapViewContainer::doGetPasteObjectsDelta(const BBox3& bounds) const {
            return m_mapView->pasteObjectsDelta(bounds);
        }
        
        void SwitchableMapViewContainer::doCenterCameraOnSelection() {
            m_mapView->centerCameraOnSelection();
        }
        
        void SwitchableMapViewContainer::doMoveCameraToPosition(const Vec3& position) {
            m_mapView->moveCameraToPosition(position);
        }
    
        bool SwitchableMapViewContainer::doCanMoveCameraToNextTracePoint() const {
            return m_mapView->canMoveCameraToNextTracePoint();
        }
        
        bool SwitchableMapViewContainer::doCanMoveCameraToPreviousTracePoint() const {
            return m_mapView->canMoveCameraToPreviousTracePoint();
        }
        
        void SwitchableMapViewContainer::doMoveCameraToNextTracePoint() {
            m_mapView->moveCameraToNextTracePoint();
        }
        
        void SwitchableMapViewContainer::doMoveCameraToPreviousTracePoint() {
            m_mapView->moveCameraToPreviousTracePoint();
        }
        
        GLContextHolder::Ptr SwitchableMapViewContainer::doGetGLContext() const {
            return m_glContext;
        }
    }
}
