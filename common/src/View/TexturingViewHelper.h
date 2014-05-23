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
             The position of the scaling origin / rotation center handle in texture coordinates (without offset and scaling applied).
             */
            Vec2f m_origin;
            Renderer::Vbo m_vbo;
        public:
            TexturingViewHelper(Renderer::OrthographicCamera& camera);
            
            bool valid() const;
            Model::BrushFace* face() const;
            const Assets::Texture* texture() const;
            
            Vec2f snapDelta(const Vec2f& delta, const Vec2f& distance) const;

            Vec2 computeStripeSize() const;
            Vec2f computeDistanceFromTextureGrid(const Vec3& position) const;

            /**
             Computes the scale origin handle lines for the current scale origin in world coordinates.
             
             Used in:
             - TexturingViewOriginTool::doPick
             */
            void computeScaleOriginHandles(Line3& xHandle, Line3& yHandle) const;
            
            /**
             Computes the vertices for the origin handle lines by intersecting them with the current camera frustum.
             
             Used in:
             - TexturingViewOriginTool::doRender via TexturingViewOriginTool::getHandleVertices
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
             Returns the origin position in rotated texture coordinates.
             
             Used in:
             - TexturingViewRotateTool::doMouseDrag to compensate for the rotation
             - TexturingViewOriginTool::doMouseDrag
             - TexturingViewScaleTool::doMouseDrag to compute the scale factors
             */
            const Vec2f originInFaceCoords() const;

            /**
             Returns the origin position in rotated, scaled and translated texture coordinates.
             
             Used in:
             - TexturingViewScaleTool::doMouseDrag to compute the scale factors
             */
            const Vec2f originInTexCoords() const;
            
            /**
             Sets the scale origin position in rotated texture coordinates.
             
             Used in:
             - TexturingViewRotateTool::doMouseDrag to compensate for the rotation
             - TexturingViewOriginTool::doMouseDrag to update the scale origin
             */
            void setOrigin(const Vec2f& originInFaceCoords);

            /**
             Returns the position of the rotation handle in rotated, scaled and translated texture coordinates.
             
             Used in:
             - TexturingViewRotateTool::doPick
             - TexturingViewRotateTool::doStartMouseDrag
             - TexturingViewRotateTool::doRender
             */
            const Vec2f angleHandleInFaceCoords(const float distance) const;
            
            // Camera related functions
            void resetCamera();
            float cameraZoom() const;

            void renderTexture(Renderer::RenderContext& renderContext);
            Vec3f::List getTextureQuad() const;
            void activateTexture(Renderer::ActiveShader& shader);
            void deactivateTexture();
        private:
            Mat4x4 worldToTexMatrix() const;

            void resetOrigin();
            
            BBox3 computeFaceBoundsInCameraCoords() const;
            Vec3 transformToCamera(const Vec3& point) const;
            Vec3 transformFromCamera(const Vec3& point) const;
        };
    }
}

#endif /* defined(__TrenchBroom__TexturingViewHelper__) */
