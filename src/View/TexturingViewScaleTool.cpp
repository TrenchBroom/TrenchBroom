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
        const Hit::HitType TexturingViewScaleTool::XHandleHit = Hit::freeHitType();
        const Hit::HitType TexturingViewScaleTool::YHandleHit = Hit::freeHitType();

        TexturingViewScaleTool::TexturingViewScaleTool(MapDocumentWPtr document, ControllerWPtr controller, TexturingViewHelper& helper, Renderer::OrthographicCamera& camera) :
        ToolImpl(document, controller),
        m_helper(helper),
        m_camera(camera),
        m_dragMode(None) {}

        void TexturingViewScaleTool::doPick(const InputState& inputState, Hits& hits) {
            if (m_helper.valid()) {
                Line3 xHandle, yHandle;
                m_helper.computeScaleHandles(xHandle, yHandle);
                
                const Ray3& pickRay = inputState.pickRay();
                const Ray3::LineDistance xDistance2 = pickRay.distanceToLineSquared(xHandle.point, xHandle.direction);
                const Ray3::LineDistance yDistance2 = pickRay.distanceToLineSquared(yHandle.point, yHandle.direction);
                
                assert(!xDistance2.parallel);
                assert(!yDistance2.parallel);
                
                const FloatType maxDistance  = 5.0 / m_camera.zoom().x();
                const FloatType maxDistance2 = maxDistance * maxDistance;
                
                if (xDistance2.distance <= maxDistance2) {
                    const Vec3 hitPoint = pickRay.pointAtDistance(xDistance2.rayDistance);
                    hits.addHit(Hit(XHandleHit, xDistance2.rayDistance, hitPoint, xHandle));
                }
                
                if (yDistance2.distance <= maxDistance2) {
                    const Vec3 hitPoint = pickRay.pointAtDistance(yDistance2.rayDistance);
                    hits.addHit(Hit(YHandleHit, yDistance2.rayDistance, hitPoint, yHandle));
                }
            }
        }

        bool TexturingViewScaleTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            const Hits& hits = inputState.hits();
            const Hit xHandleHit = hits.findFirst(XHandleHit, true);
            const Hit yHandleHit = hits.findFirst(YHandleHit, true);

            if (xHandleHit.isMatch()) {
                m_dragMode = Handle;
                m_handleSelector[0] = 1.0f;
            } else {
                m_handleSelector[0] = 0.0f;
            }
            
            if (yHandleHit.isMatch()) {
                m_dragMode = Handle;
                m_handleSelector[1] = 1.0f;
            } else {
                m_handleSelector[1] = 0.0f;
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
                delta = m_helper.snapHandle(delta * m_handleSelector);
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

            const Hits& hits = inputState.hits();
            const Hit xHandleHit = hits.findFirst(XHandleHit, true);
            const Hit yHandleHit = hits.findFirst(YHandleHit, true);
            
            const bool highlightXHandle = (m_dragMode == Handle && m_handleSelector.x() > 0.0) || (m_dragMode == None && xHandleHit.isMatch());
            const bool highlightYHandle = (m_dragMode == Handle && m_handleSelector.y() > 0.0) || (m_dragMode == None && yHandleHit.isMatch());
            
            const Color xColor = highlightXHandle ? Color(1.0f, 0.0f, 0.0f, 1.0f) : Color(0.7f, 0.0f, 0.0f, 1.0f);
            const Color yColor = highlightYHandle ? Color(1.0f, 0.0f, 0.0f, 1.0f) : Color(0.7f, 0.0f, 0.0f, 1.0f);
            
            Vec3 x1, x2, y1, y2;
            m_helper.computeScaleHandleVertices(m_camera, x1, x2, y1, y2);
            
            typedef Renderer::VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List vertices(4);
            
            vertices[0] = Vertex(Vec3f(x1), xColor);
            vertices[1] = Vertex(Vec3f(x2), xColor);
            vertices[2] = Vertex(Vec3f(y1), yColor);
            vertices[3] = Vertex(Vec3f(y2), yColor);
            
            glLineWidth(2.0f);
            Renderer::EdgeRenderer edgeRenderer(Renderer::VertexArray::swap(GL_LINES, vertices));
            edgeRenderer.render(renderContext);
            glLineWidth(1.0f);
        }
    }
}
