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

#ifndef __TrenchBroom__UVView__
#define __TrenchBroom__UVView__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/RenderView.h"
#include "Model/ModelTypes.h"
#include "Renderer/OrthographicCamera.h"
#include "View/GLContextHolder.h"
#include "View/UVViewHelper.h"
#include "View/ToolBox.h"
#include "View/ViewTypes.h"

class wxWindow;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace Renderer {
        class ActiveShader;
        class RenderContext;
    }
    
    namespace View {
        class UVRotateTool;
        class UVOriginTool;
        class UVScaleTool;
        class UVShearTool;
        class UVOffsetTool;
        class UVCameraTool;
        
        /**
         A view which allows the user to manipulate the texture projection interactively with the mouse. The user can 
         change texture offsets, scaling factors and rotation. If supported by the map format, the user can manipulate 
         the texture axes as well.
         */
        class UVView : public RenderView, public ToolBoxHelper {
        public:
            static const Hit::HitType FaceHit;
        private:
            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;
            
            Renderer::OrthographicCamera m_camera;
            UVViewHelper m_helper;

            ToolBox m_toolBox;
            
            UVRotateTool* m_rotateTool;
            UVOriginTool* m_originTool;
            UVScaleTool* m_scaleTool;
            UVShearTool* m_shearTool;
            UVOffsetTool* m_offsetTool;
            UVCameraTool* m_cameraTool;
        public:
            UVView(wxWindow* parent, GLContextHolder::Ptr sharedContext, MapDocumentWPtr document, ControllerWPtr controller);
            ~UVView();
            
            void setSubDivisions(const Vec2i& subDivisions);
        private:
            void createTools();
            void destroyTools();
            
            void bindObservers();
            void unbindObservers();
            
            void selectionDidChange(const Model::SelectionResult& result);
            void objectsDidChange(const Model::ObjectList& objects);
            void facesDidChange(const Model::BrushFaceList& faces);
            void gridDidChange();
            void cameraDidChange(const Renderer::Camera* camera);
            void preferenceDidChange(const IO::Path& path);
            
            void doUpdateViewport(int x, int y, int width, int height);
            void doRender();
            bool doShouldRenderFocusIndicator() const;

            void setupGL(Renderer::RenderContext& renderContext);

            void renderTexture(Renderer::RenderContext& renderContext);
            Vec3f::List getTextureQuad() const;
            void activateTexture(Renderer::ActiveShader& shader);
            void deactivateTexture();
            
            void renderFace(Renderer::RenderContext& renderContext);
            void renderTextureAxes(Renderer::RenderContext& renderContext);
            void renderToolBox(Renderer::RenderContext& renderContext);
        private:
            Ray3 doGetPickRay(int x, int y) const;
            Hits doPick(const Ray3& pickRay) const;
        };
    }
}

#endif /* defined(__TrenchBroom__UVView__) */
