/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "UVScaleTool.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/ModelTypes.h"
#include "Model/PickResult.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/Renderable.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexSpec.h"
#include "View/MapDocument.h"
#include "View/InputState.h"
#include "View/UVViewHelper.h"
#include "View/UVOriginTool.h"

#include <numeric>

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType UVScaleTool::XHandleHit = Model::Hit::freeHitType();
        const Model::Hit::HitType UVScaleTool::YHandleHit = Model::Hit::freeHitType();
        
        UVScaleTool::UVScaleTool(MapDocumentWPtr document, UVViewHelper& helper) :
        ToolControllerBase(),
        Tool(true),
        m_document(document),
        m_helper(helper) {}

        Tool* UVScaleTool::doGetTool() {
            return this;
        }
        
        void UVScaleTool::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            static const Model::Hit::HitType HitTypes[] = { XHandleHit, YHandleHit };
            if (m_helper.valid()) {
                m_helper.pickTextureGrid(inputState.pickRay(), HitTypes, pickResult);
            }
        }

        Vec2i UVScaleTool::getScaleHandle(const Model::Hit& xHit, const Model::Hit& yHit) const {
            const auto x = xHit.isMatch() ? xHit.target<int>() : 0;
            const auto y = yHit.isMatch() ? yHit.target<int>() : 0;
            return Vec2i(x, y);
        }
        
        Vec2f UVScaleTool::getHitPoint(const Ray3& pickRay) const {
            const auto* face = m_helper.face();
            const auto& boundary = face->boundary();
            const auto facePointDist = boundary.intersectWithRay(pickRay);
            const auto facePoint = pickRay.pointAtDistance(facePointDist);
            
            const auto toTex = face->toTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            return toTex * facePoint;
        }

        bool UVScaleTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                return false;
            }
            
            const auto& pickResult = inputState.pickResult();
            const auto& xHit = pickResult.query().type(XHandleHit).occluded().first();
            const auto& yHit = pickResult.query().type(YHandleHit).occluded().first();
            
            if (!xHit.isMatch() && !yHit.isMatch()) {
                return false;
            }
            
            m_handle = getScaleHandle(xHit, yHit);
            m_selector = Vec2b(xHit.isMatch(), yHit.isMatch());
            m_lastHitPoint = getHitPoint(inputState.pickRay());
            
            auto document = lock(m_document);
            document->beginTransaction("Scale Texture");
            return true;
        }
        
        bool UVScaleTool::doMouseDrag(const InputState& inputState) {
            const auto curPoint = getHitPoint(inputState.pickRay());
            const auto dragDeltaFaceCoords = curPoint - m_lastHitPoint;
            
            const auto curHandlePosTexCoords  = getScaledTranslatedHandlePos();
            const auto newHandlePosFaceCoords = getHandlePos() + dragDeltaFaceCoords;
            const auto newHandlePosSnapped    = snap(newHandlePosFaceCoords);
            
            const auto originHandlePosFaceCoords = m_helper.originInFaceCoords();
            const auto originHandlePosTexCoords  = m_helper.originInTexCoords();
            
            const auto newHandleDistFaceCoords = newHandlePosSnapped    - originHandlePosFaceCoords;
            const auto curHandleDistTexCoords  = curHandlePosTexCoords  - originHandlePosTexCoords;
            
            auto* face = m_helper.face();
            auto newScale = face->scale();
            for (size_t i = 0; i < 2; ++i) {
                if (m_selector[i]) {
                    newScale[i] = newHandleDistFaceCoords[i] / curHandleDistTexCoords[i];
                }
            }
            newScale.correct(4, 0.0f);

            Model::ChangeBrushFaceAttributesRequest request;
            request.setScale(newScale);
            
            auto document = lock(m_document);
            document->setFaceAttributes(request);
            
            const auto newOriginInTexCoords = m_helper.originInTexCoords().corrected(4, 0.0f);
            const auto originDelta = originHandlePosTexCoords - newOriginInTexCoords;
            
            request.clear();
            request.addOffset(originDelta);
            document->setFaceAttributes(request);
            
            m_lastHitPoint += (dragDeltaFaceCoords - newHandlePosFaceCoords + newHandlePosSnapped);
            return true;
        }
        
        void UVScaleTool::doEndMouseDrag(const InputState& inputState) {
            auto document = lock(m_document);
            document->commitTransaction();
        }
        
        void UVScaleTool::doCancelMouseDrag() {
            auto document = lock(m_document);
            document->cancelTransaction();
        }

        Vec2f UVScaleTool::getScaledTranslatedHandlePos() const {
            return Vec2f(m_handle) * Vec2f(m_helper.stripeSize());
        }

        Vec2f UVScaleTool::getHandlePos() const {
            const auto* face = m_helper.face();
            const auto toWorld = face->fromTexCoordSystemMatrix(face->offset(), face->scale(), true);
            const auto toTex   = face->toTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            
            return Vec2f(toTex * toWorld * Vec3(getScaledTranslatedHandlePos()));
        }

        Vec2f UVScaleTool::snap(const Vec2f& position) const {
            const auto* face = m_helper.face();
            const auto toTex = face->toTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            
            const auto vertices = face->vertices();
            auto distance = std::accumulate(std::begin(vertices), std::end(vertices), Vec2f::Max,
                                             [&toTex, &position](const Vec2f& current, const Model::BrushVertex* vertex) {
                                                 const Vec2f vertex2(toTex * vertex->position());
                                                 return absMin(current, position - vertex2);
                                             });
            
            for (size_t i = 0; i < 2; ++i) {
                if (Math::abs(distance[i]) > 4.0f / m_helper.cameraZoom()) {
                    distance[i] = 0.0f;
                }
            }
            
            return position - distance;
        }
        
        void UVScaleTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (m_helper.valid()) {
                // don't overdraw the origin handles
                const auto& pickResult = inputState.pickResult();
                if (!pickResult.query().type(UVOriginTool::XHandleHit | UVOriginTool::YHandleHit).occluded().first().isMatch()) {
                    auto vertices = getHandleVertices(pickResult);
                    const Color color(1.0f, 0.0f, 0.0f, 1.0f);

                    Renderer::DirectEdgeRenderer handleRenderer(Renderer::VertexArray::swap(vertices), GL_LINES);
                    handleRenderer.render(renderBatch, color, 0.5f);
                }
            }
        }

        UVScaleTool::EdgeVertex::List UVScaleTool::getHandleVertices(const Model::PickResult& pickResult) const {
            const auto& xHandleHit = pickResult.query().type(XHandleHit).occluded().first();
            const auto& yHandleHit = pickResult.query().type(YHandleHit).occluded().first();
            const auto stripeSize = m_helper.stripeSize();

            const auto xIndex = xHandleHit.target<int>();
            const auto yIndex = yHandleHit.target<int>();
            const auto pos = stripeSize * Vec2(xIndex, yIndex);

            Vec3 h1, h2, v1, v2;
            m_helper.computeScaleHandleVertices(pos, v1, v2, h1, h2);

            EdgeVertex::List vertices;
            vertices.reserve(4);
            
            if (xHandleHit.isMatch()) {
                vertices.push_back(EdgeVertex(Vec3f(v1)));
                vertices.push_back(EdgeVertex(Vec3f(v2)));
            }
            
            if (yHandleHit.isMatch()) {
                vertices.push_back(EdgeVertex(Vec3f(h1)));
                vertices.push_back(EdgeVertex(Vec3f(h2)));
            }
            
            return vertices;
        }
        
        bool UVScaleTool::doCancel() {
            return false;
        }
    }
}
