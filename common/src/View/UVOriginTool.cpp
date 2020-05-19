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

#include "UVOriginTool.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/Circle.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/PrimType.h"
#include "Renderer/Renderable.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Transformation.h"
#include "View/InputState.h"
#include "View/UVViewHelper.h"

#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/line.h>
#include <vecmath/distance.h>
#include <vecmath/intersection.h>

namespace TrenchBroom {
    namespace View {
        const Model::HitType::Type UVOriginTool::XHandleHitType = Model::HitType::freeType();
        const Model::HitType::Type UVOriginTool::YHandleHitType = Model::HitType::freeType();
        const FloatType UVOriginTool::MaxPickDistance = 5.0;
        const float UVOriginTool::OriginHandleRadius =  5.0f;

        UVOriginTool::UVOriginTool(UVViewHelper& helper) :
        ToolControllerBase(),
        Tool(true),
        m_helper(helper) {}

        Tool* UVOriginTool::doGetTool() {
            return this;
        }

        const Tool* UVOriginTool::doGetTool() const {
            return this;
        }

        void UVOriginTool::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            if (m_helper.valid()) {
                vm::line3 xHandle, yHandle;
                computeOriginHandles(xHandle, yHandle);

                const auto* face = m_helper.face();
                const auto fromTex = face->fromTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
                const auto origin = fromTex * vm::vec3(m_helper.originInFaceCoords());

                const auto& pickRay = inputState.pickRay();
                const auto oDistance = vm::distance(pickRay, origin);
                if (oDistance.distance <= static_cast<FloatType>(OriginHandleRadius / m_helper.cameraZoom())) {
                    const auto hitPoint = vm::point_at_distance(pickRay, oDistance.position);
                    pickResult.addHit(Model::Hit(XHandleHitType, oDistance.position, hitPoint, xHandle, oDistance.distance));
                    pickResult.addHit(Model::Hit(YHandleHitType, oDistance.position, hitPoint, xHandle, oDistance.distance));
                } else {
                    const auto xDistance = vm::distance(pickRay, xHandle);
                    const auto yDistance = vm::distance(pickRay, yHandle);

                    assert(!xDistance.parallel);
                    assert(!yDistance.parallel);

                    const auto maxDistance  = MaxPickDistance / static_cast<FloatType>(m_helper.cameraZoom());
                    if (xDistance.distance <= maxDistance) {
                        const auto hitPoint = vm::point_at_distance(pickRay, xDistance.position1);
                        pickResult.addHit(Model::Hit(XHandleHitType, xDistance.position1, hitPoint, xHandle, xDistance.distance));
                    }

                    if (yDistance.distance <= maxDistance) {
                        const auto hitPoint = vm::point_at_distance(pickRay, yDistance.position1);
                        pickResult.addHit(Model::Hit(YHandleHitType, yDistance.position1, hitPoint, yHandle, yDistance.distance));
                    }
                }
            }
        }

        void UVOriginTool::computeOriginHandles(vm::line3& xHandle, vm::line3& yHandle) const {
            const auto* face = m_helper.face();
            const auto toWorld = face->fromTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);

            const auto origin = vm::vec3(m_helper.originInFaceCoords());
            xHandle.point = yHandle.point = toWorld * origin;

