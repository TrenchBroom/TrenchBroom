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

#ifndef __TrenchBroom__MapView3D__
#define __TrenchBroom__MapView3D__

#include "Renderer/PerspectiveCamera.h"
#include "View/GLContextHolder.h"
#include "View/RenderView.h"
#include "View/ToolBox.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    class Logger;

    namespace Renderer {
        class MapRenderer;
        class RenderContext;
    }
    
    namespace View {
        class CameraTool;
        
        class MapView3D : public RenderView, public ToolBoxHelper {
        private:
            Logger* m_logger;
            MapDocumentWPtr m_document;
            Renderer::MapRenderer& m_renderer;
            Renderer::PerspectiveCamera m_camera;
            
            ToolBox m_toolBox;
            CameraTool* m_cameraTool;
        public:
            MapView3D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, Renderer::MapRenderer& renderer);
            ~MapView3D();
        private: // implement RenderView
            void doInitializeGL();
            void doUpdateViewport(int x, int y, int width, int height);
            bool doShouldRenderFocusIndicator() const;
            void doRender();
            void setupGL(Renderer::RenderContext& renderContext);
            void renderMap(Renderer::RenderContext& renderContext);
        private: // implement ToolBoxHelper
            Ray3 doGetPickRay(int x, int y) const;
            Hits doPick(const Ray3& pickRay) const;
            void doShowPopupMenu();
        private: // Tool related methods
            void createTools();
            void destroyTools();
        private:
            static const GLContextHolder::GLAttribs& attribs();
            static int depthBits();
            static bool multisample();
        };
    }
}

#endif /* defined(__TrenchBroom__MapView3D__) */
