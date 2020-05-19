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

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/ActiveShader.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Renderer/Circle.h"
#include "Renderer/Renderable.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/Shaders.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Transformation.h"
#include "Renderer/VboManager.h"
#include "View/MapDocument.h"
#include "View/InputState.h"
#include "View/UVViewHelper.h"

#include <kdl/memory_utils.h>

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/intersection.h>

namespace TrenchBroom {
    namespace View {
        const Model::HitType::Type UVRotateTool::AngleHandleHitType = Model::HitType::freeType();
        const double UVRotateTool::CenterHandleRadius =  2.5;
        const double UVRotateTool::RotateHandleRadius = 32.0;
        const double UVRotateTool::RotateHandleWidth  =  5.0;

        UVRotateTool::UVRotateTool(std::weak_ptr<MapDocument> document, UVViewHelper& helper) :
        ToolControllerBase(),
        Tool(true),
        m_document(document),
        m_helper(helper),
        m_initalAngle(0.0f) {}

        Tool* UVRotateTool::doGetTool() {
            return this;
        }

        const Tool* UVRotateTool::doGetTool() const {
            return this;
        }

        void UVRotateTool::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            if (!m_helper.valid())
                return;

            const auto* face = m_helper.face();
            const auto& boundary = face->boundary();

            const auto& pickRay = inputState.pickRay();
            const auto distanceToFace = vm::intersect_ray_plane(pickRay, boundary);
            if (!vm::is_nan(distanceToFace)) {
                const auto hitPoint = vm::point_at_distance(pickRay, distanceToFace);

                const auto fromFace = face->fromTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
                const auto toPlane = vm::plane_projection_matrix(boundary.distance, boundary.normal);

                const auto originOnPlane   = toPlane * fromFace * vm::vec3(m_helper.originInFaceCoords());
                const auto hitPointOnPlane = toPlane * hitPoint;

                const auto zoom = static_cast<FloatType>(m_helper.cameraZoom());
                const auto error = vm::abs(RotateHandleRadius / zoom - vm::distance(hitPointOnPlane, originOnPlane));
                if (error <= RotateHandleWidth / zoom) {
                    pickResult.addHit(Model::Hit(AngleHandleHitType, distanceToFace, hitPoint, 0, error));
                }
            }
        }

