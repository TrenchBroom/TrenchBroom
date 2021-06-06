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
#include "Model/Hit.h"
#include "Model/HitFilter.h"
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

#include <vector>

namespace TrenchBroom {
    namespace View {
        const Model::HitType::Type UVOriginTool::XHandleHitType = Model::HitType::freeType();
        const Model::HitType::Type UVOriginTool::YHandleHitType = Model::HitType::freeType();
        const FloatType UVOriginTool::MaxPickDistance = 5.0;
        const float UVOriginTool::OriginHandleRadius =  5.0f;

        UVOriginTool::UVOriginTool(UVViewHelper& helper) :
        ToolControllerBase{},
        Tool{true},
        m_helper{helper} {}

        Tool* UVOriginTool::doGetTool() {
            return this;
        }

        const Tool* UVOriginTool::doGetTool() const {
            return this;
        }

        static std::tuple<vm::line3, vm::line3> computeOriginHandles(const UVViewHelper& helper) {
            const auto toWorld = helper.face()->fromTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);

            const auto origin = vm::vec3{helper.originInFaceCoords()};
            const auto linePoint = toWorld * origin;
            return {
                vm::line3{linePoint, vm::normalize(toWorld * (origin + vm::vec3::pos_y()) - linePoint)},
                vm::line3{linePoint, (toWorld * (origin + vm::vec3::pos_x()) - linePoint)},
            };
        }

