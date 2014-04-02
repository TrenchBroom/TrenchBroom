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
#include "Model/TexCoordSystemHelper.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/OrthographicCamera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexSpec.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/TexturingViewHelper.h"
#include "View/TexturingViewScaleOriginTool.h"

namespace TrenchBroom {
    namespace View {
        const Hit::HitType TexturingViewScaleTool::XHandleHit = Hit::freeHitType();
        const Hit::HitType TexturingViewScaleTool::YHandleHit = Hit::freeHitType();
        const FloatType TexturingViewScaleTool::MaxPickDistance = 5.0;

        TexturingViewScaleTool::ScaleHandle::ScaleHandle() {
            reset();
        };
        
        void TexturingViewScaleTool::ScaleHandle::reset() {
            m_dragging[0] = false;
            m_dragging[1] = false;
        }
        
        void TexturingViewScaleTool::ScaleHandle::setX(const int index, const Assets::Texture* texture, const Vec2i& subDivisions) {
            assert(texture != NULL);
            const FloatType width  = static_cast<FloatType>(texture->width() / subDivisions.x());
            const FloatType position = width * index;
            set(0, index, position);
        }
        
        void TexturingViewScaleTool::ScaleHandle::setY(const int index, const Assets::Texture* texture, const Vec2i& subDivisions) {
            assert(texture != NULL);
            const FloatType height  = static_cast<FloatType>(texture->height() / subDivisions.y());
            const FloatType position = height * index;
            set(1, index, position);
        }
        
        void TexturingViewScaleTool::ScaleHandle::set(size_t coord, int index, float position) {
            m_index[coord] = index;
            m_position[coord] = position;
            m_dragging[coord] = true;
        }

        
        const Vec2f& TexturingViewScaleTool::ScaleHandle::position() const {
            return m_position;
        }
        
        const Vec2f TexturingViewScaleTool::ScaleHandle::selector() const {
            Vec2f result;
            for (size_t i = 0; i < 2; ++i)
                result[i] = m_dragging[i] ? 1.0f : 0.0f;
            return result;
        }

        TexturingViewScaleTool::TexturingViewScaleTool(MapDocumentWPtr document, ControllerWPtr controller, TexturingViewHelper& helper, Renderer::OrthographicCamera& camera) :
        ToolImpl(document, controller),
        m_helper(helper),
        m_camera(camera) {}

        void TexturingViewScaleTool::doPick(const InputState& inputState, Hits& hits) {
            if (m_helper.valid()) {
                const Model::BrushFace* face = m_helper.face();
                const Assets::Texture* texture = face->texture();
                
                if (texture != NULL) {
                    const Ray3& pickRay = inputState.pickRay();
                    
                    const Plane3& boundary = face->boundary();
                    const FloatType rayDistance = pickRay.intersectWithPlane(boundary.normal, boundary.anchor());
                    const Vec3 hitPoint = pickRay.pointAtDistance(rayDistance);
                    const Vec3 texHit = m_helper.transformToTex(hitPoint, true);
                    
                    const FloatType maxDistance = MaxPickDistance / m_camera.zoom().x();
                    
                    const Vec2i& subDivisions = m_helper.subDivisions();
                    const FloatType width  = static_cast<FloatType>(texture->width() / subDivisions.x());
                    const FloatType height = static_cast<FloatType>(texture->height() / subDivisions.y());
                    const FloatType xError = Math::abs(Math::remainder(texHit.x(), width));
                    const FloatType yError = Math::abs(Math::remainder(texHit.y(), height));
                    
                    if (xError <= maxDistance) {
                        const int index = Math::round(texHit.x() / width);
                        hits.addHit(Hit(XHandleHit, rayDistance, hitPoint, index, xError));
                    }
                    
                    if (yError  <= maxDistance) {
                        const int index = Math::round(texHit.y() / height);
                        hits.addHit(Hit(YHandleHit, rayDistance, hitPoint, index, yError));
                    }
                }
            }
        }

        bool TexturingViewScaleTool::doStartMouseDrag(const InputState& inputState) {
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
            
            const Assets::Texture* texture = face->texture();
            const Vec2i& subDivisions = m_helper.subDivisions();
            
            Model::TexCoordSystemHelper helper(face);
            helper.setTranslate();
            helper.setScale();
            helper.setProject();
            
            m_scaleHandle.reset();
            if (xHandleHit.isMatch())
                m_scaleHandle.setX(xHandleHit.target<int>(), texture, subDivisions);
            
            if (yHandleHit.isMatch())
                m_scaleHandle.setY(yHandleHit.target<int>(), texture, subDivisions);
            
            const Vec3 texPoint = helper.worldToTex(facePoint);
            m_lastPoint = Vec2f(texPoint);
            m_lastScaleDistance = texPoint - m_helper.handlePositionInTexCoords();
            controller()->beginUndoableGroup("Scale Texture");
            
            return true;
        }
        
