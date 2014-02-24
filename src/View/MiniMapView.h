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

#ifndef __TrenchBroom__MiniMapView__
#define __TrenchBroom__MiniMapView__

#include "Color.h"
#include "Renderer/MiniMapRenderer.h"
#include "Renderer/Vbo.h"
#include "View/ViewTypes.h"

#include <wx/glcanvas.h>

class wxPaintEvent;
class wxSizeEvent;
class wxWindow;

namespace TrenchBroom {
    namespace Renderer {
        class OrthographicCamera;
        class RenderResources;
    }
    
    namespace View {
        class MiniMapView : public wxGLCanvas {
        private:
            View::MapDocumentWPtr m_document;
            Renderer::RenderResources& m_renderResources;
            wxGLContext* m_glContext;
            Renderer::OrthographicCamera* m_camera;
            Renderer::MiniMapRenderer m_renderer;
            Renderer::Vbo m_auxVbo;
            Color m_focusColor;
            
            wxPoint m_lastPos;
        public:
            MiniMapView(wxWindow* parent, View::MapDocumentWPtr document, Renderer::RenderResources& renderResources);
            ~MiniMapView();

            void setZPosition(float zPosition);
            
            void OnMouseButton(wxMouseEvent& event);
            void OnMouseDoubleClick(wxMouseEvent& event);
            void OnMouseMotion(wxMouseEvent& event);
            void OnMouseWheel(wxMouseEvent& event);
            void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);

            void OnPaint(wxPaintEvent& event);
            void OnSize(wxSizeEvent& event);
        private:
            void bindEvents();
            void setupGL(Renderer::RenderContext& context);
            void clearBackground(Renderer::RenderContext& context);
            void renderMap(Renderer::RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__MiniMapView__) */