        void UVOriginTool::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            if (m_helper.valid()) {
                const auto [xHandle, yHandle] = computeOriginHandles(m_helper);

                const auto fromTex = m_helper.face()->fromTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
                const auto origin = fromTex * vm::vec3{m_helper.originInFaceCoords()};

                const auto& pickRay = inputState.pickRay();
                const auto oDistance = vm::distance(pickRay, origin);
                if (oDistance.distance <= static_cast<FloatType>(OriginHandleRadius / m_helper.cameraZoom())) {
                    const auto hitPoint = vm::point_at_distance(pickRay, oDistance.position);
                    pickResult.addHit(Model::Hit{XHandleHitType, oDistance.position, hitPoint, xHandle, oDistance.distance});
                    pickResult.addHit(Model::Hit{YHandleHitType, oDistance.position, hitPoint, xHandle, oDistance.distance});
                } else {
                    const auto xDistance = vm::distance(pickRay, xHandle);
                    const auto yDistance = vm::distance(pickRay, yHandle);

                    assert(!xDistance.parallel);
                    assert(!yDistance.parallel);

                    const auto maxDistance  = MaxPickDistance / static_cast<FloatType>(m_helper.cameraZoom());
                    if (xDistance.distance <= maxDistance) {
                        const auto hitPoint = vm::point_at_distance(pickRay, xDistance.position1);
                        pickResult.addHit(Model::Hit{XHandleHitType, xDistance.position1, hitPoint, xHandle, xDistance.distance});
                    }

                    if (yDistance.distance <= maxDistance) {
                        const auto hitPoint = vm::point_at_distance(pickRay, yDistance.position1);
                        pickResult.addHit(Model::Hit{YHandleHitType, yDistance.position1, hitPoint, yHandle, yDistance.distance});
                    }
                }
            }
        }

        static vm::vec2f computeHitPoint(const UVViewHelper& helper, const vm::ray3& ray) {
            const auto& boundary = helper.face()->boundary();
            const auto distance = vm::intersect_ray_plane(ray, boundary);
            const auto hitPoint = vm::point_at_distance(ray, distance);

            const auto transform = helper.face()->toTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
            return vm::vec2f{transform * hitPoint};
        }

        static vm::vec2f snapDelta(const UVViewHelper& helper, const vm::vec2f& delta) {
            assert(helper.valid());
            
            if (vm::is_zero(delta, vm::Cf::almost_zero())) {
                return delta;
            }

            // The delta is given in non-translated and non-scaled texture coordinates because that's how the origin
            // is stored. We have to convert to translated and scaled texture coordinates to do our snapping because
            // that's how the helper computes the distance to the texture grid.
            // Finally, we will convert the distance back to non-translated and non-scaled texture coordinates and
            // snap the delta to the distance.

            const auto w2fTransform = helper.face()->toTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
            const auto w2tTransform = helper.face()->toTexCoordSystemMatrix(helper.face()->attributes().offset(), helper.face()->attributes().scale(), true);
            const auto f2wTransform = helper.face()->fromTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
            const auto t2wTransform = helper.face()->fromTexCoordSystemMatrix(helper.face()->attributes().offset(), helper.face()->attributes().scale(), true);
            const auto f2tTransform = w2tTransform * f2wTransform;
            const auto t2fTransform = w2fTransform * t2wTransform;

            const auto newOriginInFaceCoords = helper.originInFaceCoords() + delta;
            const auto newOriginInTexCoords  = vm::vec2f{f2tTransform * vm::vec3{newOriginInFaceCoords}};

            // now snap to the vertices
            // TODO: this actually doesn't work because we're snapping to the X or Y coordinate of the vertices
            // instead, we must snap to the edges!
            auto distanceInTexCoords = vm::vec2f::max();
            for (const Model::BrushVertex* vertex : helper.face()->vertices()) {
                distanceInTexCoords = vm::abs_min(distanceInTexCoords, vm::vec2f{w2tTransform * vertex->position()} - newOriginInTexCoords);
            }

            // and to the texture grid
            const auto* texture = helper.face()->texture();
            if (texture != nullptr) {
                distanceInTexCoords = vm::abs_min(distanceInTexCoords, helper.computeDistanceFromTextureGrid(vm::vec3{newOriginInTexCoords}));
            }

            // finally snap to the face center
            const auto faceCenter = vm::vec2f{w2tTransform * helper.face()->boundsCenter()};
            distanceInTexCoords = vm::abs_min(distanceInTexCoords, faceCenter - newOriginInTexCoords);

            // now we have a distance in the scaled and translated texture coordinate system
            // so we transform the new position plus distance back to the unscaled and untranslated texture coordinate system
            // and take the actual distance
            const auto distanceInFaceCoords = newOriginInFaceCoords - vm::vec2f{t2fTransform * vm::vec3{newOriginInTexCoords + distanceInTexCoords}};
            return helper.snapDelta(delta, -distanceInFaceCoords);
        }

        bool UVOriginTool::doStartMouseDrag(const InputState& inputState) {
            using namespace Model::HitFilters;

            assert(m_helper.valid());

            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                return false;
            }

            const auto& xHandleHit = inputState.pickResult().first(type(XHandleHitType));
            const auto& yHandleHit = inputState.pickResult().first(type(YHandleHitType));

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

            m_lastPoint = computeHitPoint(m_helper, inputState.pickRay());
            return true;
        }

        bool UVOriginTool::doMouseDrag(const InputState& inputState) {
            const auto curPoint = computeHitPoint(m_helper, inputState.pickRay());

            const auto delta = curPoint - m_lastPoint;

            const auto snapped = snapDelta(m_helper, delta * m_selector);
            if (vm::is_zero(snapped, vm::Cf::almost_zero())) {
                return true;
            } else {
                m_helper.setOriginInFaceCoords(m_helper.originInFaceCoords() + snapped);
                m_lastPoint = m_lastPoint + snapped;

                return true;
            }
        }

        void UVOriginTool::doEndMouseDrag(const InputState&) {}
        void UVOriginTool::doCancelMouseDrag() {}

        using EdgeVertex = Renderer::GLVertexTypes::P3C4::Vertex;

        static std::vector<EdgeVertex> getHandleVertices(const InputState& inputState, const UVViewHelper& helper, const bool dragInProgress, const vm::vec2f& selector) {
            using namespace Model::HitFilters;

            const Model::Hit& xHandleHit = inputState.pickResult().first(type(UVOriginTool::XHandleHitType));
            const Model::Hit& yHandleHit = inputState.pickResult().first(type(UVOriginTool::YHandleHitType));

            const bool highlightXHandle = (dragInProgress && selector.x() > 0.0f) || (!dragInProgress && xHandleHit.isMatch());
            const bool highlightYHandle = (dragInProgress && selector.y() > 0.0f) || (!dragInProgress && yHandleHit.isMatch());

            const auto xColor = highlightXHandle ? Color{1.0f, 0.0f, 0.0f, 1.0f} : Color{0.7f, 0.0f, 0.0f, 1.0f};
            const auto yColor = highlightYHandle ? Color{1.0f, 0.0f, 0.0f, 1.0f} : Color{0.7f, 0.0f, 0.0f, 1.0f};

            vm::vec3 x1, x2, y1, y2;
            helper.computeOriginHandleVertices(x1, x2, y1, y2);

            return {
                EdgeVertex{vm::vec3f{x1}, xColor},
                EdgeVertex{vm::vec3f{x2}, xColor},
                EdgeVertex{vm::vec3f{y1}, yColor},
                EdgeVertex{vm::vec3f{y2}, yColor}
            };
        }

        static void renderLineHandles(const InputState& inputState, const UVViewHelper& helper, const bool dragInProgress, const vm::vec2f& selector, Renderer::RenderBatch& renderBatch) {
            auto edgeRenderer = Renderer::DirectEdgeRenderer{Renderer::VertexArray::move(getHandleVertices(inputState, helper, dragInProgress, selector)), Renderer::PrimType::Lines};
            edgeRenderer.renderOnTop(renderBatch, 0.25f);
        }

        namespace {
            class RenderOrigin : public Renderer::DirectRenderable {
            private:
                const UVViewHelper& m_helper;
                bool m_highlight;
                Renderer::Circle m_originHandle;
            public:
                RenderOrigin(const UVViewHelper& helper, const float originRadius, const bool highlight) :
                m_helper{helper},
                m_highlight{highlight},
                m_originHandle{makeCircle(m_helper, originRadius, 16, true)} {}
            private:
                static Renderer::Circle makeCircle(const UVViewHelper& helper, const float radius, const size_t segments, const bool fill) {
                    const float zoom = helper.cameraZoom();
                    return Renderer::Circle{radius / zoom, segments, fill};
                }
            private:
                void doPrepareVertices(Renderer::VboManager& vboManager) override {
                    m_originHandle.prepare(vboManager);
                }

                void doRender(Renderer::RenderContext& renderContext) override {
                    const auto fromFace = m_helper.face()->fromTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);

                    const auto& boundary = m_helper.face()->boundary();
                    const auto toPlane = vm::plane_projection_matrix(boundary.distance, boundary.normal);
                    const auto [invertible, fromPlane] = vm::invert(toPlane);
                    const auto originPosition(toPlane * fromFace * vm::vec3{m_helper.originInFaceCoords()});
                    assert(invertible); unused(invertible);

                    const auto& handleColor = pref(Preferences::HandleColor);
                    const auto& highlightColor = pref(Preferences::SelectedHandleColor);

                    const auto toWorldTransform = Renderer::MultiplyModelMatrix{renderContext.transformation(), vm::mat4x4f{fromPlane}};
                    const auto translation = vm::translation_matrix(vm::vec3{originPosition});
                    const auto centerTransform = Renderer::MultiplyModelMatrix{renderContext.transformation(), vm::mat4x4f{translation}};

                    auto shader = Renderer::ActiveShader{renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader};
                    shader.set("Color", m_highlight ? highlightColor : handleColor);
                    m_originHandle.render();
                }
            };
        }

        static bool shouldRenderHighlight(const InputState& inputState, const bool dragInProgress) {
            using namespace Model::HitFilters;

            if (dragInProgress) {
                return true;
            } else {
                const auto& xHandleHit = inputState.pickResult().first(type(UVOriginTool::XHandleHitType));
                const auto& yHandleHit = inputState.pickResult().first(type(UVOriginTool::YHandleHitType));
                return xHandleHit.isMatch() && yHandleHit.isMatch();
            }
        }

        static void renderOriginHandle(const InputState& inputState, const UVViewHelper& helper, const bool dragInProgress, Renderer::RenderBatch& renderBatch) {
            const auto highlight = shouldRenderHighlight(inputState, dragInProgress);
            renderBatch.addOneShot(new RenderOrigin{helper, UVOriginTool::OriginHandleRadius, highlight});
        }

        void UVOriginTool::doRender(const InputState& inputState, Renderer::RenderContext&, Renderer::RenderBatch& renderBatch) {
            if (!m_helper.valid()) {
                return;
            }

            renderLineHandles(inputState, m_helper, thisToolDragging(), m_selector, renderBatch);
            renderOriginHandle(inputState, m_helper, thisToolDragging(), renderBatch);
        }

        bool UVOriginTool::doCancel() {
            return false;
        }
    }
}
