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

#include "TexturingViewScaleTool.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Model/ModelTypes.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/OrthographicCamera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexSpec.h"
#include "View/InputState.h"
#include "View/TexturingViewHelper.h"

namespace TrenchBroom {
    namespace View {
        const Hit::HitType TexturingViewScaleTool::XOriginHit = Hit::freeHitType();
        const Hit::HitType TexturingViewScaleTool::YOriginHit = Hit::freeHitType();
        const Hit::HitType TexturingViewScaleTool::XScaleHit = Hit::freeHitType();
        const Hit::HitType TexturingViewScaleTool::YScaleHit = Hit::freeHitType();
        const FloatType TexturingViewScaleTool::MaxPickDistance = 5.0;

        TexturingViewScaleTool::TexturingViewScaleTool(MapDocumentWPtr document, ControllerWPtr controller, TexturingViewHelper& helper, Renderer::OrthographicCamera& camera) :
        ToolImpl(document, controller),
        m_helper(helper),
        m_camera(camera),
        m_dragMode(None) {}

        void TexturingViewScaleTool::doPick(const InputState& inputState, Hits& hits) {
            if (m_helper.valid()) {
                const Ray3& pickRay = inputState.pickRay();
                pickOriginHandles(pickRay, hits);
                pickScaleHandles(pickRay, hits);
            }
        }

        void TexturingViewScaleTool::pickOriginHandles(const Ray3& ray, Hits& hits) const {
            Line3 xHandle, yHandle;
            m_helper.computeScaleHandles(xHandle, yHandle);
            
            const Ray3::LineDistance xDistance2 = ray.distanceToLineSquared(xHandle.point, xHandle.direction);
            const Ray3::LineDistance yDistance2 = ray.distanceToLineSquared(yHandle.point, yHandle.direction);
            
            assert(!xDistance2.parallel);
            assert(!yDistance2.parallel);
            
            const FloatType maxDistance  = MaxPickDistance / m_camera.zoom().x();
            const FloatType maxDistance2 = maxDistance * maxDistance;
            
            if (xDistance2.distance <= maxDistance2) {
                const Vec3 hitPoint = ray.pointAtDistance(xDistance2.rayDistance);
                hits.addHit(Hit(XOriginHit, xDistance2.rayDistance, hitPoint, xHandle));
            }
            
            if (yDistance2.distance <= maxDistance2) {
                const Vec3 hitPoint = ray.pointAtDistance(yDistance2.rayDistance);
                hits.addHit(Hit(YOriginHit, yDistance2.rayDistance, hitPoint, yHandle));
            }
        }

        void TexturingViewScaleTool::pickScaleHandles(const Ray3& ray, Hits& hits) const {
            const Model::BrushFace* face = m_helper.face();
            assert(face != NULL);
            
            const Assets::Texture* texture = face->texture();
            if (texture != NULL) {
                const Plane3& boundary = face->boundary();
                const FloatType rayDistance = ray.intersectWithPlane(boundary.normal, boundary.anchor());
                const Vec3 hitPoint = ray.pointAtDistance(rayDistance);
                const Vec3 texHit = m_helper.transformToTex(hitPoint, true);
                
                const FloatType maxDistance  = MaxPickDistance / m_camera.zoom().x();
                const FloatType maxDistance2 = maxDistance * maxDistance;
                
                const Vec2i& subDivisions = m_helper.subDivisions();
                const FloatType width  = static_cast<FloatType>(texture->width() / subDivisions.x());
                const FloatType height = static_cast<FloatType>(texture->height() / subDivisions.y());
                const FloatType x = Math::remainder(texHit.x(), width);
                const FloatType y = Math::remainder(texHit.y(), height);
                
                if (x * x < maxDistance2) {
                    const int index = Math::round(texHit.x() / width);
                    hits.addHit(Hit(XScaleHit, rayDistance, hitPoint, index));
                }
                
                if (y * y < maxDistance2) {
                    const int index = Math::round(texHit.y() / width);
                    hits.addHit(Hit(YScaleHit, rayDistance, hitPoint, index));
                }
            }
        }

        bool TexturingViewScaleTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            const Hits& hits = inputState.hits();
            const Hit& xOriginHit = hits.findFirst(XOriginHit, true);
            const Hit& yOriginHit = hits.findFirst(YOriginHit, true);
            const Hit& xScaleHit = hits.findFirst(XScaleHit, true);
            const Hit& yScaleHit = hits.findFirst(YScaleHit, true);

            if (xOriginHit.isMatch()) {
                m_dragMode = Handle;
                m_originSelector[0] = 1.0f;
            } else {
                m_originSelector[0] = 0.0f;
            }
            
            if (yOriginHit.isMatch()) {
                m_dragMode = Handle;
                m_originSelector[1] = 1.0f;
            } else {
                m_originSelector[1] = 0.0f;
            }

            if (!xOriginHit.isMatch() && !yOriginHit.isMatch()) {
                if (xScaleHit.isMatch()) {
                    m_dragMode = Scale;
                    m_scaleSelector[0] = 1.0f;
                } else {
                    m_scaleSelector[0] = 0.0f;
                }
                
                if (yScaleHit.isMatch()) {
                    m_dragMode = Scale;
                    m_scaleSelector[1] = 1.0f;
                } else {
                    m_scaleSelector[1] = 0.0f;
                }
            }
            
