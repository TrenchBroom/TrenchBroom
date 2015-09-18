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

#ifndef TrenchBroom_UVView
#define TrenchBroom_UVView

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/Hit.h"
#include "Model/PickResult.h"
#include "Model/ModelTypes.h"
#include "Renderer/OrthographicCamera.h"
#include "View/RenderView.h"
#include "View/ToolBox.h"
#include "View/ToolBoxConnector.h"
#include "View/UVViewHelper.h"
#include "View/ViewTypes.h"

class wxWindow;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace Renderer {
        class ActiveShader;
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class Selection;
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
        class UVView : public RenderView, public ToolBoxConnector {
        public:
            static const Model::Hit::HitType FaceHit;
        private:
            MapDocumentWPtr m_document;
            
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
            UVView(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager);
            ~UVView();
            
            void setSubDivisions(const Vec2i& subDivisions);
        private:
            void createTools();
            void destroyTools();
            
            void bindObservers();
            void unbindObservers();
            
            void selectionDidChange(const Selection& selection);
            void nodesDidChange(const Model::NodeList& nodes);
            void brushFacesDidChange(const Model::BrushFaceList& faces);
            void gridDidChange();
            void cameraDidChange(const Renderer::Camera* camera);
            void preferenceDidChange(const IO::Path& path);
            
            void doUpdateViewport(int x, int y, int width, int height);
            void doRender();
            bool doShouldRenderFocusIndicator() const;

            void setupGL(Renderer::RenderContext& renderContext);

            class RenderTexture;
            void renderTexture(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            
            void renderFace(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderTextureAxes(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderToolBox(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            PickRequest doGetPickRequest(int x, int y) const;
            Model::PickResult doPick(const Ray3& pickRay) const;
        };
    }
}

#endif /* defined(TrenchBroom_UVView) */
