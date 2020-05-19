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

#include "UVShearTool.h"

#include "Model/BrushFace.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/UVViewHelper.h"

#include <kdl/memory_utils.h>

#include <vecmath/vec.h>
#include <vecmath/intersection.h>

namespace TrenchBroom {
    namespace View {
        const Model::HitType::Type UVShearTool::XHandleHitType = Model::HitType::freeType();
        const Model::HitType::Type UVShearTool::YHandleHitType = Model::HitType::freeType();

        UVShearTool::UVShearTool(std::weak_ptr<MapDocument> document, UVViewHelper& helper) :
        ToolControllerBase(),
        Tool(true),
        m_document(document),
        m_helper(helper) {}

        Tool* UVShearTool::doGetTool() {
            return this;
        }

        const Tool* UVShearTool::doGetTool() const {
            return this;
        }

        void UVShearTool::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            static const Model::HitType::Type HitTypes[] = { XHandleHitType, YHandleHitType };
            if (m_helper.valid())
                m_helper.pickTextureGrid(inputState.pickRay(), HitTypes, pickResult);
        }

        bool UVShearTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());

            if (!inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;

            const auto* face = m_helper.face();
            if (!face->attributes().valid()) {
                return false;
            }

            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& xHit = pickResult.query().type(XHandleHitType).occluded().first();
            const Model::Hit& yHit = pickResult.query().type(YHandleHitType).occluded().first();

            if (!(xHit.isMatch() ^ yHit.isMatch()))
                return false;

            m_selector = vm::vec2b(xHit.isMatch(), yHit.isMatch());

            m_xAxis = face->textureXAxis();
            m_yAxis = face->textureYAxis();
            m_initialHit = m_lastHit = getHit(inputState.pickRay());

            // #1350: Don't allow shearing if the shear would result in very large changes. This happens if
            // the shear handle to be dragged is very close to one of the texture axes.
            if (vm::is_zero(m_initialHit.x(), 6.0f) ||
                vm::is_zero(m_initialHit.y(), 6.0f))
                return false;

            auto document = kdl::mem_lock(m_document);
            document->startTransaction("Shear Texture");
            return true;
        }

        bool UVShearTool::doMouseDrag(const InputState& inputState) {
            const auto currentHit = getHit(inputState.pickRay());
            const auto delta = currentHit - m_lastHit;

            auto* face = m_helper.face();
            const auto origin = m_helper.origin();
            const auto oldCoords = vm::vec2f(face->toTexCoordSystemMatrix(vm::vec2f::zero(), face->attributes().scale(), true) * origin);

            auto document = kdl::mem_lock(m_document);
            if (m_selector[0]) {
                const vm::vec2f factors = vm::vec2f(-delta.y() / m_initialHit.x(), 0.0f);
                if (!vm::is_zero(factors, vm::Cf::almost_zero())) {
                    document->shearTextures(factors);
                }
            } else if (m_selector[1]) {
                const vm::vec2f factors = vm::vec2f(0.0f, -delta.x() / m_initialHit.y());
                if (!vm::is_zero(factors, vm::Cf::almost_zero())) {
                    document->shearTextures(factors);
                }
            }

            const auto newCoords = vm::vec2f(face->toTexCoordSystemMatrix(vm::vec2f::zero(), face->attributes().scale(), true) * origin);
            const auto newOffset = face->attributes().offset() + oldCoords - newCoords;

            Model::ChangeBrushFaceAttributesRequest request;
            request.setOffset(newOffset);
            document->setFaceAttributes(request);

            m_lastHit = currentHit;
            return true;
        }

        void UVShearTool::doEndMouseDrag(const InputState&) {
            auto document = kdl::mem_lock(m_document);
            document->commitTransaction();
        }

        void UVShearTool::doCancelMouseDrag() {
            auto document = kdl::mem_lock(m_document);
            document->cancelTransaction();
        }

        vm::vec2f UVShearTool::getHit(const vm::ray3& pickRay) const {
            const auto* face = m_helper.face();
            const auto& boundary = face->boundary();
            const auto hitPointDist = vm::intersect_ray_plane(pickRay, boundary);
            const auto hitPoint = vm::point_at_distance(pickRay, hitPointDist);
            const auto hitVec = hitPoint - m_helper.origin();

            return vm::vec2f(vm::dot(hitVec, m_xAxis), vm::dot(hitVec, m_yAxis));
        }

        bool UVShearTool::doCancel() {
            return false;
        }
    }
}
