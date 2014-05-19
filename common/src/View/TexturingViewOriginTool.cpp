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

#include "TexturingViewOriginTool.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Model/ModelTypes.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/OrthographicCamera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexSpec.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/TexturingViewHelper.h"

namespace TrenchBroom {
    namespace View {
        const Hit::HitType TexturingViewOriginTool::XHandleHit = Hit::freeHitType();
        const Hit::HitType TexturingViewOriginTool::YHandleHit = Hit::freeHitType();
        const FloatType TexturingViewOriginTool::MaxPickDistance = 5.0;

        TexturingViewOriginTool::TexturingViewOriginTool(MapDocumentWPtr document, ControllerWPtr controller, TexturingViewHelper& helper, Renderer::OrthographicCamera& camera) :
        ToolImpl(document, controller),
        m_helper(helper),
        m_camera(camera) {}

        void TexturingViewOriginTool::doPick(const InputState& inputState, Hits& hits) {
            if (m_helper.valid()) {
                const Ray3& pickRay = inputState.pickRay();

                Line3 xHandle, yHandle;
                m_helper.computeScaleOriginHandles(xHandle, yHandle);
                
                const Ray3::LineDistance xDistance = pickRay.distanceToLine(xHandle.point, xHandle.direction);
                const Ray3::LineDistance yDistance = pickRay.distanceToLine(yHandle.point, yHandle.direction);
                
                assert(!xDistance.parallel);
                assert(!yDistance.parallel);
                
                const FloatType maxDistance  = MaxPickDistance / m_helper.cameraZoom();
                const FloatType absXDistance = Math::abs(xDistance.distance);
                const FloatType absYDistance = Math::abs(yDistance.distance);
                
                if (absXDistance <= maxDistance) {
                    const Vec3 hitPoint = pickRay.pointAtDistance(xDistance.rayDistance);
                    hits.addHit(Hit(XHandleHit, xDistance.rayDistance, hitPoint, xHandle, absXDistance));
                }
                
                if (absYDistance <= maxDistance) {
                    const Vec3 hitPoint = pickRay.pointAtDistance(yDistance.rayDistance);
                    hits.addHit(Hit(YHandleHit, yDistance.rayDistance, hitPoint, yHandle, absYDistance));
                }
            }
        }

        bool TexturingViewOriginTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            const Hits& hits = inputState.hits();
            const Hit& xHandleHit = hits.findFirst(XHandleHit, true);
            const Hit& yHandleHit = hits.findFirst(YHandleHit, true);

            if (!xHandleHit.isMatch() && !yHandleHit.isMatch())
                return false;
            
            if (xHandleHit.isMatch())
                m_selector[0] = 1.0f;
            else
                m_selector[0] = 0.0f;
            
            if (yHandleHit.isMatch())
                m_selector[1] = 1.0f;
            else
                m_selector[1] = 0.0f;
            
            m_lastPoint = computeHitPoint(inputState.pickRay());
            return true;
        }
        
        bool TexturingViewOriginTool::doMouseDrag(const InputState& inputState) {
            const Vec2f curPoint = computeHitPoint(inputState.pickRay());
            const Vec2f delta = curPoint - m_lastPoint;
            
            const Vec2f snapped = snapDelta(delta * m_selector);
            if (snapped.null())
                return true;
            
            m_helper.setOrigin(m_helper.originInFaceCoords() + snapped);
            m_lastPoint += snapped;
            
            return true;
        }
        
        void TexturingViewOriginTool::doEndMouseDrag(const InputState& inputState) {}
        void TexturingViewOriginTool::doCancelMouseDrag(const InputState& inputState) {}
        
        Vec2f TexturingViewOriginTool::computeHitPoint(const Ray3& ray) const {
            const Model::BrushFace* face = m_helper.face();
            const Plane3& boundary = face->boundary();
            const FloatType distance = boundary.intersectWithRay(ray);
            const Vec3 hitPoint = ray.pointAtDistance(distance);
            
            const Mat4x4 transform = face->toTexCoordSystemMatrix(Vec2f::Null, Vec2f::One);
            return Vec2f(Mat4x4::ZerZ * transform * hitPoint);
        }