        bool TexturingViewScaleTool::doMouseDrag(const InputState& inputState) {
            Model::BrushFace* face = m_helper.face();
            const Plane3& boundary = face->boundary();
            
            const Ray3& pickRay = inputState.pickRay();
            const FloatType facePointDist = boundary.intersectWithRay(pickRay);
            const Vec3 facePoint = pickRay.pointAtDistance(facePointDist);
            
            Model::TexCoordSystemHelper helper(face);
            helper.setTranslate();
            helper.setScale();
            helper.setProject();

            const Vec2f curPoint(helper.worldToTex(facePoint));
            const Vec2f delta = curPoint - m_lastPoint;
            
            // the handle position in texture coordinates
            const Vec2f handlePos = m_scaleHandle.position();
            const Vec2f newHandlePos = handlePos + delta;
            const Vec2f snappedHandlePos = m_helper.snapToVertices(newHandlePos);
            
            const Vec2f lastScaleFactors = face->scale();
            
            const Vec2f oldHandlePositionInTexCoords = m_helper.handlePositionInTexCoords();
            const Vec2f scaleDistance = snappedHandlePos - oldHandlePositionInTexCoords;
            const Vec2f newScaleFactors = lastScaleFactors / m_lastScaleDistance * scaleDistance;
            
            const Model::BrushFaceList applyTo(1, face);
            const Vec2f applyFactors = m_scaleHandle.selector() * newScaleFactors;
            if (applyFactors.x() != 0.0f)
                controller()->setFaceXScale(applyTo, applyFactors.x(), false);
            if (applyFactors.y() != 0.0f)
                controller()->setFaceYScale(applyTo, applyFactors.y(), false);
            
            const Vec2f newHandlePositionInTexCoords = m_helper.handlePositionInTexCoords();
            const Vec2f handlePositionDeltaInTexCoords = newHandlePositionInTexCoords - oldHandlePositionInTexCoords;
            
            controller()->setFaceXOffset(applyTo, -handlePositionDeltaInTexCoords.x(), true);
            controller()->setFaceYOffset(applyTo, -handlePositionDeltaInTexCoords.y(), true);
            
            m_lastPoint = curPoint;
            
            return true;
        }
        
        void TexturingViewScaleTool::doEndMouseDrag(const InputState& inputState) {
            controller()->closeGroup();
        }
        
        void TexturingViewScaleTool::doCancelMouseDrag(const InputState& inputState) {
            controller()->rollbackGroup();
        }
        
        void TexturingViewScaleTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            if (!m_helper.valid())
                return;
            
            // don't overdraw the origin handles
            const Hits& hits = inputState.hits();
            if (hits.findFirst(TexturingViewScaleOriginTool::XHandleHit, true).isMatch() ||
                hits.findFirst(TexturingViewScaleOriginTool::YHandleHit, true).isMatch())
                return;
                
            EdgeVertex::List vertices = getHandleVertices(hits);
            
            glLineWidth(2.0f);
            Renderer::EdgeRenderer edgeRenderer(Renderer::VertexArray::swap(GL_LINES, vertices));
            edgeRenderer.render(renderContext);
            glLineWidth(1.0f);
        }

        TexturingViewScaleTool::EdgeVertex::List TexturingViewScaleTool::getHandleVertices(const Hits& hits) const {
            const Hit& xHandleHit = hits.findFirst(XHandleHit, true);
            const Hit& yHandleHit = hits.findFirst(YHandleHit, true);
            
            const Color color = Color(1.0f, 1.0f, 0.0f, 1.0f);

            const Model::BrushFace* face = m_helper.face();
            assert(face != NULL);
            const Assets::Texture* texture = face->texture();
            const Vec2i& subDivisions = m_helper.subDivisions();
            
            EdgeVertex::List vertices;
            vertices.resize(4);
            
            if (xHandleHit.isMatch()) {
                const int index = xHandleHit.target<int>();
                const FloatType width  = static_cast<FloatType>(texture->width() / subDivisions.x());
                const FloatType x = width * index;
                
                Vec3 v1, v2;
                m_helper.computeVLineVertices(m_camera, x, v1, v2);
                vertices.push_back(EdgeVertex(Vec3f(v1), color));
                vertices.push_back(EdgeVertex(Vec3f(v2), color));
            }
            
            if (yHandleHit.isMatch()) {
                const int index = yHandleHit.target<int>();
                const FloatType height  = static_cast<FloatType>(texture->height() / subDivisions.y());
                const FloatType y = height * index;
                
                Vec3 v1, v2;
                m_helper.computeHLineVertices(m_camera, y, v1, v2);
                vertices.push_back(EdgeVertex(Vec3f(v1), color));
                vertices.push_back(EdgeVertex(Vec3f(v2), color));
            }
            
            return vertices;
        }
    }
}
