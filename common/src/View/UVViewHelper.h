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

#ifndef __TrenchBroom__UVViewHelper__
#define __TrenchBroom__UVViewHelper__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Hit.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    class Hits;
    
    namespace Assets {
        class Texture;
    }

    namespace Model {
        class BrushFace;
    }
    
    namespace Renderer {
        class ActiveShader;
        class OrthographicCamera;
        class RenderContext;
    }

    namespace View {
        class UVViewHelper {
        private:
            Renderer::OrthographicCamera& m_camera;
            bool m_zoomValid;
            
            Model::BrushFace* m_face;

            Vec2i m_subDivisions;
            
            /**
             The position of the scaling origin / rotation center handle in texture coordinates (without offset and scaling applied).
             */
            Vec2f m_origin;
            Renderer::Vbo m_vbo;
        public:
            UVViewHelper(Renderer::OrthographicCamera& camera);
            
            bool valid() const;
            Model::BrushFace* face() const;
            const Assets::Texture* texture() const;
            void setFace(Model::BrushFace* face);
            void cameraViewportChanged();

            const Vec2i& subDivisions() const;
            Vec2 stripeSize() const;
            void setSubDivisions(const Vec2i& subDivisions);
            
            const Vec3 origin() const;
            const Vec2f originInFaceCoords() const;
            const Vec2f originInTexCoords() const;
            void setOrigin(const Vec2f& originInFaceCoords);

            float cameraZoom() const;

            void pickTextureGrid(const Ray3& ray, const Hit::HitType hitTypes[2], Hits& hits) const;
            
            Vec2f snapDelta(const Vec2f& delta, const Vec2f& distance) const;
            Vec2f computeDistanceFromTextureGrid(const Vec3& position) const;

            void computeOriginHandleVertices(Vec3& x1, Vec3& x2, Vec3& y1, Vec3& y2) const;
            void computeScaleHandleVertices(const Vec2& pos, Vec3& x1, Vec3& x2, Vec3& y1, Vec3& y2) const;
            void computeLineVertices(const Vec2& pos, Vec3& x1, Vec3& x2, Vec3& y1, Vec3& y2, const Mat4x4& toTex, const Mat4x4& toWorld) const;
        private:
            void resetOrigin();
            void resetCamera();
            void resetZoom();
            
            BBox3 computeFaceBoundsInCameraCoords() const;
            Vec3 transformToCamera(const Vec3& point) const;
            Vec3 transformFromCamera(const Vec3& point) const;
        };
    }
}

#endif /* defined(__TrenchBroom__UVViewHelper__) */