            const Vec3 texPoint = m_helper.computeTexPoint(inputState.pickRay());
            m_lastPoint = Vec2f(texPoint);

            return m_dragMode != None;
        }
        
        bool TexturingViewScaleTool::doMouseDrag(const InputState& inputState) {
            assert(m_dragMode != None);
            
            const Vec3 texPoint = m_helper.computeTexPoint(inputState.pickRay());
            const Vec2f curPoint(texPoint);
            Vec2f delta = curPoint - m_lastPoint;
            
            if (m_dragMode == Handle) {
                delta = m_helper.snapHandle(delta * m_originSelector);
                if (delta.null())
                    return true;
                
                m_helper.setHandlePosition(m_helper.handlePosition() + delta);
            }
            
            m_lastPoint += delta;
            return true;
        }
        
        void TexturingViewScaleTool::doEndMouseDrag(const InputState& inputState) {
            m_dragMode = None;
        }
        
        void TexturingViewScaleTool::doCancelMouseDrag(const InputState& inputState) {
            m_dragMode = None;
        }
        
        void TexturingViewScaleTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            if (!m_helper.valid())
                return;

            
            EdgeVertex::List vertices;
            getOriginHandleVertices(inputState.hits(), vertices);
            getScaleHandleVertices(inputState.hits(), vertices);
            
            glLineWidth(2.0f);
            Renderer::EdgeRenderer edgeRenderer(Renderer::VertexArray::swap(GL_LINES, vertices));
            edgeRenderer.render(renderContext);
            glLineWidth(1.0f);
        }

        void TexturingViewScaleTool::getOriginHandleVertices(const Hits& hits, EdgeVertex::List& vertices) const {
            const Hit& xOriginHit = hits.findFirst(XOriginHit, true);
            const Hit& yOriginHit = hits.findFirst(YOriginHit, true);
            
            const bool highlightXHandle = (m_dragMode == Handle && m_originSelector.x() > 0.0) || (m_dragMode == None && xOriginHit.isMatch());
            const bool highlightYHandle = (m_dragMode == Handle && m_originSelector.y() > 0.0) || (m_dragMode == None && yOriginHit.isMatch());
            
            const Color xColor = highlightXHandle ? Color(1.0f, 0.0f, 0.0f, 1.0f) : Color(0.7f, 0.0f, 0.0f, 1.0f);
            const Color yColor = highlightYHandle ? Color(1.0f, 0.0f, 0.0f, 1.0f) : Color(0.7f, 0.0f, 0.0f, 1.0f);
            
            Vec3 x1, x2, y1, y2;
            m_helper.computeScaleHandleVertices(m_camera, x1, x2, y1, y2);

            vertices.push_back(EdgeVertex(Vec3f(x1), xColor));
            vertices.push_back(EdgeVertex(Vec3f(x2), xColor));
            vertices.push_back(EdgeVertex(Vec3f(y1), yColor));
            vertices.push_back(EdgeVertex(Vec3f(y2), yColor));
        }

        void TexturingViewScaleTool::getScaleHandleVertices(const Hits& hits, EdgeVertex::List& vertices) const {
            const Hit& xOriginHit = hits.findFirst(XOriginHit, true);
            const Hit& yOriginHit = hits.findFirst(YOriginHit, true);
            if (xOriginHit.isMatch() || yOriginHit.isMatch())
                return;

            const Hit& xScaleHit = hits.findFirst(XScaleHit, true);
            const Hit& yScaleHit = hits.findFirst(YScaleHit, true);
            
            const Color color = Color(1.0f, 1.0f, 0.0f, 1.0f);

            const Model::BrushFace* face = m_helper.face();
            assert(face != NULL);
            
            if (xScaleHit.isMatch()) {
                const Assets::Texture* texture = face->texture();
                const Vec2i& subDivisions = m_helper.subDivisions();
                const FloatType width  = static_cast<FloatType>(texture->width() / subDivisions.x());
                
                const int index = xScaleHit.target<int>();
                const FloatType x = width * index;
                
                Vec3 v1, v2;
                m_helper.computeVLineVertices(m_camera, x, v1, v2);
                vertices.push_back(EdgeVertex(Vec3f(v1), color));
                vertices.push_back(EdgeVertex(Vec3f(v2), color));
            }
            
            if (yScaleHit.isMatch()) {
                const Assets::Texture* texture = face->texture();
                const Vec2i& subDivisions = m_helper.subDivisions();
                const FloatType height  = static_cast<FloatType>(texture->height() / subDivisions.y());
                
                const int index = yScaleHit.target<int>();
                const FloatType y = height * index;
                
                Vec3 v1, v2;
                m_helper.computeHLineVertices(m_camera, y, v1, v2);
                vertices.push_back(EdgeVertex(Vec3f(v1), color));
                vertices.push_back(EdgeVertex(Vec3f(v2), color));
            }
        }
    }
}
