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

#ifndef __TrenchBroom__MiniMapZView__
#define __TrenchBroom__MiniMapZView__

#include "TrenchBroom.h"
#include "VecMath.h"

#include "View/MiniMapBaseView.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class MiniMapRenderer;
        class OrthographicCamera;
        class RenderResources;
        class Vbo;
    }
    
    namespace View {
        class MiniMapZView : public MiniMapBaseView {
        private:
            Renderer::OrthographicCamera* m_camera;
            BBox2f m_xyRange;
        public:
            MiniMapZView(wxWindow* parent, View::MapDocumentWPtr document, Renderer::RenderResources& renderResources, Renderer::MiniMapRenderer& renderer, Renderer::Camera& camera);
            ~MiniMapZView();
            
            BBox1f zRange() const;
            void setXYRange(const BBox2f& xyRange);
        private:
            const Renderer::Camera& doGetViewCamera() const;
            void doComputeBounds(BBox3f& bounds);
            void doUpdateViewport(const Renderer::Camera::Viewport& viewport);
            void doPanView(const Vec3f& diff);
            void doZoomView(const Vec3f& factors);
            
            void doShowDrag3DCameraCursor();
            void doDrag3DCamera(const Vec3f& delta, Renderer::Camera& camera);
            float doPick3DCamera(const Ray3f& pickRay, const Renderer::Camera& camera) const;
            void doRender3DCamera(Renderer::RenderContext& renderContext, Renderer::Vbo& vbo, const Renderer::Camera& camera);
        };
    }
}

#endif /* defined(__TrenchBroom__MiniMapZView__) */
