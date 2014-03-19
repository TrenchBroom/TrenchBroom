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

#ifndef __TrenchBroom__TexturingView__
#define __TrenchBroom__TexturingView__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/RenderView.h"
#include "Model/ModelTypes.h"
#include "Renderer/OrthographicCamera.h"
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
        class RenderResources;
    }
    
    namespace View {
        class TexturingViewState {
        private:
            Model::BrushFace* m_face;
            Vec3 m_origin;
            Vec3 m_xAxis;
            Vec3 m_yAxis;
            Vec3 m_zAxis;
            Mat4x4 m_toFaceTransform;
            Mat4x4 m_fromFaceTransform;
        public:
            TexturingViewState();
            
            bool valid() const;
            Model::BrushFace* face() const;
            const Vec3& origin() const;
            const Vec3& xAxis() const;
            const Vec3& yAxis() const;
            const Vec3& zAxis() const;

            BBox3 computeBounds() const;
            Vec3 transformToFace(const Vec3& point) const;
            Vec3 transformFromFace(const Vec3& point) const;
            Vec2f textureCoords(const Vec3f& point) const;
            
            Vec3::List textureSeamVertices(const Renderer::OrthographicCamera& camera) const;
            
            void activateTexture(Renderer::ActiveShader& shader);
            void deactivateTexture();
            
            Hits pick(const Ray3& pickRay) const;
            
            void setFace(Model::BrushFace* face);
        private:
            void validate();
        };
        
        class TexturingViewOffsetTool;
        class TexturingViewCameraTool;
        
        /**
         A view which allows the user to manipulate the texture projection interactively with the mouse. The user can 
         change texture offsets, scaling factors and rotation. If supported by the map format, the user can manipulate 
         the texture axes as well.
         */
        class TexturingView : public RenderView, public ToolBoxHelper {
        public:
            static const Hit::HitType FaceHit;
        private:
            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;
            
            Renderer::RenderResources& m_renderResources;
            Renderer::OrthographicCamera m_camera;
            TexturingViewState m_state;

            ToolBox m_toolBox;
            
            TexturingViewOffsetTool* m_offsetTool;
            TexturingViewCameraTool* m_cameraTool;
        public:
            TexturingView(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller, Renderer::RenderResources& renderResources);
            ~TexturingView();
        private:
            void createTools();
            void destroyTools();
            
            void bindObservers();
            void unbindObservers();
            
            void objectDidChange(Model::Object* object);
            void faceDidChange(Model::BrushFace* face);
            void selectionDidChange(const Model::SelectionResult& result);
            
            void preferenceDidChange(const IO::Path& path);
            
            void doUpdateViewport(int x, int y, int width, int height);
            void doRender();
            void setupCamera();
            void setupGL(Renderer::RenderContext& renderContext);
            void renderTexture(Renderer::RenderContext& renderContext);
            void renderFace(Renderer::RenderContext& renderContext);
            void renderTextureSeams(Renderer::RenderContext& renderContext);
            
            float computeZoomFactor() const;
            Vec3f::List getTextureQuad() const;
        private:
            Ray3 doGetPickRay(int x, int y) const;
            Hits doPick(const Ray3& pickRay) const;
        };
    }
}

#endif /* defined(__TrenchBroom__TexturingView__) */
