/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef TrenchBroom_UVViewHelper
#define TrenchBroom_UVViewHelper

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/Hit.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace Model {
        class BrushFace;
        class PickResult;
    }
    
    namespace Renderer {
        class ActiveShader;
        class Camera;
        class OrthographicCamera;
        class RenderContext;
    }

    namespace View {
        class UVViewHelper {
        private:
            Renderer::OrthographicCamera& m_camera;
            bool m_zoomValid;
            
            Model::BrushFace* m_face;

            vec2i m_subDivisions;
            
            /**
             The position of the scaling origin / rotation center handle in world coords.
             */
            vec3 m_origin;
        public:
            UVViewHelper(Renderer::OrthographicCamera& camera);
            
            bool valid() const;
            Model::BrushFace* face() const;
            const Assets::Texture* texture() const;
            void setFace(Model::BrushFace* face);
            void cameraViewportChanged();

            const vec2i& subDivisions() const;
            vec2 stripeSize() const;
            void setSubDivisions(const vec2i& subDivisions);
            
            const vec3 origin() const;
            const vec2f originInFaceCoords() const;
            const vec2f originInTexCoords() const;
            void setOriginInFaceCoords(const vec2f& originInFaceCoords);

            const Renderer::Camera& camera() const;
            float cameraZoom() const;

            void pickTextureGrid(const ray3& ray, const Model::Hit::HitType hitTypes[2], Model::PickResult& pickResult) const;
            
            vec2f snapDelta(const vec2f& delta, const vec2f& distance) const;
            vec2f computeDistanceFromTextureGrid(const vec3& position) const;

            void computeOriginHandleVertices(vec3& x1, vec3& x2, vec3& y1, vec3& y2) const;
            void computeScaleHandleVertices(const vec2& pos, vec3& x1, vec3& x2, vec3& y1, vec3& y2) const;
            void computeLineVertices(const vec2& pos, vec3& x1, vec3& x2, vec3& y1, vec3& y2, const mat4x4& toTex, const mat4x4& toWorld) const;
        private:
            void resetOrigin();
            void resetCamera();
            void resetZoom();
            
            bbox3 computeFaceBoundsInCameraCoords() const;
            vec3 transformToCamera(const vec3& point) const;
            vec3 transformFromCamera(const vec3& point) const;
        };
    }
}

#endif /* defined(TrenchBroom_UVViewHelper) */