        Vec2f TexturingViewOriginTool::snapDelta(const Vec2f& delta) const {
            if (delta.null())
                return delta;
            
            const Model::BrushFace* face = m_helper.face();
            assert(face != NULL);
            
            const Mat4x4 w2fTransform = Mat4x4::ZerZ * face->toTexCoordSystemMatrix(Vec2f::Null, Vec2f::One);
            const Mat4x4 w2tTransform = Mat4x4::ZerZ * face->toTexCoordSystemMatrix(face->offset(), face->scale());
            const Mat4x4 f2wTransform = face->projectToBoundaryMatrix() * face->fromTexCoordSystemMatrix(Vec2f::Null, Vec2f::One);
            const Mat4x4 t2wTransform = face->projectToBoundaryMatrix() * face->fromTexCoordSystemMatrix(face->offset(), face->scale());
            const Mat4x4 f2tTransform = w2tTransform * f2wTransform;
            const Mat4x4 t2fTransform = w2fTransform * t2wTransform;
            
            const Vec2f newOriginInFaceCoords = m_helper.originInFaceCoords() + delta;
            const Vec2f newOriginInTexCoords  = Vec2f(f2tTransform * Vec3(newOriginInFaceCoords));
            
            // now snap to the vertices
            const Model::BrushVertexList& vertices = face->vertices();
            Vec2f distanceInTexCoords = Vec2f::Max;
            
            for (size_t i = 0; i < vertices.size(); ++i)
                distanceInTexCoords = absMin(distanceInTexCoords, newOriginInTexCoords - Vec2f(w2tTransform * vertices[i]->position));
            
            // and to the texture grid
            const Assets::Texture* texture = face->texture();
            if (texture != NULL)
                distanceInTexCoords = absMin(distanceInTexCoords, m_helper.computeDistanceFromTextureGrid(Vec3(newOriginInTexCoords)));
            
            // now we have a distance in the scaled and translated texture coordinate system
            // so we transform the new position plus distance back to the unscaled and untranslated texture coordinate system
            // and take the actual distance
            const Vec2f distanceInFaceCoords = newOriginInFaceCoords - Vec2f(t2fTransform * Vec3(newOriginInTexCoords + distanceInTexCoords));
            return m_helper.snapDelta(delta, distanceInFaceCoords);
        }

        void TexturingViewOriginTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            if (!m_helper.valid())
                return;
            
            EdgeVertex::List vertices = getHandleVertices(inputState.hits());
            
            glLineWidth(2.0f);
            Renderer::EdgeRenderer edgeRenderer(Renderer::VertexArray::swap(GL_LINES, vertices));
            edgeRenderer.render(renderContext);
            glLineWidth(1.0f);
        }

        TexturingViewOriginTool::EdgeVertex::List TexturingViewOriginTool::getHandleVertices(const Hits& hits) const {
            const Hit& xHandleHit = hits.findFirst(XHandleHit, true);
            const Hit& yHandleHit = hits.findFirst(YHandleHit, true);
            
            const bool highlightXHandle = (dragging() && m_selector.x() > 0.0) || (!dragging() && xHandleHit.isMatch());
            const bool highlightYHandle = (dragging() && m_selector.y() > 0.0) || (!dragging() && yHandleHit.isMatch());
            
            const Color xColor = highlightXHandle ? Color(1.0f, 0.0f, 0.0f, 1.0f) : Color(0.7f, 0.0f, 0.0f, 1.0f);
            const Color yColor = highlightYHandle ? Color(1.0f, 0.0f, 0.0f, 1.0f) : Color(0.7f, 0.0f, 0.0f, 1.0f);
            
            Vec3 x1, x2, y1, y2;
            m_helper.computeScaleOriginHandleVertices(m_camera, x1, x2, y1, y2);

            EdgeVertex::List vertices(4);
            vertices[0] = EdgeVertex(Vec3f(x1), xColor);
            vertices[1] = EdgeVertex(Vec3f(x2), xColor);
            vertices[2] = EdgeVertex(Vec3f(y1), yColor);
            vertices[3] = EdgeVertex(Vec3f(y2), yColor);
            return vertices;
        }
    }
}