        bool UVRotateTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());

            // If Ctrl is pressed, allow starting the drag anywhere, not just on the handle
            const bool ctrlPressed = inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd);

            if (!(inputState.modifierKeysPressed(ModifierKeys::MKNone) || ctrlPressed) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                return false;
            }

            const auto& pickResult = inputState.pickResult();
            const auto& angleHandleHit = pickResult.query().type(AngleHandleHitType).occluded().first();

            const auto* face = m_helper.face();
            if (!face->attributes().valid()) {
                return false;
            }

            const auto toFace = face->toTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);

            vm::vec2f hitPointInFaceCoords;
            if (angleHandleHit.isMatch()) {
                hitPointInFaceCoords = vm::vec2f(toFace * angleHandleHit.hitPoint());
            } else if (ctrlPressed) {
                const auto& boundary = face->boundary();
                const auto& pickRay = inputState.pickRay();
                const auto distanceToFace = vm::intersect_ray_plane(pickRay, boundary);
                if (vm::is_nan(distanceToFace)) {
                    return false;
                }
                const auto hitPoint = vm::point_at_distance(pickRay, distanceToFace);
                hitPointInFaceCoords = vm::vec2f(toFace * hitPoint);
            } else {
                return false;
            }

            m_initalAngle = measureAngle(hitPointInFaceCoords) - face->attributes().rotation();

            auto document = kdl::mem_lock(m_document);
            document->startTransaction("Rotate Texture");

            return true;
        }

        bool UVRotateTool::doMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());

            auto* face = m_helper.face();
            const auto& boundary = face->boundary();
            const auto& pickRay = inputState.pickRay();
            const auto curPointDistance = vm::intersect_ray_plane(pickRay, boundary);
            const auto curPoint = vm::point_at_distance(pickRay, curPointDistance);

            const auto toFaceOld = face->toTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
            const auto toWorld = face->fromTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);

            const auto curPointInFaceCoords = vm::vec2f(toFaceOld * curPoint);
            const auto curAngle = measureAngle(curPointInFaceCoords);

            const auto angle = curAngle - m_initalAngle;
            const auto snappedAngle = vm::correct(snapAngle(angle), 4, 0.0f);

            const auto oldCenterInFaceCoords = m_helper.originInFaceCoords();
            const auto oldCenterInWorldCoords = toWorld * vm::vec3(oldCenterInFaceCoords);

            Model::ChangeBrushFaceAttributesRequest request;
            request.setRotation(snappedAngle);

            auto document = kdl::mem_lock(m_document);
            document->setFaceAttributes(request);

            // Correct the offsets.
            const auto toFaceNew = face->toTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
            const auto newCenterInFaceCoords = vm::vec2f(toFaceNew * oldCenterInWorldCoords);

            const auto delta = (oldCenterInFaceCoords - newCenterInFaceCoords) / face->attributes().scale();
            const auto newOffset = correct(face->attributes().offset() + delta, 4, 0.0f);

            request.clear();
            request.setOffset(newOffset);
            document->setFaceAttributes(request);

            return true;
        }

        float UVRotateTool::measureAngle(const vm::vec2f& point) const {
            const auto* face = m_helper.face();
            const auto origin = m_helper.originInFaceCoords();
            return vm::mod(face->measureTextureAngle(origin, point), 360.0f);
        }

        float UVRotateTool::snapAngle(const float angle) const {
            const auto* face = m_helper.face();

            const float angles[] = {
                vm::mod(angle +   0.0f, 360.0f),
                vm::mod(angle +  90.0f, 360.0f),
                vm::mod(angle + 180.0f, 360.0f),
                vm::mod(angle + 270.0f, 360.0f),
            };
            auto minDelta = std::numeric_limits<float>::max();

            const auto toFace = face->toTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
            for (const auto* edge : face->edges()) {
                const auto startInFaceCoords = vm::vec2f(toFace * edge->firstVertex()->position());
                const auto endInFaceCoords   = vm::vec2f(toFace * edge->secondVertex()->position());
                const auto edgeAngle         = vm::mod(face->measureTextureAngle(startInFaceCoords, endInFaceCoords), 360.0f);

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

        void UVRotateTool::doEndMouseDrag(const InputState&) {
            auto document = kdl::mem_lock(m_document);
            document->commitTransaction();
        }

        void UVRotateTool::doCancelMouseDrag() {
            auto document = kdl::mem_lock(m_document);
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
                const auto zoom = helper.cameraZoom();
                return Renderer::Circle(radius / zoom, segments, fill);
            }
        private:
            void doPrepareVertices(Renderer::VboManager& vboManager) override {
                m_center.prepare(vboManager);
                m_outer.prepare(vboManager);
            }

            void doRender(Renderer::RenderContext& renderContext) override {
                const auto* face = m_helper.face();
                const auto fromFace = face->fromTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);

                const auto& boundary = face->boundary();
                const auto toPlane = vm::plane_projection_matrix(boundary.distance, boundary.normal);
                const auto [invertible, fromPlane] = invert(toPlane);
                assert(invertible); unused(invertible);

                const auto originPosition(toPlane * fromFace * vm::vec3(m_helper.originInFaceCoords()));
                const auto faceCenterPosition(toPlane * m_helper.face()->boundsCenter());

                const auto& handleColor = pref(Preferences::HandleColor);
                const auto& highlightColor = pref(Preferences::SelectedHandleColor);

                Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
                const Renderer::MultiplyModelMatrix toWorldTransform(renderContext.transformation(), vm::mat4x4f(fromPlane));
                {
                    const auto translation = vm::translation_matrix(vm::vec3(originPosition));
                    const Renderer::MultiplyModelMatrix centerTransform(renderContext.transformation(), vm::mat4x4f(translation));
                    if (m_highlight) {
                        shader.set("Color", highlightColor);
                    } else {
                        shader.set("Color", handleColor);
                    }
                    m_outer.render();
                }

                {
                    const auto translation =vm::translation_matrix(vm::vec3(faceCenterPosition));
                    const Renderer::MultiplyModelMatrix centerTransform(renderContext.transformation(), vm::mat4x4f(translation));
                    shader.set("Color", highlightColor);
                    m_center.render();
                }
            }
        };

        void UVRotateTool::doRender(const InputState& inputState, Renderer::RenderContext&, Renderer::RenderBatch& renderBatch) {
            if (!m_helper.valid()) {
                return;
            }

            const auto* face = m_helper.face();
            if (!face->attributes().valid()) {
                return;
            }

            const auto& pickResult = inputState.pickResult();
            const auto& angleHandleHit = pickResult.query().type(AngleHandleHitType).occluded().first();
            const auto highlight = angleHandleHit.isMatch() || thisToolDragging();

            renderBatch.addOneShot(new Render(m_helper, static_cast<float>(CenterHandleRadius), static_cast<float>(RotateHandleRadius), highlight));
        }

        bool UVRotateTool::doCancel() {
            return false;
        }
    }
}
