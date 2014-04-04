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

#include "TexturingViewScaleOriginTool.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Model/ModelTypes.h"
#include "Model/TexCoordSystemHelper.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/OrthographicCamera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexSpec.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/TexturingViewHelper.h"

namespace TrenchBroom {
    namespace View {
        const Hit::HitType TexturingViewScaleOriginTool::XHandleHit = Hit::freeHitType();
        const Hit::HitType TexturingViewScaleOriginTool::YHandleHit = Hit::freeHitType();
        const FloatType TexturingViewScaleOriginTool::MaxPickDistance = 5.0;

        TexturingViewScaleOriginTool::TexturingViewScaleOriginTool(MapDocumentWPtr document, ControllerWPtr controller, TexturingViewHelper& helper, Renderer::OrthographicCamera& camera) :
        ToolImpl(document, controller),
        m_helper(helper),
        m_camera(camera) {}

        void TexturingViewScaleOriginTool::doPick(const InputState& inputState, Hits& hits) {
            if (m_helper.valid()) {
                const Ray3& pickRay = inputState.pickRay();

                Line3 xHandle, yHandle;
                m_helper.computeScaleOriginHandles(xHandle, yHandle);
                
                const Ray3::LineDistance xDistance = pickRay.distanceToLine(xHandle.point, xHandle.direction);
                const Ray3::LineDistance yDistance = pickRay.distanceToLine(yHandle.point, yHandle.direction);
                
                assert(!xDistance.parallel);
                assert(!yDistance.parallel);
                
                const FloatType maxDistance  = MaxPickDistance / m_camera.zoom().x();
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

        bool TexturingViewScaleOriginTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            const Hits& hits = inputState.hits();
            const Hit& xHandleHit = hits.findFirst(XHandleHit, true);
            const Hit& yHandleHit = hits.findFirst(YHandleHit, true);

            if (!xHandleHit.isMatch() && !yHandleHit.isMatch())
                return false;
            
            const Model::BrushFace* face = m_helper.face();
            const Plane3& boundary = face->boundary();
            
            const Ray3& pickRay = inputState.pickRay();
            const FloatType facePointDist = boundary.intersectWithRay(pickRay);
            const Vec3 facePoint = pickRay.pointAtDistance(facePointDist);
            
            if (xHandleHit.isMatch())
                m_selector[0] = 1.0f;
            else
                m_selector[0] = 0.0f;
            
            if (yHandleHit.isMatch())
                m_selector[1] = 1.0f;
            else
                m_selector[1] = 0.0f;
            
            Model::TexCoordSystemHelper helper(face);
            helper.setProject();
            
            const Vec3 texPoint = helper.worldToTex(facePoint);
            m_lastPoint = Vec2f(texPoint);
            return true;
        }
        
        bool TexturingViewScaleOriginTool::doMouseDrag(const InputState& inputState) {
            const Model::BrushFace* face = m_helper.face();
            const Plane3& boundary = face->boundary();
            
            const Ray3& pickRay = inputState.pickRay();
            const FloatType facePointDist = boundary.intersectWithRay(pickRay);
            const Vec3 facePoint = pickRay.pointAtDistance(facePointDist);
            
            Model::TexCoordSystemHelper helper(face);
            helper.setProject();
            
            const Vec3 texPoint = helper.worldToTex(facePoint);
            const Vec2f curPoint(texPoint);
            
            const Vec2f delta = m_helper.snapScaleOrigin((curPoint - m_lastPoint) * m_selector);
            if (delta.null())
                return true;
            
            m_helper.setScaleOrigin(m_helper.scaleOriginInFaceCoords() + delta);
            m_lastPoint += delta;
            
            return true;
        }
        
        void TexturingViewScaleOriginTool::doEndMouseDrag(const InputState& inputState) {}
        void TexturingViewScaleOriginTool::doCancelMouseDrag(const InputState& inputState) {}
        
        void TexturingViewScaleOriginTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            if (!m_helper.valid())
                return;
            
            EdgeVertex::List vertices = getHandleVertices(inputState.hits());
            
            glLineWidth(2.0f);
            Renderer::EdgeRenderer edgeRenderer(Renderer::VertexArray::swap(GL_LINES, vertices));
            edgeRenderer.render(renderContext);
            glLineWidth(1.0f);
        }

        TexturingViewScaleOriginTool::EdgeVertex::List TexturingViewScaleOriginTool::getHandleVertices(const Hits& hits) const {
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
