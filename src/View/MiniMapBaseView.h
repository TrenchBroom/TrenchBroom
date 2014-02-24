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

#include "Renderer/Vbo.h"
#include "Renderer/Camera.h"
#include "View/ViewTypes.h"

#include <wx/glcanvas.h>

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class MiniMapRenderer;
        class RenderContext;
        class RenderResources;
    }
    
    namespace View {
        class MiniMapBaseView : public wxGLCanvas {
        private:
            View::MapDocumentWPtr m_document;
            Renderer::RenderResources& m_renderResources;
            wxGLContext* m_glContext;
            Renderer::MiniMapRenderer& m_renderer;
            Renderer::Vbo m_auxVbo;
            
            wxPoint m_lastPos;
        public:
            virtual ~MiniMapBaseView();
            
            void OnMouseButton(wxMouseEvent& event);
            void OnMouseDoubleClick(wxMouseEvent& event);
            void OnMouseMotion(wxMouseEvent& event);
            void OnMouseWheel(wxMouseEvent& event);
            void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
            
            void OnPaint(wxPaintEvent& event);
            void OnSize(wxSizeEvent& event);
        protected:
            MiniMapBaseView(wxWindow* parent, View::MapDocumentWPtr document, Renderer::RenderResources& renderResources, Renderer::MiniMapRenderer& renderer);
            
            View::MapDocumentSPtr document() const;
        private:
            void bindEvents();
            void setupGL(Renderer::RenderContext& context);
            void clearBackground(Renderer::RenderContext& context);
            void renderMap(Renderer::RenderContext& context);
            
            virtual const Renderer::Camera& camera() const = 0;
            virtual void updateViewport(const Renderer::Camera::Viewport& viewport) = 0;
            virtual void moveCamera(const Vec3f& diff) = 0;
            virtual void zoomCamera(const Vec3f& factors) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__MiniMapBaseView__) */
