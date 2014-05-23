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
#include "View/TexturingViewOriginTool.h"

namespace TrenchBroom {
    namespace View {
        const Hit::HitType TexturingViewScaleTool::XHandleHit = Hit::freeHitType();
        const Hit::HitType TexturingViewScaleTool::YHandleHit = Hit::freeHitType();
        const FloatType TexturingViewScaleTool::MaxPickDistance = 5.0;

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
                    const Vec3 hitPointInWorldCoords = pickRay.pointAtDistance(rayDistance);
                    const Vec3 hitPointInTexCoords = face->toTexCoordSystemMatrix(face->offset(), face->scale(), true) * hitPointInWorldCoords;
                    
                    const FloatType maxDistance = MaxPickDistance / m_camera.zoom().x();
                    const Vec2 stripeSize = m_helper.stripeSize();
                    
                    static const Hit::HitType HitTypes[] = { XHandleHit, YHandleHit };
                    for (size_t i = 0; i < 2; ++i) {
                        const FloatType error = Math::abs(Math::remainder(hitPointInTexCoords[i], stripeSize[i]));
                        if (error <= maxDistance) {
                            const int index = static_cast<int>(Math::round(hitPointInTexCoords[i] / stripeSize[i]));
                            hits.addHit(Hit(HitTypes[i], rayDistance, hitPointInWorldCoords, index, error));
                        }
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

            m_handle = getScaleHandle(xHandleHit, yHandleHit);
            m_selector = Vec2b(xHandleHit.isMatch(), yHandleHit.isMatch());

            const Ray3& pickRay = inputState.pickRay();
            m_lastHitPoint = getHitPointInFaceCoords(pickRay);
            
            controller()->beginUndoableGroup("Scale Texture");
            return true;
        }
        
        bool TexturingViewScaleTool::doMouseDrag(const InputState& inputState) {
            const Ray3& pickRay = inputState.pickRay();
            const Vec2f curPointFaceCoords = getHitPointInFaceCoords(pickRay);
            const Vec2f dragDeltaFaceCoords = curPointFaceCoords - m_lastHitPoint;

            const Vec2f curHandlePosTexCoords  = getScaleHandlePositionInTexCoords(m_handle);
            const Vec2f newHandlePosFaceCoords = getScaleHandlePositionInFaceCoords(m_handle) + dragDeltaFaceCoords;
            const Vec2f newHandlePosSnapped    = snap(newHandlePosFaceCoords);

            const Vec2f originHandlePosFaceCoords = m_helper.originInFaceCoords();
            const Vec2f originHandlePosTexCoords  = m_helper.originInTexCoords();
            
            const Vec2f newHandleDistFaceCoords = newHandlePosSnapped    - originHandlePosFaceCoords;
            const Vec2f curHandleDistTexCoords  = curHandlePosTexCoords  - originHandlePosTexCoords;
            
            Model::BrushFace* face = m_helper.face();
            Vec2f newScale = face->scale();
            for (size_t i = 0; i < 2; ++i)
                if (m_selector[i])
                    newScale[i] = newHandleDistFaceCoords[i] / curHandleDistTexCoords[i];
            
            const Model::BrushFaceList applyTo(1, face);
            controller()->setFaceXScale(applyTo, newScale.x(), false);
            controller()->setFaceYScale(applyTo, newScale.y(), false);
            
            const Vec2f newOriginInTexCoords = m_helper.originInTexCoords();
            const Vec2f originDelta = originHandlePosTexCoords - newOriginInTexCoords;

            controller()->setFaceXOffset(applyTo, originDelta.x(), true);
            controller()->setFaceYOffset(applyTo, originDelta.y(), true);

            m_lastHitPoint += dragDeltaFaceCoords - (newHandlePosFaceCoords - newHandlePosSnapped);
            
            return true;
        }

        Vec2i TexturingViewScaleTool::getScaleHandle(const Hit& xHandleHit, const Hit& yHandleHit) const {
            const int x = xHandleHit.isMatch() ? xHandleHit.target<int>() : 0;
            const int y = yHandleHit.isMatch() ? yHandleHit.target<int>() : 0;
            return Vec2i(x, y);
        }
        
        Vec2f TexturingViewScaleTool::getHitPointInFaceCoords(const Ray3& pickRay) const {
            const Model::BrushFace* face = m_helper.face();
            const Plane3& boundary = face->boundary();
            const FloatType facePointDist = boundary.intersectWithRay(pickRay);
            const Vec3 facePoint = pickRay.pointAtDistance(facePointDist);
            
            const Model::TexCoordSystemHelper faceCoordSystem = Model::TexCoordSystemHelper::faceCoordSystem(face);
            return faceCoordSystem.worldToTex(facePoint);
        }

        Vec2f TexturingViewScaleTool::getScaleHandlePositionInTexCoords(const Vec2i& scaleHandle) const {
            const Model::BrushFace* face = m_helper.face();
            const Assets::Texture* texture = face->texture();
            const Vec2i& subDivisions = m_helper.subDivisions();
            
            const float width  = static_cast<float>(texture->width())  / static_cast<float>(subDivisions.x());
            const float height = static_cast<float>(texture->height()) / static_cast<float>(subDivisions.y());
            
            return Vec2f(width  * scaleHandle.x(),
                         height * scaleHandle.y());
        }

        Vec2f TexturingViewScaleTool::getScaleHandlePositionInFaceCoords(const Vec2i& scaleHandle) const {
            const Model::BrushFace* face = m_helper.face();
            const Vec2f positionInTexCoords = getScaleHandlePositionInTexCoords(scaleHandle);
            return Model::TexCoordSystemHelper::texToFace(face, positionInTexCoords);
        }

        Vec2f TexturingViewScaleTool::snap(const Vec2f& position) const {
            const Model::BrushFace* face = m_helper.face();
            const Mat4x4 toTex = face->toTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            
            Vec2f distance = Vec2f::Max;
            
            const Model::BrushVertexList& vertices = face->vertices();
            for (size_t i = 0; i < vertices.size(); ++i) {
                const Vec2f vertex(toTex * vertices[i]->position);
                distance = absMin(distance, position - vertex);
            }
            
            for (size_t i = 0; i < 2; ++i) {
                if (Math::abs(distance[i]) > 4.0f / m_helper.cameraZoom())
                    distance[i] = 0.0f;
            }
            return position - distance;
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
            if (hits.findFirst(TexturingViewOriginTool::XHandleHit, true).isMatch() ||
                hits.findFirst(TexturingViewOriginTool::YHandleHit, true).isMatch())
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
            const Vec2 stripeSize = m_helper.stripeSize();
            const Color color = Color(1.0f, 1.0f, 0.0f, 1.0f);

            EdgeVertex::List vertices;
            vertices.resize(4);
            
            if (xHandleHit.isMatch()) {
                const int index = xHandleHit.target<int>();
                const FloatType x = stripeSize.x() * index;
                
                Vec3 v1, v2;
                m_helper.computeVLineVertices(m_camera, x, v1, v2);
                vertices.push_back(EdgeVertex(Vec3f(v1), color));
                vertices.push_back(EdgeVertex(Vec3f(v2), color));
            }
            
            if (yHandleHit.isMatch()) {
                const int index = yHandleHit.target<int>();
                const FloatType y = stripeSize.y() * index;
                
                Vec3 v1, v2;
                m_helper.computeHLineVertices(m_camera, y, v1, v2);
                vertices.push_back(EdgeVertex(Vec3f(v1), color));
                vertices.push_back(EdgeVertex(Vec3f(v2), color));
            }
            
            return vertices;
        }
    }
}
