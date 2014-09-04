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

#ifndef __TrenchBroom__MiniMapBaseView__
#define __TrenchBroom__MiniMapBaseView__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "LayerObserver.h"
#include "Model/ModelTypes.h"
#include "Renderer/Vbo.h"
#include "Renderer/Camera.h"
#include "View/RenderView.h"
#include "View/GLContextHolder.h"
#include "View/ViewTypes.h"

#include <wx/event.h>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace Model {
        class SelectionResult;
    }

    namespace Renderer {
        class Camera;
        class MiniMapRenderer;
        class RenderContext;
    }
    
    namespace View {
        class MiniMapBaseView : public RenderView {
        private:
            View::MapDocumentWPtr m_document;
            Renderer::Camera& m_camera3D;
            
            LayerObserver m_layerObserver;
            
            Renderer::MiniMapRenderer& m_renderer;
            Renderer::Vbo m_auxVbo;
            
            wxPoint m_lastPos;
        public:
            virtual ~MiniMapBaseView();
            
            void OnMouseButton(wxMouseEvent& event);
            void OnMouseDoubleClick(wxMouseEvent& event);
            void OnMouseMotion(wxMouseEvent& event);
        private:
            void drag3DCamera(const wxPoint& lastPos, const wxPoint& currentPos);
            void panView(const wxPoint& lastPos, const wxPoint& currentPos);
        public:
            void OnMouseWheel(wxMouseEvent& event);
            void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
        protected:
            MiniMapBaseView(wxWindow* parent, GLContextHolder::Ptr sharedContext, View::MapDocumentWPtr document, Renderer::MiniMapRenderer& renderer, Renderer::Camera& camera3D);
            
            View::MapDocumentSPtr document() const;
        private:
            const Renderer::Camera& viewCamera() const;
            void panView(const Vec3f& delta);
            void zoomView(const Vec3f& factors);
        private:
            void bindObservers();
            void unbindObservers();
            
            void documentWasCleared();
            void documentWasNewedOrLoaded();

            void objectsWereAdded(const Model::ObjectList& objects);
            void objectsWillBeRemoved(const Model::ObjectList& objects);
            void objectsDidChange(const Model::ObjectList& objects);

            void layersWereAdded(const Model::LayerList& layers);
            void layersWereRemoved(const Model::LayerList& layers);
            void layerDidChange(Model::Layer* layer, const Model::Layer::Attr_Type attr);
            
            void filterDidChange();
            void renderConfigDidChange();
            void preferenceDidChange(const IO::Path& path);
            void selectionDidChange(const Model::SelectionResult& result);
            
            void cameraDidChange(const Renderer::Camera* camera);

            void bindEvents();
            
            void fireChangeEvent();
            
            void doRender();
            bool doShouldRenderFocusIndicator() const;
            
            void setupGL(Renderer::RenderContext& context);
            void clearBackground(Renderer::RenderContext& context);
            void renderMap(Renderer::RenderContext& context);
            void render3DCamera(Renderer::RenderContext& context);

            float pick3DCamera(const Ray3f& pickRay) const;

            virtual const Renderer::Camera& doGetViewCamera() const = 0;
            virtual void doComputeBounds(BBox3f& bounds) = 0;
            virtual void doPanView(const Vec3f& delta) = 0;
            virtual void doZoomView(const Vec3f& factors) = 0;

            virtual void doShowDrag3DCameraCursor() = 0;
            virtual void doDrag3DCamera(const Vec3f& delta, Renderer::Camera& camera) = 0;
            virtual void doRender3DCamera(Renderer::RenderContext& renderContext, Renderer::Vbo& vbo, const Renderer::Camera& camera) = 0;
            virtual float doPick3DCamera(const Ray3f& pickRay, const Renderer::Camera& camera) const = 0;
        };
    }
}

#define WXDLLIMPEXP_CUSTOM_EVENT

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CUSTOM_EVENT, EVT_MINIMAP_VIEW_CHANGED_EVENT, 1)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*miniMapViewChangedEventFunction)(wxCommandEvent&);

#define EVT_MINIMAP_VIEW_CHANGED_HANDLER(func) \
    (wxObjectEventFunction) \
    (miniMapViewChangedEventFunction) & func

#define EVT_MINIMAP_VIEW_CHANGED(id,func) \
    DECLARE_EVENT_TABLE_ENTRY(EVT_MINIMAP_VIEW_CHANGED_EVENT, \
        id, \
        wxID_ANY, \
        (wxObjectEventFunction) \
        (miniMapViewChangedEventFunction) & func, \
        (wxObject*) NULL ),


#endif /* defined(__TrenchBroom__MiniMapBaseView__) */
