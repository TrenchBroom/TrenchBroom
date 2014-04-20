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

#ifndef __TrenchBroom__TexturingViewHelper__
#define __TrenchBroom__TexturingViewHelper__

#include "TrenchBroom.h"
#include "VecMath.h"

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
    }

    namespace View {
        class TexturingViewHelper {
        private:
            Renderer::OrthographicCamera& m_camera;
            
            Model::BrushFace* m_face;

            Vec2i m_subDivisions;
            
            /**
             The position of the scaling origin handle in texture coordinates (without offset and scaling applied).
             */
            Vec2f m_scaleOrigin;
            Vec2f m_rotationCenter;
        public:
            TexturingViewHelper(Renderer::OrthographicCamera& camera);
            
            bool valid() const;
            Model::BrushFace* face() const;
            const Assets::Texture* texture() const;

            Vec2f textureCoords(const Vec3f& point) const;
            
            Vec3 computeTexPoint(const Ray3& ray) const;
            Vec3 transformToTex(const Vec3& worldPoint, bool withOffset = false) const;
            Vec3::List transformToTex(const Vec3::List& worldPoints, bool withOffset = false) const;

            Vec2f snapOffset(const Vec2f& delta) const;
            Vec2f snapScaleOrigin(const Vec2f& deltaInFaceCoords) const;
            Vec2f snapScaleHandle(const Vec2f& scaleHandleInFaceCoords) const;
            Vec2f snapRotationCenter(const Vec2f& rotationCenterInFaceCoords) const;
            Vec2f snapToPoints(const Vec2f& pointInFaceCoords, const Vec3::List& points) const;

            float measureRotationAngle(const Vec2f& point) const;
            float snapRotationAngle(float angle) const;
            
            void computeScaleOriginHandles(Line3& xHandle, Line3& yHandle) const;
            void computeScaleOriginHandleVertices(const Renderer::OrthographicCamera& camera, Vec3& x1, Vec3& x2, Vec3& y1, Vec3& y2) const;
            void computeHLineVertices(const Renderer::OrthographicCamera& camera, FloatType y, Vec3& v1, Vec3& v2) const;
            void computeVLineVertices(const Renderer::OrthographicCamera& camera, FloatType x, Vec3& v1, Vec3& v2) const;
            
            Mat4x4 worldToTexMatrix() const;
            
            void activateTexture(Renderer::ActiveShader& shader);
            void deactivateTexture();
            
            Hits pick(const Ray3& pickRay) const;
            
            void setFace(Model::BrushFace* face);
            
            // Camera related functions
            void resetCamera();

            const Vec2i& subDivisions() const;
            void setSubDivisions(const Vec2i& subDivisions);
            
            const Vec2f scaleOriginInFaceCoords() const;
            const Vec2f scaleOriginInTexCoords() const;
            void setScaleOrigin(const Vec2f& scaleOriginInFaceCoords);
            
            const Vec2f rotationCenterInFaceCoords() const;
            const Vec2f angleHandleInFaceCoords(const float distance) const;
            void setRotationCenter(const Vec2f& rotationCenterInFaceCoords);
        private:
            void resetScaleOrigin();
            void resetRotationCenter();
            
            float cameraZoom() const;
            BBox3 computeFaceBoundsInCameraCoords() const;
            Vec3 transformToCamera(const Vec3& point) const;
            Vec3 transformFromCamera(const Vec3& point) const;
        };
    }
}

#endif /* defined(__TrenchBroom__TexturingViewHelper__) */
