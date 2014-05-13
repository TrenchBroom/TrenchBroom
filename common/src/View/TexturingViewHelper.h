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
            Renderer::Vbo m_vbo;
        public:
            TexturingViewHelper(Renderer::OrthographicCamera& camera);
            
            bool valid() const;
            Model::BrushFace* face() const;
            const Assets::Texture* texture() const;

            /**
             Snaps the given delta in rotated and scaled texture coordinates to the face vertices. The X and Y coords
             are snapped individually.
             
             Used in:
             - TexturingViewOffsetTool::doMouseDrag
             */
            Vec2f snapOffset(const Vec2f& delta) const;
            
            /**
             Snaps the given delta in rotated texture coordinates to the vertices of the face and to the texture grid.
             
             Used in:
             - TexturingViewScaleOriginTool::doMouseDrag
             */
            Vec2f snapScaleOrigin(const Vec2f& deltaInFaceCoords) const;
            
            /**
             Snaps the given point in rotated, scaled and translated texture coordinates to the face's vertices.
             
             Used in:
             - TexturingViewScaleTool::doMouseDrag
             */
            Vec2f snapScaleHandle(const Vec2f& scaleHandleInFaceCoords) const;
            
            /**
             Snaps the given point in rotated, scaled and translated texture coordinates to the face's vertices and to
             its center.
             
             Used in:
             - TexturingViewRotateTool::doMouseDrag
             */
            Vec2f snapRotationCenter(const Vec2f& rotationCenterInFaceCoords) const;
            
            /**
             Snaps the given point in rotated, scaled and translated texture coordinates to the given points in world
             coordinates.
             
             Used in:
             - TexturingViewHelper::snapRotationCenter
             */
            Vec2f snapToPoints(const Vec2f& pointInFaceCoords, const Vec3::List& points) const;

            /**
             Measures the angle between the texture coordinate system's X axis and the vector from the current rotation
             center handle to the given point in rotated, scaled and translated texture coordinates.
             
             Used in:
             - TexturingViewRotateTool::doMouseDrag
             */
            float measureRotationAngle(const Vec2f& point) const;
            
            /**
             Snaps the given rotation angle so that it aligns with the edges of the current face.
             
             Used in:
             - TexturingViewRotateTool::doMouseDrag
             */
            float snapRotationAngle(float angle) const;
            
        private:
            Vec2 computeStripeSize() const;
            Vec2f computeDistanceFromTextureGrid(const Vec3& position) const;
        public:
            /**
             Computes the scale origin handle lines for the current scale origin in world coordinates.
             
             Used in:
             - TexturingViewScaleOriginTool::doPick
             */
            void computeScaleOriginHandles(Line3& xHandle, Line3& yHandle) const;
            
            /**
             Computes the vertices for the origin handle lines by intersecting them with the current camera frustum.
             
             Used in:
             - TexturingViewScaleOriginTool::doRender via TexturingViewScaleOriginTool::getHandleVertices
             */
            void computeScaleOriginHandleVertices(const Renderer::OrthographicCamera& camera, Vec3& x1, Vec3& x2, Vec3& y1, Vec3& y2) const;
            
            /**
             Computes the vertices for the scale handle line at the given Y coordinate by intersecting it with the current
             camera frustum.
             
             Used in:
             - TexturingViewScaleTool::doRender via TexturingViewScaleTool::getHandleVertices
             */
            void computeHLineVertices(const Renderer::OrthographicCamera& camera, FloatType y, Vec3& v1, Vec3& v2) const;
            void computeVLineVertices(const Renderer::OrthographicCamera& camera, FloatType x, Vec3& v1, Vec3& v2) const;
            
            /**
             Creates a hit container for the given pick ray. The container is either empty or contains a face hit for
             the current face.
             
             Used in:
             - TexturingView::doPick
             */
            Hits pick(const Ray3& pickRay) const;
            
            void setFace(Model::BrushFace* face);

            /**
             Returns the sub divisions of the texture size for the texture grid.
             
             Used in:
             - TexturingViewScaleTool::doMouseDrag via getScaleHandlePositionInTexCoords and getScaleHandlePositionInFaceCoords
             */
            const Vec2i& subDivisions() const;
            
            /**
             Sets the sub divisions of the texture size for the texture grid
             
             Used in:
             TexturingView::setSubDivisions
             */
            void setSubDivisions(const Vec2i& subDivisions);
            
            /**
             Returns the size of one texture grid box.
             
             Used in:
             - TexturingViewScaleTool::doPick
             - TexturingViewScaleTool::doRender via TexturingViewScaleTool::getHandleVertices
             */
            Vec2 stripeSize() const;
            
            /**
             Returns the scale origin position in rotated texture coordinates.
             
             Used in:
             - TexturingViewRotateTool::doMouseDrag to compensate for the rotation
             - TexturingViewScaleOriginTool::doMouseDrag
             - TexturingViewScaleTool::doMouseDrag to compute the scale factors
             */
            const Vec2f scaleOriginInFaceCoords() const;

            /**
             Returns the scale origin position in rotated, scaled and translated texture coordinates.
             
             Used in:
             - TexturingViewScaleTool::doMouseDrag to compute the scale factors
             */
            const Vec2f scaleOriginInTexCoords() const;
            
            /**
             Sets the scale origin position in rotated texture coordinates.
             
             Used in:
             - TexturingViewRotateTool::doMouseDrag to compensate for the rotation
             - TexturingViewScaleOriginTool::doMouseDrag to update the scale origin
             */
            void setScaleOrigin(const Vec2f& scaleOriginInFaceCoords);
            
            /**
             Returns the rotation center in rotated, scaled and translated texture coordinates.
             
             Used in:
             - TexturingViewRotateTool::doPick
             - TexturingViewRotateTool::doStartMouseDrag
             - TexturingViewRotateTool::doMouseDrag to compensate for the rotation
             - TexturingViewRotateTool::doRender
             */
            const Vec2f rotationCenterInFaceCoords() const;
            
            /**
             Returns the position of the rotation handle in rotated, scaled and translated texture coordinates.
             
             Used in:
             - TexturingViewRotateTool::doPick
             - TexturingViewRotateTool::doStartMouseDrag
             - TexturingViewRotateTool::doRender
             */
            const Vec2f angleHandleInFaceCoords(const float distance) const;
            
            /**
             Sets the position of the rotation center in rotated, sclaed and translated texture coordinates.
             
             Used in:
             - TexturingView::doMouseDrag for updating the rotation center and for compensating
             */
            void setRotationCenter(const Vec2f& rotationCenterInFaceCoords);
            
            // Camera related functions
            void resetCamera();

            void renderTexture(Renderer::RenderContext& renderContext);
            Vec3f::List getTextureQuad() const;
            void activateTexture(Renderer::ActiveShader& shader);
            void deactivateTexture();
        private:
            Mat4x4 worldToTexMatrix() const;

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
