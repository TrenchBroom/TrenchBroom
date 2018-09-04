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

#include "UVRotateTool.h"

#include "VecMath.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/Circle.h"
#include "Renderer/Renderable.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/Shaders.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "View/MapDocument.h"
#include "View/InputState.h"
#include "View/UVViewHelper.h"

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType UVRotateTool::AngleHandleHit = Model::Hit::freeHitType();
        const float UVRotateTool::CenterHandleRadius =  2.5f;
        const float UVRotateTool::RotateHandleRadius = 32.0f;
        const float UVRotateTool::RotateHandleWidth  =  5.0f;

        UVRotateTool::UVRotateTool(MapDocumentWPtr document, UVViewHelper& helper) :
        ToolControllerBase(),
        Tool(true),
        m_document(document),
        m_helper(helper),
        m_initalAngle(0.0f) {}
        
        Tool* UVRotateTool::doGetTool() {
            return this;
        }
        
        void UVRotateTool::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            if (!m_helper.valid())
                return;

            const auto* face = m_helper.face();
            const auto fromFace = face->fromTexCoordSystemMatrix(vec2f::zero, vec2f::one, true);

            const auto& boundary = face->boundary();
            const auto toPlane = planeProjectionMatrix(boundary.distance, boundary.normal);

            const auto& pickRay = inputState.pickRay();
            const auto distanceToFace = intersect(pickRay, boundary);
            assert(!Math::isnan(distanceToFace));
            const auto hitPoint = pickRay.pointAtDistance(distanceToFace);
            
            const auto originOnPlane   = toPlane * fromFace * vec3(m_helper.originInFaceCoords());
            const auto hitPointOnPlane = toPlane * hitPoint;

            const auto zoom = m_helper.cameraZoom();
            const auto error = Math::abs(RotateHandleRadius / zoom - distance(hitPointOnPlane, originOnPlane));
            if (error <= RotateHandleWidth / zoom) {
                pickResult.addHit(Model::Hit(AngleHandleHit, distanceToFace, hitPoint, 0, error));
            }
        }
        
        bool UVRotateTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;

            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& angleHandleHit = pickResult.query().type(AngleHandleHit).occluded().first();

            if (!angleHandleHit.isMatch())
                return false;

            const Model::BrushFace* face = m_helper.face();
            const mat4x4 toFace = face->toTexCoordSystemMatrix(vec2f::zero, vec2f::one, true);

            const vec2f hitPointInFaceCoords(toFace * angleHandleHit.hitPoint());
            m_initalAngle = measureAngle(hitPointInFaceCoords) - face->rotation();

            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction("Rotate Texture");
            
            return true;
        }
        
        bool UVRotateTool::doMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            auto* face = m_helper.face();
            const auto& boundary = face->boundary();
            const auto& pickRay = inputState.pickRay();
            const auto curPointDistance = intersect(pickRay, boundary);
            const auto curPoint = pickRay.pointAtDistance(curPointDistance);
            
            const auto toFaceOld = face->toTexCoordSystemMatrix(vec2f::zero, vec2f::one, true);
            const auto toWorld = face->fromTexCoordSystemMatrix(vec2f::zero, vec2f::one, true);

            const auto curPointInFaceCoords = vec2f(toFaceOld * curPoint);
            const auto curAngle = measureAngle(curPointInFaceCoords);

            const auto angle = curAngle - m_initalAngle;
            const auto snappedAngle = Math::correct(snapAngle(angle), 4, 0.0f);

            const auto oldCenterInFaceCoords = m_helper.originInFaceCoords();
            const auto oldCenterInWorldCoords = toWorld * vec3(oldCenterInFaceCoords);
            
            Model::ChangeBrushFaceAttributesRequest request;
            request.setRotation(snappedAngle);

            auto document = lock(m_document);
            document->setFaceAttributes(request);
            
            // Correct the offsets.
            const auto toFaceNew = face->toTexCoordSystemMatrix(vec2f::zero, vec2f::one, true);
            const auto newCenterInFaceCoords = vec2f(toFaceNew * oldCenterInWorldCoords);

            const auto delta = (oldCenterInFaceCoords - newCenterInFaceCoords) / face->scale();
            const auto newOffset = correct(face->offset() + delta, 4, 0.0f);
            
            request.clear();
            request.setOffset(newOffset);
            document->setFaceAttributes(request);
            
            return true;
        }
        
        float UVRotateTool::measureAngle(const vec2f& point) const {
            const Model::BrushFace* face = m_helper.face();
            const vec2f origin = m_helper.originInFaceCoords();
            return Math::mod(face->measureTextureAngle(origin, point), 360.0f);
        }
        
        float UVRotateTool::snapAngle(const float angle) const {
            const auto* face = m_helper.face();
            
            const float angles[] = {
                Math::mod(angle +   0.0f, 360.0f),
                Math::mod(angle +  90.0f, 360.0f),
                Math::mod(angle + 180.0f, 360.0f),
                Math::mod(angle + 270.0f, 360.0f),
            };
            auto minDelta = std::numeric_limits<float>::max();
            
            const auto toFace = face->toTexCoordSystemMatrix(vec2f::zero, vec2f::one, true);
            for (const auto* edge : face->edges()) {
                const auto startInFaceCoords = vec2f(toFace * edge->firstVertex()->position());
                const auto endInFaceCoords   = vec2f(toFace * edge->secondVertex()->position());
                const auto edgeAngle         = Math::mod(face->measureTextureAngle(startInFaceCoords, endInFaceCoords), 360.0f);
                
                for (size_t i = 0; i < 4; ++i) {
                    if (std::abs(angles[i] - edgeAngle) < std::abs(minDelta)) {
                        minDelta = angles[i] - edgeAngle;
                    }
                }
            }
            
            if (std::abs(minDelta) < 3.0f) {
                return angle - minDelta;
            }
            return angle;
        }

        void UVRotateTool::doEndMouseDrag(const InputState& inputState) {
            MapDocumentSPtr document = lock(m_document);
            document->commitTransaction();
        }
        
        void UVRotateTool::doCancelMouseDrag() {
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
        }

        class UVRotateTool::Render : public Renderer::DirectRenderable {
        private:
            const UVViewHelper& m_helper;
            bool m_highlight;
            Renderer::Circle m_center;
            Renderer::Circle m_outer;
        public:
            Render(const UVViewHelper& helper, const float centerRadius, const float outerRadius, const bool highlight) :
            m_helper(helper),
            m_highlight(highlight),
            m_center(makeCircle(helper, centerRadius, 10, true)),
            m_outer(makeCircle(helper, outerRadius, 32, false)) {}
        private:
            static Renderer::Circle makeCircle(const UVViewHelper& helper, const float radius, const size_t segments, const bool fill) {
                const float zoom = helper.cameraZoom();
                return Renderer::Circle(radius / zoom, segments, fill);
            }
        private:
            void doPrepareVertices(Renderer::Vbo& vertexVbo) override {
                m_center.prepare(vertexVbo);
                m_outer.prepare(vertexVbo);
            }
            
            void doRender(Renderer::RenderContext& renderContext) override {
                const auto* face = m_helper.face();
                const auto fromFace = face->fromTexCoordSystemMatrix(vec2f::zero, vec2f::one, true);
                
                const auto& boundary = face->boundary();
                const auto toPlane = planeProjectionMatrix(boundary.distance, boundary.normal);
                const auto [invertible, fromPlane] = invert(toPlane);
                assert(invertible); unused(invertible);

                const auto originPosition(toPlane * fromFace * vec3(m_helper.originInFaceCoords()));
                const auto faceCenterPosition(toPlane * m_helper.face()->boundsCenter());

                const auto& handleColor = pref(Preferences::HandleColor);
                const auto& highlightColor = pref(Preferences::SelectedHandleColor);

                Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
                const Renderer::MultiplyModelMatrix toWorldTransform(renderContext.transformation(), mat4x4f(fromPlane));
                {
                    const auto translation = translationMatrix(vec3(originPosition));
                    const Renderer::MultiplyModelMatrix centerTransform(renderContext.transformation(), mat4x4f(translation));
                    if (m_highlight) {
                        shader.set("Color", highlightColor);
                    } else {
                        shader.set("Color", handleColor);
                    }
                    m_outer.render();
                }
                
                {
                    const auto translation = translationMatrix(vec3(faceCenterPosition));
                    const Renderer::MultiplyModelMatrix centerTransform(renderContext.transformation(), mat4x4f(translation));
                    shader.set("Color", highlightColor);
                    m_center.render();
                }
            }
        };
        
        void UVRotateTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (!m_helper.valid())
                return;
            
            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& angleHandleHit = pickResult.query().type(AngleHandleHit).occluded().first();
            const bool highlight = angleHandleHit.isMatch() || thisToolDragging();
            
            renderBatch.addOneShot(new Render(m_helper, CenterHandleRadius, RotateHandleRadius, highlight));
        }
        
        bool UVRotateTool::doCancel() {
            return false;
        }
    }
}
