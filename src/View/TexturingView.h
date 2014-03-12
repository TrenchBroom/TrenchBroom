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
#include "View/ViewTypes.h"

class wxWindow;

namespace TrenchBroom {
    namespace Renderer {
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
        public:
            TexturingViewState();
            
            bool valid() const;
            Model::BrushFace* face() const;
            const Vec3& origin() const;
            const Vec3& xAxis() const;
            const Vec3& yAxis() const;
            const Vec3& zAxis() const;
            
            void setFace(Model::BrushFace* face);
        private:
            void validate();
        };
        
        class TexturingView : public RenderView {
        private:
            MapDocumentWPtr m_document;
            Renderer::RenderResources& m_renderResources;
            Renderer::OrthographicCamera m_camera;
            TexturingViewState m_state;
        public:
            TexturingView(wxWindow* parent, MapDocumentWPtr document, Renderer::RenderResources& renderResources);
            ~TexturingView();
        private:
            void bindObservers();
            void unbindObservers();
            
            void faceDidChange(Model::BrushFace* face);
            void selectionDidChange(const Model::SelectionResult& result);
            
            void doUpdateViewport(int x, int y, int width, int height);
            void doRender();
            void setupCamera();
            void setupGL(Renderer::RenderContext& renderContext);
            void renderTexture(Renderer::RenderContext& renderContext);
            void renderFace(Renderer::RenderContext& renderContext);
        };
    }
}

#endif /* defined(__TrenchBroom__TexturingView__) */
