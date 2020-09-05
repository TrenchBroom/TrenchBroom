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

#include "FloatType.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "View/MapDocument.h"
#include "View/InputState.h"
#include "View/UVViewHelper.h"
#include "View/UVOriginTool.h"

#include <kdl/memory_utils.h>

#include <vecmath/vec.h>
#include <vecmath/ray.h>
#include <vecmath/intersection.h>

#include <numeric>

namespace TrenchBroom {
    namespace View {
        const Model::HitType::Type UVScaleTool::XHandleHitType = Model::HitType::freeType();
        const Model::HitType::Type UVScaleTool::YHandleHitType = Model::HitType::freeType();

        UVScaleTool::UVScaleTool(std::weak_ptr<MapDocument> document, UVViewHelper& helper) :
        ToolControllerBase(),
        Tool(true),
        m_document(std::move(document)),
        m_helper(helper) {}

        Tool* UVScaleTool::doGetTool() {
            return this;
        }

        const Tool* UVScaleTool::doGetTool() const {
            return this;
        }

        void UVScaleTool::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            static const Model::HitType::Type HitTypes[] = { XHandleHitType, YHandleHitType };
            if (m_helper.valid()) {
                m_helper.pickTextureGrid(inputState.pickRay(), HitTypes, pickResult);
            }
        }

        vm::vec2i UVScaleTool::getScaleHandle(const Model::Hit& xHit, const Model::Hit& yHit) const {
            const auto x = xHit.isMatch() ? xHit.target<int>() : 0;
            const auto y = yHit.isMatch() ? yHit.target<int>() : 0;
            return vm::vec2i(x, y);
        }

        vm::vec2f UVScaleTool::getHitPoint(const vm::ray3& pickRay) const {
            const auto& boundary = m_helper.face()->boundary();
            const auto facePointDist = vm::intersect_ray_plane(pickRay, boundary);
            const auto facePoint = vm::point_at_distance(pickRay, facePointDist);

            const auto toTex = m_helper.face()->toTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
            return vm::vec2f(toTex * facePoint);
        }

        bool UVScaleTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());

            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                return false;
            }

            if (!m_helper.face()->attributes().valid()) {
                return false;
            }

            const auto& pickResult = inputState.pickResult();
            const auto& xHit = pickResult.query().type(XHandleHitType).occluded().first();
            const auto& yHit = pickResult.query().type(YHandleHitType).occluded().first();

            if (!xHit.isMatch() && !yHit.isMatch()) {
                return false;
            }

            m_handle = getScaleHandle(xHit, yHit);
            m_selector = vm::vec2b(xHit.isMatch(), yHit.isMatch());
            m_lastHitPoint = getHitPoint(inputState.pickRay());

            auto document = kdl::mem_lock(m_document);
            document->startTransaction("Scale Texture");
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

            auto newScale = m_helper.face()->attributes().scale();
            for (size_t i = 0; i < 2; ++i) {
                if (m_selector[i]) {
                    const auto value = newHandleDistFaceCoords[i] / curHandleDistTexCoords[i];
                    if (value != 0.0f) {
                        newScale[i] = value;
                    }
                }
            }
            newScale = correct(newScale, 4, 0.0f);

            Model::ChangeBrushFaceAttributesRequest request;
            request.setScale(newScale);

            auto document = kdl::mem_lock(m_document);
            document->setFaceAttributes(request);

            const auto newOriginInTexCoords = correct(m_helper.originInTexCoords(), 4, 0.0f);
            const auto originDelta = originHandlePosTexCoords - newOriginInTexCoords;

            request.clear();
            request.addOffset(originDelta);
            document->setFaceAttributes(request);

            m_lastHitPoint = m_lastHitPoint + (dragDeltaFaceCoords - newHandlePosFaceCoords + newHandlePosSnapped);
            return true;
        }

        void UVScaleTool::doEndMouseDrag(const InputState&) {
            auto document = kdl::mem_lock(m_document);
            document->commitTransaction();
        }

        void UVScaleTool::doCancelMouseDrag() {
            auto document = kdl::mem_lock(m_document);
            document->cancelTransaction();
        }

        vm::vec2f UVScaleTool::getScaledTranslatedHandlePos() const {
            return vm::vec2f(m_handle) * vm::vec2f(m_helper.stripeSize());
        }

        vm::vec2f UVScaleTool::getHandlePos() const {
            const auto toWorld = m_helper.face()->fromTexCoordSystemMatrix(m_helper.face()->attributes().offset(), m_helper.face()->attributes().scale(), true);
            const auto toTex   = m_helper.face()->toTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);

            return vm::vec2f(toTex * toWorld * vm::vec3(getScaledTranslatedHandlePos()));
        }

        vm::vec2f UVScaleTool::snap(const vm::vec2f& position) const {
            const auto toTex = m_helper.face()->toTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);

            const auto vertices = m_helper.face()->vertices();
            auto distance = std::accumulate(std::begin(vertices), std::end(vertices), vm::vec2f::max(),
                                             [&toTex, &position](const vm::vec2f& current, const Model::BrushVertex* vertex) {
                                                 const vm::vec2f vertex2(toTex * vertex->position());
                                                 return vm::abs_min(current, position - vertex2);
                                             });

            for (size_t i = 0; i < 2; ++i) {
                if (vm::abs(distance[i]) > 4.0f / m_helper.cameraZoom()) {
                    distance[i] = 0.0f;
                }
            }

            return position - distance;
        }

        void UVScaleTool::doRender(const InputState& inputState, Renderer::RenderContext&, Renderer::RenderBatch& renderBatch) {
            if (!m_helper.valid()) {
                return;
            }

            if (!m_helper.face()->attributes().valid()) {
                return;
            }

            // don't overdraw the origin handles
            const auto& pickResult = inputState.pickResult();
            if (!pickResult.query().type(UVOriginTool::XHandleHitType | UVOriginTool::YHandleHitType).occluded().first().isMatch()) {
                const Color color(1.0f, 0.0f, 0.0f, 1.0f);

                Renderer::DirectEdgeRenderer handleRenderer(Renderer::VertexArray::move(getHandleVertices(pickResult)), Renderer::PrimType::Lines);
                handleRenderer.render(renderBatch, color, 0.5f);
            }
        }

        std::vector<UVScaleTool::EdgeVertex> UVScaleTool::getHandleVertices(const Model::PickResult& pickResult) const {
            const auto& xHandleHit = pickResult.query().type(XHandleHitType).occluded().first();
            const auto& yHandleHit = pickResult.query().type(YHandleHitType).occluded().first();
            const auto stripeSize = m_helper.stripeSize();

            const auto xIndex = xHandleHit.isMatch() ? xHandleHit.target<int>() : 0;
            const auto yIndex = yHandleHit.isMatch() ? yHandleHit.target<int>() : 0;
            const auto pos = stripeSize * vm::vec2(xIndex, yIndex);

            vm::vec3 h1, h2, v1, v2;
            m_helper.computeScaleHandleVertices(pos, v1, v2, h1, h2);

            std::vector<EdgeVertex> vertices;
            vertices.reserve(4);

            if (xHandleHit.isMatch()) {
                vertices.push_back(EdgeVertex(vm::vec3f(v1)));
                vertices.push_back(EdgeVertex(vm::vec3f(v2)));
            }

            if (yHandleHit.isMatch()) {
                vertices.push_back(EdgeVertex(vm::vec3f(h1)));
                vertices.push_back(EdgeVertex(vm::vec3f(h2)));
            }

            return vertices;
        }

        bool UVScaleTool::doCancel() {
            return false;
        }
    }
}