            xHandle.direction = normalize(toWorld * (origin + vm::vec3::pos_y()) - xHandle.point);
            yHandle.direction = (toWorld * (origin + vm::vec3::pos_x()) - yHandle.point);
        }

        bool UVOriginTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());

            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                return false;
            }

            const auto& pickResult = inputState.pickResult();
            const auto& xHandleHit = pickResult.query().type(XHandleHitType).occluded().first();
            const auto& yHandleHit = pickResult.query().type(YHandleHitType).occluded().first();

            if (!xHandleHit.isMatch() && !yHandleHit.isMatch()) {
                return false;
            }

            if (xHandleHit.isMatch()) {
                m_selector[0] = 1.0f;
            } else {
                m_selector[0] = 0.0f;
            }

            if (yHandleHit.isMatch()) {
                m_selector[1] = 1.0f;
            } else {
                m_selector[1] = 0.0f;
            }

            m_lastPoint = computeHitPoint(inputState.pickRay());
            return true;
        }

        bool UVOriginTool::doMouseDrag(const InputState& inputState) {
            const auto curPoint = computeHitPoint(inputState.pickRay());

            const auto delta = curPoint - m_lastPoint;

            const auto snapped = snapDelta(delta * m_selector);
            if (vm::is_zero(snapped, vm::Cf::almost_zero())) {
                return true;
            } else {
                m_helper.setOriginInFaceCoords(m_helper.originInFaceCoords() + snapped);
                m_lastPoint = m_lastPoint + snapped;

                return true;
            }
        }

        vm::vec2f UVOriginTool::computeHitPoint(const vm::ray3& ray) const {
            const auto* face = m_helper.face();
            const auto& boundary = face->boundary();
            const auto distance = vm::intersect_ray_plane(ray, boundary);
            const auto hitPoint = vm::point_at_distance(ray, distance);

            const auto transform = face->toTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
            return vm::vec2f(transform * hitPoint);
        }

        vm::vec2f UVOriginTool::snapDelta(const vm::vec2f& delta) const {
            if (vm::is_zero(delta, vm::Cf::almost_zero())) {
                return delta;
            }

            const auto* face = m_helper.face();
            ensure(face != nullptr, "face is null");

            // The delta is given in non-translated and non-scaled texture coordinates because that's how the origin
            // is stored. We have to convert to translated and scaled texture coordinates to do our snapping because
            // that's how the helper computes the distance to the texture grid.
            // Finally, we will convert the distance back to non-translated and non-scaled texture coordinates and
            // snap the delta to the distance.

            const auto w2fTransform = face->toTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
            const auto w2tTransform = face->toTexCoordSystemMatrix(face->offset(), face->scale(), true);
            const auto f2wTransform = face->fromTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
            const auto t2wTransform = face->fromTexCoordSystemMatrix(face->offset(), face->scale(), true);
            const auto f2tTransform = w2tTransform * f2wTransform;
            const auto t2fTransform = w2fTransform * t2wTransform;

            const auto newOriginInFaceCoords = m_helper.originInFaceCoords() + delta;
            const auto newOriginInTexCoords  = vm::vec2f(f2tTransform * vm::vec3(newOriginInFaceCoords));

            // now snap to the vertices
            // TODO: this actually doesn't work because we're snapping to the X or Y coordinate of the vertices
            // instead, we must snap to the edges!
            auto distanceInTexCoords = vm::vec2f::max();
            for (const Model::BrushVertex* vertex : face->vertices()) {
                distanceInTexCoords = vm::abs_min(distanceInTexCoords, vm::vec2f(w2tTransform * vertex->position()) - newOriginInTexCoords);
            }

            // and to the texture grid
            const auto* texture = face->texture();
            if (texture != nullptr) {
                distanceInTexCoords = vm::abs_min(distanceInTexCoords, m_helper.computeDistanceFromTextureGrid(vm::vec3(newOriginInTexCoords)));
            }

            // finally snap to the face center
            const auto faceCenter = vm::vec2f(w2tTransform * face->boundsCenter());
            distanceInTexCoords = vm::abs_min(distanceInTexCoords, faceCenter - newOriginInTexCoords);

            // now we have a distance in the scaled and translated texture coordinate system
            // so we transform the new position plus distance back to the unscaled and untranslated texture coordinate system
            // and take the actual distance
            const auto distanceInFaceCoords = newOriginInFaceCoords - vm::vec2f(t2fTransform * vm::vec3(newOriginInTexCoords + distanceInTexCoords));
            return m_helper.snapDelta(delta, -distanceInFaceCoords);
        }

        void UVOriginTool::doEndMouseDrag(const InputState&) {}
        void UVOriginTool::doCancelMouseDrag() {}

        void UVOriginTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (!m_helper.valid())
                return;

            renderLineHandles(inputState, renderContext, renderBatch);
            renderOriginHandle(inputState, renderContext, renderBatch);
        }

        void UVOriginTool::renderLineHandles(const InputState& inputState, Renderer::RenderContext&, Renderer::RenderBatch& renderBatch) {
            Renderer::DirectEdgeRenderer edgeRenderer(Renderer::VertexArray::move(getHandleVertices(inputState)), Renderer::PrimType::Lines);
            edgeRenderer.renderOnTop(renderBatch, 0.25f);
        }

        std::vector<UVOriginTool::EdgeVertex> UVOriginTool::getHandleVertices(const InputState& inputState) const {
            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& xHandleHit = pickResult.query().type(XHandleHitType).occluded().first();
            const Model::Hit& yHandleHit = pickResult.query().type(YHandleHitType).occluded().first();

            const bool highlightXHandle = (thisToolDragging() && m_selector.x() > 0.0f) || (!thisToolDragging() && xHandleHit.isMatch());
            const bool highlightYHandle = (thisToolDragging() && m_selector.y() > 0.0f) || (!thisToolDragging() && yHandleHit.isMatch());

            const Color xColor = highlightXHandle ? Color(1.0f, 0.0f, 0.0f, 1.0f) : Color(0.7f, 0.0f, 0.0f, 1.0f);
            const Color yColor = highlightYHandle ? Color(1.0f, 0.0f, 0.0f, 1.0f) : Color(0.7f, 0.0f, 0.0f, 1.0f);

            vm::vec3 x1, x2, y1, y2;
            m_helper.computeOriginHandleVertices(x1, x2, y1, y2);

            return {
                EdgeVertex(vm::vec3f(x1), xColor),
                EdgeVertex(vm::vec3f(x2), xColor),
                EdgeVertex(vm::vec3f(y1), yColor),
                EdgeVertex(vm::vec3f(y2), yColor)
            };
        }

        class UVOriginTool::RenderOrigin : public Renderer::DirectRenderable {
        private:
            const UVViewHelper& m_helper;
            bool m_highlight;
            Renderer::Circle m_originHandle;
        public:
            RenderOrigin(const UVViewHelper& helper, const float originRadius, const bool highlight) :
            m_helper(helper),
            m_highlight(highlight),
            m_originHandle(makeCircle(m_helper, originRadius, 16, true)) {}
        private:
            static Renderer::Circle makeCircle(const UVViewHelper& helper, const float radius, const size_t segments, const bool fill) {
                const float zoom = helper.cameraZoom();
                return Renderer::Circle(radius / zoom, segments, fill);
            }
        private:
            void doPrepareVertices(Renderer::VboManager& vboManager) override {
                m_originHandle.prepare(vboManager);
            }

            void doRender(Renderer::RenderContext& renderContext) override {
                const auto* face = m_helper.face();
                const auto fromFace = face->fromTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);

                const auto& boundary = face->boundary();
                const auto toPlane = vm::plane_projection_matrix(boundary.distance, boundary.normal);
                const auto [invertible, fromPlane] = invert(toPlane);
                const auto originPosition(toPlane * fromFace * vm::vec3(m_helper.originInFaceCoords()));
                assert(invertible); unused(invertible);

                const auto& handleColor = pref(Preferences::HandleColor);
                const auto& highlightColor = pref(Preferences::SelectedHandleColor);

                const Renderer::MultiplyModelMatrix toWorldTransform(renderContext.transformation(), vm::mat4x4f(fromPlane));
                const auto translation = vm::translation_matrix(vm::vec3(originPosition));
                const Renderer::MultiplyModelMatrix centerTransform(renderContext.transformation(), vm::mat4x4f(translation));

                Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
                shader.set("Color", m_highlight ? highlightColor : handleColor);
                m_originHandle.render();
            }
        };

        void UVOriginTool::renderOriginHandle(const InputState& inputState, Renderer::RenderContext&, Renderer::RenderBatch& renderBatch) {
            const auto highlight = renderHighlight(inputState);
            renderBatch.addOneShot(new RenderOrigin(m_helper, OriginHandleRadius, highlight));
        }

        bool UVOriginTool::renderHighlight(const InputState& inputState) const {
            if (thisToolDragging()) {
                return true;
            } else {
                const auto& pickResult = inputState.pickResult();
                const auto& xHandleHit = pickResult.query().type(XHandleHitType).occluded().first();
                const auto& yHandleHit = pickResult.query().type(YHandleHitType).occluded().first();
                return xHandleHit.isMatch() && yHandleHit.isMatch();
            }
        }

        bool UVOriginTool::doCancel() {
            return false;
        }
    }
}
