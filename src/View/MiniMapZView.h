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
    }
    
    namespace View {
        class MiniMapZView : public MiniMapBaseView {
            Renderer::OrthographicCamera* m_camera;
        public:
            MiniMapZView(wxWindow* parent, View::MapDocumentWPtr document, BBox3f& bounds, Renderer::RenderResources& renderResources, Renderer::MiniMapRenderer& renderer);
            ~MiniMapZView();
            
            void setXYPosition(const Vec2f& xyPosition);
        private:
            const Renderer::Camera& camera() const;
            void doUpdateBounds(BBox3f& bounds);
            void doUpdateViewport(const Renderer::Camera::Viewport& viewport);
            void doMoveCamera(const Vec3f& diff);
            void doZoomCamera(const Vec3f& factors);
        };
    }
}

#endif /* defined(__TrenchBroom__MiniMapZView__) */
