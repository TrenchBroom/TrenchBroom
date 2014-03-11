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
        class TexturingView : public RenderView {
        private:
            MapDocumentWPtr m_document;
            Renderer::RenderResources& m_renderResources;
            Renderer::OrthographicCamera m_camera;
            Model::BrushFace* m_face;
        public:
            TexturingView(wxWindow* parent, MapDocumentWPtr document, Renderer::RenderResources& renderResources);
        private:
            void doUpdateViewport(int x, int y, int width, int height);
            void doRender();
            void setupGL(Renderer::RenderContext& renderContext);
            void setupCamera(Renderer::RenderContext& renderContext, const Mat4x4& transform);
            void renderTexture(Renderer::RenderContext& renderContext, const Mat4x4& transform);
            void renderFace(Renderer::RenderContext& renderContext, const Mat4x4& transform);
            
            Mat4x4 faceCoordinateSystem() const;
            Vec3::List transformVertices(const Mat4x4& transform) const;
            FloatType zoomFactor(const BBox3& bounds) const;
        };
    }
}

#endif /* defined(__TrenchBroom__TexturingView__) */
